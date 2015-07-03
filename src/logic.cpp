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
 * @version     1.0
 * @brief       This file is the implementation of SQL queries
 */
#include <stdexcept>
#include <tzplatform_config.h>

#include <cchecker/logic.h>
#include <cchecker/log.h>
#include <cchecker/sql_query.h>

using namespace std;

namespace CCHECKER {

namespace {
const char *const DB_PATH = tzplatform_mkpath(TZ_SYS_DB, ".cert-checker.db");
}

Logic::~Logic(void)
{
    LogDebug("Cert-checker cleaning.");

    // wait and join processing thread
    if (m_thread.joinable()) {
        LogDebug("Waiting for join processing thread");
        set_should_exit();
        m_to_process.notify_one();
        m_thread.join();
        LogDebug("Processing thread joined");
    }
    else
        LogDebug("No thread to join");

    if (m_proxy_connman)
        g_object_unref(m_proxy_connman);
    if (m_proxy_pkgmgr_install)
        g_object_unref(m_proxy_pkgmgr_install);
    if (m_proxy_pkgmgr_uninstall)
        g_object_unref(m_proxy_pkgmgr_uninstall);
    delete m_sqlquery;
}

Logic::Logic(void) :
        m_sqlquery(NULL),
        m_was_setup_called(false),
        m_is_online(true),
        m_should_exit(false),
        m_proxy_connman(NULL),
        m_proxy_pkgmgr_install(NULL),
        m_proxy_pkgmgr_uninstall(NULL)
{}

bool Logic::get_online() const
{
    return m_is_online;
}

void Logic::set_online(bool online)
{
    m_is_online = online;
}

error_t Logic::setup_db()
{
    // TODO: If database doesn't exist -should we create a new one?
    Try {
        m_sqlquery = new DB::SqlQuery(DB_PATH);
    } Catch (runtime_error) {
        LogError("Error while creating SqlQuery object");
        return DATABASE_ERROR;
    }

    if(!m_sqlquery) {
        LogError("Cannot open database");
        return DATABASE_ERROR;
    }

    return NO_ERROR;
}

error_t  Logic::setup()
{
    // Check if setup was called
    if (m_was_setup_called) {
        LogError("You can call setup only once");
        return INTERNAL_ERROR;
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

    // FIXME: pkgmanager API signal handling was temporarily replaced
    //        by dbus API

    // Add pkgmgr install callback
    LogDebug("register pkgmgr install event callback start");
    if (register_dbus_signal_handler(m_proxy_pkgmgr_install,
            "org.tizen.slp.pkgmgr_status",
            "/org/tizen/slp/pkgmgr/install",
            "org.tizen.slp.pkgmgr.install",
            pkgmgr_install_callback) != NO_ERROR) {
        LogError("Error in register_pkgmgr_install_signal_handler");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register pkgmgr install event callback success");

    // Add pkgmgr uninstall callback
    LogDebug("register pkgmgr uninstall event callback start");
    if (register_dbus_signal_handler(m_proxy_pkgmgr_uninstall,
            "org.tizen.slp.pkgmgr_status",
            "/org/tizen/slp/pkgmgr/uninstall",
            "org.tizen.slp.pkgmgr.uninstall",
            pkgmgr_uninstall_callback) != NO_ERROR) {
        LogError("Error in register_pkgmgr_uninstall_signal_handler");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register pkgmgr uninstall event callback success");

    // Add connman callback
    LogDebug("register connman event callback start");
    if (register_dbus_signal_handler(m_proxy_connman,
            "net.connman",
            "/",
            "net.connman.Manager",
            connman_callback) != NO_ERROR) {
        LogError("Error in register_connman_signal_handler");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register connman event callback success");

    return NO_ERROR;
}

error_t Logic::register_dbus_signal_handler(GDBusProxy *proxy,
        const char *name,
        const char *object_path,
        const char *interface_name,
        void (*callback) (GDBusProxy *proxy,
                          gchar      *sender_name,
                          gchar      *signal_name,
                          GVariant   *parameters,
                          void *logic_ptr)
        )
{
    GError *error = NULL;
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;

    // Obtain a connection to the System Bus
    proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
            flags,
            NULL, /* GDBusInterfaceInfo */
            name,
            object_path,
            interface_name,
            NULL, /* GCancellable */
            &error);

    if (proxy == NULL) {
        if (error) {
            LogError("Error creating D-Bus proxy for /'" << interface_name <<"/': " << error->message);
            g_error_free(error);
        } else {
            LogError("Error creating D-Bus proxy for /'" << interface_name <<"/'. Unknown error");
        }
        return DBUS_ERROR;
    }

    // Connect to g-signal to receive signals from proxy
    if (g_signal_connect(proxy, "g-signal", G_CALLBACK(callback), this) < 1) {
        LogError("g_signal_connect error while connecting " << interface_name);
        return REGISTER_CALLBACK_ERROR;
    }

    return NO_ERROR;
}

// FIXME: pkgmgr callback doesn't receive signals with successful installation/uninstallation.
//        For now it will be replaced by low-level signal handling from DBUS.
void Logic::pkgmgr_install_callback(GDBusProxy */*proxy*/,
                                    gchar      */*sender_name*/,
                                    gchar      */*signal_name*/,
                                    GVariant   *parameters,
                                    void       *logic_ptr)
{
    LogDebug("----------------- pkgmgr_install_callback -----------------\n");

    Logic *logic = static_cast<Logic*> (logic_ptr);
    logic->pkgmgr_callback_internal(parameters, EVENT_INSTALL);
}

void Logic::pkgmgr_uninstall_callback(GDBusProxy */*proxy*/,
                                      gchar      */*sender_name*/,
                                      gchar      */*signal_name*/,
                                      GVariant   *parameters,
                                      void       *logic_ptr)
{
    LogDebug("----------------- pkgmgr_uninstall_callback -----------------\n");

    Logic *logic = static_cast<Logic*> (logic_ptr);
    logic->pkgmgr_callback_internal(parameters, EVENT_UNINSTALL);
}

void Logic::pkgmgr_callback_internal(GVariant       *parameters,
                                     pkgmgr_event_t event)
{
    gchar *parameters_g = g_variant_print(parameters, TRUE);
    std::string params_str = std::string(parameters_g);
    LogDebug("params: " << params_str);
    g_free (parameters_g);

    /* DBus message format from pkgmgr:
     * uint32 5001
     * string "/usr/share/widget_demo/mancala.wgt_-427832739"
     * string "wgt"
     * string "yKrWwxz1KX"
     * string "end"
     * string "ok"
     *
     * Check if numbers of children (vaules) fits - should be 6:
     */
    int num = g_variant_n_children(parameters);
    if (num != 6) {
        LogError("Wrong number of children in g_variant: " << num << ", but should be 6.");
        return;
    }

    guint32 uid;
    gchar *pkgid = NULL;
    gchar *state = NULL;
    gchar *status = NULL;

    uid = g_variant_get_uint32(g_variant_get_child_value(parameters, 0));
    pkgid = g_variant_dup_string(g_variant_get_child_value(parameters, 3), NULL);
    state = g_variant_dup_string(g_variant_get_child_value(parameters, 4), NULL);
    status = g_variant_dup_string(g_variant_get_child_value(parameters, 5), NULL);

    // FIXME: No information about app_id in the signal. Use stub.
    app_t app(TEMP_APP_ID, pkgid, uid, {});

    if (std::string(state) == "end" && std::string(status) == "ok") {
        if (event == EVENT_INSTALL) {
            LogDebug("Install: uid : " << uid << ", pkgid: " << pkgid <<
                    ", state: " << state << ", status: " << status);
            m_queue.push_event(event_t(app, event_t::event_type_t::APP_INSTALL));
        }
        else if (event == EVENT_UNINSTALL) {
            LogDebug("Uninstall: uid : " << uid << ", pkgid: " << pkgid <<
                    ", state: " << state << ", status: " << status);
            m_queue.push_event(event_t(app, event_t::event_type_t::APP_UNINSTALL));
        }

        m_to_process.notify_one();
    }
    else
        LogDebug("Wrong state (" << std::string(state) << ") or status (" << std::string(status) << ")");

    g_free(pkgid);
    g_free(state);
    g_free(status);
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
    g_free (parameters_g);

    Logic *logic = static_cast<Logic*> (logic_ptr);

    if (params_str == "('State', <'online'>)") {
        LogDebug("Device online");
        logic->set_online(true);

        logic->m_to_process.notify_one();
    }
    else if (params_str == "('State', <'offline'>)") {
        LogDebug("Device offline");
        logic->set_online(false);
    }
}

void Logic::check_ocsp(app_t &app)
{
    (void)app;
}

void Logic::add_ocsp_url(const string &issuer, const string &url, int64_t date)
{
    m_sqlquery->set_url(issuer, url, date);
}

void Logic::pkgmanager_uninstall(const app_t &app)
{
    (void)app;
}

void Logic::get_certs_from_signature(const string &signature, vector<string> &cert)
{
    (void)signature;
    (void)cert;
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
    while(m_queue.pop_event(event)) {
        process_event(event);
    }
}

error_t Logic::process_buffer(void)
{
    for (auto iter = m_buffer.begin(); iter != m_buffer.end();) {
        // If OCSP checking fails we should remove application from buffer and database
        Certs::ocsp_response_t ret;
        ret = m_certs.check_ocsp(*iter);
        if (ret == Certs::ocsp_response_t::OCSP_APP_OK ||
                ret == Certs::ocsp_response_t::OCSP_CERT_ERROR) {
            LogDebug(iter->str() << " OCSP verified (or not available for app's chains)");
            app_t app_cpy = *iter;
            iter++;
            remove_app_from_buffer_and_database(app_cpy);
        }
        else if (ret == Certs::ocsp_response_t::OCSP_APP_REVOKED) {
            LogDebug(iter->str() << " certificate has been revoked. Popup should be shown");
            app_t app_cpy = *iter;
            iter++;
            remove_app_from_buffer_and_database(app_cpy);
            call_popup(app_cpy);
        }
        else {
            LogDebug(iter->str() << " should be checked again later");
            // If check_ocsp returns Certs::ocsp_response_t::OCSP_CHECK_AGAIN
            // app should be checked again later
            iter++;
        }
    }
    return NO_ERROR;
}

void Logic::process_all()
{
    //Check if should't exit
    while (!get_should_exit()) {
        std::unique_lock<std::mutex> lock(m_mutex_cv);

        if (m_queue.empty()) {
            LogDebug("Processing thread: waiting on condition");
            m_to_process.wait(lock);
        }
        else
            LogDebug("Processing thread: More events in queue - processing again");

        LogDebug("Processing thread: running");

        process_queue();
        if (get_online())
            process_buffer();
        else
            LogDebug("No network. Buffer won't be processed");

        LogDebug("Processing done");
    }

    // should process queue just before exit
    process_queue();
}

void Logic::process_event(const event_t &event)
{
    if (event.event_type == event_t::event_type_t::APP_INSTALL) {
        // pulling out certificates from signatures
        app_t app = event.app;
        ocsp_urls_t ocsp_urls;
        m_certs.get_certificates(app, ocsp_urls);
        add_app_to_buffer_and_database(app);

        // Adding OCSP URLs - if found any
        if (!ocsp_urls.empty()){
            LogDebug("Some OCSP url has been found. Adding to database");
            for (auto iter = ocsp_urls.begin(); iter != ocsp_urls.end(); iter++){
                m_sqlquery->set_url(iter->issuer, iter->url, iter->date);
            }
        }
    }
    else if (event.event_type == event_t::event_type_t::APP_UNINSTALL) {
        remove_app_from_buffer_and_database(event.app);
    }
    else
        LogError("Unknown event type");
}

void Logic::add_app_to_buffer_and_database(const app_t &app)
{
    // First add app to DB
    if(!m_sqlquery->add_app_to_check_list(app)) {
        LogError("Failed to add " << app.str() << "to database");
        // We can do nothing about it. We can only log the error.
    }

    // Then add app to buffer
    m_buffer.push_back(app);
}

void Logic::remove_app_from_buffer_and_database(const app_t &app)
{
    // First remove app from DB
    m_sqlquery->remove_app_from_check_list(app);

    // Then remove app from buffer
    for (auto iter = m_buffer.begin(); iter != m_buffer.end(); ++iter) {
        if (iter->app_id == app.app_id &&
                iter->pkg_id == app.pkg_id &&
                iter->uid == app.uid) {
            LogDebug(iter->str() << " found in buffer - will be removed");
            m_buffer.erase(iter);
            break;
        }
    }
}

bool Logic::get_should_exit(void) const
{
    return m_should_exit;
}

void Logic::set_should_exit(void)
{
    m_should_exit = true;

}

} //CCHECKER
