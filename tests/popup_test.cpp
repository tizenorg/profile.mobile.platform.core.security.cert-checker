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
/*
 * @file        popup_test.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Cert-checker popup test
 */

#include <cchecker/UIBackend.h>
#include <cchecker/log.h>

using namespace CCHECKER;

int main(int argc, char** argv)
{
    LogDebug("Cert-checker popup-test start!");

    int timeout = 60;
    if (argc > 1) {
        timeout = atoi(argv[1]);
    }
    LogDebug("popup-test timeout: " << timeout);

    setlocale(LC_ALL, "");

    UI::UIBackend ui(timeout);

    app_t app(std::string("test_APP_ID"),
            std::string("test PKG ID"),
            5005,
            {});

    ui.call_popup(app);

    LogDebug("Cert-checker popup-test exit!");
    return 0;
}
