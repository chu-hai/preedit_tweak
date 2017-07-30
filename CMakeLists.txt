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
set(PACKAGE_VERSION_MINOR 2)
set(PACKAGE_VERSION_BUILD 0)
set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_BUILD}")

option(ENABLE_NLS "Enable National language support." ON)

set(PLUGIN_SOURCES
	src/preedit_tweak.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GEANY geany>=1.27)
if(NOT GEANY_FOUND)
	message(FATAL_ERROR "${RED}Geany or required packages are not installed.${RESET}")
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

if(NOT PACKAGE_SHARE_DIR)
    set(PACKAGE_SHARE_DIR   "${CMAKE_INSTALL_PREFIX}/share")
endif()

if(ENABLE_NLS)
    set(GETTEXT_PACKAGE "geany_plugin_${PACKAGE}")
    if(NOT PACKAGE_LOCALE_DIR)
        set(PACKAGE_LOCALE_DIR  "${PACKAGE_SHARE_DIR}/locale")
    endif()
    add_subdirectory(po)
endif()

configure_file(
	"${PROJECT_SOURCE_DIR}/src/config.h.in"
	"${PROJECT_BINARY_DIR}/src/config.h"
)
include_directories("${PROJECT_BINARY_DIR}/src")

find_program(CMD_AWK_EXECUTABLE awk)
find_program(CMD_CUT_EXECUTABLE cut)
if(CMD_AWK_EXECUTABLE AND CMD_CUT_EXECUTABLE)
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --print-requires geany
                    COMMAND ${CMD_AWK_EXECUTABLE} "/^gtk\\+-/{print}"
                    COMMAND ${CMD_AWK_EXECUTABLE} "{print $3}"
                    COMMAND ${CMD_CUT_EXECUTABLE} -d. -f1
                    OUTPUT_VARIABLE _used_gtk_version OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(USED_GTK_VERSION "(GTK${_used_gtk_version})")
endif()

message("")
message(STATUS "${GREEN}Geany Version:        ${YELLOW}${GEANY_VERSION} ${USED_GTK_VERSION}${RESET}")
message("")
message(STATUS "${WHITE}ENABLE_NLS:           ${CYAN}${ENABLE_NLS}${RESET}")
message("")
message(STATUS "${WHITE}CMAKE_INSTALL_PREFIX: ${YELLOW}${CMAKE_INSTALL_PREFIX}${RESET}")
message(STATUS "${WHITE}PACKAGE_PLUGIN_DIR:   ${YELLOW}${PACKAGE_PLUGIN_DIR}${RESET}")
message(STATUS "${WHITE}PACKAGE_SHARE_DIR:    ${YELLOW}${PACKAGE_SHARE_DIR}${RESET}")
message(STATUS "${WHITE}PACKAGE_LOCALE_DIR:   ${YELLOW}${PACKAGE_LOCALE_DIR}${RESET}")
message("")
message(STATUS "${WHITE}GETTEXT_PACKAGE:      ${YELLOW}${GETTEXT_PACKAGE}${RESET}")
message("")
