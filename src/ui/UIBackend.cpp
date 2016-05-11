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

#include <app_control.h>

#include "common/log.h"
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
    return run_popup(app, m_responseTimeout);
}

bool UIBackend::call_popup(const app_t &app)
{
    response_e resp;

    resp = run(app);
    LogDebug(app.str() << " response: " << resp);
    if (resp == response_e::RESPONSE_ERROR) {
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
