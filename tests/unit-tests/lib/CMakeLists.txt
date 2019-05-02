file(GLOB SRCS *.cpp)

add_executable(test_lib ${SRCS})

TARGET_LINK_LIBRARIES(test_lib "${GTEST_LIB}" "${SCHED_LIB}" "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" "${GTEST_MAIN_LIB}" pthread)

add_test(NAME test_lib COMMAND test_lib)
