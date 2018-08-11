message("entered api directory")

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/lib")

# library for both main and graphics apps
set(api_SOURCES boinc_api.cpp graphics2_util.cpp reduce_main.cpp)
# library for graphics apps
set(graphics2_SOURCES gutil.cpp gutil_text.cpp reduce_lib.cpp graphics2.cpp)
if(WIN32)
    set(graphics2_SOURCES ${graphics2_SOURCES} graphics2_win.cpp ${PROJECT_SOURCE_DIR}/samples/image_libs/bmplib.cpp ${PROJECT_SOURCE_DIR}/samples/image_libs/tgalib.cpp)
else()
    set(graphics2_SOURCES ${graphics2_SOURCES} graphics2_unix.cpp)
endif()
if(APPLE)
    set(graphics2_SOURCES ${graphics2_SOURCES} macglutfix.m)
endif()

set(opencl_SOURCES boinc_opencl.cpp)

add_library(boinc_api ${api_SOURCES})
add_library(boinc_graphics2 ${graphics2_SOURCES})
target_include_directories(boinc_graphics2 PUBLIC ${GLUT_INCLUDE_DIR})
target_include_directories(boinc_graphics2 PUBLIC ${JPEG_INCLUDE_DIR})
target_link_libraries(boinc_graphics2 ${GLUT_LIBRARIES})
target_link_libraries(boinc_graphics2 ${JPEG_LIBRARIES})
add_library(boinc_opencl ${opencl_SOURCES})
