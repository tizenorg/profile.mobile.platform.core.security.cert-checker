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
 * @file        logic.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of SQL queries
 */
#include "logic.h"

#include <stdexcept>
#include <set>

#include <cchecker/sql_query.h>
#include <cchecker/UIBackend.h>

#include "common/binary-queue.h"
#include "common/log.h"

using namespace std;

namespace CCHECKER {

namespace {

struct PkgmgrinfoEvent {
	PkgmgrinfoEvent(uid_t _uid, const char *_pkgid)
		: uid(_uid)
		, pkgid(_pkgid) {}

	inline bool operator==(const PkgmgrinfoEvent &rhs) const
	{
		return uid == rhs.uid && pkgid.compare(rhs.pkgid) == 0;
	}

	inline bool operator<(const PkgmgrinfoEvent &rhs) const
	{
		if (uid < rhs.uid)
			return true;
		else if (uid < rhs.uid)
			return false;
		else
			return pkgid.compare(rhs.pkgid) < 0;
	}

	inline bool operator>(const PkgmgrinfoEvent &rhs) const
	{
		if (uid > rhs.uid)
			return true;
		else if (uid < rhs.uid)
			return false;
		else
			return pkgid.compare(rhs.pkgid) > 0;
	}

	uid_t uid;
	std::string pkgid;
	pkgmgr_event_t type;
};

std::set<PkgmgrinfoEvent> pkgmgrinfo_event_set;
const char *const DB_PATH = DB_INSTALL_DIR"/.cert-checker.db";

} // namespace Anonymous

Logic::~Logic(void)
{
	clean();
}

void Logic::clean(void)
{
	LogDebug("Cert-checker cleaning start.");

	// wait and join processing thread
	if (m_thread.joinable()) {
		LogDebug("Waiting for join processing thread");
		{
			std::lock_guard<std::mutex> lock(m_mutex_cv);
			set_should_exit();
			LogDebug("Notify thread : enforced by cleaning");
			m_to_process.notify_one();
		}
		m_thread.join();
		LogDebug("Processing thread joined");
	} else {
		LogDebug("No thread to join");
	}

	if (m_proxy_connman)
		g_object_unref(m_proxy_connman);

	if (m_loop)
		g_main_loop_unref(m_loop);

	delete m_sqlquery;
	timerStop();
	LogDebug("Cert-checker cleaning finish.");
}

Logic::Logic(void) :
	m_loop(g_main_loop_new(NULL, FALSE)),
	m_sqlquery(NULL),
	m_was_setup_called(false),
	m_is_online(false),
	m_is_online_enabled(false),
	m_should_exit(false),
	m_proxy_connman(NULL),
	m_pc_install(nullptr, nullptr),
	m_pc_uninstall(nullptr, nullptr)
{
}

bool Logic::get_online() const
{
	return m_is_online;
}

void Logic::set_online(bool online)
{
	std::lock_guard<std::mutex> lock(m_mutex_cv);
	m_is_online = online;

	if (m_is_online) {
		m_is_online_enabled = true;
		LogDebug("Notify thread : Network connected");
		m_to_process.notify_one();
	}
}

error_t Logic::setup_db()
{
	// TODO: If database doesn't exist -should we create a new one?
	Try {
		m_sqlquery = new DB::SqlQuery(DB_PATH);
	} Catch(runtime_error) {
		LogError("Error while creating SqlQuery object");
		return DATABASE_ERROR;
	}

	if (!m_sqlquery) {
		LogError("Cannot open database");
		return DATABASE_ERROR;
	}

	return NO_ERROR;
}

error_t  Logic::setup()
{
	// Check if setup was called
	if (m_was_setup_called) {
		LogDebug("Setup is already done.");
		return NO_ERROR;
	}

	m_was_setup_called = true;
	// Check if DB exists and create a new one if it doesn't
	error_t err = setup_db();

	if (err != NO_ERROR) {
		LogError("Database error");
		return err;
	}

	load_database_to_buffer();
	// run process thread - thread will be waiting on condition variable
	m_thread = std::thread(&Logic::process_all, this);
	// Add connman callback
	LogDebug("register connman event callback start");

	if (register_dbus_signal_handler(&m_proxy_connman,
									 "net.connman",
									 "/",
									 "net.connman.Manager",
									 connman_callback) != NO_ERROR) {
		LogError("Error in register_connman_signal_handler");
		return REGISTER_CALLBACK_ERROR;
	}

	LogDebug("register connman event callback success");
	set_connman_online_state();
	// Add pkgmgrinfo callback
	LogDebug("Register package event handler start");
	std::unique_ptr<pkgmgrinfo_client, int(*)(pkgmgrinfo_client *)>
	_pcInstall(pkgmgrinfo_client_new(PMINFO_LISTENING), pkgmgrinfo_client_free);
	std::unique_ptr<pkgmgrinfo_client, int(*)(pkgmgrinfo_client *)>
	_pcUninstall(pkgmgrinfo_client_new(PMINFO_LISTENING), pkgmgrinfo_client_free);
	m_pc_install = std::move(_pcInstall);
	m_pc_uninstall = std::move(_pcUninstall);

	if (!m_pc_install || !m_pc_uninstall) {
		LogError("Get pkgmgrinfo client failed");
		return REGISTER_CALLBACK_ERROR;
	}

	int ret_status_install;
	int ret_status_uninstall;
	ret_status_install = pkgmgrinfo_client_set_status_type(
							 m_pc_install.get(), PKGMGR_CLIENT_STATUS_INSTALL);
	ret_status_uninstall = pkgmgrinfo_client_set_status_type(
							   m_pc_uninstall.get(), PKGMGR_CLIENT_STATUS_UNINSTALL);

	if (ret_status_install == PMINFO_R_ERROR || ret_status_uninstall == PMINFO_R_ERROR) {
		LogError("Set pkgmgrinfo status fail");
		return REGISTER_CALLBACK_ERROR;
	}

	m_reqid_install = pkgmgrinfo_client_listen_status(
						  m_pc_install.get(), pkgmgrinfo_event_handler_static, this);
	m_reqid_uninstall = pkgmgrinfo_client_listen_status(
							m_pc_uninstall.get(), pkgmgrinfo_event_handler_static, this);

	if (m_reqid_install < 0 || m_reqid_uninstall < 0) {
		LogError("Register pacakge install event handler fail");
		return REGISTER_CALLBACK_ERROR;
	}

	// Init for gio timeout.
	m_is_first_run = false;
	LogDebug("Register package event handler success");
	return NO_ERROR;
}

void Logic::run(guint timeout)
{
	LogDebug("Running the main loop");
	g_timeout_add_seconds(timeout, timeout_cb, m_loop);
	g_main_loop_run(m_loop);
}

int Logic::pkgmgrinfo_event_handler_static(
	uid_t uid,
	int reqid,
	const char *pkgtype,
	const char *pkgid,
	const char *key,
	const char *val,
	const void *pmsg,
	void *data)
{
	LogDebug("pkgmgrinfo event handler start!!");

	if (data == nullptr)
		return -1;

	std::string keyStr(key);
	std::string valStr(val);
	LogDebug("pkgmgrinfo event was caught. type : " << valStr << ", status : " << keyStr);

	if ((valStr.compare("install") == 0 || valStr.compare("uninstall") == 0)
			&& keyStr.compare("start") == 0)  {
		return static_cast<Logic *>(data)->pkgmgrinfo_event_handler(
				   uid, reqid, pkgtype, pkgid, key, val, pmsg, data);
	} else if (keyStr.compare("end") == 0 && valStr.compare("ok") == 0) {
		return static_cast<Logic *>(data)->push_pkgmgrinfo_event(uid, pkgid);
	} else {
		// TODO(sangwan.kwon) if get untreat event like fail, must quit loop
		LogDebug("Untreated event was caught : " << val);
		return -1;
	}
}

int Logic::pkgmgrinfo_event_handler(
	uid_t uid,
	int reqid,
	const char */*pkgtype*/,
	const char *pkgid,
	const char *key,
	const char *val,
	const void */*pmsg*/,
	void */*data*/)
{
	if (pkgid == nullptr || key == nullptr || val == nullptr) {
		LogError("Invalid parameter.");
		return -1;
	}

	std::string keyStr(key);
	std::string valStr(val);
	LogDebug("uid: " << uid << " pkgid: " << pkgid << " key: " << keyStr << " val: " << valStr);
	PkgmgrinfoEvent event(uid, pkgid);

	if (valStr.compare("install") == 0) {
		if (reqid != m_reqid_install) {
			LogError("pkgmgrinfo event reqid unmatched");
			return -1;
		}

		event.type = EVENT_INSTALL;
	} else if (valStr.compare("uninstall") == 0) {
		if (reqid != m_reqid_uninstall) {
			LogError("pkgmgrinfo event reqid unmatched");
			return -1;
		}

		event.type = EVENT_UNINSTALL;
	}

	pkgmgrinfo_event_set.insert(event);
	return 0;
}

int Logic::push_pkgmgrinfo_event(uid_t uid, const char *pkgid)
{
	PkgmgrinfoEvent event(uid, pkgid);
	std::set<PkgmgrinfoEvent>::iterator pkgmgrinfo_event_iter = pkgmgrinfo_event_set.find(event);

	if (pkgmgrinfo_event_iter != pkgmgrinfo_event_set.end()) {
		// FIXME: No information about app_id in the signal. Use stub.
		app_t app(TEMP_APP_ID, pkgid, uid, {});

		if (pkgmgrinfo_event_iter->type == EVENT_INSTALL) {
			LogDebug("Successfully Installed. uid: " << uid << ", pkgid: " << pkgid);
			push_event(event_t(app, event_t::event_type_t::APP_INSTALL));
		} else if (pkgmgrinfo_event_iter->type == EVENT_UNINSTALL) {
			LogDebug("Successfully Uninstalled. uid: " << uid << ", pkgid: " << pkgid);
			push_event(event_t(app, event_t::event_type_t::APP_UNINSTALL));
		}

		LogDebug("push pkgmgrifo success. pkgid: " << pkgid << ", uid: " << uid);
		pkgmgrinfo_event_set.erase(event);
		return 0;
	} else {
		// if update status, return fail
		LogDebug("push pkgmgrifo fail. pkgid: " << pkgid << ", uid: " << uid);
		return -1;
	}
}

error_t Logic::register_dbus_signal_handler(GDBusProxy **proxy,
		const char *name,
		const char *object_path,
		const char *interface_name,
		void (*callback)(GDBusProxy *proxy,
						 gchar      *sender_name,
						 gchar      *signal_name,
						 GVariant   *parameters,
						 void *logic_ptr)
										   )
{
	GError *error = NULL;
	GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;
	// Obtain a connection to the System Bus
	*proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
										   flags,
										   NULL, /* GDBusInterfaceInfo */
										   name,
										   object_path,
										   interface_name,
										   NULL, /* GCancellable */
										   &error);

