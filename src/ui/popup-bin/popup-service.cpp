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
 * @file        popup-service.cpp
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Popup ui service for cert-checker
 */
#include "popup-service.h"

#include "popup.h"

#include "service/app.h"
#include "common/binary-queue.h"
#include "common/log.h"

namespace CCHECKER {
namespace UI {

PopupService::PopupService(const std::string &address) :
	Service(address)
{
}

void PopupService::onMessageProcess(const ConnShPtr &connection)
{
	LogDebug("Start to process message on popup service.");
	auto in = connection->receive();
	connection->send(this->process(connection, in));
	LogDebug("Finish processing message on popup service.");
}

RawBuffer PopupService::process(const ConnShPtr &, RawBuffer &data)
{
	LogDebug("Start to receive data from connection.");
	BinaryQueue q;
	q.push(data);

	app_t app;
	q.Deserialize(app);

	Popup popup(app);
	popup.run();

	LogInfo("Request dispatch on popup-service. app : " << app.str());

	return BinaryQueue::Serialize(true).pop();
}

} // namespace UI
} // namespace CCHECKER
