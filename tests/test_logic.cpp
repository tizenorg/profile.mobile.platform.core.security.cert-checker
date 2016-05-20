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
 * @file        test_logic.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @author      Sangwan Kwon (sangwan.kwon@samsung.com)
 * @version     1.0
 * @brief       Tests of Logic class
 */

#define BOOST_TEST_MODULE CERT_CHECKER_TESTS
#include <boost/test/unit_test.hpp>
#include <string>
#include <unistd.h>

#include <logic_.h>
#include <app_event_operators.h>

#include "common/log.h"

using namespace CCHECKER;

namespace {

std::string log_apps(std::list<app_t> &apps, std::list<app_t> buff)
{
    std::string ret = "\nApps:\n";

    for (const auto &iter : apps) {
        ret += iter.str() + "\n";
    }

    ret += "Buff:\n";
    for (const auto &iter : buff) {
         ret+= iter.str() + "\n";
    }

    return ret;
}
} // anonymous namespace

BOOST_FIXTURE_TEST_SUITE(LOGIC_TEST, LogicWrapper)

BOOST_AUTO_TEST_CASE(logic_setup) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    // double setup
    BOOST_REQUIRE(setup() == NO_ERROR);

    // double setup
    BOOST_REQUIRE(setup() == NO_ERROR);
}

BOOST_AUTO_TEST_CASE(logic_workflow_mixed) {
    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_1", 5001, {});
    pkgmgr_install_manual_(app2);

    app_t app3("app_2", "pkg_2", 5001, {});
    pkgmgr_install_manual_(app3);

    // remove app 2
    pkgmgr_uninstall_manual_(app2);

    wait_for_worker(3, 1);

    std::list<app_t> buff = get_buffer_();
    std::list<app_t> apps = {app1, app2, app3};
    BOOST_REQUIRE(buff != apps);

    apps = {app1, app3};
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    // Add other apps
    pkgmgr_install_manual_(app2);

    // install the same app again
    // it should never happen in real workflow
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 5002, {});
    pkgmgr_install_manual_(app4);

    wait_for_worker(3);

    buff = get_buffer_();
    BOOST_REQUIRE(buff != apps);

    apps = {app1, app3, app2, app4};

    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    // turn on the Internet - buffer should be processed
    connman_callback_manual_(true);

    wait_for_worker(0, 0, 4);
    buff = get_buffer_();
    BOOST_CHECK(buff.empty());
}

BOOST_AUTO_TEST_CASE(logic_workflow_mixed_2) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_2", 5002, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app2);

    app_t app3("app_3", "pkg_3", 5003, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 5004, {{"OCSP_APP_REVOKED"}}); // popup will succeed
    pkgmgr_install_manual_(app4);

    app_t app5("app_5", "pkg_5", 100, {{"OCSP_APP_REVOKED"}}); // popup will fail
    pkgmgr_install_manual_(app5);

    // turn on the Internet - buffer should be processed
    connman_callback_manual_(true);

    wait_for_worker(5, 0, 5);
    std::list<app_t> buff = get_buffer_();
#if POPUP
    std::list<app_t> apps = {app3, app5};
#else
    std::list<app_t> apps = {app3}; // only if popup is disabled
#endif

    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));
}

BOOST_AUTO_TEST_CASE(logic_workflow_mixed_3) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_2", 5002, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app2);

    app_t app3("app_3", "pkg_3", 100, {{"OCSP_APP_REVOKED"}}); // popup will fail
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 5004, {{"OCSP_APP_REVOKED"}}); // popup will succeed
    pkgmgr_install_manual_(app4);

    app_t app5("app_5", "pkg_5", 5005, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app5);

    app_t app6("app_6", "pkg_6", 101, {{"OCSP_APP_REVOKED"}}); // popup will fail
    pkgmgr_install_manual_(app6);

    wait_for_worker(6);

    std::list<app_t> buff = get_buffer_();
    std::list<app_t> apps = {app1, app2, app3, app4, app5, app6};

    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    connman_callback_manual_(true);
    wait_for_worker(0, 0, 6);
    buff = get_buffer_();
#if POPUP
    apps = {app1, app3, app6};
#else
    apps = {app1}; // only if popup is disabled
#endif

    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));
}

