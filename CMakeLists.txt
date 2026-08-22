cmake_minimum_required(VERSION 3.16)

project(tfkiss VERSION 2.0.0 LANGUAGES C)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CTest)
include(GNUInstallDirs)

option(TFKISS_ENABLE_HIBAUD "Enable Linux baud rates above 38400" OFF)
option(TFKISS_ENABLE_XPID "Enable PID change support" OFF)
option(TFKISS_ENABLE_FLEXNET "Enable FLEXNET frame decoding" OFF)
option(TFKISS_ENABLE_AXIP "Enable AXIP support" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_tfkiss_bluetooth_default ON)
else()
    set(_tfkiss_bluetooth_default OFF)
endif()
option(TFKISS_ENABLE_BLUETOOTH "Enable Bluetooth RFCOMM support" ${_tfkiss_bluetooth_default})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    set(_tfkiss_bindir_default "${CMAKE_INSTALL_SBINDIR}")
    set(_tfkiss_confdir_default "${CMAKE_INSTALL_DATADIR}/tfkiss/conf")
    set(_tfkiss_docdir_default "${CMAKE_INSTALL_DATADIR}/tfkiss/doc")
    set(_tfkiss_workdir_default "/var/spool/tfkiss")
    set(_tfkiss_logdir_default "/var/log")
    set(_tfkiss_rundir_default "/var/run")
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    set(_tfkiss_bindir_default "tfkiss/sbin")
    set(_tfkiss_confdir_default "tfkiss")
    set(_tfkiss_docdir_default "tfkiss/doc")
    set(_tfkiss_workdir_default "${CMAKE_INSTALL_PREFIX}/tfkiss/work")
    set(_tfkiss_logdir_default "${CMAKE_INSTALL_PREFIX}/tfkiss/work")
    set(_tfkiss_rundir_default "${CMAKE_INSTALL_PREFIX}/tfkiss/work/bin")
else()
    message(FATAL_ERROR "${CMAKE_SYSTEM_NAME} is currently not supported")
endif()

set(TFKISS_INSTALL_BINDIR "${_tfkiss_bindir_default}" CACHE PATH
    "Installation directory for the tfkiss executable")
set(TFKISS_INSTALL_CONFDIR "${_tfkiss_confdir_default}" CACHE PATH
    "Installation directory for tfkiss configuration files")
set(TFKISS_INSTALL_DOCDIR "${_tfkiss_docdir_default}" CACHE PATH
    "Installation directory for tfkiss documentation")
set(TFKISS_WORK_DIR "${_tfkiss_workdir_default}" CACHE PATH "Default working directory")
set(TFKISS_LOG_DIR "${_tfkiss_logdir_default}" CACHE PATH "Default log directory")
set(TFKISS_RUN_DIR "${_tfkiss_rundir_default}" CACHE PATH "Default runtime directory")

if(IS_ABSOLUTE "${TFKISS_INSTALL_CONFDIR}")
    set(TFKISS_CONF_FULL_DIR "${TFKISS_INSTALL_CONFDIR}")
else()
    set(TFKISS_CONF_FULL_DIR "${CMAKE_INSTALL_PREFIX}/${TFKISS_INSTALL_CONFDIR}")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(HAVE_CRTSCTS 1)
    set(_tfkiss_platform_config "${CMAKE_CURRENT_SOURCE_DIR}/examples/linux.cfg")
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    set(HAVE_CRTSCTS 1)
    set(_tfkiss_platform_config "${CMAKE_CURRENT_SOURCE_DIR}/examples/netbsd.cfg")
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    set(_tfkiss_platform_config "${CMAKE_CURRENT_SOURCE_DIR}/examples/solaris.cfg")
endif()

if(TFKISS_ENABLE_HIBAUD AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "TFKISS_ENABLE_HIBAUD is supported only on Linux")
endif()

check_include_file(errno.h HAVE_ERRNO_H)
if(TFKISS_ENABLE_BLUETOOTH)
    check_include_files(
        "stdint.h;bluetooth/bluetooth.h;bluetooth/rfcomm.h"
        HAVE_BLUEZ_HEADERS
        LANGUAGE C
    )
    if(NOT HAVE_BLUEZ_HEADERS)
        message(FATAL_ERROR
            "Bluetooth support requires the BlueZ development headers; "
            "install them or configure with -DTFKISS_ENABLE_BLUETOOTH=OFF")
    endif()
endif()

set(USE_HIBAUD ${TFKISS_ENABLE_HIBAUD})
set(USE_XPID ${TFKISS_ENABLE_XPID})
set(USE_FLEXDEC ${TFKISS_ENABLE_FLEXNET})
set(USE_AXIP ${TFKISS_ENABLE_AXIP})
set(USE_BLUETOOTH ${TFKISS_ENABLE_BLUETOOTH})
set(DEF_INI_DIR "${TFKISS_CONF_FULL_DIR}")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/config.h"
)

set(tfkiss_sources
    src/l2a.c
    src/l2b.c
    src/l2c.c
    src/l2d.c
    src/l2e.c
    src/l2s.c
    src/l2v.c
    src/tfa.c
    src/tfb.c
    src/tfc.c
    src/tfd.c
    src/tfv.c
    src/crc.c
    src/init.c
    src/main.c
    src/process.c
    src/routing.c
)

add_executable(tfkiss ${tfkiss_sources})
set_target_properties(tfkiss PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src"
)
target_compile_options(tfkiss PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-funsigned-char>
)
target_include_directories(tfkiss PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    target_link_libraries(tfkiss PRIVATE nsl socket)
endif()

set(tfkiss_conf_dir "${TFKISS_CONF_FULL_DIR}")
set(tfkiss_log_dir "${TFKISS_LOG_DIR}")
set(tfkiss_run_dir "${TFKISS_RUN_DIR}")
file(READ "${_tfkiss_platform_config}" cfg)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/examples")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/tfkiss.ini.in"
    "${CMAKE_CURRENT_BINARY_DIR}/examples/tfkiss.ini"
    @ONLY
)

install(TARGETS tfkiss RUNTIME DESTINATION "${TFKISS_INSTALL_BINDIR}")
install(FILES
    doc/alas.eng
    doc/alas.txt
    doc/copyrght.txt
    doc/rfc1226
    doc/tfkiss.doc
    DESTINATION "${TFKISS_INSTALL_DOCDIR}"
)

set(TFKISS_GENERATED_INI "${CMAKE_CURRENT_BINARY_DIR}/examples/tfkiss.ini")
set(TFKISS_AXIP_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/examples/tfkiss.cfg")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/InstallConfig.cmake"
    @ONLY
)
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/InstallConfig.cmake")

if(BUILD_TESTING)
    add_test(NAME tfkiss_help COMMAND $<TARGET_FILE:tfkiss> --help)
    set_tests_properties(tfkiss_help PROPERTIES
        PASS_REGULAR_EXPRESSION "Usage : tfkiss"
    )
endif()
