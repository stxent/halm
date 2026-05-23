/*
 * startup.S
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

.section .startup

.globl vector_trampoline
vector_trampoline:
    j start_entry
    .align 2

.globl trap_trampoline
trap_trampoline:
    j commonIrqHandler
    .align 2

.globl commonIrqHandler
.type commonIrqHandler,@function

.globl start_entry
.type start_entry,@function

start_entry:
    /* Initialize GP and SP */
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    la sp, _stack

    call coreStartup
    call platformStartup
    call main
