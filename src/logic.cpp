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

#include <gio/gio.h>
#include <package_manager.h>

#include <logic.h>
#include <log.h>

namespace {

const char * eventTypeStr(package_manager_event_type_e type) {
    if (type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL)
        return "PACKAGE_MANAGER_EVENT_TYPE_INSTALL";
    if (type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL)
        return "PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL";
    if (type == PACKAGE_MANAGER_EVENT_TYPE_UPDATE)
        return "PACKAGE_MANAGER_EVENT_TYPE_UPDATE";
    return "UNKNOWN";
}

const char * eventStateStr(package_manager_event_state_e type) {
    if (type == PACKAGE_MANAGER_EVENT_STATE_STARTED)
        return "PACKAGE_MANAGER_EVENT_STATE_STARTED";
    if (type == PACKAGE_MANAGER_EVENT_STATE_PROCESSING)
        return "PACKAGE_MANAGER_EVENT_STATE_PROCESSING";
    if (type == PACKAGE_MANAGER_EVENT_STATE_COMPLETED)
        return "PACKAGE_MANAGER_EVENT_STATE_COMPLETED";
    if (type == PACKAGE_MANAGER_EVENT_STATE_FAILED)
        return "PACKAGE_MANAGER_EVENT_STATE_FAILED";
    return "UNKNOWN";
}
} //anonymus


namespace CCHECKER {
namespace LOGIC {

Logic::~Logic(void)
{
    LogDebug("Cert-checker cleaning.");
    if (m_proxy)
        g_object_unref(m_proxy);
    package_manager_destroy(m_request);
}

Logic::Logic(void) :
        m_is_online(false),
        m_proxy(NULL)
{}

int Logic::setup()
{
    // Add pkgmgr callback
    package_manager_create(&m_request);

    LogDebug("register installedApp event callback start");
    if (0 != package_manager_set_event_cb(m_request, LOGIC::Logic::pkgManager_callback, this)) {
        LogError("Error in package_manager_set_event_cb");
        return REGISTER_CALLBACK_ERROR;
    }
    LogDebug("register installedApp event callback success");

    LogDebug("register connman event callback start");
    if (register_connman_signal_handler() != NO_ERROR) {
        LogError("Error in register_connman_signal_handler");
        return REGISTER_CALLBACK_ERROR;;
    }
    LogDebug("register connman event callback success");

    return load_database_to_buffer();
}

ERRORS Logic::register_connman_signal_handler(void)
{
    GError *error = NULL;
    GDBusProxyFlags flags = G_DBUS_PROXY_FLAGS_NONE;

    // Obtain a connection to the System Bus
    m_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
            flags,
            NULL, /* GDBusInterfaceInfo */
            "net.connman",
            "/",
            "net.connman.Manager",
            NULL, /* GCancellable */
            &error);

    if (m_proxy == NULL) {
        LogError("Error creating D-Bus proxy: " << error->message);
        g_error_free (error);
        return DBUS_ERROR;
    }

    // Connect to g-signal to receive signals from proxy
    g_signal_connect (m_proxy, "g-signal", G_CALLBACK (Logic::connman_callback), this);

    return NO_ERROR;
}

void Logic::pkgManager_callback(
        const char *type,
        const char *package,
        package_manager_event_type_e eventType,
        package_manager_event_state_e eventState,
        int progress,
        package_manager_error_e error,
        void *logic_ptr)
{
    LogDebug("---- packageInstalledEventCallback ----\n");
    LogDebug("Type: " << type << ", package: " << package << ", Event type: " <<
            eventTypeStr(eventType) << ", Event state: " << eventStateStr(eventState) <<
            ", progress: " << progress <<", error: " << error);
    (void) type;
    (void) progress;
    Logic *logic = static_cast<Logic*>(logic_ptr);

    if (eventType != PACKAGE_MANAGER_EVENT_TYPE_INSTALL||
    eventState != PACKAGE_MANAGER_EVENT_STATE_COMPLETED ||
    error != PACKAGE_MANAGER_ERROR_NONE ||
    package == NULL) {
        LogDebug("PackageInstalled Callback error or Invalid Param");
    } else {
        LogDebug("PackageInstalled Callback. Instalation of: " << package <<
                ", error: " << error << ", progress: " << progress);
        // TODO: Add event to queue here
        (void) logic;
    }
}

void Logic::connman_callback(GDBusProxy *proxy,
                             gchar      *sender_name,
                             gchar      *signal_name,
                             GVariant   *parameters,
                             void *logic_ptr)
{
    (void) proxy;
    (void) sender_name;

    std::string signal_name_str = std::string(signal_name);
    if (signal_name_str != "PropertyChanged") {
        // Invalid param. Nothing to do here.
        return;
    }

    gchar *parameters_g = g_variant_print(parameters, TRUE);
    std::string params_str = std::string(parameters_g);
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

void Logic::check_OCSP(app_t &app)
{
    (void)app;
}

void Logic::add_OCSP_url(const std::string &issuer, const std::string &url)
{
    (void)issuer;
    (void)url;
}

void Logic::pkgmanager_uninstall(const app_t &app)
{
    (void)app;
}

void Logic::get_certs_from_signature(const std::string &signature, std::vector<std::string> &cert)
{
    (void)signature;
    (void)cert;
}

ERRORS Logic::load_database_to_buffer()
{
    return ERRORS::NO_ERROR;
}

} //LOGIC
} //CCHECKER
