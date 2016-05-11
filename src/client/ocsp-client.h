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
 * @file        ocsp-client.h
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Client request to ocsp service
 */
#pragma once

#include <string>
#include <memory>

#include "common/dispatcher.h"
#include "client/error.h"

namespace CCHECKER {
namespace Client {

class OcspClient {
public:
	OcspClient();
	virtual ~OcspClient();

	OcspClient(const OcspClient &) = delete;
	OcspClient &operator=(const OcspClient &) = delete;

	CCerr request();

private:
	std::string m_address;
	std::unique_ptr<Dispatcher> m_dispatcher;
};

} // namespace Client
} // namespace CCHECKER
