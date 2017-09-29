#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

project(halm C)
cmake_minimum_required(VERSION 3.6)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build: Debug Release." FORCE)
endif()

option(USE_LTO "Enable Link Time Optimization." OFF)

set(PATH_XCORE "${PROJECT_SOURCE_DIR}/../xcore" CACHE STRING "Path to the XCORE library.")
set(CONFIG_FILE "${PROJECT_SOURCE_DIR}/.config" CACHE STRING "Path to the config file.")

file(STRINGS ${CONFIG_FILE} CONFIG_DATA)
set(FLAGS_LIBRARY "")

foreach(ENTRY ${CONFIG_DATA})
    string(FIND "${ENTRY}" "CONFIG_" ENTRY_IS_CONFIG)
    if(${ENTRY_IS_CONFIG} EQUAL 0)
        string(FIND "${ENTRY}" "=y" ENTRY_IS_BOOL)
        if(NOT ${ENTRY_IS_BOOL} EQUAL -1)
            string(REPLACE "=y" "" STRIPPED_ENTRY "${ENTRY}")
            set(${STRIPPED_ENTRY} ON)
            set(FLAGS_LIBRARY "${FLAGS_LIBRARY} -D${STRIPPED_ENTRY}")
        else()
            string(REPLACE "\"" "" CLEANED_ENTRY "${ENTRY}")
            string(REPLACE "=" ";" SPLITTED_ENTRY "${CLEANED_ENTRY}")
            list(GET SPLITTED_ENTRY 0 ENTRY_NAME)
            list(GET SPLITTED_ENTRY 1 ENTRY_VALUE)
            set(${ENTRY_NAME} ${ENTRY_VALUE})
            set(FLAGS_LIBRARY "${FLAGS_LIBRARY} -D${ENTRY_NAME}=${${ENTRY_NAME}}")
        endif()
    endif()
endforeach()

#Gather files for the library
function(prepend FUNCTION_RESULT STRING_PREFIX)
    set(RESULT_LIST "")
    foreach(ENTRY ${ARGN})
        list(APPEND RESULT_LIST "${STRING_PREFIX}/${ENTRY}")
    endforeach()
    set(${FUNCTION_RESULT} "${RESULT_LIST}" PARENT_SCOPE)
endfunction()

set(LIBRARY_SOURCES "")
include("${PROJECT_SOURCE_DIR}/core/CMakeLists.txt")
include("${PROJECT_SOURCE_DIR}/generic/CMakeLists.txt")
include("${PROJECT_SOURCE_DIR}/platform/CMakeLists.txt")
include("${PROJECT_SOURCE_DIR}/pm/CMakeLists.txt")
include("${PROJECT_SOURCE_DIR}/usb/CMakeLists.txt")

#Default compiler flags
set(FLAGS_DEFAULT "-fdata-sections -ffunction-sections -fmessage-length=0 -Wall -Wextra -Wshadow -pedantic")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_DEFAULT} ${FLAGS_CPU} ${FLAGS_PLATFORM} ${FLAGS_LIBRARY}")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

if(${USE_LTO})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -ffat-lto-objects")
endif()

include_directories("${PATH_XCORE}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(${PROJECT_NAME} ${LIBRARY_SOURCES})
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
