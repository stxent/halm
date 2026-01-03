# Copyright (C) 2017 xent
# Project is distributed under the terms of the MIT License

string(FIND ${CMAKE_SYSTEM_PROCESSOR} "cortex" CPU_IS_CORTEX)
string(FIND ${CMAKE_SYSTEM_PROCESSOR} "risc" CPU_IS_RISC)

if(${CPU_IS_CORTEX} EQUAL 0)
    add_subdirectory(cortex)
elseif(${CPU_IS_RISC} EQUAL 0)
    # Among RISC architectures only RISC-V is supported
    add_subdirectory(riscv)
else()
    # Default architecture is x86
    add_subdirectory(x86)
endif()
