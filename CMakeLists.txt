cmake_minimum_required(VERSION 3.0.2)
project(preedit_tweak)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(FATAL_ERROR "This operating system is not supported. [${CMAKE_SYSTEM_NAME}]")
else()
	string(ASCII 27 _escape)
	set(GRAY	"${_escape}[1;30m")
	set(RED		"${_escape}[1;31m")
	set(GREEN	"${_escape}[1;32m")
	set(YELLOW	"${_escape}[1;33m")
	set(BLUE	"${_escape}[1;34m")
	set(MAGENTA	"${_escape}[1;35m")
	set(CYAN	"${_escape}[1;36m")
	set(WHITE	"${_escape}[1;37m")
	set(RESET	"${_escape}[0m")
endif()

set(PACKAGE "preedit_tweak")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 1)
set(PACKAGE_VERSION_BUILD 0)
set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_BUILD}")

set(PLUGIN_SOURCES
	src/preedit_tweak.c
)

configure_file(
	"${PROJECT_SOURCE_DIR}/src/config.h.in"
	"${PROJECT_BINARY_DIR}/src/config.h"
)
include_directories("${PROJECT_BINARY_DIR}/src")

find_package(PkgConfig)
pkg_check_modules(GEANY geany)
if(NOT GEANY_FOUND)
	message(FATAL_ERROR "${RED}Geany is not installed.${NONE}")
endif()
include_directories(${GEANY_INCLUDE_DIRS})
add_definitions(${GEANY_CFLAGS_OTHER})
if(NOT PACKAGE_PLUGIN_DIR)
	set(PACKAGE_PLUGIN_DIR "${GEANY_LIBDIR}/geany")
endif()

add_library(${PACKAGE} MODULE ${PLUGIN_SOURCES})
target_link_libraries(${PACKAGE} ${GEANY_LIBRARIES})
set_target_properties(${PACKAGE} PROPERTIES PREFIX "")

install(
	TARGETS		${PACKAGE}
	DESTINATION	"${PACKAGE_PLUGIN_DIR}"
)

message("")
message(STATUS "${WHITE}CMAKE_INSTALL_PREFIX: ${YELLOW}${CMAKE_INSTALL_PREFIX}${RESET}")
message(STATUS "${WHITE}PACKAGE_PLUGIN_DIR:   ${YELLOW}${PACKAGE_PLUGIN_DIR}${RESET}")
message("")
