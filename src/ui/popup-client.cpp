/*
 *  Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        popup-client.cpp
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Client about popup service
 */
#include "popup-client.h"

#include "common/log.h"

using namespace CCHECKER::UI;

PopupClient::PopupClient() : m_address(POPUP_STREAM)
{
	m_dispatcher.reset(new Dispatcher(m_address));
}

bool PopupClient::dispatch(const app_t &app)
{
	LogDebug("Dispatch popup service. app : " << app.str());
	return m_dispatcher->methodCall<bool>(app);
}
