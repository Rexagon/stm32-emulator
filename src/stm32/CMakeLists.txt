set(SUBPROJ_NAME stm32)
set(${SUBPROJ_NAME}_NAMESPACE stm32)

set(${SUBPROJ_NAME}_CXX_STANDARD 20)
set(${SUBPROJ_NAME}_CXX_EXTENSIONS OFF)
set(${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED YES)

set(${SUBPROJ_NAME}_MAJOR_VERSION 0)
set(${SUBPROJ_NAME}_MINOR_VERSION 0)
set(${SUBPROJ_NAME}_PATCH_VERSION 1)

# Insert here your source files
set(${SUBPROJ_NAME}_HEADERS
        "cpu.hpp"
        "cpu_register_set.hpp"
        "math.hpp"
        "memory.hpp"
        "opcodes.hpp"
        "stm32.hpp"
        "system_control_registers.hpp"
        "utils.hpp")

set(${SUBPROJ_NAME}_SOURCES
        "cpu.cpp"
        "cpu_register_set.cpp"
        "memory.cpp"
        "stm32.cpp"
        "system_control_registers.cpp")

# ############################################################### #
# Options ####################################################### #
# ############################################################### #

include(OptionHelpers)
generate_basic_options_library(${SUBPROJ_NAME})

# ############################################################### #
# Library version ############################################### #
# ############################################################### #

set(${SUBPROJ_NAME}_VERSION
        ${${SUBPROJ_NAME}_MAJOR_VERSION}.${${SUBPROJ_NAME}_MINOR_VERSION}.${${SUBPROJ_NAME}_PATCH_VERSION})

# Set build type to library target
if (${SUBPROJ_NAME}_BUILD_SHARED)
    set(${SUBPROJ_NAME}_TARGET_TYPE "SHARED")
else ()
    set(${SUBPROJ_NAME}_TARGET_TYPE "STATIC")
endif ()

string(TOLOWER ${${SUBPROJ_NAME}_TARGET_TYPE} ${SUBPROJ_NAME}_TARGET_TYPE_LOWER)


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

# Library target
add_library(
        ${SUBPROJ_NAME}
        ${${SUBPROJ_NAME}_TARGET_TYPE}
        ${${SUBPROJ_NAME}_ALL_SRCS})

# Enable C++ standard
set_target_properties(
        ${SUBPROJ_NAME} PROPERTIES
        CXX_STANDARD ${${SUBPROJ_NAME}_CXX_STANDARD}
        CXX_EXTENSIONS ${${SUBPROJ_NAME}_CXX_EXTENSIONS}
        CXX_STANDARD_REQUIRED ${${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED})

set_target_properties(
        ${SUBPROJ_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        OUTPUT_NAME "${SUBPROJ_NAME}$<$<CONFIG:Debug>:d>")

target_include_directories(
        ${SUBPROJ_NAME}
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>)

target_compile_options(${SUBPROJ_NAME} PUBLIC
        -O2
        -W
        -Wall
        -Wconversion
        -Wctor-dtor-privacy
        -Weffc++
        -Wenum-compare
        -Werror
        -Wextra
        -Wfloat-equal
        -Wlogical-op
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wshadow
        -Wsign-conversion
        -Wstrict-overflow=2
        -Wunreachable-code)

if (MINGW)
    # Fix struct packing
    target_compile_options(${SUBPROJ_NAME} PUBLIC -mno-ms-bitfields)
endif ()

# ############################################################### #
# Installing #################################################### #
# ############################################################### #

# Create export targets
install(
        TARGETS ${SUBPROJ_NAME}
        EXPORT ${SUBPROJ_NAME}-targets)

# Install headers
install(
        FILES ${${SUBPROJ_NAME}_HEADERS}
        DESTINATION ${${SUBPROJ_NAME}_INSTALL_INCLUDE_PREFIX})

# Set out paths
install(
        TARGETS ${SUBPROJ_NAME}
        RUNTIME DESTINATION ${${SUBPROJ_NAME}_INSTALL_BIN_PREFIX}
        ARCHIVE DESTINATION ${${SUBPROJ_NAME}_INSTALL_LIB_PREFIX}
        LIBRARY DESTINATION ${${SUBPROJ_NAME}_INSTALL_LIB_PREFIX})

set(SUBPROJ_TARGETS_FILE "${SUBPROJ_NAME}-${${SUBPROJ_NAME}_TARGET_TYPE_LOWER}-targets.cmake")

# Create config-targets cmake file
install(
        EXPORT ${SUBPROJ_NAME}-targets
        FILE ${SUBPROJ_TARGETS_FILE}
        NAMESPACE ${${SUBPROJ_NAME}_NAMESPACE}::
        DESTINATION ${${SUBPROJ_NAME}_INSTALL_CMAKE_PREFIX})

# Create config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${SUBPROJ_NAME}-config-version.cmake"
        VERSION ${cmake-test-library_VERSION}
        COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
        "${PROJECT_ROOT_DIR}/cmake/${SUBPROJ_NAME}-config.cmake.in"
        "${PROJECT_BINARY_DIR}/${SUBPROJ_NAME}-config.cmake"
        INSTALL_DESTINATION ${${SUBPROJ_NAME}_INSTALL_CMAKE_PREFIX})

# Install config files
install(
        FILES
        "${PROJECT_BINARY_DIR}/${SUBPROJ_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${SUBPROJ_NAME}-config-version.cmake"
        DESTINATION ${${SUBPROJ_NAME}_INSTALL_CMAKE_PREFIX})