	if (*proxy == NULL) {
		if (error) {
			LogError("Error creating D-Bus proxy for /'" << interface_name << "/': " << error->message);
			g_error_free(error);
		} else {
			LogError("Error creating D-Bus proxy for /'" << interface_name << "/'. Unknown error");
		}

		return DBUS_ERROR;
	}

	// Connect to g-signal to receive signals from proxy
	if (g_signal_connect(*proxy, "g-signal", G_CALLBACK(callback), this) < 1) {
		LogError("g_signal_connect error while connecting " << interface_name);
		return REGISTER_CALLBACK_ERROR;
	}

	return NO_ERROR;
}

void Logic::set_connman_online_state()
{
	GError *error = NULL;
	GVariant *response;

	if (m_proxy_connman == NULL) {
		LogError("connman proxy is NULL");
		return;
	}

	response = g_dbus_proxy_call_sync(m_proxy_connman,
									  "GetProperties",
									  NULL, // GetProperties gets no parameters
									  G_DBUS_CALL_FLAGS_NONE,
									  -1, // Default timeout
									  NULL,
									  &error);

	if (error) {
		LogError("Error while calling connman GetProperties() Dbus API: " << error->message);
		g_error_free(error);
		return;
	}

	if (response == NULL) {
		// This should never happen
		return;
	}

	gchar *resp_g = g_variant_print(response, TRUE);
	std::string resp_s(resp_g);
	LogDebug("response: " << resp_s);
	g_free(resp_g);

	// Response should look like this:
	// ({'State': <'online'>, 'OfflineMode': <false>, 'SessionMode': <false>},)
	if (resp_s.find("'State': <'online'>", 0) != std::string::npos) {
		LogDebug("Connman has returned: online");
		set_online(true);
	}

	// free memory
	g_variant_unref(response);
}

