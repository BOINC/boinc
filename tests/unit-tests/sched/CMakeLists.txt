file(GLOB SRCS *.cpp)

add_executable(test_sched ${SRCS})

TARGET_COMPILE_OPTIONS(test_sched PUBLIC ${MYSQL_CFLAGS})

TARGET_LINK_LIBRARIES(test_sched "${SCHED_LIB}" "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" ${MYSQL_LIB} pthread GTest::gtest GTest::gtest_main)

add_test(NAME test_sched COMMAND test_sched)
