cmake_minimum_required(VERSION 3.11)
project(pixbufloader-svg C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GDK_PIXBUF gdk-pixbuf-2.0 IMPORTED_TARGET REQUIRED)
pkg_check_modules(LIBRSVG librsvg-2.0 IMPORTED_TARGET REQUIRED)

Set(prefix "${CMAKE_INSTALL_PREFIX}")
Set(exec_prefix "\${prefix}")
Set(libdir "\${prefix}/lib")
Set(includedir "\${prefix}/include")

set(PIXBUFLOADERSVG_SOURCES
    gdk-pixbuf-loader/io-svg.c
)

if (BUILD_SHARED_LIBS)
    add_library(pixbufloader-svg MODULE ${PIXBUFLOADERSVG_SOURCES})
else()
    add_library(pixbufloader-svg ${PIXBUFLOADERSVG_SOURCES})
endif()
target_include_directories(pixbufloader-svg
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_compile_definitions(pixbufloader-svg PRIVATE
    -DRSVG_COMPILATION
    -D_CRT_SECURE_NO_WARNINGS
    -DSRCDIR=""
    -DGDK_PIXBUF_ENABLE_BACKEND
    -DG_LOG_DOMAIN="libpixbufloader-svg"
)
target_link_libraries(pixbufloader-svg
    PRIVATE
        PkgConfig::LIBRSVG
        PkgConfig::GDK_PIXBUF
)
if (BUILD_SHARED_LIBS)
    install(TARGETS pixbufloader-svg
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
    )
else()
    install(TARGETS pixbufloader-svg
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pixbufloader-svg.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/pixbufloader-svg.pc" @ONLY)
file(READ "${CMAKE_CURRENT_BINARY_DIR}/pixbufloader-svg.pc" pixbufloader_svg_pc)
if (NOT BUILD_SHARED_LIBS)
    string(REPLACE "-lm" "-lpixbufloader-svg -lm" pixbufloader_svg_pc "${pixbufloader_svg_pc}")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/pixbufloader-svg.pc" "${pixbufloader_svg_pc}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/pixbufloader-svg.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
