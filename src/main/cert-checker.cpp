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
 * @file        cert-checker.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Cert-checker daemon main loop.
 */

#include <glib.h>

#include <cchecker/log.h>

#include "service/logic.h"
#include "service/ocsp-service.h"

using namespace CCHECKER;

int main(void)
{
	try {
		LogInfo("Cert-checker start!");

		CCHECKER::OcspService service(SERVICE_STREAM);

		setlocale(LC_ALL, "");

		// Set timeout about socket read event.
		service.setTimeout(50);
		service.start();

		LogInfo("Cert-checker exit!");
		return 0;

	} catch (const std::exception &e) {
		LogError("Exception occured in cert-checker main : " << e.what());
		return -1;
	} catch (...) {
		LogError("Unhandled exception occured in cert-checker main.");
		return -1;
	}
}
