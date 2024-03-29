/*
 * Copyright 2012 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __IMAGE_SYMBOLS_H__
#define __IMAGE_SYMBOLS_H__

#include <stdint.h>

// C level variable definitions for symbols defined in the linker script.

extern uint8_t _start;
extern uint8_t _gbb_copy_start;
extern uint8_t _gbb_copy_end;
extern uint8_t _edata;
extern uint8_t _heap;
extern uint8_t _eheap;
extern uint8_t _estack;
extern uint8_t _stack;
extern uint8_t _exc_estack;
extern uint8_t _exc_stack;
extern uint8_t _end;
extern uint8_t _tramp_start;
extern uint8_t _tramp_end;
extern uint8_t _tramp_stack;
extern uint8_t _tramp_estack;
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;
extern uint8_t _init_funcs_start;
extern uint8_t _init_funcs_end;

#define CPARAMS __attribute__((section(".cparams")))
#define SHARED_DATA __attribute__((section(".shared_data")))
#define GBB_COPY __attribute__((section(".gbb_copy")))

#endif /* __IMAGE_SYMBOLS_H__ */
