PKG_CHECK_MODULES(CERT_CHECKER_TESTS_DEP
    REQUIRED
    capi-appfw-application
    db-util
    dbus-1
    dbus-glib-1
    elementary
    glib-2.0
    gio-2.0
    icu-i18n
    key-manager
    libsystemd-journal
    sqlite3
    pkgmgr
    pkgmgr-info
    )

FIND_PACKAGE(Threads REQUIRED)
ADD_DEFINITIONS( "-DBOOST_TEST_DYN_LINK" )
ADD_DEFINITIONS("-DTEST_APP_SIGNATURES_DIR=\"${TEST_APP_SIGNATURES_DIR}\"")

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
    ${CERT_CHECKER_SERVICE_PATH}/app.cpp
    ${CERT_CHECKER_SERVICE_PATH}/queue.cpp
    ${CERT_CHECKER_SERVICE_PATH}/certs.cpp
    # logs
    ${CERT_CHECKER_LOG_PATH}/log.cpp
    # dpl
    ${DPL_CORE_SRC_PATH}/assert.cpp
    ${DPL_CORE_SRC_PATH}/char_traits.cpp
    ${DPL_CORE_SRC_PATH}/colors.cpp
    ${DPL_CORE_SRC_PATH}/errno_string.cpp
    ${DPL_CORE_SRC_PATH}/exception.cpp
    ${DPL_CORE_SRC_PATH}/noncopyable.cpp
    ${DPL_CORE_SRC_PATH}/string.cpp
    # dpl DB
    ${DPL_DB_SRC_PATH}/sql_connection.cpp
    ${DPL_DB_SRC_PATH}/naive_synchronization_object.cpp
    # DB
    ${CERT_CHECKER_DB_PATH}/sql_query.cpp
    )

SET(CERT_CHECKER_TESTS_LOGIC_SOURCES
    # tests
    ${CERT_CHECKER_TESTS_SRC_PATH}/main.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/colour_log_formatter.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/test_logic.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/logic_.cpp
    ${CERT_CHECKER_TESTS_SRC_PATH}/stubs_.cpp
    # cert-checker
    ${CERT_CHECKER_SERVICE_PATH}/logic.cpp
    ${CERT_CHECKER_SERVICE_PATH}/app.cpp
    ${CERT_CHECKER_SERVICE_PATH}/queue.cpp
    # logs
    ${CERT_CHECKER_LOG_PATH}/log.cpp
    # dpl
    ${DPL_CORE_SRC_PATH}/colors.cpp
)

SET(CERT_CHECKER_POPUP_TEST_SOURCES
    # tests
    ${CERT_CHECKER_TESTS_SRC_PATH}/popup_test.cpp
    # cert-checker
    ${CERT_CHECKER_SERVICE_PATH}/app.cpp
    ${CERT_CHECKER_UI_PATH}/popup-runner.cpp
    ${CERT_CHECKER_UI_PATH}/UIBackend.cpp
    # logs
    ${CERT_CHECKER_LOG_PATH}/log.cpp
    )

INCLUDE_DIRECTORIES(SYSTEM
    ${CERT_CHECKER_TESTS_DEP_INCLUDE_DIRS}
    ${CERT_CHECKER_SRC_PATH}/
    ${CERT_CHECKER_SRC_PATH}/include/
    ${DPL_CORE_PATH}/include/
    ${DPL_DB_PATH}/include/
    ${CERT_CHECKER_TESTS_SRC_PATH}/
    )

ADD_EXECUTABLE(${TARGET_CERT_CHECKER_TESTS} ${CERT_CHECKER_TESTS_SOURCES})
ADD_EXECUTABLE(${TARGET_CERT_CHECKER_TESTS_LOGIC} ${CERT_CHECKER_TESTS_LOGIC_SOURCES})
ADD_EXECUTABLE(${TARGET_CERT_CHECKER_POPUP_TEST} ${CERT_CHECKER_POPUP_TEST_SOURCES})

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_TESTS}
    ${TARGET_CERT_CHECKER_COMMON}
    ${CERT_CHECKER_TESTS_DEP_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    boost_unit_test_framework
    -ldl
    )

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_TESTS_LOGIC}
    ${TARGET_CERT_CHECKER_COMMON}
    ${CERT_CHECKER_TESTS_DEP_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    boost_unit_test_framework
    -ldl
    )

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER_POPUP_TEST}
    ${TARGET_CERT_CHECKER_COMMON}
    ${CERT_CHECKER_TESTS_DEP_LIBRARIES}
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
INSTALL(FILES
    files/app3/signature1.xml
    DESTINATION ${TEST_APP_SIGNATURES_DIR}/app3/)
INSTALL(FILES
    files/app4/author-signature.xml
    files/app4/signature1.xml
    DESTINATION ${TEST_APP_SIGNATURES_DIR}/app4/)

INSTALL(TARGETS ${TARGET_CERT_CHECKER_TESTS} DESTINATION ${BIN_DIR})
INSTALL(TARGETS ${TARGET_CERT_CHECKER_TESTS_LOGIC} DESTINATION ${BIN_DIR})
INSTALL(TARGETS ${TARGET_CERT_CHECKER_POPUP_TEST} DESTINATION ${BIN_DIR})
