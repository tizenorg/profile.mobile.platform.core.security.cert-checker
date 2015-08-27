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
#include <chrono>
#include <boost/test/unit_test.hpp>

#include <logic_.h>

namespace CCHECKER {

Logic_::Logic_(void) :
        Logic(),
        m_installCnt(0),
        m_uninstallCnt(0),
        m_bufferCnt(0)
{}

Logic_::~Logic_(void)
{
    clean();
}

// For tests only

void Logic_::connman_callback_manual_(bool state)
{
    Logic::set_online(state);
}

void Logic_::pkgmgr_install_manual_(const app_t &app)
{
    push_event(event_t(app, event_t::event_type_t::APP_INSTALL));
}

void Logic_::pkgmgr_uninstall_manual_(const app_t &app)
{
    push_event(event_t(app, event_t::event_type_t::APP_UNINSTALL));
}

void Logic_::process_event(const event_t &event)
{
    Logic::process_event(event);

    std::lock_guard<std::mutex> lock(_m_mutex_wait_cv);
    switch(event.event_type)
    {
    case event_t::event_type_t::APP_INSTALL:
        m_installCnt++;
        LogDebug(m_installCnt << " " << m_uninstallCnt << " " << m_bufferCnt);
        break;
    case event_t::event_type_t::APP_UNINSTALL:
        m_uninstallCnt++;
        LogDebug(m_installCnt << " " << m_uninstallCnt << " " << m_bufferCnt);
        break;
    default:
        return;
    }
    // notify caller
    _m_wait_for_process.notify_one();
}

void Logic_::app_processed()
{
    std::lock_guard<std::mutex> lock(_m_mutex_wait_cv);
    m_bufferCnt++;
    LogDebug(m_installCnt << " " << m_uninstallCnt << " " << m_bufferCnt);

    // notify caller
    _m_wait_for_process.notify_one();
}

void Logic_::reset_cnt()
{
    m_installCnt = 0;
    m_uninstallCnt = 0;
    m_bufferCnt = 0;
}

void Logic_::wait_for_worker(int installCnt, int uninstallCnt, int bufferCnt)
{
    LogDebug("Wait for: " << installCnt << " " << uninstallCnt << " " << bufferCnt);
    std::unique_lock<std::mutex> lock(_m_mutex_wait_cv);
    bool timeout = !_m_wait_for_process.wait_for(
            lock,
            std::chrono::seconds(10),
            [this, installCnt, uninstallCnt, bufferCnt]{
                return m_installCnt == installCnt &&
                       m_uninstallCnt == uninstallCnt &&
                       m_bufferCnt == bufferCnt;
            }
    );
    _m_mutex_wait_cv.unlock();
    BOOST_REQUIRE(!timeout);
    reset_cnt();
}

const std::list<app_t>& Logic_::get_buffer_()
{
    return m_buffer;
}

} // CCHECKER
