# Tests CMakeLists.txt for Open5066
# Integrates Unity framework with CTest

# Unity test framework
set(UNITY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit)
set(UNITY_SRC ${UNITY_DIR}/unity.c)

# Include directories for tests
include_directories(
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/serial
    ${CMAKE_SOURCE_DIR}/src/protocols
    ${UNITY_DIR}
)

# Helper function to create unit tests
function(add_unit_test test_name)
    add_executable(${test_name}
        unit/${test_name}.c
        ${UNITY_SRC}
    )
    target_link_libraries(${test_name} pthread)

    # Add to CTest
    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Set test properties
    set_tests_properties(${test_name} PROPERTIES
        LABELS "unit"
        TIMEOUT 30
    )
endfunction()

# Discover and add all unit tests
file(GLOB UNIT_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/test_*.c")
foreach(test_source ${UNIT_TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_unit_test(${test_name})
endforeach()

# Security tests
add_executable(test_protocol_security
    security/test_protocol_security.c
    ${UNITY_SRC}
)
target_link_libraries(test_protocol_security pthread)

add_test(
    NAME test_protocol_security
    COMMAND test_protocol_security
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_tests_properties(test_protocol_security PROPERTIES
    LABELS "security"
    TIMEOUT 30
)

# Integration tests (shell script based)
add_test(
    NAME integration_tests
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/integration/test_protocol_integration.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/integration
)

set_tests_properties(integration_tests PROPERTIES
    LABELS "integration"
    TIMEOUT 60
    DEPENDS s5066d
)

# Custom targets for running specific test groups
add_custom_target(test-unit
    COMMAND ${CMAKE_CTEST_COMMAND} -L unit --output-on-failure
    DEPENDS ${UNIT_TEST_BINS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running unit tests"
)

add_custom_target(test-security
    COMMAND ${CMAKE_CTEST_COMMAND} -L security --output-on-failure
    DEPENDS test_protocol_security
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running security tests"
)

add_custom_target(test-integration
    COMMAND ${CMAKE_CTEST_COMMAND} -L integration --output-on-failure
    DEPENDS s5066d
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running integration tests"
)

# Coverage target (if enabled)
if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)

    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
            COMMAND ${LCOV} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info '/usr/*' --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
            COMMAND ${GENHTML} ${CMAKE_BINARY_DIR}/coverage/coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage/html
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report: ${CMAKE_BINARY_DIR}/coverage/html/index.html"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    endif()
endif()

# Print test configuration
message(STATUS "Test Configuration:")
message(STATUS "  Unity framework: ${UNITY_DIR}")
message(STATUS "  Unit tests:      ${UNIT_TEST_SOURCES}")
message(STATUS "  Security tests:  Enabled")
message(STATUS "  Integration:     Enabled")
