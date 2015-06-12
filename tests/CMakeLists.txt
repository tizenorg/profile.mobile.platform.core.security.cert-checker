PKG_CHECK_MODULES(CERT_CHECKER_TESTS_DEP
    REQUIRED
    dbus-1
    dbus-glib-1
    db-util
    glib-2.0
    gio-2.0
    icu-i18n
    notification
    libsystemd-journal
    libtzplatform-config
    sqlite3
    )

FIND_PACKAGE(Threads REQUIRED)
ADD_DEFINITIONS( "-DBOOST_TEST_DYN_LINK" )

SET(CERT_CHECKER_SRC_PATH ${PROJECT_SOURCE_DIR}/src)
SET(CERT_CHECKER_TESTS_SRC_PATH ${PROJECT_SOURCE_DIR}/tests)

SET(CERT_CHECKER_TESTS_SOURCES
    # tests
    ${CERT_CHECKER_TESTS_SRC_PATH}/main.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/dbfixture.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/colour_log_formatter.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/test_db.cpp
    # cert-checker
    ${CERT_CHECKER_SRC_PATH}/app.cpp
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

INSTALL(TARGETS ${TARGET_CERT_CHECKER_TESTS} DESTINATION ${BINDIR})
