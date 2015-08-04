/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        app_test_class.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of app_test class (from app_t)
 */

#include <cchecker/app.h>
#include <cchecker/queue.h>

#ifndef CCHECKER_APP_TEST_CLASS_H
#define CCHECKER_APP_TEST_CLASS_H

namespace CCHECKER {

void sort_buffer(std::list<app_t> &buff);

void sort(app_t &app);
bool operator ==(const app_t &app1, const app_t &app2);
bool operator !=(const app_t &app1, const app_t &app2);
bool operator < (const app_t &app1, const app_t &app2);

bool operator ==(const event_t &event1, const event_t &event2);
bool operator !=(const event_t &event1, const event_t &event2);

} // CCHECKER

#endif //CCHECKER_APP_TEST_CLASS_H
