SET(TARGET_DB ".cert-checker.db")

ADD_CUSTOM_COMMAND(
    OUTPUT ${TARGET_DB}
    COMMAND sqlite3 ${TARGET_DB} <cert-checker.sql
    )

# Add a dummy build target to trigger building of ${TARGET_DB}
ADD_CUSTOM_TARGET(DB ALL DEPENDS ${TARGET_DB})

INSTALL(FILES ${TARGET_DB} DESTINATION ${DB_INSTALL_DIR})


# Needed for tests
SET(TARGET_DB_TESTS ".cert-checker-test.db")

ADD_CUSTOM_COMMAND(
    OUTPUT ${TARGET_DB_TESTS}
    COMMAND sqlite3 ${TARGET_DB_TESTS} <cert-checker.sql
    )

# Add a dummy build target to trigger building of ${TARGET_DB}
ADD_CUSTOM_TARGET(DB_TESTS ALL DEPENDS ${TARGET_DB_TESTS})

INSTALL(FILES ${TARGET_DB_TESTS} DESTINATION ${DB_INSTALL_DIR})
