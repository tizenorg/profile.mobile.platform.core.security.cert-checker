/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        popup-runner.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>

#include <cchecker/app.h>
#include <cchecker/logic.h>
#include <cchecker/log.h>
#include <cchecker/UIBackend.h>
#include <cchecker/popup-runner.h>

namespace { // anonymous

using namespace CCHECKER::UI;

const char *POPUP_EXEC = "/usr/bin/cert-checker-popup"; // check-checker-popup binary

std::string response_to_str (response_e response)
{
    switch (response) {
    case response_e::DONT_UNINSTALL:
        return "DONT_UNINSTALL";
    case response_e::UNINSTALL:
        return "UNINSTALL";
    default:
        return "RESPONSE_ERROR";
    }
}

int wait_for_popup (int popup_pid, int timeout)
{
    int status;
    int ret;

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);
    siginfo_t info;
    struct timespec time = {timeout, 0L};

    if (timeout > 0)
        ret = TEMP_FAILURE_RETRY(sigtimedwait(&set, &info, &time));
    else
        ret = TEMP_FAILURE_RETRY(sigwaitinfo(&set, &info));

    sigprocmask(SIG_UNBLOCK, &set, NULL);

    if  (ret == -1 && errno == EAGAIN) {
        LogError("POPUP TIMEOUT");
        goto err;
    }
    else if (ret == SIGCHLD && info.si_pid == popup_pid) {
        // call waitpid on the child process to get rid of zombie process
        waitpid(popup_pid, NULL, 0);

        // The proper signal has been caught and its pid matches to popup_pid
        // Now check the popup exit status
        status = WEXITSTATUS(info.si_status);
        LogDebug("STATUS EXIT ON POPUP (CHILD: " << info.si_pid << "): " << status);

        switch (static_cast<popup_status>(status)) {

        case popup_status::NO_ERROR:
            LogDebug("NO_ERROR");
            return 0;

        case popup_status::EXIT_ERROR:
            LogError("ERROR");
            return -1;

        default: // Unknown exit status
            LogError("UNKNOWN_ERROR");
           return -1;
        }
    }
    else {
        LogError("Some other signal has been caught (pid: " << info.si_pid << ", signal: " << info.si_signo << ")");
        goto err;
    }

err:
    // kill popup process and return error
    kill(popup_pid, SIGKILL);

    // call waitpid on the child process to get rid of zombie process
    waitpid(popup_pid, NULL, 0);
    return -1;
}

void child_process (int fd_send_to_child[2], int fd_send_to_parent[2])
{
    LogDebug("Child");

    // read data from parent
    close(fd_send_to_child[1]);

    // send data to parent
    close(fd_send_to_parent[0]);

    std::stringstream pipe_in_buff;
    std::stringstream pipe_out_buff;
    pipe_in_buff  << fd_send_to_parent[1];
    pipe_out_buff << fd_send_to_child[0];
    std::string pipe_in  = pipe_in_buff.str();
    std::string pipe_out = pipe_out_buff.str();

    LogDebug("Passed file descriptors: " << fd_send_to_child[0] << ", "<< fd_send_to_parent[1]);

    if (execl(POPUP_EXEC, POPUP_EXEC, pipe_out.c_str(), pipe_in.c_str(), NULL) < 0){
        LogError("execl FAILED");
    }

    LogError("This should not happen!!!");
    _exit(response_e::RESPONSE_ERROR);
}

int send_message_to_child(const BinaryStream &stream, int fd_send_to_child)
{
    LogDebug("Sending message to popup-bin process");
    unsigned int begin = 0;
    int tmp;
    while (begin < stream.size()) {
        tmp = TEMP_FAILURE_RETRY(write(fd_send_to_child,
                                       stream.char_pointer() + begin,
                                       stream.size() - begin));
        if(-1 == tmp){
            LogError("Write to pipe failed!");
            return -1;
        }
        begin += tmp;
    }
    LogDebug("Message has been sent");
    return 0;
}

response_e parse_response (int count, char *data)
{
    LogDebug("RESULT FROM POPUP PIPE (CHILD) : [ " << count << " ]");
    int response_int;
    response_e response;
    BinaryStream stream_in;
    stream_in.Write(count, data);
    CCHECKER::Deserialization::Deserialize(stream_in, response_int);
    response = static_cast <response_e> (response_int);
    LogDebug("response :" << response_to_str(response));
    return response;
}

} // anonymous namespace

namespace CCHECKER {
namespace UI {

// BinaryStream class implementation
void BinaryStream::Read(size_t num, void * bytes)
{
    size_t max_size = m_data.size();
    for (size_t i = 0; i < num; ++i) {
        if( i + m_readPosition >= max_size){
            return;
        }
        static_cast <unsigned char*> (bytes)[i] = m_data[i + m_readPosition];
    }
    m_readPosition += num;
}

void BinaryStream::Write(size_t num, const void * bytes)
{
    for (size_t i = 0; i < num; ++i) {
        m_data.push_back(static_cast <const unsigned char*> (bytes)[i]);
    }
}

BinaryStream::BinaryStream() :
    m_readPosition(0)
{}

BinaryStream::~BinaryStream() {}

const unsigned char* BinaryStream::char_pointer() const
{
    return &m_data[0];
}

size_t BinaryStream::size() const
{
    return m_data.size();
}
// BinaryStream

response_e run_popup(
        const app_t &app,
        int timeout)
{
    LogDebug(app.str());

    // serialization
    BinaryStream stream;
    CCHECKER::Serialization::Serialize(stream, app.app_id);
    CCHECKER::Serialization::Serialize(stream, app.pkg_id);

    int   fd_send_to_child[2];
    int   fd_send_to_parent[2];
    pid_t childpid;

    if(0 != pipe(fd_send_to_child)){
        LogError("Cannot create pipes!");
        return response_e::RESPONSE_ERROR;
    }
    if(0 != pipe(fd_send_to_parent)){
        LogError("Cannot create pipes!");
        close(fd_send_to_child[0]);
        close(fd_send_to_child[1]);
        return response_e::RESPONSE_ERROR;
    }

    if ((childpid = fork()) == -1) {
        LogError("Fork() ERROR");
        close(fd_send_to_child[0]);
        close(fd_send_to_parent[1]);
        goto error;
    }

    if(childpid == 0) { // Child process
        child_process (fd_send_to_child, fd_send_to_parent);
    }
    else { // Parent process
        LogDebug("Parent (child pid: " << childpid << ")");

        // send data to child
        close(fd_send_to_child[0]);

        // read data from child
        close(fd_send_to_parent[1]);

        // writing to child
        if (send_message_to_child(stream, fd_send_to_child[1]))
            goto error;

        // wait for child
        if (wait_for_popup(childpid, timeout) != 0 )
            goto error;

        // Read message from popup (child)
        int  buff_size = 1024;
        char result[buff_size];
        int count;
        count = TEMP_FAILURE_RETRY(read(fd_send_to_parent[0], result, buff_size));

        // Parsing response from child
        response_e response;
        if (0 < count)
            response = parse_response(count, result);
        else {
            LogDebug("ERROR, count = " << count);;
            goto error;
        }

        LogDebug("popup-runner: EXIT SUCCESS");
        // cleanup
        close(fd_send_to_parent[0]);
        close(fd_send_to_child[1]);
        return response;
    }

    LogError("This should not happen!!!");
error:
    // cleanup
    LogDebug("popup-runner: EXIT ERROR");
    close(fd_send_to_parent[0]);
    close(fd_send_to_child[1]);
    return response_e::RESPONSE_ERROR;
}

} // UI
} // CCHECKER
