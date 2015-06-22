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
/**
 * @file        UIBackend.h
 * @author      Janusz Kozerski <j.kozerski@samsung.com>
 * @brief       This file declares class for ask user window
 */

#pragma once

#include <notification.h>

#include <cchecker/app.h>

namespace CCHECKER {
namespace UI {

enum response_e {
    DONT_UNINSTALL,
    UNINSTALL,
    ERROR
};

// FIXME: notification framework is corrupted and it doesn't work as it should

class UIBackend {
public:
    UIBackend(); // default timeout is 60 seconds
    UIBackend(int timeout); //timeout in seconds
    virtual ~UIBackend();

    bool call_popup(const app_t &app);

private:
    void run(response_e &response);
    bool createUI(const std::string &app_id, const std::string &pkg_id);

    notification_h m_notification;
    const int m_responseTimeout; // seconds
};

} // UI
} // CCHECKER
