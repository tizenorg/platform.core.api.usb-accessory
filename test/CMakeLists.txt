CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(acc_test C)
SET(SRCS acc_test.c)
pkg_check_modules(pkgs REQUIRED eina elementary ecore-x appcore-efl aul capi-system-usb-accessory)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden")
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -g ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
ADD_EXECUTABLE(acc_test acc_test.c)
TARGET_LINK_LIBRARIES(acc_test ${pkgs_LDFLAGS} capi-system-usb-accessory)
INSTALL(TARGETS acc_test DESTINATION /opt/apps/acc_test/bin)

# install desktop file
CONFIGURE_FILE(acc_test.desktop.in ${CMAKE_CURRENT_SOURCE_DIR}/acc_test.desktop)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/acc_test.desktop DESTINATION /opt/share/applications/)
