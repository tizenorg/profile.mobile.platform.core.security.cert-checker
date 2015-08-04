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
 * @file        test_db.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Database tests
 */

#define BOOST_TEST_MODULE CERT_CHECKER_TESTS
#include <boost/test/unit_test.hpp>
#include <string>

#include <cchecker/log.h>
#include <dbfixture.h>
#include <app_event_operators.h>

BOOST_FIXTURE_TEST_SUITE(DB_TEST, DBFixture)

BOOST_AUTO_TEST_CASE(DB_url) {
    std::string url;
    std::string url_org = "url://url";
    std::string url_update = "http://issuer";
    std::string url_org2 = "address";
    std::string url_update2 = "random_text";
    std::string url_org3 = "########";
    std::string url_update3 = "@@@";

    // No url in database
    BOOST_REQUIRE(get_url("Issuer_test1", url)==false);
    BOOST_REQUIRE(get_url("Issuer_test2", url)==false);
    BOOST_REQUIRE(get_url("Issuer_test3", url)==false);

    // Url should be added
    set_url("Issuer_test1", url_org, 500);
    BOOST_REQUIRE(get_url("Issuer_test1", url)==true);
    BOOST_REQUIRE(url==url_org);

    // Url for issuer 2 and 3 should remain empty
    BOOST_REQUIRE(get_url("Issuer_test2", url)==false);
    BOOST_REQUIRE(get_url("Issuer_test3", url)==false);

    // Should NOT be updated. Should get original url.
    set_url("Issuer_test1", url_update, 400);
    BOOST_REQUIRE(get_url("Issuer_test1", url)==true);
    BOOST_REQUIRE(url==url_org);

    // Should be updated. Should get updated url.
    set_url("Issuer_test1", url_update, 600);
    BOOST_REQUIRE(get_url("Issuer_test1", url)==true);
    BOOST_REQUIRE(url==url_update);

    // Add url for new issuer
    set_url("Issuer_test2", url_org2, 200);
    BOOST_REQUIRE(get_url("Issuer_test2", url)==true);
    BOOST_REQUIRE(url==url_org2);

    // Url for issuer 3 should remain empty
    BOOST_REQUIRE(get_url("Issuer_test3", url)==false);

    // Add url for issuer 3
    set_url("Issuer_test3", url_org3, 1000);
    BOOST_REQUIRE(get_url("Issuer_test3", url)==true);
    BOOST_REQUIRE(url==url_org3);

    // Urls for issuer 1 and 2 should remain as they were
    BOOST_REQUIRE(get_url("Issuer_test1", url)==true);
    BOOST_REQUIRE(url==url_update);
    BOOST_REQUIRE(get_url("Issuer_test2", url)==true);
    BOOST_REQUIRE(url==url_org2);

    // Update url for issuer 3
    set_url("Issuer_test3", url_update3, 1001);
    BOOST_REQUIRE(get_url("Issuer_test3", url)==true);
    BOOST_REQUIRE(url==url_update3);

    // Urls for issuer 1 and 2 should remain as they were
    BOOST_REQUIRE(get_url("Issuer_test1", url)==true);
    BOOST_REQUIRE(url==url_update);
    BOOST_REQUIRE(get_url("Issuer_test2", url)==true);
    BOOST_REQUIRE(url==url_org2);
}

BOOST_AUTO_TEST_CASE(DB_app_positive) {

    std::list<app_t> buffer;
    app_t app1("app_1", "pkg_1", 5001, {});

    chain_t chain2 = {"cert2"};
    app_t app2("app_2", "pkg 2", 5002, {chain2});

    chain_t chain2r = {"cert2r"};
    app_t app2r("app_2_remove", "pkg 2", 5002, {chain2, chain2r});

    chain_t chain31 = {"cert_3.1", "cert 3.2"};
    chain_t chain32 = {"cert_3.1"};
    app_t app3("app 3", "pkg 3", 5003, {chain31, chain32});

    chain_t chain41 = {"cert_4.1", "cert 4.2"};
    chain_t chain42 = {"cert_4.2.1", "cert 4.2.2", "cert 4.2.3"};
    chain_t chain43 = {"cert_4.3.1"};

    app_t app4("app 4", "pkg 4", 5004, {chain41, chain42, chain43});

    BOOST_REQUIRE(add_app_to_check_list(app1)==true);
    BOOST_REQUIRE(add_app_to_check_list(app2)==true);
    BOOST_REQUIRE(add_app_to_check_list(app2r)==true);
    BOOST_REQUIRE(add_app_to_check_list(app3)==true);
    BOOST_REQUIRE(add_app_to_check_list(app4)==true);

    mark_as_verified(app2, app_t::verified_t::NO);
    mark_as_verified(app3, app_t::verified_t::YES);
    remove_app_from_check_list(app2r);

    app2.verified = app_t::verified_t::NO;
    app3.verified = app_t::verified_t::YES;

    std::list<app_t> buffer_ok = {app1, app2, app3, app4};

    get_app_list(buffer);

    // Need to sort buffer
    sort_buffer(buffer);
    sort_buffer(buffer_ok);

    BOOST_REQUIRE(buffer_ok == buffer);
}

BOOST_AUTO_TEST_CASE(DB_app_negative) {

    std::list<app_t> buffer;
    app_t app1("app_1", "pkg_1", 5001, {});

    chain_t chain2 = {"cert2"};
    app_t app2("app_2", "pkg 2", 5002, {chain2});

    chain_t chain2r = {"cert2r"};
    app_t app2r("app_2_remove", "pkg 2", 5002, {chain2, chain2r});

    chain_t chain31 = {"cert_3.1", "cert 3.2"};
    chain_t chain32 = {"cert_3.1"};
    app_t app3("app 3", "pkg 3", 5003, {chain31, chain32});

    chain_t chain41 = {"cert_4.1", "cert 4.2"};
    chain_t chain42 = {"cert_4.2.1", "cert 4.2.2", "cert 4.2.3"};
    chain_t chain43 = {"cert_4.3.1"};

    app_t app4("app 4", "pkg 4", 5004, {chain41, chain42, chain43});

    BOOST_REQUIRE(add_app_to_check_list(app1)==true);
    BOOST_REQUIRE(add_app_to_check_list(app2)==true);
    BOOST_REQUIRE(add_app_to_check_list(app2r)==true);
    // Skipp adding app3 to database
    BOOST_REQUIRE(add_app_to_check_list(app4)==true);

    mark_as_verified(app2, app_t::verified_t::NO);
    mark_as_verified(app3, app_t::verified_t::YES);
    remove_app_from_check_list(app2r);

    app2.verified = app_t::verified_t::NO;
    app3.verified = app_t::verified_t::YES;

    std::list<app_t> buffer_ok = {app1, app2, app3, app4};

    get_app_list(buffer);

    // list has to be sorted before comparison.
    sort_buffer(buffer);
    sort_buffer(buffer_ok);
    BOOST_REQUIRE(buffer_ok != buffer);
}

BOOST_AUTO_TEST_SUITE_END()
