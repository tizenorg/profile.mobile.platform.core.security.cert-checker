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

const char *const DB_PATH = tzplatform_mkpath(TZ_SYS_DB, ".cert-checker.db");

Logic::~Logic(void)
{
    LogDebug("Cert-checker cleaning.");
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
        m_is_online(false),
        m_proxy_connman(NULL),
        m_proxy_pkgmgr_install(NULL),
        m_proxy_pkgmgr_uninstall(NULL)
{}

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
    // Check if DB exists and create a new one if it doesn't
    error_t err = setup_db();
    if (err != NO_ERROR) {
        LogError("Database error");
        return err;
    }

    // FIXME: pkgmanager API signal handling was temporarily replaced
    //        by dbus API

    // Add pkgmgr install callback
    LogDebug("register pkgmgr install event callback start");
    if (register_pkgmgr_install_signal_handler() != NO_ERROR) {
        LogError("Error in register_pkgmgr_install_signal_handler");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register pkgmgr install event callback success");

    // Add pkgmgr uninstall callback
    LogDebug("register pkgmgr uninstall event callback start");
    if (register_pkgmgr_uninstall_signal_handler() != NO_ERROR) {
        LogError("Error in register_pkgmgr_uninstall_signal_handler");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register pkgmgr uninstall event callback success");

    // Add connman callback
    LogDebug("register connman event callback start");
    if (register_connman_signal_handler() != NO_ERROR) {
        LogError("Error in register_connman_signal_handler");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register connman event callback success");

    load_database_to_buffer();

    return NO_ERROR;
}

error_t Logic::register_pkgmgr_install_signal_handler(void)
{
    GError *error = NULL;
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;

    // Obtain a connection to the System Bus
    m_proxy_pkgmgr_install = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
            flags,
            NULL, /* GDBusInterfaceInfo */
            "org.tizen.slp.pkgmgr_status",
            "/org/tizen/slp/pkgmgr/install",
            "org.tizen.slp.pkgmgr.install",
            NULL, /* GCancellable */
            &error);

    if (m_proxy_pkgmgr_install == NULL) {
        if (error) {
            LogError("Error creating D-Bus proxy: " << error->message);
            g_error_free (error);
        }
        else {
            LogError("Error creating D-Bus proxy. Unknown error");
        }
        return DBUS_ERROR;
    }

    // Connect to g-signal to receive signals from proxy
    if (g_signal_connect (m_proxy_pkgmgr_install, "g-signal", G_CALLBACK (Logic::pkgmgr_install_callback), this) < 1) {
        LogError("g_signal_connect error while connecting pkgmgr install signal");
        return REGISTER_CALLBACK_ERROR;
    }

    return NO_ERROR;
}

error_t Logic::register_pkgmgr_uninstall_signal_handler(void)
{
    GError *error = NULL;
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;

    // Obtain a connection to the System Bus
    m_proxy_pkgmgr_uninstall = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
            flags,
            NULL, /* GDBusInterfaceInfo */
            "org.tizen.slp.pkgmgr_status",
            "/org/tizen/slp/pkgmgr/uninstall",
            "org.tizen.slp.pkgmgr.uninstall",
            NULL, /* GCancellable */
            &error);

    if (m_proxy_pkgmgr_install == NULL) {
        if (error) {
            LogError("Error creating D-Bus proxy: " << error->message);
            g_error_free (error);
        }
        else {
            LogError("Error creating D-Bus proxy. Unknown error");
        }
        return DBUS_ERROR;
    }

    // Connect to g-signal to receive signals from proxy
    if (g_signal_connect (m_proxy_pkgmgr_uninstall, "g-signal", G_CALLBACK (Logic::pkgmgr_uninstall_callback), this) < 1) {
        LogError("g_signal_connect error while connecting pkgmgr uninstall signal");
        return REGISTER_CALLBACK_ERROR;
    }

    return NO_ERROR;
}

error_t Logic::register_connman_signal_handler(void)
{
    GError *error = NULL;
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;

    // Obtain a connection to the System Bus
    m_proxy_connman = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
            flags,
            NULL, /* GDBusInterfaceInfo */
            "net.connman",
            "/",
            "net.connman.Manager",
            NULL, /* GCancellable */
            &error);

    if (m_proxy_connman == NULL) {
        if (error) {
            LogError("Error creating D-Bus proxy: " << error->message);
            g_error_free (error);
        }
        else {
            LogError("Error creating D-Bus proxy. Unknown error");
        }
        return DBUS_ERROR;
    }

    // Connect to g-signal to receive signals from proxy
    if (g_signal_connect (m_proxy_connman, "g-signal", G_CALLBACK (Logic::connman_callback), this) < 1) {
        LogError("g_signal_connect error while connecting connman signal");
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
    (void) logic;

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
    (void) logic;

    logic->pkgmgr_callback_internal(parameters, EVENT_UNINSTALL);
}

void Logic::pkgmgr_callback_internal(GVariant       *parameters,
                                     pkgmgr_event_t event)
{
    gchar *parameters_g = g_variant_print(parameters, TRUE);
    std::string params_str = std::string(parameters_g);
    LogDebug("params: " << params_str);
    g_free (parameters_g);

    //Check if numbers of children fits - should be 6
    int num = g_variant_n_children(parameters);
    if (num != 6) {
        LogError("Wrong number of children in g_variant: " << num << ", but should be 6.");
        return;
    }

    /* Message format:
     * uint32 5001
     * string "/usr/share/widget_demo/mancala.wgt_-427832739"
     * string "wgt"
     * string "yKrWwxz1KX"
     * string "end"
     * string "ok"
     */
    guint32 uid;
    gchar *pkgid = NULL;
    gchar *state = NULL;
    gchar *status = NULL;

    uid = g_variant_get_uint32(g_variant_get_child_value(parameters, 0));
    pkgid = g_variant_dup_string(g_variant_get_child_value(parameters, 3), NULL);
    state = g_variant_dup_string(g_variant_get_child_value(parameters, 4), NULL);
    status = g_variant_dup_string(g_variant_get_child_value(parameters, 5), NULL);

    if (std::string(state) == "end" && std::string(status) == "ok") {
        if (event == EVENT_INSTALL) {
            LogDebug("Install: uid : " << uid << ", pkgid: " << pkgid <<
                    ", state: " << state << ", status: " << status);
        }
        else if (event == EVENT_UNINSTALL) {
            LogDebug("Uninstall: uid : " << uid << ", pkgid: " << pkgid <<
                    ", state: " << state << ", status: " << status);
        }
    }
    else
        LogError("Wrong state (" << std::string(state) << ") or status (" << std::string(status) << ")");

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
        logic->m_is_online = true;
    }
    else if (params_str == "('State', <'offline'>)") {
        LogDebug("Device offline");
        logic->m_is_online = false;
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

} //CCHECKER
