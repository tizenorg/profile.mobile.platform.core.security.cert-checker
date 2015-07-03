PKG_CHECK_MODULES(CERT_CHECKER_TESTS_DEP
    REQUIRED
    cert-svc-vcore
    dbus-1
    dbus-glib-1
    db-util
    glib-2.0
    gio-2.0
    icu-i18n
    key-manager
    libsystemd-journal
    libtzplatform-config
    sqlite3
    )

FIND_PACKAGE(Threads REQUIRED)
ADD_DEFINITIONS( "-DBOOST_TEST_DYN_LINK" )
ADD_DEFINITIONS("-DTEST_APP_SIGNATURES_DIR=\"${TEST_APP_SIGNATURES_DIR}\"")

SET(CERT_CHECKER_SRC_PATH ${PROJECT_SOURCE_DIR}/src)
SET(CERT_CHECKER_TESTS_SRC_PATH ${PROJECT_SOURCE_DIR}/tests)

SET(CERT_CHECKER_TESTS_SOURCES
    # tests
    ${CERT_CHECKER_TESTS_SRC_PATH}/main.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/dbfixture.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/colour_log_formatter.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/app_event_operators.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/test_app.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/test_db.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/test_queue.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/queue_test_thread.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/test_certs.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/certs_.cpp
    # cert-checker
    ${CERT_CHECKER_SRC_PATH}/app.cpp
    ${CERT_CHECKER_SRC_PATH}/queue.cpp
    ${CERT_CHECKER_SRC_PATH}/certs.cpp
    # logs
    ${CERT_CHECKER_SRC_PATH}/log/log.cpp
    # dpl
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/assert.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/char_traits.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/colors.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/errno_string.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/exception.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/noncopyable.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/string.cpp
    # dpl DB
    ${CERT_CHECKER_SRC_PATH}/dpl/db/src/sql_connection.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/db/src/naive_synchronization_object.cpp
    # DB
    ${CERT_CHECKER_SRC_PATH}/db/sql_query.cpp
    )

INCLUDE_DIRECTORIES(SYSTEM
    ${CERT_CHECKER_DEP_INCLUDE_DIRS}
    ${CERT_CHECKER_SRC_PATH}/include/
    ${CERT_CHECKER_SRC_PATH}/dpl/core/include/
    ${CERT_CHECKER_SRC_PATH}/dpl/db/include/
    ${CERT_CHECKER_TESTS_SRC_PATH}/
    )

ADD_EXECUTABLE(${TARGET_CERT_CHECKER_TESTS} ${CERT_CHECKER_TESTS_SOURCES})

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_TESTS}
    ${CERT_CHECKER_TESTS_DEP_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    boost_unit_test_framework
    -ldl
    )

#### Test files/signatures
INSTALL(FILES
    files/app1/author-signature.xml
    files/app1/signature1.xml
    DESTINATION ${TEST_APP_SIGNATURES_DIR}/app1/)
INSTALL(FILES
    files/app_2/signature1.xml
    DESTINATION ${TEST_APP_SIGNATURES_DIR}/app_2/)

INSTALL(TARGETS ${TARGET_CERT_CHECKER_TESTS} DESTINATION ${BINDIR})