void Logic::connman_callback(GDBusProxy */*proxy*/,
							 gchar      */*sender_name*/,
							 gchar      *signal_name,
							 GVariant   *parameters,
							 void *logic_ptr)
{
	string signal_name_str = string(signal_name);

	if (signal_name_str != "PropertyChanged") {
		// Invalid param. Nothing to do here.
		return;
	}

	gchar *parameters_g = g_variant_print(parameters, TRUE);
	string params_str = string(parameters_g);
	g_free(parameters_g);
	Logic *logic = static_cast<Logic *>(logic_ptr);

	if (params_str == "('State', <'online'>)") {
		LogDebug("Device online");
		logic->set_online(true);
	} else if (params_str == "('State', <'offline'>)") {
		LogDebug("Device offline");
		logic->set_online(false);
	}
}

void Logic::load_database_to_buffer()
{
	LogDebug("Loading database to the buffer");
	m_sqlquery->get_app_list(m_buffer);
}

/**
 * This function should move all event from queue to the buffer
 **/
void Logic::process_queue(void)
{
	event_t event;

	while (m_queue.pop_event(event)) {
		process_event(event);
	}
}

bool Logic::call_ui(const app_t &app)
{
	UI::UIBackend ui;

	if (ui.call_popup(app)) { // If calling popup or app_controll service will fail,
		// do not remove application, and ask about it once again later
		LogDebug("Popup shown correctly. Application will be removed from DB and buffer");
		return true;
	}

	LogDebug("Popup error. Application will be marked to show popup later.");
	return false;
}

