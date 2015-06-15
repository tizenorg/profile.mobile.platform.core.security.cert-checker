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
 * @file        queue.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of thread-safe queue
 */

#include <mutex>

#include <cchecker/queue.h>

namespace CCHECKER {

event_t::event_t():
        event_type(event_type_t::EVENT_TYPE_UNKNOWN),
        app()
{}

event_t::event_t(const app_t &app, event_type_t type):
        event_type(type),
        app(app)
{}

void Queue::push_event(const event_t &event)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_event_list.push(event);
}

bool Queue::pop_event(event_t &event)
{
    if (m_event_list.empty())
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    event = m_event_list.front();
    m_event_list.pop();
    return true;
}

bool Queue::top(event_t &event)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_event_list.empty()) {
        return false;
    }

    event = m_event_list.front();
    return true;
}

bool Queue::empty()
{
    return m_event_list.empty();
}

} //CCHECKER
