cmake_minimum_required(VERSION 3.13)

set(_PROJECT_NAME          stm32-emulator)
set(_PROJECT_LANGUAGE      CXX)

set(_PROJECT_MAJOR_VERSION 0)
set(_PROJECT_MINOR_VERSION 0)
set(_PROJECT_PATCH_VERSION 1)

set(SUBPROJECT_LIST
    "src/app"
    "src/stm32"
    )
set(TEST_LIST
    "test/app"
    "test/stm32"
    )

# Cmake module path
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_ROOT_DIR}/cmake/modules")

set(_PROJECT_VERSION
  ${_PROJECT_MAJOR_VERSION}.${_PROJECT_MINOR_VERSION}.${_PROJECT_PATCH_VERSION})

project(${_PROJECT_NAME} LANGUAGES ${_PROJECT_LANGUAGE} VERSION ${_PROJECT_VERSION})

foreach(SUBPROJ ${SUBPROJECT_LIST})
    add_subdirectory(${SUBPROJ})
endforeach()

enable_testing()
foreach(TEST ${TEST_LIST})
    string(REGEX REPLACE "^test\/" "" TEST_NAME ${TEST})
    if(${${TEST_NAME}_BUILD_TESTS})
        add_subproject(${TEST})
    endif()
endforeach()