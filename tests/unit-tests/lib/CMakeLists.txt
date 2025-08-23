file(GLOB SRCS *.cpp)

add_executable(test_lib ${SRCS})

# remove BOINCZIP_LIB when done
TARGET_LINK_LIBRARIES(test_lib "${SCHED_LIB}" "${BOINC_CRYPT_LIB}" "${BOINC_LIB}" "${BOINCZIP_LIB}" pthread GTest::gtest GTest::gtest_main)

add_test(NAME test_lib COMMAND test_lib)
