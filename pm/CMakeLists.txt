# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(SOURCE_FILES)
    add_library(pm OBJECT ${SOURCE_FILES})
endif()
