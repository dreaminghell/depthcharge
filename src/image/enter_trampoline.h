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

#ifndef __IMAGE_ENTER_TRAMPOLINE_H__
#define __IMAGE_ENTER_TRAMPOLINE_H__

#include "base/elf.h"

void enter_trampoline(Elf32_Ehdr *ehdr);
void tramp_load_elf(Elf32_Ehdr *ehdr, void *param);

#endif /* __IMAGE_ENTER_TRAMPOLINE_H__ */
