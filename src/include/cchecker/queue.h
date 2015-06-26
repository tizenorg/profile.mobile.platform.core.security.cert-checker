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
 * @file        queue.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of thread-safe queue
 */

#ifndef CCHECKER_QUEUE_H
#define CCHECKER_QUEUE_H

#include <mutex>
#include <queue>

#include <cchecker/app.h>

namespace CCHECKER {

struct event_t {
    enum class event_type_t {
        APP_INSTALL,
        APP_UNINSTALL,
        EVENT_TYPE_UNKNOWN
    };

    event_type_t event_type;
    app_t        app;

    event_t();
    event_t(const app_t &app, event_type_t type);

    bool operator ==(const event_t &event) const;
    bool operator !=(const event_t &event) const;
};

class Queue {
    public:
        void push_event(const event_t &event);
        bool pop_event(event_t &event);
        bool empty();

    private:
        std::mutex          m_mutex;
        std::queue<event_t> m_event_list;
};

} // CCHECKER

#endif // CCHECKER_QUEUE_H
