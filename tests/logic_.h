/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        logic_.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       This file is the tesst implementation of Logic class
 */

#ifndef CCHECKER_LOGIC__H
#define CCHECKER_LOGIC__H

/*
 * This Class makes all methods from Logic Class public - for testing purpose.
 * Some of methods are stubbed, and some of them just calls corresponding methods from Logic Class.
 */

#include "service/logic.h"

namespace CCHECKER {

class Logic_ : public Logic {
public:
	Logic_(void);
	virtual ~Logic_(void);
	virtual void clean(void);

	// For tests only
	void connman_callback_manual_(bool state);
	void pkgmgr_install_manual_(const app_t &app);
	void pkgmgr_uninstall_manual_(const app_t &app);
	const std::list<app_t> &get_buffer_();

	void reset_cnt();
	void wait_for_worker(int installCnt, int uninstallCnt, int bufferCnt);

protected:
	void job(void) override;

private:
	int m_installCnt;
	int m_uninstallCnt;
	int m_bufferCnt;

	void process_event(const event_t &event);
	void app_processed() override;
	std::condition_variable _m_wait_for_process;
	std::mutex _m_mutex_wait_cv;
};

class LogicWrapper {
public:
	LogicWrapper() {}
	~LogicWrapper()
	{
		m_logic.clean();
	}

	error_t setup()
	{
		return m_logic.setup();
	}
	void clean()
	{
		m_logic.clean();
	}
	void connman_callback_manual_(bool state)
	{
		m_logic.connman_callback_manual_(state);
	}
	void pkgmgr_install_manual_(const app_t &app)
	{
		m_logic.pkgmgr_install_manual_(app);
	}
	void pkgmgr_uninstall_manual_(const app_t &app)
	{
		m_logic.pkgmgr_uninstall_manual_(app);
	}
	const std::list<app_t> &get_buffer_()
	{
		return m_logic.get_buffer_();
	}

	void wait_for_worker(int installCnt = 0, int uninstallCnt = 0, int bufferCnt = 0)
	{
		m_logic.wait_for_worker(installCnt, uninstallCnt, bufferCnt);
	}

	// timer operation
	void timerStart(int interval)
	{
		m_logic.timerStart(interval);
	}
	void timerStop()
	{
		m_logic.timerStop();
	}

	// gio operation
	void run(guint timeout)
	{
		m_logic.run(timeout);
	}
	bool is_running()
	{
		return m_logic.is_running();
	}

private:
	Logic_ m_logic;
};

} // CCHECKER

#endif //CCHECKER_LOGIC__H
