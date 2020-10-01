# Copyright (C) 2020 xent
# Project is distributed under the terms of the GNU General Public License v3.0

find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARY NAMES uv libuv)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUV
        REQUIRED_VARS
        LIBUV_LIBRARY
        LIBUV_INCLUDE_DIR
)

# Set output variables
if(LIBUV_FOUND)
    set(LIBUV_INCLUDE_DIRS "${LIBUV_INCLUDE_DIR}")
    set(LIBUV_LIBRARIES "${LIBUV_LIBRARY}")
endif()

# Make internal variables invisible
mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)
