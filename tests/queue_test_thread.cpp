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

#include "service/app.h"
#include "service/queue.h"
#include "common/log.h"

#include <queue_test_thread.h>

namespace CCHECKER {

#define APPS_COUNT 10

app_t app[APPS_COUNT] = {
	app_t("app_id0", "###",     0, {}),
	app_t("app_id1", "pkg_id1", 1, {}),
	app_t("app_id2", "pkg_id2", 2, {}),
	app_t("app_id3", "###",     3, {}),
	app_t("app_id4", "###",     4, {}),
	app_t("app_id5", "###",     5, {}),
	app_t("app_id6", "###",     6, {}),
	app_t("app_id7", "###",     7, {}),
	app_t("app_id8", "###",     8, {}),
	app_t("app_id9", "###",     9, {})
};

void TestQueue::add_events_th(int number_of_threads)
{
	if (number_of_threads < 1) {
		LogDebug("Too few threads, at least 1 should be used");
		number_of_threads = 1;
	}

	m_sent = 0;
	m_max_events = number_of_threads * 2 * APPS_COUNT;
	LogDebug("Running threads");

	for (int i = 0; i < number_of_threads; i++)
		std::thread(&TestQueue::add_events, this).detach();

	LogDebug("Running threads done");
}

void TestQueue::add_events()
{
	for (int i = 0; i < APPS_COUNT; i++) {
		m_queue.push_event(event_t(app[i], event_t::event_type_t::APP_INSTALL));
		m_sent++;
		LogDebug("Add event : " << i);
	}

	for (int i = 0; i < APPS_COUNT; i++) {
		m_queue.push_event(event_t(app[i], event_t::event_type_t::APP_UNINSTALL));
		m_sent++;
		LogDebug("Add event : " << i);
	}
}

bool TestQueue::pop_events()
{
	int i_install = 0;
	int i_uninstall = 0;
	event_t ev;
	LogDebug("Looking events...");

	do {
		while (m_queue.pop_event(ev)) {
			if (ev.event_type == event_t::event_type_t::APP_INSTALL)
				i_install++;
			else if (ev.event_type == event_t::event_type_t::APP_UNINSTALL)
				i_uninstall++;
		}
	} while (m_sent < m_max_events);

	// Read all events once again - just for case
	while (m_queue.pop_event(ev)) {
		if (ev.event_type == event_t::event_type_t::APP_INSTALL)
			i_install++;
		else if (ev.event_type == event_t::event_type_t::APP_UNINSTALL)
			i_uninstall++;
	}

	LogDebug("Found " << i_install + i_uninstall << " events.");

	if (i_install != m_max_events / 2 || i_uninstall != m_max_events / 2)
		return false;

	return true;
}

} // CCHECKER
