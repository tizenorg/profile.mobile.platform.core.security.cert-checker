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
#include <boost/test/unit_test.hpp>
#include <tzplatform_config.h>
#include <fstream>

#include <dbfixture.h>

using namespace CCHECKER;

namespace {
const char *TEST_DB_PATH      = DB_INSTALL_DIR "/.cert-checker-test.db" ;
const char *TEST_DB_PATH_TEMP = DB_INSTALL_DIR "/.cert-checker-test-temp.db";
} // anonymus namespace

DBFixture::DBFixture() :
        DB::SqlQuery()
{
    // Remove temporary databse
    BOOST_CHECK(unlink(TEST_DB_PATH_TEMP) == 0 || errno == ENOENT);

    // Restore original database
    std::ifstream f1(TEST_DB_PATH, std::fstream::binary);
    std::ofstream f2(TEST_DB_PATH_TEMP, std::fstream::trunc|std::fstream::binary);
    f2 << f1.rdbuf();
    f2.close();
    f1.close();

    // Open temporary database
    BOOST_REQUIRE(connect(TEST_DB_PATH_TEMP));
};

DBFixture::~DBFixture()
{};