bool Logic::process_app(app_t &app)
{
	// Check if app hasn't already been verified.
	// If yes then just try to display popup once again, and go the next app.
#if POPUP
	if (app.verified == app_t::verified_t::NO) {
		LogDebug(app.str() << " has been verified before. Popup should be shown.");
		return call_ui(app);
	}

#endif
	Certs::ocsp_response_t ret;
	ret = m_certs.check_ocsp(app);

	// If OCSP returns success or OCSP checking fails we should remove application from buffer and database
	if (ret == Certs::ocsp_response_t::OCSP_APP_OK ||
			ret == Certs::ocsp_response_t::OCSP_CERT_ERROR) {
		LogDebug(app.str() << " OCSP verified (or not available for app's chains)");
		return true;
	} else if (ret == Certs::ocsp_response_t::OCSP_APP_REVOKED) {
		LogDebug(app.str() << " certificate has been revoked. Popup should be shown");
		app.verified = app_t::verified_t::NO;
#if POPUP
		// Do not remove app here - just waits for user answer from popup
		return call_ui(app);
#else
		return true;
#endif
	} else {
		LogDebug(app.str() << " should be checked again later");
		// If check_ocsp returns Certs::ocsp_response_t::OCSP_CHECK_AGAIN
		// app should be checked again later
	}

	return false;
}

void Logic::process_buffer(void)
{
	for (auto iter = m_buffer.begin(); iter != m_buffer.end();) {
		bool remove = process_app(*iter);
		auto prev = *iter;
		iter++;

		if (remove)
			remove_app_from_buffer_and_database(prev);

		app_processed();
	}
}

void Logic::push_event(event_t event)
{
	std::lock_guard<std::mutex> lock(m_mutex_cv);
	m_queue.push_event(std::move(event));
	LogDebug("Notify thread : pkgmgr event added");
	m_to_process.notify_one();
}

