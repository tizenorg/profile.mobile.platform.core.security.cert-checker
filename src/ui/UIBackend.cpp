/*
 *  Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */
/**
 * @file        UIBackend.cpp
 * @author      Janusz Kozerski <j.kozerski@samsung.com>
 * @brief       This file implements class for ask user window
 */

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <libintl.h>
#include <app_control.h>

#include <cchecker/log.h>
#include <cchecker/UIBackend.h>
#include <cchecker/popup-runner.h>

namespace CCHECKER {
namespace UI {

UIBackend::UIBackend(int timeout) :
    m_responseTimeout(timeout)
{}

UIBackend::~UIBackend()
{}

response_e UIBackend::run(const app_t &app)
{
    response_e response = response_e::ERROR;
    try {

        response_e ret = run_popup(app, response, m_responseTimeout);
        LogDebug("run_popup finished with ret code: [" << ret << "]");

        if (ret != response_e::NO_ERROR) {
            response = response_e::ERROR;
        }
    } catch (const std::exception &e) {
        LogError("Unexpected exception: <" << e.what() << ">");
    } catch (...) {
        LogError("Unexpected unknown exception caught!");
    }
    return response;
}

bool UIBackend::call_popup(const app_t &app)
{
    response_e resp;

    resp = run(app);
    LogDebug(app.str() << " response: " << resp);
    if (resp == response_e::ERROR) {
        return false;
    }

    else if (resp == response_e::UNINSTALL) {
        app_control_h service = NULL;
        int result = 0;
        result = app_control_create(&service);
        if (!service || result != APP_CONTROL_ERROR_NONE) {
            return false;
        }
        app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
        app_control_set_app_id(service, "setting-manage-applications-efl");
        app_control_add_extra_data(service, "viewtype", "application-info");
        app_control_add_extra_data(service, "pkgname", app.pkg_id.c_str());
        app_control_send_launch_request(service, NULL, NULL);
        app_control_destroy(service);
    }
    return true;
}

} // UI
} // CCHECKER
