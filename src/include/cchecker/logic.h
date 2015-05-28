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

#include <gio/gio.h>
#include <package_manager.h>
#include <string>
#include <vector>
#include <list>

#include <cchecker/app.h>
#include <cchecker/sql_query.h>

namespace CCHECKER {

enum error_t {
    NO_ERROR,
    REGISTER_CALLBACK_ERROR,
    DBUS_ERROR,
    PACKAGE_MANAGER_ERROR,
    DATABASE_ERROR
};

class Logic {
    public:
        Logic(void);
        virtual ~Logic(void);
        error_t  setup();
        static void pkg_manager_callback(
                const char *type,
                const char *package,
                package_manager_event_type_e eventType,
                package_manager_event_state_e eventState,
                int progress,
                package_manager_error_e error,
                void *logic_ptr);
        static void connman_callback(GDBusProxy *proxy,
                gchar      *sender_name,
                gchar      *signal_name,
                GVariant   *parameters,
                void *logic_ptr);

    private:
        //TODO: implement missing members

        error_t setup_db();
        void check_ocsp(app_t &app);
        void add_ocsp_url(const std::string &issuer, const std::string &url, int64_t date);
        void pkgmanager_uninstall(const app_t &app);
        void get_certs_from_signature(const std::string &signature, std::vector<std::string> &cert);
        void load_database_to_buffer();
        error_t register_connman_signal_handler(void);

        std::list<app_t> m_buffer;
        DB::SqlQuery *m_sqlquery;
        bool m_is_online;
        package_manager_h m_request;
        GDBusProxy *m_proxy;

};

} // CCHECKER

#endif //CCHECKER_LOGIC_H