void Logic::process_all()
{
	LogInfo("[thread] Start to process event.");

	for (;;) {
		std::unique_lock<std::mutex> lock(m_mutex_cv);

		// Wait condition.
		if (m_queue.empty() && !m_is_online_enabled) {
			LogDebug("[thread] wait condition <queue, Network> : "
					 << !m_queue.empty() << ", " << get_online());
			m_to_process.wait(lock); // spurious wakeups do not concern us
			LogDebug("[thread] wake up! running stage");
			m_is_first_run = true;
		}

		// Value for prevent infinite loop.
		m_is_online_enabled = false;
		// Move event data from queue to buffer & database.
		process_queue();
		lock.unlock();

		if (get_online() && !m_buffer.empty()) {
			process_buffer();

			// This is for OCSP_CHECK_AGAIN case.
			if (m_buffer.empty()) {
				LogInfo("[thread] Finish processing event.");
				g_main_loop_quit(m_loop);
				break;
			} else {
				LogDebug("[thread] Check again : " << m_buffer.size());
				// Timer running periodically
				timerStart(TIMEOUT_TIMER);
			}
		} else if (!get_online()) {
			LogDebug("[thread] No network. Buffer won't be processed.");
		} else {
			LogDebug("[thread] No event since cert-checker started.");
		}

		if (m_should_exit)
			break;
	}
}

void Logic::job(void)
{
	std::lock_guard<std::mutex> lock(m_mutex_cv);

	if (m_buffer.empty()) {
		LogDebug("[timer] Buffer is empty.");
		timerStop();
	} else {
		LogDebug("[timer] Notify thread - periodic wakeup");
		m_to_process.notify_one();
	}
}

void Logic::process_event(const event_t &event)
{
	LogDebug("Move event from queue to (buffer and db).");

	if (event.event_type == event_t::event_type_t::APP_INSTALL) {
		// pulling out certificates from signatures
		app_t app = event.app;
		m_certs.get_certificates(app);
		add_app_to_buffer_and_database(app);
	} else if (event.event_type == event_t::event_type_t::APP_UNINSTALL) {
		remove_app_from_buffer_and_database(event.app);
	} else {
		LogError("Unknown event type");
	}
}

void Logic::add_app_to_buffer_and_database(const app_t &app)
{
	// First add app to DB
	if (!m_sqlquery->add_app_to_check_list(app)) {
		LogError("Failed to add " << app.str() << "to database");
		// We can do nothing about it. We can only log the error.
	}

	// Then add app to buffer - skip if already added.
	// FIXME: What to do if the same app will be installed twice?
	//        Add it twice to the buffer, or check if apps in buffer are unique?
	//        At the moment doubled apps are skipped.
	for (auto &iter : m_buffer) {
		if (iter.app_id == app.app_id &&
				iter.pkg_id == app.pkg_id &&
				iter.uid == app.uid) {
			LogDebug(app.str() << " already in buffer. Skip.");
			return;
		}
	}

	// Then add app to buffer
	m_buffer.push_back(app);
}

// Notice that this operator doesn't compare list of certificate, because it isn't needed here.
// This operator is implemented only for using in m_buffer.remove() method;
// Operator which compares certificates is implemented in tests.
bool operator ==(const app_t &app1, const app_t &app2)
{
	return app1.app_id == app2.app_id &&
		   app1.pkg_id == app2.pkg_id &&
		   app1.uid == app2.uid;
}

void Logic::remove_app_from_buffer_and_database(const app_t &app)
{
	// First remove app from DB
	m_sqlquery->remove_app_from_check_list(app);
	// Then remove app from buffer
	m_buffer.remove(app);
}

bool Logic::get_should_exit(void) const
{
	return m_should_exit;
}

void Logic::set_should_exit(void)
{
	m_should_exit = true;
}

std::atomic<bool> Logic::m_is_first_run(false);

bool Logic::is_running()
{
	return g_main_loop_is_running(m_loop);
}

gboolean Logic::timeout_cb(gpointer data)
{
	if (!m_is_first_run) {
		LogDebug("No event Since cchecker launched. timeout.");
		g_main_loop_quit(static_cast<GMainLoop *>(data));
	}

	return FALSE;
}

} // namespace CCHECKER
