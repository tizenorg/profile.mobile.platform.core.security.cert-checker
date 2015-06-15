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
 * @file        test_queue.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       Tests of thread-safe queue
 */

#include <boost/test/unit_test.hpp>
#include <string>
#include <thread>

#include <cchecker/app.h>
#include <cchecker/log.h>
#include <cchecker/queue.h>

#include <queue_test_thread.h>

using namespace CCHECKER;

BOOST_FIXTURE_TEST_SUITE(QUEUE_TEST, Queue)

BOOST_AUTO_TEST_CASE(Queue) {

    app_t app1("app_id1", "pkg_id1", 1, {});
    app_t app2("app_id2", "pkg_id2", 2, {});
    app_t app3("app_id@", "###",     3, {});

    event_t ev1(app1, event_t::event_type_t::APP_INSTALL);
    event_t ev2(app1, event_t::event_type_t::APP_UNINSTALL);

    event_t ev3(app2, event_t::event_type_t::APP_INSTALL);
    event_t ev4(app2, event_t::event_type_t::APP_UNINSTALL);

    event_t ev;

    BOOST_REQUIRE(empty() == true);
    BOOST_REQUIRE(pop_event(ev) == false);
    BOOST_REQUIRE(empty() == true);
    push_event(ev1);
    BOOST_REQUIRE(empty() == false);

    BOOST_REQUIRE(pop_event(ev) == true);
    BOOST_REQUIRE(ev1.app.app_id == ev.app.app_id);
    BOOST_REQUIRE(ev1.app.pkg_id == ev.app.pkg_id);
    BOOST_REQUIRE(ev1.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_REQUIRE(ev1.app.certificates == ev.app.certificates);
    BOOST_REQUIRE(ev1.app.verified == ev.app.verified);
    BOOST_REQUIRE(empty() == true);

    push_event(ev2);
    push_event(ev3);
    BOOST_REQUIRE(empty() == false);

    BOOST_REQUIRE(pop_event(ev) == true);
    BOOST_REQUIRE(ev2.app.app_id == ev.app.app_id);
    BOOST_REQUIRE(ev2.app.pkg_id == ev.app.pkg_id);
    BOOST_REQUIRE(ev2.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_REQUIRE(ev2.app.certificates == ev.app.certificates);
    BOOST_REQUIRE(ev2.app.verified == ev.app.verified);


    BOOST_REQUIRE(pop_event(ev) == true);
    BOOST_REQUIRE(ev3.app.app_id == ev.app.app_id);
    BOOST_REQUIRE(ev3.app.pkg_id == ev.app.pkg_id);
    BOOST_REQUIRE(ev3.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_REQUIRE(ev3.app.certificates == ev.app.certificates);
    BOOST_REQUIRE(ev3.app.verified == ev.app.verified);

    BOOST_REQUIRE(empty() == true);
    BOOST_REQUIRE(pop_event(ev) == false);
    push_event(ev4);
    BOOST_REQUIRE(empty() == false);
    BOOST_REQUIRE(pop_event(ev) == true);
    BOOST_REQUIRE(pop_event(ev) == false);
    BOOST_REQUIRE(pop_event(ev) == false);
    BOOST_REQUIRE(empty() == true);

    push_event(ev4);

    BOOST_REQUIRE(pop_event(ev) == true);
    BOOST_REQUIRE(ev4.app.app_id == ev.app.app_id);
    BOOST_REQUIRE(ev4.app.pkg_id == ev.app.pkg_id);
    BOOST_REQUIRE(ev4.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_REQUIRE(ev4.app.certificates == ev.app.certificates);
    BOOST_REQUIRE(ev4.app.verified == ev.app.verified);

    BOOST_REQUIRE(pop_event(ev) == false);
    BOOST_REQUIRE(empty() == true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(QUEUE_MULTI_THREAD_TEST, TestQueue)

BOOST_AUTO_TEST_CASE(TestQueue) {

    add_events_th ();
    BOOST_REQUIRE(pop_events() == true);
}

BOOST_AUTO_TEST_SUITE_END()
