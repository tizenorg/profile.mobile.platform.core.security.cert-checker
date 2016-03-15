PKG_CHECK_MODULES(CERT_CHECKER_DEP
    REQUIRED
    capi-appfw-application
    dbus-1
    dbus-glib-1
    db-util
    elementary
    glib-2.0
    gio-2.0
    icu-i18n
    key-manager
    libsystemd-journal
    libtzplatform-config
    sqlite3
    pkgmgr
    pkgmgr-info
    )

SET(CERT_CHECKER_SRC_PATH ${PROJECT_SOURCE_DIR}/src)

SET(CERT_CHECKER_SOURCES
    ${CERT_CHECKER_SRC_PATH}/cert-checker.cpp
    ${CERT_CHECKER_SRC_PATH}/app.cpp
    ${CERT_CHECKER_SRC_PATH}/logic.cpp
    ${CERT_CHECKER_SRC_PATH}/queue.cpp
    ${CERT_CHECKER_SRC_PATH}/certs.cpp
    # logs
    ${CERT_CHECKER_SRC_PATH}/log/log.cpp
    # dpl
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/assert.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/char_traits.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/errno_string.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/exception.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/noncopyable.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/string.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/core/src/serialization.cpp
    # dpl DB
    ${CERT_CHECKER_SRC_PATH}/dpl/db/src/sql_connection.cpp
    ${CERT_CHECKER_SRC_PATH}/dpl/db/src/naive_synchronization_object.cpp
    # DB
    ${CERT_CHECKER_SRC_PATH}/db/sql_query.cpp
    # UI
    ${CERT_CHECKER_SRC_PATH}/ui/UIBackend.cpp
    ${CERT_CHECKER_SRC_PATH}/ui/popup-runner.cpp
    )

INCLUDE_DIRECTORIES(SYSTEM
    ${CERT_CHECKER_DEP_INCLUDE_DIRS}
    ${CERT_CHECKER_SRC_PATH}/include/
    ${CERT_CHECKER_SRC_PATH}/dpl/core/include/
    ${CERT_CHECKER_SRC_PATH}/dpl/db/include/
    )

ADD_EXECUTABLE(${TARGET_CERT_CHECKER} ${CERT_CHECKER_SOURCES})

SET_TARGET_PROPERTIES(${TARGET_CERT_CHECKER} PROPERTIES
    COMPILE_FLAGS
    -fpie
)

TARGET_LINK_LIBRARIES(${TARGET_CERT_CHECKER}
    ${CERT_CHECKER_DEP_LIBRARIES}
    -pie
    )

INSTALL(TARGETS ${TARGET_CERT_CHECKER} DESTINATION ${BINDIR})

ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(ui)
