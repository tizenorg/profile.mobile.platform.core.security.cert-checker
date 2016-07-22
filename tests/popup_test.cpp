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
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Cert-checker popup test
 */

#include "service/app.h"
#include "common/log.h"
#include "ui/popup-client.h"

int main(int, char **)
{
	LogDebug("Cert-checker popup-test start!");
	setlocale(LC_ALL, "");

	CCHECKER::app_t app(
			std::string("test_APP_ID"),
			std::string("test_PKG_ID"),
			5005,
			{});

	CCHECKER::UI::PopupClient client;
	// TODO(sangwan.kwon) Add boost TC for output FAIL, SUCCESS
	if (client.dispatch(app))
		LogDebug("Success to launch popup.");
	else
		LogError("Failed to launch popup.");

	LogDebug("Cert-checker popup-test exit!");
	return 0;
}
