cmake_minimum_required(VERSION 3.15)

# Project declaration
project(Open5066
    VERSION 0.5.0
    DESCRIPTION "NATO STANAG 5066 Protocol Stack Implementation"
    LANGUAGES C
)

# Require C17 standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build test suite" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Version and build info
set(VERSION_HEX 0x000005)
set(REL "0.5")

# Source directories
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)
set(CORE_DIR ${SRCDIR}/core)
set(PROTO_DIR ${SRCDIR}/protocols)
set(SERIAL_DIR ${SRCDIR}/serial)
set(INCLUDE_DIR ${SRCDIR}/include)
set(TOOLS_DIR ${SRCDIR}/tools)

# Include directories
include_directories(
    ${INCLUDE_DIR}
    ${CORE_DIR}
    ${SERIAL_DIR}
    ${PROTO_DIR}
)

# Compiler definitions
add_compile_definitions(
    _GNU_SOURCE
    _REENTRANT
    DEBUG
    S5066D=${VERSION_HEX}
    TEST_PING
    REL="${REL}"
)

# Platform-specific definitions
if(UNIX AND NOT APPLE)
    add_compile_definitions(LINUX)
    set(PLATFORM_LIBS pthread)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
    add_compile_definitions(SUNOS BYTE_ORDER=4321 BIG_ENDIAN=4321)
    set(PLATFORM_LIBS pthread xnet socket)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0 -fno-strict-aliasing")

# Security flags (enabled by default for Release builds)
set(SECURITY_FLAGS
    -Wall
    -Wextra
    -Wformat=2
    -Wformat-security
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)

# Warning flags (suppress warnings from legacy code patterns)
set(WARN_FLAGS
    -Wno-error
    -Wno-unused-label
    -Wno-unknown-pragmas
    -Wno-unused-value
    -Wno-format-nonliteral
    -Wno-missing-braces
    -Wno-missing-field-initializers
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-format-extra-args
    -Wno-sign-compare
    -Wno-return-type
    -Wno-implicit-function-declaration
    -Wno-builtin-declaration-mismatch
    -Wno-unused-but-set-variable
)

# Apply flags based on build type
add_compile_options(${SECURITY_FLAGS} ${WARN_FLAGS})

# PIE and RELRO flags for executables
if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    if(UNIX AND NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now")
    endif()
endif()

# Coverage support
if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# Sanitizers support
if(ENABLE_SANITIZERS)
    message(STATUS "Sanitizers enabled")
    add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

# Generate license.c
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/license.c
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/generate_license.sh ${CMAKE_CURRENT_BINARY_DIR}/license.c
    DEPENDS ${CMAKE_SOURCE_DIR}/COPYING
            ${CMAKE_SOURCE_DIR}/COPYING_sis5066_h
            ${CMAKE_SOURCE_DIR}/scripts/generate_license.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating license.c"
)

# Source files for s5066d
set(S5066D_SOURCES
    ${CORE_DIR}/s5066d.c
    ${CORE_DIR}/hiios.c
    ${CORE_DIR}/hiwrite.c
    ${CORE_DIR}/hiread.c
    ${CORE_DIR}/util.c
    ${PROTO_DIR}/sis.c
    ${PROTO_DIR}/dts.c
    ${PROTO_DIR}/smtp.c
    ${PROTO_DIR}/http.c
    ${PROTO_DIR}/testping.c
    ${SERIAL_DIR}/serial_sync.c
    ${SERIAL_DIR}/globalcounter.c
    ${CMAKE_CURRENT_BINARY_DIR}/license.c
)

# Main daemon executable
add_executable(s5066d ${S5066D_SOURCES})
target_link_libraries(s5066d ${PLATFORM_LIBS})

# synccat utility
add_executable(synccat
    ${SERIAL_DIR}/serial_sync.c
    ${SERIAL_DIR}/globalcounter.c
    ${TOOLS_DIR}/synccat.c
)
target_link_libraries(synccat ${PLATFORM_LIBS})

# iocat utility
add_executable(iocat
    ${SERIAL_DIR}/serial_sync.c
    ${SERIAL_DIR}/globalcounter.c
    ${TOOLS_DIR}/iocat.c
    ${CMAKE_CURRENT_BINARY_DIR}/license.c
)
target_link_libraries(iocat ${PLATFORM_LIBS})

# sizeof utility
add_executable(sizeof ${TOOLS_DIR}/sizeof.c)

# Installation
install(TARGETS s5066d synccat iocat sizeof
    RUNTIME DESTINATION sbin
    COMPONENT applications
)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Open5066 Configuration Summary")
message(STATUS "==============================")
message(STATUS "Version:          ${PROJECT_VERSION}")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard:       C${CMAKE_C_STANDARD}")
message(STATUS "Install prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Build tests:      ${BUILD_TESTS}")
message(STATUS "Code coverage:    ${ENABLE_COVERAGE}")
message(STATUS "Sanitizers:       ${ENABLE_SANITIZERS}")
message(STATUS "PIE enabled:      ${CMAKE_POSITION_INDEPENDENT_CODE}")
message(STATUS "")
