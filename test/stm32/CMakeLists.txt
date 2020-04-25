set(SUBPROJ_NAME stm32Test)
set(TESTABLE_TARGET stm32)

set(${SUBPROJ_NAME}_CXX_STANDARD 20)
set(${SUBPROJ_NAME}_CXX_EXTENSIONS OFF)
set(${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED YES)

# Insert here your source files
set(${SUBPROJ_NAME}_HEADERS
        "test_math.hpp"
        "test_cpu.hpp"
        "test_memory.hpp"
        "test_system_control_registers.hpp"
        "utils.hpp")

set(${SUBPROJ_NAME}_SOURCES
        "test.cpp")

# ############################################################### #
# Set all target sources ######################################## #
# ############################################################### #

set(
        ${SUBPROJ_NAME}_ALL_SRCS
        ${${SUBPROJ_NAME}_HEADERS}
        ${${SUBPROJ_NAME}_SOURCES})

# ############################################################### #
# Create target for build ####################################### #
# ############################################################### #

add_executable(
        ${SUBPROJ_NAME}
        ${${SUBPROJ_NAME}_ALL_SRCS} utils.hpp)

# Enable C++17 on this project
set_target_properties(
        ${SUBPROJ_NAME} PROPERTIES
        CXX_STANDARD ${${SUBPROJ_NAME}_CXX_STANDARD}
        CXX_EXTENSIONS ${${SUBPROJ_NAME}_CXX_EXTENSIONS}
        CXX_STANDARD_REQUIRED ${${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED})

# Set specific properties
set_target_properties(
        ${SUBPROJ_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        OUTPUT_NAME "${SUBPROJ_NAME}$<$<CONFIG:Debug>:d>")

# Set include directories for this project and if it will use as installing
target_include_directories(
        ${SUBPROJ_NAME}
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(
        ${SUBPROJ_NAME}
        ${TESTABLE_TARGET})

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${SUBPROJ_NAME}
        GTest::gtest
        GTest::gtest_main)

add_test(
        NAME All${TESTABLE_TARGET}Tests
        COMMAND ${SUBPROJ_NAME})
