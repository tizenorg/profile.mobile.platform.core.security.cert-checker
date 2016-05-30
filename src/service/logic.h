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
 * @file        logic.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of SQL queries
 */

#ifndef CCHECKER_LOGIC_H
#define CCHECKER_LOGIC_H

#include <condition_variable>
#include <gio/gio.h>
#include <string>
#include <vector>
#include <list>
#include <thread>
#include <memory>

#include "common/types.h"
#include "common/timer.h"
#include "common/binary-queue.h"
#include "service/app.h"
#include "service/certs.h"
#include "service/queue.h"

#include <package-manager.h>
#include <pkgmgr-info.h>

namespace CCHECKER {

const int TIMEOUT_TIMER = 3600;

namespace DB {
class SqlQuery;
} // namespace DB

enum error_t {
	NO_ERROR,
	REGISTER_CALLBACK_ERROR,
	DBUS_ERROR,
	PACKAGE_MANAGER_ERROR,
	DATABASE_ERROR,
	INTERNAL_ERROR
};

enum pkgmgr_event_t {
	EVENT_INSTALL,
	EVENT_UNINSTALL
};

class Logic : public Timer {
public:
	Logic(void);
	virtual ~Logic(void);
	error_t  setup(void);
	void run(guint timeout);
	bool is_running();
	virtual void clean(void);

	static void connman_callback(GDBusProxy *proxy,
								 gchar      *sender_name,
								 gchar      *signal_name,
								 GVariant   *parameters,
								 void *logic_ptr);

protected:
	// Timer function
	void job(void) override;

	virtual void process_event(const event_t &event);
	virtual void app_processed() {}; // for tests
	void set_should_exit(void);
	void push_event(event_t event);
	void set_online(bool online);

	std::list<app_t> m_buffer;
	std::condition_variable m_to_process;
	std::mutex m_mutex_cv;
	std::thread m_thread;

private:
	error_t setup_db();
	void load_database_to_buffer();
	void add_app_to_buffer_and_database(const app_t &app);
	void remove_app_from_buffer_and_database(const app_t &app);

	void set_connman_online_state();
	error_t register_dbus_signal_handler(GDBusProxy **proxy,
										 const char *name,
										 const char *object_path,
										 const char *interface_name,
										 void (*callback)(GDBusProxy *proxy,
												 gchar      *sender_name,
												 gchar      *signal_name,
												 GVariant   *parameters,
												 void *logic_ptr)
										);
	bool get_online(void) const;

	static int pkgmgrinfo_event_handler_static(
		uid_t uid,
		int reqid,
		const char *pkgtype,
		const char *pkgid,
		const char *key,
		const char *val,
		const void *pmsg,
		void *data);
	int pkgmgrinfo_event_handler(
		uid_t uid,
		int reqid,
		const char *pkgtype,
		const char *pkgid,
		const char *key,
		const char *val,
		const void *pmsg,
		void *data);
	int push_pkgmgrinfo_event(uid_t uid, const char *pkgid);

	void process_all(void);
	void process_queue(void);
	bool process_app(app_t &app);
	void process_buffer(void);
	bool get_should_exit(void) const;

	bool call_ui(const app_t &app);

	static gboolean timeout_cb(gpointer data);

	// main event loop data type
	GMainLoop *m_loop;

	Queue m_queue;
	Certs m_certs;
	DB::SqlQuery *m_sqlquery;
	bool m_was_setup_called;

	bool m_is_online;
	// TODO: use m_queue for online events
	bool m_is_online_enabled;
	bool m_should_exit;
	static std::atomic<bool> m_is_first_run;

	GDBusProxy *m_proxy_connman;

	// about pkgmgr event
	int m_reqid_install;
	int m_reqid_uninstall;
	std::unique_ptr<pkgmgrinfo_client, int(*)(pkgmgrinfo_client *)> m_pc_install;
	std::unique_ptr<pkgmgrinfo_client, int(*)(pkgmgrinfo_client *)> m_pc_uninstall;
};

} // namespace CCHECKER

#endif //CCHECKER_LOGIC_H
