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
 * @file        ocsp-service.cpp
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief
 */
#include "service/ocsp-service.h"

#include <cchecker/log.h>

namespace CCHECKER {

OcspService::OcspService(const std::string &address) :
	Service(address)
{
}

OcspService::~OcspService()
{
}

void OcspService::onMessageProcess(const ConnShPtr &connection)
{
	LogDebug("Start to process message on ocsp service.");

	auto in = connection->receive();
	connection->send(this->process(connection, in));

	if (m_logic.setup() != NO_ERROR)
		throw std::logic_error("Cannot setup logic.");

	LogDebug("Start to process message on ocsp service.");
}

RawBuffer OcspService::process(const ConnShPtr &, RawBuffer &data)
{
	BinaryQueue q;
	q.push(data);

	int cmd;
	q.Deserialize(cmd);

	LogInfo("Request dispatch on ocsp-service.");
	switch (static_cast<CommandId>(cmd)) {
	case CommandId::CC_OCSP_SYN: {
		// TODO(sangwan.kwon) : set protocol with client
		return BinaryQueue::Serialize(1).pop();
	}
	case CommandId::CC_OCSP_ACK:
	default:
		throw std::logic_error("Protocol error. unknown command id.");
	}
}

} // namespace CCHECKER
