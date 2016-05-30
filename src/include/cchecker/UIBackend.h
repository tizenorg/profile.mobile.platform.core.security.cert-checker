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
/**
 * @file        UIBackend.h
 * @author      Janusz Kozerski <j.kozerski@samsung.com>
 * @brief       This file declares class for ask user window
 */

#pragma once

#include "service/app.h"

namespace CCHECKER {
namespace UI {

enum popup_status : int {
	NO_ERROR   = 0,
	EXIT_ERROR = 1
};

enum response_e : int {
	DONT_UNINSTALL = 2,
	UNINSTALL      = 3,
	RESPONSE_ERROR = 4
};

class UIBackend {
public:
	explicit UIBackend(int timeout = 60); //timeout in seconds (zero or less means infinity)
	virtual ~UIBackend();

	// Displays popup with question, and - if needed - app_control for removing application.
	// Returns true if UI was displayed correctly and user's response was collected.
	// If there was a problem with displaying UI or a timeout has been reached (no user's response)
	// then returns false.
	bool call_popup(const app_t &app);

private:
	response_e run(const app_t &app);

	const int m_responseTimeout; // seconds
};

} // UI
} // CCHECKER
