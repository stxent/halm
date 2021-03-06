# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

project(halm C)
cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0069 NEW)
cmake_policy(SET CMP0079 NEW)

option(USE_LTO "Enable Link Time Optimization." OFF)
set(HALM_CONFIG_FILE "${PROJECT_SOURCE_DIR}/.config" CACHE STRING "Path to the config file.")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(ParseConfig)
parse_config("${HALM_CONFIG_FILE}" CONFIG_DEFS)

# Default compiler flags
set(FLAGS_DEFAULT "-fdata-sections -ffunction-sections -Wall -Wextra -Wshadow -pedantic")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_DEFAULT} ${FLAGS_CPU}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

find_package(xcore QUIET)
if(xcore_FOUND)
    message(STATUS "Found xcore package")
endif()

if(${USE_LTO})
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_COMPILE_OPTIONS_IPO -flto -ffat-lto-objects)
    endif()
endif()

if(DEFINED PLATFORM)
    set(PUBLIC_DEFS -D${PLATFORM})
elseif(UNIX)
    set(PUBLIC_DEFS -D_POSIX_C_SOURCE=200809L)
else()
    set(PUBLIC_DEFS "")
endif()

set(DIRECTORY_LIST core generic platform pm usb)
set(LIBRARY_OBJECTS "")
set(LIBRARY_TARGETS "")

foreach(ENTRY ${DIRECTORY_LIST})
    add_subdirectory(${ENTRY})
    if(TARGET halm_${ENTRY})
        list(APPEND LIBRARY_TARGETS halm_${ENTRY})
        list(APPEND LIBRARY_OBJECTS $<TARGET_OBJECTS:halm_${ENTRY}>)
        target_compile_definitions(halm_${ENTRY}
                PUBLIC ${PUBLIC_DEFS}
                PRIVATE ${CONFIG_DEFS}
        )
        target_include_directories(halm_${ENTRY} PUBLIC
                $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>
        )
        target_link_libraries(halm_${ENTRY}
                PUBLIC xcore
        )
        install(TARGETS halm_${ENTRY} EXPORT halm-targets)
    endif()
endforeach()

add_library(${PROJECT_NAME} ${LIBRARY_OBJECTS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_TARGETS})

# Library installation configuration
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/halm
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)
install(TARGETS ${PROJECT_NAME}
        EXPORT halm-targets
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
)
install(EXPORT halm-targets
        FILE halm-targets.cmake
        DESTINATION lib/halm
)
install(FILES "${PROJECT_SOURCE_DIR}/config.cmake.in"
        DESTINATION lib/halm
        RENAME halm-config.cmake
)