BOOST_AUTO_TEST_CASE(logic_workflow_mixed_4) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {});
    pkgmgr_install_manual_(app1);

    wait_for_worker(1);

    std::list<app_t> apps = {app1};
    std::list<app_t> buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    app_t app2("app_2", "pkg_2", 5002, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app2);

    wait_for_worker(1);

    apps = {app1, app2};
    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    app_t app3("app_3", "pkg_3", 5003, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app3);

    wait_for_worker(1);

    apps = {app1, app2, app3};
    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    app_t app4("app_4", "pkg_4", 5004, {{"OCSP_APP_REVOKED"}}); // popup will succeed
    pkgmgr_install_manual_(app4);

    wait_for_worker(1);

    apps = {app1, app2, app3, app4};
    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    app_t app5("app_5", "pkg_5", 100, {{"OCSP_APP_REVOKED"}}); // popup will fail
    pkgmgr_install_manual_(app5);

    wait_for_worker(1);

    apps = {app1, app2, app3, app4, app5};
    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));
}

// OCSP_CHECK_AGAIN - apps should stay in buffer
BOOST_AUTO_TEST_CASE(logic_workflow_OCSP_CHECK_AGAIN) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_2", 5002, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app2);

    app_t app3("app_3", "pkg_3", 100, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 5004, {{"OCSP_CHECK_AGAIN"}});
    pkgmgr_install_manual_(app4);

    connman_callback_manual_(true);

    wait_for_worker(4, 0, 4);

    std::list<app_t> buff = get_buffer_();
    std::list<app_t> apps = {app1, app2, app3, app4};

    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    connman_callback_manual_(true);

    wait_for_worker(0, 0, 4);

    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));
}

// OCSP_CERT_ERROR - apps should be removed from buffer
// OCSP_APP_OK should have the same effect
BOOST_AUTO_TEST_CASE(logic_workflow_OCSP_CERT_ERROR) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_2", 5002, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app2);


    app_t app3("app_3", "pkg_3", 100, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 5004, {{"OCSP_CERT_ERROR"}});
    pkgmgr_install_manual_(app4);

    connman_callback_manual_(true);
    wait_for_worker(4, 0, 4);
    std::list<app_t> buff = get_buffer_();

    BOOST_REQUIRE(buff.empty());

    connman_callback_manual_(true);

    wait_for_worker();

    buff = get_buffer_();
    BOOST_REQUIRE(buff.empty());

    connman_callback_manual_(true);

    wait_for_worker();

    buff = get_buffer_();
    BOOST_REQUIRE(buff.empty());
}

// OCSP_APP_REVOKED - popup OK - apps should be removed from buffer
BOOST_AUTO_TEST_CASE(logic_workflow_OCSP_APP_REVOKED) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 5001, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_2", 5002, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app2);

    app_t app3("app_3", "pkg_3", 5003, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 5004, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app4);

    connman_callback_manual_(true);
    wait_for_worker(4, 0, 4);

    std::list<app_t> buff = get_buffer_();

    BOOST_REQUIRE(buff.empty());

    connman_callback_manual_(true);

    wait_for_worker();

    buff = get_buffer_();
    BOOST_REQUIRE(buff.empty());

    connman_callback_manual_(true);

    wait_for_worker();

    buff = get_buffer_();
    BOOST_REQUIRE(buff.empty());
}

// OCSP_APP_REVOKED - popup fail - apps should stay in buffer
BOOST_AUTO_TEST_CASE(logic_workflow_OCSP_APP_REVOKED_2) {

    BOOST_REQUIRE(setup() == NO_ERROR);

    wait_for_worker();

    // turn off the network
    connman_callback_manual_(false);

    // add applications:
    app_t app1("app_1", "pkg_1", 1001, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app1);

    app_t app2("app_2", "pkg_2", 1002, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app2);

    app_t app3("app_3", "pkg_3", 1003, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app3);

    app_t app4("app_4", "pkg_4", 1004, {{"OCSP_APP_REVOKED"}});
    pkgmgr_install_manual_(app4);

    connman_callback_manual_(true);

    wait_for_worker(4, 0, 4);

    std::list<app_t> buff = get_buffer_();
    std::list<app_t> apps = {app1, app2, app3, app4};

    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    connman_callback_manual_(true);

    wait_for_worker(0, 0, 4);

    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));

    connman_callback_manual_(true);

    wait_for_worker(0, 0, 4);

    buff = get_buffer_();
    BOOST_CHECK_MESSAGE(buff == apps, log_apps(apps, buff));
}

BOOST_AUTO_TEST_CASE(logic_workflow_timer) {

    // Wait for setup.
    BOOST_REQUIRE(setup() == NO_ERROR);
    wait_for_worker();

    // Job() will run. Install process.
    timerStart(1);
    wait_for_worker(5);
    timerStop();

    // If Timer is not work, wait_for_worker throw exception.
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(logic_workflow_gio_timeout) {

    // Wait for setup.
    BOOST_REQUIRE(setup() == NO_ERROR);
    wait_for_worker();

    // Run gmainloop.
    run(3);

    BOOST_REQUIRE(is_running() == false);
}

BOOST_AUTO_TEST_SUITE_END()
