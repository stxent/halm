# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

string(FIND ${CMAKE_SYSTEM_PROCESSOR} "cortex" CPU_IS_CORTEX)
string(FIND ${CMAKE_SYSTEM_PROCESSOR} "x86" CPU_IS_X86)

if(${CPU_IS_CORTEX} EQUAL 0)
    add_subdirectory(cortex)
elseif(${CPU_IS_X86} EQUAL 0)
    add_subdirectory(x86)
endif()
