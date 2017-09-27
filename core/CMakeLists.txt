#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

string(FIND "${CMAKE_SYSTEM_PROCESSOR}" "cortex" CPU_IS_CORTEX)
string(FIND "${CMAKE_SYSTEM_PROCESSOR}" "x86" CPU_IS_X86)

if(${CPU_IS_CORTEX} EQUAL 0)
    include("${PROJECT_SOURCE_DIR}/core/cortex/CMakeLists.txt")
elseif(${CPU_IS_X86} EQUAL 0)
    include("${PROJECT_SOURCE_DIR}/core/x86/CMakeLists.txt")
endif()
