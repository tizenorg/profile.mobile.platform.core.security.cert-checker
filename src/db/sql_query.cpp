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
 * @file        sql_query.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       This file is the implementation of SQL queries
 */

#include <cchecker/dpl/db/sql_connection.h>
#include <cchecker/sql_query.h>
#include <cchecker/log.h>

namespace {

    // TODO: Make defines with these identifiers.
    // 101 - issuer
    // 102 - url
    // 103 - date
    // 104 - app_id
    // 105 - pkg_id
    // 106 - uid
    // 107 - check_id
    // 108 - certificate
    // 109 - verified
    // 110 - chain_id

    // setup
    const char *DB_CMD_SETUP = "VACUUM; PRAGMA foregin_keys=ON;";

    const char *DB_CMD_GET_LAST_INSERTED_ROW = "SELECT last_insert_rowid();";

    // urls
    const char *DB_CMD_GET_URL =
            "SELECT url, date FROM ocsp_urls WHERE issuer = ?101;";

    const char *DB_CMD_SET_URL =
            "INSERT INTO  ocsp_urls(issuer, url, date) VALUES(?101, ?102, ?103);";

    const char *DB_CMD_UPDATE_URL =
            "UPDATE ocsp_urls SET url=?102, date=?103 WHERE issuer=?101;"; // Issuer should be unique

    // apps
    const char *DB_CMD_ADD_APP =
            "INSERT INTO to_check(app_id, pkg_id, uid, verified) VALUES(?104, ?105, ?106, ?109);";

    const char *DB_CMD_GET_CHECK_ID =
            "SELECT check_id FROM to_check WHERE app_id=?104 AND pkg_id=?105 AND uid=?106;";

    const char *DB_CMD_ADD_CHAIN =
            "INSERT INTO chains_to_check(check_id) VALUES(?107);";

    const char *DB_CMD_ADD_CERT =
            "INSERT INTO certs_to_check(chain_id, certificate) VALUES(?110, ?108);";

    const char *DB_CMD_GET_CHAINS =
            "SELECT chain_id FROM chains_to_check WHERE check_id=?107;";

    const char *DB_CMD_REMOVE_APP =
            "DELETE FROM to_check WHERE app_id=?104 AND pkg_id=?105 AND uid=?106;";

    const char *DB_CMD_GET_APPS =
            "SELECT * FROM to_check";

    const char *DB_CMD_GET_CERTS =
            "SELECT certificate FROM certs_to_check WHERE chain_id=?110;";

    const char *DB_CMD_SET_APP_AS_VERIFIED =
            "UPDATE to_check SET verified=?109 WHERE check_id=?107";
}

