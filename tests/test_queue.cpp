#include <boost/test/unit_test.hpp>
#include <string>

#include <cchecker/app.h>
#include <cchecker/log.h>
#include <cchecker/queue.h>

using namespace CCHECKER;

BOOST_FIXTURE_TEST_SUITE(QUEUE_TEST, Queue)

BOOST_AUTO_TEST_CASE(Queue) {

    app_t app1("app_id1", "pkg_id1", 1, {});
    app_t app2("app_id2", "pkg_id2", 2, {});
    app_t app3("app_id@", "###",     3, {});

    event_t ev1(app1, event_t::event_type_t::APP_INSTALL);
    event_t ev2(app1, event_t::event_type_t::APP_UNINSTALL);

    event_t ev3(app2, event_t::event_type_t::APP_UNINSTALL);
    event_t ev4(app2, event_t::event_type_t::APP_UNINSTALL);

    event_t ev;

    BOOST_CHECK(empty() == true);
    push_event(ev1);

    BOOST_CHECK(top(ev) == true);
    BOOST_CHECK(ev1.app.app_id == ev.app.app_id);
    BOOST_CHECK(ev1.app.pkg_id == ev.app.pkg_id);
    BOOST_CHECK(ev1.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_CHECK(ev1.app.certificates == ev.app.certificates);
    BOOST_CHECK(ev1.app.verified == ev.app.verified);

    push_event(ev2);

    BOOST_CHECK(top(ev) == true);
    BOOST_CHECK(ev1.app.app_id == ev.app.app_id);
    BOOST_CHECK(ev1.app.pkg_id == ev.app.pkg_id);
    BOOST_CHECK(ev1.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_CHECK(ev1.app.certificates == ev.app.certificates);
    BOOST_CHECK(ev1.app.verified == ev.app.verified);

    push_event(ev3);
    pop_event();

    BOOST_CHECK(top(ev) == true);
    BOOST_CHECK(ev2.app.app_id == ev.app.app_id);
    BOOST_CHECK(ev2.app.pkg_id == ev.app.pkg_id);
    BOOST_CHECK(ev2.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_CHECK(ev2.app.certificates == ev.app.certificates);
    BOOST_CHECK(ev2.app.verified == ev.app.verified);

    pop_event();
    push_event(ev4);
    pop_event();

    BOOST_CHECK(top(ev) == true);
    BOOST_CHECK(ev4.app.app_id == ev.app.app_id);
    BOOST_CHECK(ev4.app.pkg_id == ev.app.pkg_id);
    BOOST_CHECK(ev4.app.uid == ev.app.uid);
    // Certs and verified flag aren't used in queue, but can be tested
    BOOST_CHECK(ev4.app.certificates == ev.app.certificates);
    BOOST_CHECK(ev4.app.verified == ev.app.verified);

    pop_event();
    BOOST_CHECK(top(ev) == false);
}

BOOST_AUTO_TEST_SUITE_END()
