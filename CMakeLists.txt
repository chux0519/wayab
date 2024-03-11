cmake_minimum_required(VERSION 3.10)

project(wayab LANGUAGES C)

# AddressSanitize
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
    FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
include_directories(${WAYLAND_CLIENT_INCLUDE_DIRS})

pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
include_directories(${WAYLAND_EGL_INCLUDE_DIRS})

pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
include_directories(${WAYLAND_PROTOCOLS_INCLUDE_DIRS})

pkg_get_variable(WAYLAND_PROTOCOLS_DATA_DIR wayland-protocols pkgdatadir)
message(STATUS "wayland protocols data dir: ${WAYLAND_PROTOCOLS_DATA_DIR}")

include(ExternalProject)
set(LIBCAIRO_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcairo-1.17.4)
set(LIBCAIRO_SHARED_LIB ${LIBCAIRO_PREFIX}/lib/libcairo.so.2.11704.0)
set(LIBCAIRO_INCLUDES ${LIBCAIRO_PREFIX}/include)
set(LIBCAIRO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/cairo)

file(MAKE_DIRECTORY ${LIBCAIRO_INCLUDES})

ExternalProject_Add(
  libcairo
  SOURCE_DIR ${LIBCAIRO_SOURCE}
  DOWNLOAD_COMMAND cd ${LIBCAIRO_SOURCE} && git clean -dfX && ${LIBCAIRO_SOURCE}/autogen.sh
  CONFIGURE_COMMAND ${LIBCAIRO_SOURCE}/configure 
    --prefix=${LIBCAIRO_PREFIX} 
    --enable-glesv3=yes 
    --enable-egl=yes 
    --enable-xlib=no
    --enable-xlib-xrender=no
    --enable-xlib-xcb=no
    --enable-xcb=no
    --enable-xcb-shm=no
    --enable-quartz=no
    --enable-win32=no
    --enable-script=no
    --enable-ft=no
    --enable-fc=no
    --enable-pdf=no
    --enable-ps=no
    --enable-svg=no
    --enable-gobject=no
    --enable-interpreter=no
    --enable-symbol-lookup=no
    --enable-full-testing=no
    --enable-trace=no
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  #BUILD_BYPRODUCTS ${LIBCAIRO_SHARED_LIB}
  BUILD_IN_SOURCE 1
)
add_library(cairo_1_17_4 SHARED IMPORTED GLOBAL)
add_dependencies(cairo_1_17_4 libcairo)
set_target_properties(cairo_1_17_4 PROPERTIES IMPORTED_LOCATION ${LIBCAIRO_SHARED_LIB})
set_target_properties(cairo_1_17_4 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIBCAIRO_INCLUDES})

set(PROTOCOLS_DIR "${CMAKE_SOURCE_DIR}/protocols")
set(WAYLAND_PROTOCOLS_NAME ${WAYLAND_PROTOCOLS_DATA_DIR}/stable/xdg-shell/xdg-shell.xml ${WAYLAND_PROTOCOLS_DATA_DIR}/unstable/xdg-output/xdg-output-unstable-v1.xml)
set(WLR_PROTOCOLS_NAME wlr-layer-shell-unstable-v1.xml)

set(PROTO_SOURCES "")
foreach(PROTO IN LISTS WAYLAND_PROTOCOLS_NAME WLR_PROTOCOLS_NAME)
  get_filename_component(PROTO_BASENAME ${PROTO} NAME_WE)
  message(STATUS "generate proto: ${PROTO}")
  execute_process(
    COMMAND wayland-scanner private-code ${PROTO} ${PROTO_BASENAME}-protocol.c
    COMMAND wayland-scanner client-header ${PROTO} ${PROTO_BASENAME}-client-protocol.h
    WORKING_DIRECTORY ${PROTOCOLS_DIR}
  )
  list(APPEND PROTO_SOURCES ${PROTOCOLS_DIR}/${PROTO_BASENAME}-protocol.c)
endforeach()

include_directories(${LIBCAIRO_INCLUDES} ${LIBCAIRO_INCLUDES}/cairo)

add_executable(wayab main.c render.c wl.c image.c ${PROTO_SOURCES})

target_link_libraries(
  wayab
  ${WAYLAND_EGL_LIBRARIES}
  ${WAYLAND_CLIENT_LIBRARIES}
  ${WAYLAND_PROTOCOLS_LIBRARIES}
  cairo_1_17_4
  EGL
  GLESv2
)
