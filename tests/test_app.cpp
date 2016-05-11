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

#include "common/log.h"
#include <app_event_operators.h>

using namespace CCHECKER;

BOOST_FIXTURE_TEST_SUITE(APP_TEST, app_t)

BOOST_AUTO_TEST_CASE(App_positive) {

    app_t app1("app_1", "pkg_1", 5001, {{"aaaaaa"}});
    app_t app2("app_1", "pkg_1", 5001, {{"aaaaaa"}});

    app_t app3("app_2", "pkg_1", 5002, {{"aaa", "bbbb"}, {"ccccc"}});
    app_t app4("app_2", "pkg_1", 5002, {{"ccccc"}, {"aaa", "bbbb"}});

    chain_t chain411 = {"cert_4.1", "cert 4.2"};
    chain_t chain421 = {"cert_4.2.1", "cert 4.2.2", "cert 4.2.3"};
    chain_t chain43 = {"cert_4.3.1"};

    app_t app5("app_3", "pkg_1", 5003, {chain411, chain421, chain43});
    app_t app6("app_3", "pkg_1", 5003, {chain421, chain411, chain43});
    app_t app7("app_3", "pkg_1", 5003, {chain411, chain43, chain421});
    app_t app8("app_3", "pkg_1", 5003, {chain421, chain43, chain411});
    app_t app9("app_3", "pkg_1", 5003, {chain43, chain411, chain421});
    app_t app10("app_3", "pkg_1", 5003, {chain43, chain421, chain411});

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

    BOOST_REQUIRE(app1 == app2);

    BOOST_REQUIRE(app3 == app4);

    BOOST_REQUIRE(app5 == app6);
    BOOST_REQUIRE(app6 == app7);
    BOOST_REQUIRE(app7 == app8);
    BOOST_REQUIRE(app8 == app9);
    BOOST_REQUIRE(app9 == app10);
}

BOOST_AUTO_TEST_CASE(App_negative) {

    app_t app1("app_1", "pkg_1", 5001, {{"aaaaaa"}});
    app_t app2("app_2", "pkg_1", 5001, {{"aaaaaa"}});
    app_t app3("app_2", "pkg_2", 5001, {{"aaaaaa"}});
    app_t app4("app_2", "pkg_2", 5002, {{"aaaaaa"}});

    chain_t chain411 = {"cert_4.1", "cert 4.2"};
    chain_t chain412 = {"cert 4.2", "cert_4.1"};

    chain_t chain421 = {"cert_4.2.1", "cert 4.2.2", "cert 4.2.3"};
    chain_t chain422 = {"cert_4.2.1", "cert 4.2.3", "cert_4.2.2"};
    chain_t chain423 = {"cert_4.2.3", "cert 4.2.1", "cert_4.2.2"};
    chain_t chain424 = {"cert_4.2.3", "cert 4.2.2", "cert_4.2.1"};
    chain_t chain425 = {"cert_4.2.2", "cert 4.2.1", "cert_4.2.3"};
    chain_t chain426 = {"cert_4.2.2", "cert 4.2.3", "cert_4.2.1"};

    chain_t chain427 = {"cert",       "cert_4.2.1", "cert 4.2.2"};
    chain_t chain428 = {"cert_4.2.1", "          ", "cert 4.2.2"};
    chain_t chain429 = {"cert 4.2.3", "cert 4.2.2"};
    chain_t chain420 = {"cert 4.2"  , "cert_4.2"  , "cert 4.2"  };

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

    app_t app17("app_3", "pkg_1", 5003, {chain421});
    app_t app18("app_3", "pkg_1", 5003, {chain422});
    app_t app19("app_3", "pkg_1", 5003, {chain423});
    app_t app20("app_3", "pkg_1", 5003, {chain424});
    app_t app21("app_3", "pkg_1", 5003, {chain425});
    app_t app22("app_3", "pkg_1", 5003, {chain426});
    app_t app23("app_3", "pkg_1", 5003, {chain427});
    app_t app24("app_3", "pkg_1", 5003, {chain428});
    app_t app25("app_3", "pkg_1", 5003, {chain429});
    app_t app26("app_3", "pkg_1", 5003, {chain420});

    app_t apps[26] = {app1, app2, app3, app4, app5, app6, app7, app8,
            app9, app10, app11, app12, app13, app14, app15, app16, app17,
            app18, app19, app20, app21, app22, app23, app24, app25, app26};

    for (int i=0; i<26; i++) {
        sort(apps[i]);
    }

    for (int i=0; i<26; i++) {
        for (int j=0; j<26; j++) {
            if (i != j)
                BOOST_REQUIRE(apps[i] != apps[j]);
            else
                BOOST_REQUIRE(apps[i] == apps[j]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
