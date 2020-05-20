set(SUBPROJ_NAME app)

set(${SUBPROJ_NAME}_CXX_STANDARD 17)
set(${SUBPROJ_NAME}_CXX_EXTENSIONS OFF)
set(${SUBPROJ_NAME}_CXX_STANDARD_REQUIRED YES)

set(${SUBPROJ_NAME}_MAJOR_VERSION 0)
set(${SUBPROJ_NAME}_MINOR_VERSION 0)
set(${SUBPROJ_NAME}_PATCH_VERSION 1)

# Insert here your source files
set(${SUBPROJ_NAME}_HEADERS
        "utils/general.hpp"
        "models/assembly_view_model.hpp"
        "models/settings.hpp"
        "widgets/assembly_view.hpp"
        "widgets/custom_splitter.hpp"
        "widgets/hex_view.hpp"
        "widgets/main_menu.hpp"
        "widgets/main_toolbar.hpp"
        "widgets/memory_view.hpp"
        "widgets/registers_view.hpp"
        "windows/main_window.hpp"
        "application.hpp")

set(${SUBPROJ_NAME}_SOURCES
        "main.cpp"
        "models/assembly_view_model.cpp"
        "models/settings.cpp"
        "widgets/assembly_view.cpp"
        "widgets/custom_splitter.cpp"
        "widgets/hex_view.cpp"
        "widgets/main_menu.cpp"
        "widgets/registers_view.cpp"
        "widgets/memory_view.cpp"
        "widgets/main_toolbar.cpp"
        "windows/main_window.cpp"
        "application.cpp")

qt5_add_resources(RESOURCES ${PROJECT_ROOT_DIR}/res/app.qrc)

# ############################################################### #
# Options ####################################################### #
# ############################################################### #

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(OptionHelpers)
generate_basic_options_executable(${SUBPROJ_NAME})

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
        ${${SUBPROJ_NAME}_ALL_SRCS}
        ${RESOURCES})

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

# Set version
set_target_properties(
        ${SUBPROJ_NAME} PROPERTIES
        VERSION ${${SUBPROJ_NAME}_MAJOR_VERSION}.${${SUBPROJ_NAME}_MINOR_VERSION}.${${SUBPROJ_NAME}_PATCH_VERSION})

# Set include directories for this project and if it will use as installing
target_include_directories(
        ${SUBPROJ_NAME}
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${SUBPROJ_NAME} PRIVATE stm32 Qt5::Core Qt5::Widgets)

# ############################################################### #
# Installing #################################################### #
# ############################################################### #

# Create export targets
install(
        TARGETS ${SUBPROJ_NAME}
        EXPORT ${SUBPROJ_NAME}-targets)

# Set out paths
install(
        TARGETS ${SUBPROJ_NAME}
        RUNTIME DESTINATION ${${SUBPROJ_NAME}_INSTALL_BIN_PREFIX}
        ARCHIVE DESTINATION ${${SUBPROJ_NAME}_INSTALL_LIB_PREFIX}
        LIBRARY DESTINATION ${${SUBPROJ_NAME}_INSTALL_LIB_PREFIX})
