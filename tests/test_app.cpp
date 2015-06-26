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
 * @file        test_app.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 * @brief       App structure tests
 */

#include <boost/test/unit_test.hpp>
#include <string>

#include <cchecker/log.h>
#include <app_event_operators.h>

using namespace CCHECKER;

BOOST_FIXTURE_TEST_SUITE(APP_TEST, app_t)

BOOST_AUTO_TEST_CASE(App_positive) {

    app_t app1("app_1", "pkg_1", 5001, {{"aaaaaa"}});
    app_t app2("app_1", "pkg_1", 5001, {{"aaaaaa"}});

    app_t app3("app_2", "pkg_1", 5002, {{"aaa", "bbbb"}});
    app_t app4("app_2", "pkg_1", 5002, {{"bbbb", "aaa"}});

    chain_t chain411 = {"cert_4.1", "cert 4.2"};
    chain_t chain412 = {"cert 4.2", "cert_4.1"};
    chain_t chain421 = {"cert_4.2.1", "cert 4.2.2", "cert 4.2.3"};
    chain_t chain422 = {"cert 4.2.2", "cert 4.2.3", "cert_4.2.1"};
    chain_t chain423 = {"cert 4.2.3", "cert_4.2.1", "cert 4.2.2"};
    chain_t chain424 = {"cert_4.2.1", "cert 4.2.3", "cert 4.2.2"};
    chain_t chain425 = {"cert 4.2.3", "cert 4.2.2", "cert_4.2.1"};
    chain_t chain426 = {"cert 4.2.2", "cert_4.2.1", "cert 4.2.3"};
    chain_t chain43 = {"cert_4.3.1"};

    app_t app5("app_3", "pkg_1", 5003, {chain411, chain421, chain43});
    app_t app6("app_3", "pkg_1", 5003, {chain411, chain422, chain43});
    app_t app7("app_3", "pkg_1", 5003, {chain411, chain423, chain43});
    app_t app8("app_3", "pkg_1", 5003, {chain411, chain424, chain43});
    app_t app9("app_3", "pkg_1", 5003, {chain411, chain425, chain43});
    app_t app10("app_3", "pkg_1", 5003, {chain411, chain426, chain43});
    app_t app11("app_3", "pkg_1", 5003, {chain412, chain421, chain43});
    app_t app12("app_3", "pkg_1", 5003, {chain412, chain422, chain43});
    app_t app13("app_3", "pkg_1", 5003, {chain412, chain423, chain43});
    app_t app14("app_3", "pkg_1", 5003, {chain412, chain424, chain43});
    app_t app15("app_3", "pkg_1", 5003, {chain412, chain425, chain43});
    app_t app16("app_3", "pkg_1", 5003, {chain412, chain426, chain43});

    sort(app1);
    sort(app2);
    sort(app3);
    sort(app4);
    sort(app5);
    sort(app6);
    sort(app7);
    sort(app8);
    sort(app9);
    sort(app10);
    sort(app11);
    sort(app12);
    sort(app13);
    sort(app14);
    sort(app15);
    sort(app16);

    BOOST_REQUIRE(app1 == app2);

    BOOST_REQUIRE(app3 == app4);

    BOOST_REQUIRE(app5 == app6);
    BOOST_REQUIRE(app6 == app7);
    BOOST_REQUIRE(app7 == app8);
    BOOST_REQUIRE(app8 == app9);
    BOOST_REQUIRE(app9 == app10);
    BOOST_REQUIRE(app10 == app11);
    BOOST_REQUIRE(app11 == app12);
    BOOST_REQUIRE(app12 == app13);
    BOOST_REQUIRE(app13 == app14);
    BOOST_REQUIRE(app14 == app15);
    BOOST_REQUIRE(app15 == app16);
    BOOST_REQUIRE(app16 == app5);
}

BOOST_AUTO_TEST_CASE(App_negative) {

    app_t app1("app_1", "pkg_1", 5001, {{"aaaaaa"}});
    app_t app2("app_2", "pkg_1", 5001, {{"aaaaaa"}});

    app_t app3("app_2", "pkg_2", 5001, {{"aaa", "bbbb"}});
    app_t app4("app_2", "pkg_1", 5002, {{"bbbb", "ccc"}});

    chain_t chain411 = {"cert_4.1", "cert 4.2"};
    chain_t chain412 = {"cert_4.1"};

    chain_t chain421 = {"cert_4.2.1", "cert 4.2.2", "cert 4.2.3"};
    chain_t chain422 = {""            "cert 4.2.3", "cert_4.2.1"};
    chain_t chain423 = {"cert",       "cert_4.2.1", "cert 4.2.2"};
    chain_t chain424 = {"cert_4.2.1", "          ", "cert 4.2.2"};
    chain_t chain425 = {"cert 4.2.3", "cert 4.2.2"};
    chain_t chain426 = {"cert 4.2", "cert_4.2", "cert 4.2"};

    chain_t chain43 = {"cert_4.3.1"};

    app_t app5("app_3", "pkg_1", 5003, {chain411, chain421, chain43});
    app_t app6("app_3", "pkg_1", 5003, {chain411, chain422, chain43});
    app_t app7("app_3", "pkg_1", 5003, {chain411, chain423, chain43});
    app_t app8("app_3", "pkg_1", 5003, {chain411, chain424, chain43});
    app_t app9("app_3", "pkg_1", 5003, {chain411, chain425, chain43});
    app_t app10("app_3", "pkg_1", 5003, {chain411, chain426, chain43});
    app_t app11("app_3", "pkg_1", 5003, {chain412, chain421, chain43});
    app_t app12("app_3", "pkg_1", 5003, {chain412, chain422, chain43});
    app_t app13("app_3", "pkg_1", 5003, {chain412, chain423, chain43});
    app_t app14("app_3", "pkg_1", 5003, {chain412, chain424, chain43});
    app_t app15("app_3", "pkg_1", 5003, {chain412, chain425, chain43});
    app_t app16("app_3", "pkg_1", 5003, {chain412, chain426, chain43});

    app_t apps[12] = {app5, app6, app7, app8, app9, app10, app11, app12, app13, app14, app15, app16};

    sort(app1);
    sort(app2);
    sort(app3);
    sort(app4);

    for (int i=0; i<12; i++) {
        sort(apps[i]);
    }

    BOOST_REQUIRE(app1 != app2);
    BOOST_REQUIRE(app1 != app3);
    BOOST_REQUIRE(app1 != app4);
    BOOST_REQUIRE(app2 != app3);
    BOOST_REQUIRE(app2 != app4);
    BOOST_REQUIRE(app3 != app4);

    for (int i=0; i<12; i++) {
        for (int j=0; j<12; j++) {
            if (i != j)
                BOOST_REQUIRE(apps[i] != apps[j]);
            else
                BOOST_REQUIRE(apps[i] == apps[j]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
