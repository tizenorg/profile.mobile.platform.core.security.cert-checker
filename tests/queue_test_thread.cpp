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
 * @file        queue_test_thread.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of class for multi-thread testing Queue
 */

#include <thread>
#include <unistd.h>

#include <cchecker/app.h>
#include <cchecker/queue.h>
#include <cchecker/log.h>

#include <queue_test_thread.h>

namespace CCHECKER {

app_t app0("app_id0", "###",     0, {});
app_t app1("app_id1", "pkg_id1", 1, {});
app_t app2("app_id2", "pkg_id2", 2, {});
app_t app3("app_id3", "###",     3, {});
app_t app4("app_id4", "###",     4, {});
app_t app5("app_id5", "###",     5, {});
app_t app6("app_id6", "###",     6, {});
app_t app7("app_id7", "###",     7, {});
app_t app8("app_id8", "###",     8, {});
app_t app9("app_id9", "###",     9, {});

app_t app[10] = {app0, app1, app2, app3, app4, app5, app6, app7, app8, app9};

event_t ev1(app1, event_t::event_type_t::APP_INSTALL);
event_t ev2(app1, event_t::event_type_t::APP_UNINSTALL);

event_t ev3(app2, event_t::event_type_t::APP_INSTALL);
event_t ev4(app2, event_t::event_type_t::APP_UNINSTALL);

void TestQueue::add_events_th()
{
    LogDebug("Running threads");
    for (int i=0; i<10; i++)
        std::thread(&TestQueue::add_events, this, i, event_t::event_type_t::APP_INSTALL).detach();

    for (int i=0; i<10; i++)
        std::thread(&TestQueue::add_events, this, i, event_t::event_type_t::APP_UNINSTALL).detach();

    LogDebug("Running threads done");
}

void TestQueue::add_events(int num, event_t::event_type_t type)
{
    if (num>9 || num <0) {
        return;
    }

    m_queue.push_event(event_t(app[num], type));
    LogDebug("Add event : " << num);

    m_done++;
}

bool TestQueue::pop_events()
{
    int i = 0;
    event_t ev;

    LogDebug("Looking events...");
    do {
        while(m_queue.pop_event(ev)) {
            i++;
            LogDebug("Found event no. " << i);
        }
    } while (m_done < 20);

    // Read all events once again - just for case
    while(m_queue.pop_event(ev)) {
        i++;
        LogDebug("Found event no. " << i);
    }
    LogDebug("Found " << i << " events.");

    if (i != 20)
        return false;
    return true;
}

} // CCHECKER
