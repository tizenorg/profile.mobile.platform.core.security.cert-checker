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
 * @file        app_test_class.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of app_test class (from app_t)
 */

#include "common/log.h"
#include <app_event_operators.h>

//using namespace CCHECKER;

namespace CCHECKER {

void sort_buffer(std::list<app_t> &buff)
{
	for (auto &iter : buff) {
		sort(iter);
	}

	buff.sort();
}

void sort(app_t &app)
{
	app.signatures.sort();
}

// Needed for sort()
bool operator < (const app_t &app1, const app_t &app2)
{
	if (app1.app_id != app2.app_id)
		return app1.app_id < app2.app_id;

	if (app1.pkg_id != app2.pkg_id)
		return app1.pkg_id < app2.pkg_id;

	if (app1.uid != app2.uid)
		return app1.uid < app2.uid;

	return app1.signatures < app2.signatures;
}

bool operator ==(const app_t &app1, const app_t &app2)
{
	if (app1.app_id != app2.app_id ||
			app1.pkg_id != app2.pkg_id ||
			app1.uid != app2.uid ||
			app1.signatures.size() != app2.signatures.size() ||
			app1.verified != app2.verified) {
		LogDebug("app_t compare error: " << app1.str() << " is different than: " << app2.str());
		return false;
	}

	return app1.signatures == app2.signatures;
}

bool operator !=(const app_t &app1, const app_t &app2)
{
	return !(app1 == app2);
}

bool operator ==(const event_t &event1, const event_t &event2)
{
	if (event1.event_type != event2.event_type)
		return false;

	if (event1.app != event2.app)
		return false;

	return true;
}

bool operator !=(const event_t &event1, const event_t &event2)
{
	return !(event1 == event2);
}

} // CHCHECKER
