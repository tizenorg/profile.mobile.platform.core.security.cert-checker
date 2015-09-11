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

#include <cchecker/app.h>
#include <cchecker/certs.h>
#include <cchecker/queue.h>

namespace CCHECKER {

namespace DB {
class SqlQuery;
}

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

class Logic {
    public:
        Logic(void);
        virtual ~Logic(void);
        error_t  setup(void);
        virtual void clean(void);

        static void pkgmgr_install_callback(GDBusProxy *proxy,
                gchar      *sender_name,
                gchar      *signal_name,
                GVariant   *parameters,
                void *logic_ptr);
        static void pkgmgr_uninstall_callback(GDBusProxy *proxy,
                gchar      *sender_name,
                gchar      *signal_name,
                GVariant   *parameters,
                void *logic_ptr);
        static void connman_callback(GDBusProxy *proxy,
                gchar      *sender_name,
                gchar      *signal_name,
                GVariant   *parameters,
                void *logic_ptr);


    protected:
        error_t setup_db();
        void load_database_to_buffer();

        void add_ocsp_url(const std::string &issuer, const std::string &url, int64_t date);
        void add_app_to_buffer_and_database(const app_t &app);
        void remove_app_from_buffer_and_database(const app_t &app);

        void set_connman_online_state();
        void pkgmgr_callback_internal(GVariant *parameters, pkgmgr_event_t event);
        error_t register_dbus_signal_handler(GDBusProxy **proxy,
                const char *name,
                const char *object_path,
                const char *interface_name,
                void (*callback) (GDBusProxy *proxy,
                        gchar      *sender_name,
                        gchar      *signal_name,
                        GVariant   *parameters,
                        void *logic_ptr)
                );

        void push_event(event_t event);

        void process_all(void);
        void process_queue(void);
        virtual void process_event(const event_t &event);

        bool process_app(app_t& app);
        void process_buffer(void);
        virtual void app_processed() {}; // for tests

        bool get_online(void) const;
        void set_online(bool online);

        bool get_should_exit(void) const;
        void set_should_exit(void);

        bool call_ui(const app_t &app);

        Queue m_queue;
        Certs m_certs;
        std::list<app_t> m_buffer;
        DB::SqlQuery *m_sqlquery;
        bool m_was_setup_called;

        bool m_is_online;
        // TODO: use m_queue for online events
        bool m_is_online_enabled;
        std::condition_variable m_to_process;
        std::mutex m_mutex_cv;
        std::thread m_thread;
        bool m_should_exit;

        GDBusProxy *m_proxy_connman;
        GDBusProxy *m_proxy_pkgmgr_install;
        GDBusProxy *m_proxy_pkgmgr_uninstall;
};

} // CCHECKER

#endif //CCHECKER_LOGIC_H
