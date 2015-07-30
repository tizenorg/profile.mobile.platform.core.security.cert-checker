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
 * @file        logic_.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of SQL queries
 */

/*
 * This Class makes all methods from Logic Class public - for testing purpose.
 * Some of methods are stubbed, and some of them just calls corresponding methods from Logic Class.
 */

#include <cchecker/log.h>

#include <logic_.h>

namespace CCHECKER {

Logic_::Logic_(void) :
        Logic(),
        _done(false),
        _done_buffer(false)
{}

Logic_::~Logic_(void)
{}

// For tests only

void Logic_::connman_callback_manual_(bool state)
{
    set_online(state);

    if (state) {
        m_to_process.notify_one();
    }
}

void Logic_::pkgmgr_install_manual_(const app_t &app)
{
    m_queue.push_event(event_t(app, event_t::event_type_t::APP_INSTALL));
    m_to_process.notify_one();
}

void Logic_::pkgmgr_uninstall_manual_(const app_t &app)
{
    m_queue.push_event(event_t(app, event_t::event_type_t::APP_UNINSTALL));
    m_to_process.notify_one();
}

void Logic_::_notify(bool buffer)
{
    if (buffer) {
        _done_buffer = true;
        return;
    }

    _done = true;
    _m_wait_for_process.notify_one();
}

void Logic_::_wait(int line)
{
    LogDebug("_wait begin: " << line);
    std::unique_lock<std::mutex> lock(_m_mutex_wait_cv);
    _done = false;
    _done_buffer = false;
    bool should_wait = true;

    // Protection against deadlock
    while (!_done || !_done_buffer || should_wait) {
        if (should_wait && m_do_not_sleep.try_lock()) {
            _m_wait_for_process.wait(lock);
            should_wait = false;
            m_do_not_sleep.unlock();
        }
        else
            m_to_process.notify_one();
    }

    LogDebug("_wait end: " << line);
}

const std::list<app_t>& Logic_::get_buffer_()
{
    return m_buffer;
}

} // CCHECKER
