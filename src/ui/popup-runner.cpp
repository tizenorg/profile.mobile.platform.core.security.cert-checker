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

static int timeout_child (int timeout, int popup_pid)
{
    int timeout_pid;
    if ((timeout_pid = fork()) == -1) {
        kill(popup_pid, SIGKILL);
        return -1;
    }
    if (timeout_pid == 0) {
        sleep(timeout);
        _exit(0);
    }

    return timeout_pid;
}

static std::string response_to_str (response_e response)
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

static int wait_for_popup (int popup_pid, int timeout_pid)
{
    int status;
    pid_t wpid;
    do {
        wpid = waitpid(0, &status, 0);

        if (wpid == timeout_pid) {
            LogError("POPUP TIMEOUT");
            kill(popup_pid, SIGKILL);
            return -1;
        }
        else
            if (wpid == popup_pid) {
            // timeout process isn't needed any longer - kill it
            kill(timeout_pid, SIGKILL);
            break;
        }
        LogError("Some child process has exited (pid: " << wpid << ")");
    } while (true);

    if (WIFEXITED(status))
        status = WEXITSTATUS(status);
    else {
        LogError("Popup terminated abnormally");
        return -1;
    }

    LogDebug("STATUS EXIT ON POPUP (CHILD: " << wpid << "): " << status);

    switch (static_cast <popup_status> (status)) {

    case popup_status::NO_ERROR:
        LogDebug("NO_ERROR");
        return 0;

    case popup_status::EXIT_ERROR:
        LogDebug("ERROR");
        return -1;

    default: // Unknown exit status
        LogDebug("UNKNOWN_ERROR");
        return -1;
    }

    return -1;
}

} // anonymous namespace

namespace CCHECKER {
namespace UI {

// BinaryStream class implementation
void BinaryStream::Read(size_t num, void * bytes) {
    size_t max_size = m_data.size();
    for (size_t i = 0; i < num; ++i) {
        if( i + m_readPosition >= max_size){
            return;
        }
        static_cast <unsigned char*> (bytes)[i] = m_data[i + m_readPosition];
    }
    m_readPosition += num;
}

void BinaryStream::Write(size_t num, const void * bytes) {
    for (size_t i = 0; i < num; ++i) {
        m_data.push_back(static_cast <const unsigned char*> (bytes)[i]);
    }
}

BinaryStream::BinaryStream() :
    m_readPosition(0)
{}

BinaryStream::~BinaryStream() {}

const unsigned char* BinaryStream::char_pointer() const {
    return &m_data[0];
}

size_t BinaryStream::size() const {
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

        LogError("This should not happened!!!");
        _exit(response_e::RESPONSE_ERROR);

    } // end of child process - from now you can use DEBUG LOGS
    else { // Parent process
        LogDebug("Parent (child pid: " << childpid << ")");

        int  buff_size = 1024;
        char result[buff_size];

        // send data to child
        close(fd_send_to_child[0]);

        // read data from child
        close(fd_send_to_parent[1]);

        //writing to child
        LogDebug("Sending message to popup-bin process");
        int begin = 0, tmp;
        while (begin * sizeof(char) < stream.size()) {
            tmp = TEMP_FAILURE_RETRY(write(fd_send_to_child[1],
                        stream.char_pointer() + sizeof(char) * begin,
                        stream.size() - sizeof(char)*begin));
            if(-1 == tmp){
                LogError("Write to pipe failed!");
                goto error;
            }
            begin += tmp;
        }
        LogDebug("Message has been sent");

        int timeout_pid;
        // TODO: implement popup timeout using sigtimedwait()?
        if (timeout > 0 && (timeout_pid = timeout_child(timeout, childpid)) == -1) {
            LogError("Cannot setup timeout process. Popup process should be killed.");
            goto error;
        }

        if (wait_for_popup(childpid, timeout_pid) != 0 ) {
            goto error;
        }

        int count;
        count = TEMP_FAILURE_RETRY(read(fd_send_to_parent[0], result, buff_size));

        response_e response;
        if (0 < count) {
            LogDebug("RESULT FROM POPUP PIPE (CHILD) : [ " << count << " ]");
            int response_int;
            BinaryStream stream_in;
            stream_in.Write(count, result);
            CCHECKER::Deserialization::Deserialize(stream_in, response_int);
            response = static_cast <response_e> (response_int);
            LogDebug("response :" << response_to_str(response));

        }
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

    LogError("This should not happend!!!");
error:
    // cleanup
    LogDebug("popup-runner: EXIT ERROR");
    close(fd_send_to_parent[0]);
    close(fd_send_to_child[1]);
    return response_e::RESPONSE_ERROR;
}

} // UI
} // CCHECKER