namespace CCHECKER {
namespace DB {

SqlQuery::SqlQuery(const std::string& path)
{
    m_connection = NULL;
    m_inUserTransaction = false;

    if (!connect(path))
        throw std::runtime_error("Database error");
}

bool SqlQuery::connect(const std::string& path)
{
    if (m_connection != NULL) {
        LogError("Already connected!");
        return true;
    }

    Try {
        m_connection = new SqlConnection(path, SqlConnection::Flag::None, SqlConnection::Flag::Option::CRW);
        m_connection->ExecCommand(DB_CMD_SETUP);
        return true;
    } Catch(std::bad_alloc) {
        LogError("Couldn't allocate SqlConnection");
    } Catch(SqlConnection::Exception::ConnectionBroken) {
        LogError("Couldn't connect to database: " << path);
    } Catch(SqlConnection::Exception::InvalidColumn) {
        LogError("Couldn't set the key for database");
    } Catch(SqlConnection::Exception::SyntaxError) {
        LogError("Couldn't initiate the database");
    } Catch(SqlConnection::Exception::InternalError) {
        LogError("Couldn't create the database");
    }
    return false;
}

SqlQuery::~SqlQuery()
{
    delete m_connection;
}

bool SqlQuery::get_url(const std::string &issuer, std::string &url)
{
    SqlConnection::DataCommandAutoPtr getUrlCommand =
                    m_connection->PrepareDataCommand(DB_CMD_GET_URL);
    getUrlCommand->BindString(101, issuer.c_str());

    if (getUrlCommand->Step()) {
        url = getUrlCommand->GetColumnString(0);
        LogDebug("Url for " << issuer << " found in databse: " << url);
        return true;
    }

    LogDebug("No url for " << issuer << " in databse.");
    return false;
}

void SqlQuery::set_url(const std::string &issuer, const std::string &url, const int64_t &date)
{
    m_connection->BeginTransaction();
    SqlConnection::DataCommandAutoPtr getUrlCommand =
                    m_connection->PrepareDataCommand(DB_CMD_GET_URL);
    getUrlCommand->BindString(101, issuer.c_str());

    if (getUrlCommand->Step()) { // This means that url already exists in database for this issuer
                                 // There's need to check the date
        LogDebug("Url for " << issuer << " already exists. Checking the date");
        int64_t db_date = getUrlCommand->GetColumnInt64(1);
        if (db_date < date) {
            LogDebug("Url for " << issuer << " in database is older. Update is needed");
            // Url in DB is older - update is needed
            SqlConnection::DataCommandAutoPtr updateUrlCommand =
                            m_connection->PrepareDataCommand(DB_CMD_UPDATE_URL);
            updateUrlCommand->BindString(101, issuer.c_str());
            updateUrlCommand->BindString(102, url.c_str());
            updateUrlCommand->BindInt64(103, date);
            updateUrlCommand->Step();
        } else // Url in DB is up-to-date, no need for update
            LogDebug("Url for " << issuer << " in databse is up-to-date. No update needed");

    } else { // No url in database for this issuer, add the new one
        LogDebug("No url for "<< issuer << " in databse. Adding the new one.");
        SqlConnection::DataCommandAutoPtr setUrlCommand =
                        m_connection->PrepareDataCommand(DB_CMD_SET_URL);
        setUrlCommand->BindString(101, issuer.c_str());
        setUrlCommand->BindString(102, url.c_str());
        setUrlCommand->BindInt64(103, date);
        setUrlCommand->Step();
    }
    m_connection->CommitTransaction();
}

bool SqlQuery::check_if_app_exists(const app_t &app)
{
    int32_t check_id;
    return get_check_id(app, check_id);
}

bool SqlQuery::get_check_id(const app_t &app, int32_t &check_id)
{
    SqlConnection::DataCommandAutoPtr getCheckIDCommand =
                    m_connection->PrepareDataCommand(DB_CMD_GET_CHECK_ID);
    getCheckIDCommand->BindString(104, app.app_id.c_str());
    getCheckIDCommand->BindString(105, app.pkg_id.c_str());
    getCheckIDCommand->BindInt64(106, app.uid);
    if (getCheckIDCommand->Step()) {
        check_id = getCheckIDCommand->GetColumnInt32(0);
        LogDebug("Found check id: " << check_id << ", for app: " << app.app_id);
        return true;
    }
    LogDebug("No check_id for app: " << app.app_id << " in database");
    return false;
}

bool SqlQuery::add_chain_id(const int32_t check_id, int32_t &chain_id)
{
    // Add new chain for an app
    SqlConnection::DataCommandAutoPtr addChainCommand =
            m_connection->PrepareDataCommand(DB_CMD_ADD_CHAIN);
    addChainCommand->BindInt32(107, check_id);
    addChainCommand->Step();

    // get chain_id
    SqlConnection::DataCommandAutoPtr getLastInserted =
            m_connection->PrepareDataCommand(DB_CMD_GET_LAST_INSERTED_ROW);
    if(getLastInserted->Step()) {
        chain_id = getLastInserted->GetColumnInt32(0);
        LogDebug("Found chain_id: " << chain_id << ", for check_id " << check_id);
        return true;
    }
    return false;
}

bool SqlQuery::add_app_to_check_list(const app_t &app)
{
    //Check if app exists in DB
    if (check_if_app_exists(app)) {
        LogDebug(app.str() << " already exists in database");
        return true;
    }

    m_connection->BeginTransaction();
    //Add app to to_check table
    SqlConnection::DataCommandAutoPtr addAppCommand =
            m_connection->PrepareDataCommand(DB_CMD_ADD_APP);
    addAppCommand->BindString(104, app.app_id.c_str());
    addAppCommand->BindString(105, app.pkg_id.c_str());
    addAppCommand->BindInt64(106, app.uid);
    addAppCommand->BindInt32(109, static_cast<int32_t>(app_t::verified_t::UNKNOWN));  // Set app as not-verified
    addAppCommand->Step();
    LogDebug("App " << app.app_id << " added to to_check table, adding certificates.");

    // Get check_id
    int32_t check_id;
    if (!get_check_id(app, check_id)) { // If get check_id failed return false;
        LogDebug("Failed while addind app "<< app.app_id << " to to_check table.");
        m_connection->RollbackTransaction();
        return false;
    }

    // If get check_id succeed we can add chain to database
    int32_t chain_id;
    for (const auto &iter : app.signatures) {
        // Add chain
        if (add_chain_id(check_id, chain_id)) {
            // add certificates from chain
            for (const auto &iter_cert : iter) {
                SqlConnection::DataCommandAutoPtr addCertCommand =
                        m_connection->PrepareDataCommand(DB_CMD_ADD_CERT);
                addCertCommand->BindInt32(110, chain_id);
                addCertCommand->BindString(108, iter_cert.c_str());
                addCertCommand->Step();
                LogDebug("Certificate for app " << app.app_id << "added");
            }
        } else {
            LogDebug("Failed to add certificates chain");
            m_connection->RollbackTransaction();
            return false;
        }

     }
     m_connection->CommitTransaction();
     return true;
}

void SqlQuery::remove_app_from_check_list(const app_t &app)
{
    LogDebug("Removing app: " << app.str());

    //Remove app from to_check table
    SqlConnection::DataCommandAutoPtr removeAppCommand =
            m_connection->PrepareDataCommand(DB_CMD_REMOVE_APP);
    removeAppCommand->BindString(104, app.app_id.c_str());
    removeAppCommand->BindString(105, app.pkg_id.c_str());
    removeAppCommand->BindInt32(106, app.uid);
    removeAppCommand->Step();
    LogDebug("Removed app: " << app.str());

    // Removing certificates should be done automatically by DB because of
    // ON DELETE CASCADE for check_id
}

void SqlQuery::mark_as_verified(const app_t &app, const app_t::verified_t &verified)
{
    int32_t check_id;

    if (get_check_id(app, check_id)) {
        SqlConnection::DataCommandAutoPtr setVerifiedCommand =
                        m_connection->PrepareDataCommand(DB_CMD_SET_APP_AS_VERIFIED);
        setVerifiedCommand->BindInt32(107, check_id);
        setVerifiedCommand->BindInt32(109, static_cast<int32_t>(verified));
        setVerifiedCommand->Step();
        LogDebug("App: " << app.str() << " marked as verified: " << static_cast<int32_t>(verified));
    }
}

void SqlQuery::get_apps(std::list<app_t> &apps_buffer)
{
    // This function will fill buffer with check_id, app_id, pkg_id, uid and verified fields -
    // it leaves certificates' list empty.
    SqlConnection::DataCommandAutoPtr getAppsCommand =
                   m_connection->PrepareDataCommand(DB_CMD_GET_APPS);

    while (getAppsCommand->Step()) {
        app_t app;
        app.check_id = getAppsCommand->GetColumnInt32(0);
        app.app_id   = getAppsCommand->GetColumnString(1);
        app.pkg_id   = getAppsCommand->GetColumnString(2);
        app.uid      = getAppsCommand->GetColumnInt64(3);
        app.verified = static_cast<app_t::verified_t>(getAppsCommand->GetColumnInt32(4));
        app.signatures = {};
        LogDebug("App read from DB: app_id: " << app.str() << ", verified: " << static_cast<int32_t>(app.verified));
        apps_buffer.push_back(app);
    }
}

void SqlQuery::get_app_list(std::list<app_t> &apps_buffer)
{
    get_apps(apps_buffer);

    m_connection->BeginTransaction();
    // Get chain for every application
    for (auto &iter_app : apps_buffer) {
        SqlConnection::DataCommandAutoPtr getChainsCommand =
                m_connection->PrepareDataCommand(DB_CMD_GET_CHAINS);
        getChainsCommand->BindInt32(107, iter_app.check_id);

        // Get all certs from chain
        while (getChainsCommand->Step()) {
            chain_t chain;
            int32_t chain_id;
            chain_id = getChainsCommand->GetColumnInt32(0);
            LogDebug("Found chain (" << chain_id << ") for " << iter_app.str());

            SqlConnection::DataCommandAutoPtr getCertsCommand =
                    m_connection->PrepareDataCommand(DB_CMD_GET_CERTS);
            getCertsCommand->BindInt32(110, chain_id);

            // Add found certs to chain
            while (getCertsCommand->Step()) {
                chain.push_back(getCertsCommand->GetColumnString(0));
                LogDebug("Found certificate (" << chain.size() << ") in chain no. " << chain_id);
            }
            iter_app.signatures.push_back(chain);
        }
    }
    m_connection->CommitTransaction();
}

} // DB
} // CCHECKER
