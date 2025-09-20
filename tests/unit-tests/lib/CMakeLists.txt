file(GLOB SRCS *.cpp)

add_executable(test_lib ${SRCS})

# Ensure OpenSSL is linked for cryptography symbols used by libboinc_crypt and tests
find_package(OpenSSL REQUIRED)

TARGET_LINK_LIBRARIES(test_lib
	"${SCHED_LIB}"
	"${BOINC_CRYPT_LIB}"
	"${BOINC_LIB}"
	OpenSSL::SSL
	OpenSSL::Crypto
	pthread
	GTest::gtest
	GTest::gtest_main
)

add_test(NAME test_lib COMMAND test_lib)
