#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

set(SOURCE_FILES "")
set(SOURCE_FILES_LIST "")

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "pm.c")
endif()

prepend(SOURCE_FILES_LIST "${PROJECT_SOURCE_DIR}/pm" ${SOURCE_FILES})
list(APPEND LIBRARY_SOURCES ${SOURCE_FILES_LIST})
