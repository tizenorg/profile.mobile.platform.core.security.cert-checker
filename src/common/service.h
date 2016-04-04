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
 * @file        service.h
 * @author      Kyungwook Tak (k.tak@samsung.com)
 * @author      Sangwan Kwon  (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief
 */
#pragma once

#include <string>
#include <functional>

#include "common/connection.h"
#include "common/mainloop.h"

namespace CCHECKER {

using ConnCallback = std::function<void(const ConnShPtr &)>;

class Service {
public:
	Service(const std::string &address);
	virtual ~Service();

	Service(const Service &) = delete;
	Service &operator=(const Service &) = delete;
	Service(Service &&) = delete;
	Service &operator=(Service &&) = delete;

	virtual void start(void) final;
	virtual void stop(void) final;

	/* ConnCallback param should throw exception to handle error */
	virtual void setNewConnectionCallback(const ConnCallback &) final;
	virtual void setCloseConnectionCallback(const ConnCallback &) final;

	virtual void setTimeout(int timeout) final;

private:
	virtual void onMessageProcess(const ConnShPtr &) = 0;

	ConnCallback m_onNewConnection;
	ConnCallback m_onCloseConnection;

	int m_timeout;
	std::unordered_map<int, ConnShPtr> m_connectionRegistry;
	Mainloop m_loop;
	std::string m_address;
};

} // namespace CCHECKER
