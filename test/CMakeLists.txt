CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(acc_test C)
SET(SRCS acc_test.c)
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED eina elementary ecore-x appcore-efl aul capi-system-usb-accessory)
FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden")
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -g ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
ADD_EXECUTABLE(acc_test acc_test.c)
TARGET_LINK_LIBRARIES(acc_test ${pkgs_LDFLAGS} capi-system-usb-accessory)
INSTALL(TARGETS acc_test DESTINATION /opt/apps/acc_test/bin)

# install manifest file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/acc_test.xml DESTINATION /opt/share/packages/)
