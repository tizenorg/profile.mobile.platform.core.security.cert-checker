/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        main.cpp
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Popup service main
 */
#include <exception>

#include <Elementary.h>

#include "common/log.h"
#include "popup-service.h"

using namespace CCHECKER::UI;

int main(int argc, char **argv)
{
	try {
		LogDebug("Cert-checker popup start. set locale first.");
		setlocale(LC_ALL, "");

		elm_init(argc, argv);
		PopupService service(POPUP_STREAM);
		service.start();

		return 0;
	} catch (const std::exception &e) {
		LogError("Exception occured in popup main : " << e.what());
		return -1;
	} catch (...) {
		LogError("Unhandled exception occured in popup main.");
		return -1;
	}
}
