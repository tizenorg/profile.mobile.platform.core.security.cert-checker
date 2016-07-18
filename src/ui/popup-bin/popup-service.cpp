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

#include <memory>

#include <app_control.h>

#include "popup.h"
#include "service/app.h"
#include "common/binary-queue.h"
#include "common/log.h"

namespace CCHECKER {
namespace UI {

namespace {
struct AppControl {
	AppControl() { app_control_create(&handle); }
	~AppControl() { app_control_destroy(handle); }

	app_control_h handle;
};

bool launchSettingManager(const std::string &pkgId)
{
	LogDebug("Start to launch setting manager about pkg : " << pkgId);
	std::unique_ptr<AppControl> ac(new AppControl);
	app_control_set_operation(ac->handle, APP_CONTROL_OPERATION_DEFAULT);
	app_control_set_app_id(ac->handle, "setting-manage-applications-efl");
	app_control_add_extra_data(ac->handle, "viewtype", "application-info");
	app_control_add_extra_data(ac->handle, "pkgname", pkgId.c_str());
	auto ret = app_control_send_launch_request(ac->handle, NULL, NULL);
	return (ret == APP_CONTROL_ERROR_NONE) ? true : false;
}
} // Anonymous namespace

PopupService::PopupService(const std::string &address) :
	Service(address)
{
}

void PopupService::onMessageProcess(const ConnShPtr &connection)
{
	LogDebug("Start to process message on popup service.");
	auto in = connection->receive();
	connection->send(this->process(connection, in));
}

RawBuffer PopupService::process(const ConnShPtr &, RawBuffer &data)
{
	LogDebug("Start to receive data from connection.");
	BinaryQueue q;
	q.push(data);

	app_t app;
	q.Deserialize(app);

	LogDebug("Processing popup-service. [" << app.str() << "]");
	Popup popup(app);
	auto response = popup.run();

	switch (response) {
	case ResponseType::UNINSTALL : {
		LogDebug("User decide to uninstall package.");
		if(launchSettingManager(app.pkg_id))
			LogDebug("Success to launch setting manager.");
		else
			LogError("Failed to launch setting manager.");
		return BinaryQueue::Serialize(true).pop();
	}
	case ResponseType::KEEP : {
		LogDebug("User decide to keep package.");
		return BinaryQueue::Serialize(true).pop();
	}
	default : {
		LogError("Protocol broken on popup-service.");
		return BinaryQueue::Serialize(false).pop();
	}
	}
}

} // namespace UI
} // namespace CCHECKER
