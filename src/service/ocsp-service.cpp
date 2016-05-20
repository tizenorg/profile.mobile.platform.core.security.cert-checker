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
#include "common/command-id.h"

#include "common/log.h"

namespace CCHECKER {

OcspService::OcspService(const std::string &address) :
	Service(address)
{
}

OcspService::~OcspService()
{
	if(m_thread.joinable())
		m_thread.join();
}

void OcspService::run(void)
{
	m_logic.run(TIMEOUT_G_SERVICE);
}

void OcspService::onMessageProcess(const ConnShPtr &connection)
{
	LogDebug("Start to process message on ocsp service.");

	auto in = connection->receive();
	connection->send(this->process(connection, in));

	// Run gmainloop for event listening.
	if(!m_logic.is_running())
		m_thread = std::thread(&OcspService::run,this);

	LogDebug("Finish processing message on ocsp service.");
}

RawBuffer OcspService::process(const ConnShPtr &, RawBuffer &data)
{
	BinaryQueue q;
	q.push(data);

	CommandId cid;
	q.Deserialize(cid);

	LogInfo("Request dispatch on ocsp-service.");
	switch (cid) {
	case CommandId::CC_OCSP_SYN: {
		if (m_logic.setup() != NO_ERROR) {
			BinaryQueue::Serialize(CommandId::CC_OCSP_ERR).pop();
			throw std::logic_error("Cannot setup logic.");
		}

		LogDebug("Success to receive SYN and setup. reply ACK cmd.");
		return BinaryQueue::Serialize(CommandId::CC_OCSP_ACK).pop();
	}
	case CommandId::CC_OCSP_ACK:
	default:
		throw std::logic_error("Protocol error. unknown command id.");
	}
}

} // namespace CCHECKER
