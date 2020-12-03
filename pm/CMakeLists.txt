# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

if(${CONFIG_PM})
    list(APPEND SOURCE_FILES "pm.c")
endif()

if(SOURCE_FILES)
    add_library(halm_pm OBJECT ${SOURCE_FILES})
endif()
