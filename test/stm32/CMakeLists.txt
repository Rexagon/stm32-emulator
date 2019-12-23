set(SUBPROJ_NAME                          stm32Test)
set(TESTABLE_TARGET                       stm32)

set(${SUBPROJ_NAME}_CXX_STANDARD          17)
set(${SUBPROJ_NAME}_CXX_EXTENSIONS        OFF)
set(${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED YES)

# Insert here your source files
set(${SUBPROJ_NAME}_HEADERS
    "")

set(${SUBPROJ_NAME}_SOURCES
    "")

# ############################################################### #
# Options ####################################################### #
# ############################################################### #

# Insert here your specififc options for build:
# .............................................

# ############################################################### #
# Set all target sources ######################################## #
# ############################################################### #

# Include file with all source and header files
include(${SUBPROJ_NAME}Files)

set(
    ${SUBPROJ_NAME}_HEADERS
    ${HEADERS})

set(
    ${SUBPROJ_NAME}_SOURCES
    ${SOURCES})

# Unset temp variable
unset(HEADERS)
unset(SOURCES)

set(
    ${SUBPROJ_NAME}_ALL_SRCS
    ${${SUBPROJ_NAME}_HEADERS}
    ${${SUBPROJ_NAME}_SOURCES})

# ############################################################### #
# Create target for build ####################################### #
# ############################################################### #

add_executable(
    ${SUBPROJ_NAME}
    ${${SUBPROJ_NAME}_ALL_SRCS})

# Enable C++17 on this project
set_target_properties(
    ${SUBPROJ_NAME} PROPERTIES
    CXX_STANDARD          ${${SUBPROJ_NAME}_CXX_STANDARD}
    CXX_EXTENSIONS        ${${SUBPROJ_NAME}_CXX_EXTENSIONS}
    CXX_STANDARD_REQUIRED ${${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED})

# Set specific properties
set_target_properties(
    ${SUBPROJ_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    OUTPUT_NAME              "${SUBPROJ_NAME}$<$<CONFIG:Debug>:d>")

# Set include directories for this project and if it will use as installing
target_include_directories(
    ${SUBPROJ_NAME}
    PRIVATE   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(
    ${SUBPROJ_NAME}
    ${TESTABLE_TARGET})

add_test(
    NAME All${TESTABLE_TARGET}Tests
    COMMAND ${SUBPROJ_NAME})