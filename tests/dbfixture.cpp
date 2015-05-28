/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        dbfixture.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Implementation of DB test class
 */
#include <tzplatform_config.h>

#include <dbfixture.h>

using namespace CCHECKER;

namespace {
const char *TEST_DB_PATH = tzplatform_mkpath(TZ_SYS_DB, ".cert-checker-test.db");

const char *DB_CMD_CLEAR_URL =
            "DELETE FROM ocsp_urls;";

const char *DB_CMD_CLEAR_TO_CHECK =
            "DELETE FROM to_check;";

const char *DB_CMD_CLEAR_CERTS_TO_CHECK =
            "DELETE FROM certs_to_check;";
} // anonymus namespace

DBFixture::DBFixture() :
    DB::SqlQuery(TEST_DB_PATH)
{};

DBFixture::~DBFixture()
{};

void DBFixture::clear_database ()
{
    // TODO: Restore DB from copy instead of removing all data from it.

    DB::SqlConnection::DataCommandAutoPtr getUrlCommand =
            m_connection->PrepareDataCommand(DB_CMD_CLEAR_URL);
    getUrlCommand->Step();

    getUrlCommand = m_connection->PrepareDataCommand(DB_CMD_CLEAR_CERTS_TO_CHECK);
    getUrlCommand->Step();

    getUrlCommand = m_connection->PrepareDataCommand(DB_CMD_CLEAR_TO_CHECK);
    getUrlCommand->Step();
}
