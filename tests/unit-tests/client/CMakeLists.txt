file(GLOB SRCS *.cpp)

LIST(APPEND SRCS "${PROJECT_SOURCE_DIR}/../../client/hostinfo_linux.cpp")

add_executable(test_client ${SRCS})

TARGET_LINK_LIBRARIES(test_client "${SCHED_LIB}" "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" pthread GTest::gtest GTest::gtest_main)

add_test(NAME test_client COMMAND test_client)
