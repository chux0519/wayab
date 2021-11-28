cmake_minimum_required(VERSION 3.10)

project(wayab LANGUAGES C)

# AddressSanitize
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
    FORCE)


find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
include_directories(${WAYLAND_CLIENT_INCLUDE_DIRS})

pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
include_directories(${WAYLAND_EGL_INCLUDE_DIRS})

pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
include_directories(${WAYLAND_PROTOCOLS_INCLUDE_DIRS})

pkg_get_variable(WAYLAND_PROTOCOLS_DATA_DIR wayland-protocols pkgdatadir)
message(STATUS "wayland protocols data dir: ${WAYLAND_PROTOCOLS_DATA_DIR}")

pkg_check_modules(CAIRO REQUIRED cairo)
include_directories(${CAIRO_INCLUDE_DIRS})


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

add_executable(wayab main.c render.c wl.c image.c ${PROTO_SOURCES})

target_link_libraries(
  wayab
  ${WAYLAND_EGL_LIBRARIES}
  ${WAYLAND_CLIENT_LIBRARIES}
  ${WAYLAND_PROTOCOLS_LIBRARIES}
  ${CAIRO_LIBRARIES}
  ${GDKPIXBUF_LIBRARIES}
  EGL
  GLESv2
)
