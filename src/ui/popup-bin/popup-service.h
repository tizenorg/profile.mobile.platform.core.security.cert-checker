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
 * @file        popup-service.h
 * @author      Sangwan Kwon (sagnwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Popup ui service for cert-checker
 */
#pragma once

#include <string>

#include "common/service.h"

namespace CCHECKER {
namespace UI {

class PopupService : public Service {
public:
	PopupService(const std::string &address);
	virtual ~PopupService() = default;

	PopupService(const PopupService &) = delete;
	PopupService &operator=(const PopupService &) = delete;
	PopupService(PopupService &&) = delete;
	PopupService &operator=(PopupService &&) = delete;

private:
	virtual void onMessageProcess(const ConnShPtr &) override;
	RawBuffer process(const ConnShPtr &, RawBuffer &);
};

} // namespace UI
} // namespace CCHECKER
