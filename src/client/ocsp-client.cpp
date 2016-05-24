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
 * @file        ocsp-client.cpp
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Client request OcspService with dispatcher in it
 */

#include <string>

#include "client/ocsp-client.h"
#include "cchecker/error.h"
#include "common/command-id.h"
#include "common/dispatcher.h"
#include "common/log.h"

namespace CCHECKER {

class OcspClient::Impl
{
public:
	Impl() : m_address(SERVICE_STREAM) {
		m_dispatcher.reset(new Dispatcher(m_address));
	}
	virtual ~Impl() {}

	CCerr request() {
		LogDebug("Request to oscp service.");
		auto ret = m_dispatcher->methodCall<CommandId>(CommandId::CC_OCSP_SYN);

		LogDebug("Response ret : " << static_cast<int>(ret));
		if(ret == CommandId::CC_OCSP_ACK)
			return E_CC_NONE;
		else
			return E_CC_INTERNAL;
	}

private:
	std::string m_address;
	std::unique_ptr<Dispatcher> m_dispatcher;
};

OcspClient::OcspClient() : m_impl(new Impl)
{
}

OcspClient::~OcspClient()
{
}

CCerr OcspClient::request()
{
	return m_impl->request();
}

} // namespace CCHECKER
