cmake_minimum_required(VERSION 2.8)

project(BOINC_unit_testing)
enable_testing()

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror --coverage")


find_package(Threads REQUIRED)

# There is no easy way to interface with the autotools driven build system in boinc
# so the paths below are hardcoded and are mainly suited for building on Travis CI
# TODO: make paths configurable and generate them from boinc make or switch boinc to cmake

include_directories(BEFORE "${PROJECT_SOURCE_DIR}/../../3rdParty/buildCache/linux/include")
include_directories("${PROJECT_SOURCE_DIR}/../..")
include_directories("${PROJECT_SOURCE_DIR}/../../sched")
include_directories("${PROJECT_SOURCE_DIR}/../../db")
include_directories("${PROJECT_SOURCE_DIR}/../../lib")
include_directories("${PROJECT_SOURCE_DIR}/../../tools")
include_directories("/usr/include/mariadb")
include_directories("/usr/include/mariadb/mysql")
include_directories("/usr/include/mysql")


find_library(GTEST_MAIN_LIB libgtest_main.a PATHS ${PROJECT_SOURCE_DIR}/../../3rdParty/buildCache/linux/lib NO_DEFAULT_PATH)
message(STATUS "gtest_main_lib: ${GTEST_MAIN_LIB}")
find_library(GTEST_LIB gtest PATHS ${PROJECT_SOURCE_DIR}/../../3rdParty/buildCache/linux/lib NO_DEFAULT_PATH)
message(STATUS "gtest_lib: ${GTEST_LIB}")
find_library(SCHED_LIB libsched.a PATHS ${PROJECT_SOURCE_DIR}/../../sched NO_DEFAULT_PATH)
find_library(BOINC_CRYPT_LIB libboinc_crypt.a PATHS ${PROJECT_SOURCE_DIR}/../../lib NO_DEFAULT_PATH)
find_library(BOINC_LIB libboinc.a PATHS ${PROJECT_SOURCE_DIR}/../../lib NO_DEFAULT_PATH)
find_library(MYSQL_LIB NAMES mariadb mysqlclient)
message(STATUS "mysql_lib: ${MYSQL_LIB}")

add_subdirectory(lib)
add_subdirectory(sched)
