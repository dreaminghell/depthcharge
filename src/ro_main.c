/*
 * Copyright (c) 2012 The Chromium OS Authors.
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <libpayload.h>
#include <stdint.h>
#include <vboot_api.h>

#include "base/commonparams.h"
#include "base/dc_image.h"
#include "base/flag.h"
#include "base/fmap.h"
#include "base/memory.h"
#include "base/startrw.h"
#include "base/timestamp.h"
#include "boot/commandline.h"
#include "boot/zimage.h"
#include "drivers/ahci.h"
#include "drivers/ec/chromeos/mkbp.h"
#include "drivers/power_management.h"

#define CMD_LINE_SIZE 4096

static char cmd_line_buf[2 * CMD_LINE_SIZE];

static int vboot_init(int dev_switch, int rec_switch,
		      int wp_switch, int oprom_loaded)
{
	VbInitParams iparams = {
		.flags = 0
	};

	// Decide what flags to pass into VbInit.
	if (dev_switch)
		iparams.flags |= VB_INIT_FLAG_DEV_SWITCH_ON;
	if (rec_switch)
		iparams.flags |= VB_INIT_FLAG_REC_BUTTON_PRESSED;
	if (wp_switch)
		iparams.flags |= VB_INIT_FLAG_WP_ENABLED;
	if (oprom_loaded)
		iparams.flags |= VB_INIT_FLAG_OPROM_LOADED;
	iparams.flags |= VB_INIT_FLAG_OPROM_MATTERS;
	iparams.flags |= VB_INIT_FLAG_RO_NORMAL_SUPPORT;

	printf("Calling VbInit().\n");
	VbError_t res = VbInit(&cparams, &iparams);

	if (res == VBERROR_VGA_OPROM_MISMATCH) {
		printf("Doing a cold reboot.\n");
		cold_reboot();
	}
	if (res != VBERROR_SUCCESS)
		return 1;

	if (iparams.out_flags && VB_INIT_OUT_CLEAR_RAM) {
		if (wipe_unused_memory())
			return 1;
	}
	return 0;
};

static int vboot_select_firmware(void)
{
	VbSelectFirmwareParams fparams = {
		.verification_block_A = NULL,
		.verification_block_B = NULL,
		.verification_size_A = 0,
		.verification_size_B = 0
	};

	// Set up the fparams structure.
	FmapArea *vblock_a = fmap_find_area(main_fmap, "VBLOCK_A");
	FmapArea *vblock_b = fmap_find_area(main_fmap, "VBLOCK_B");
	if (!vblock_a || !vblock_b) {
		printf("Couldn't find one of the vblocks.\n");
		return 1;
	}
	fparams.verification_block_A = \
		(void *)(vblock_a->area_offset + main_rom_base);
	fparams.verification_size_A = vblock_a->area_size;
	fparams.verification_block_B = \
		(void *)(vblock_b->area_offset + main_rom_base);
	fparams.verification_size_B = vblock_b->area_size;

	printf("Calling VbSelectFirmware().\n");
	VbError_t res = VbSelectFirmware(&cparams, &fparams);
	if (res != VBERROR_SUCCESS)
		return 1;

	// If an RW firmware was selected, start it.
	if (fparams.selected_firmware == VB_SELECT_FIRMWARE_A ||
	    fparams.selected_firmware == VB_SELECT_FIRMWARE_B) {
		FmapArea *rw_area;
		if (fparams.selected_firmware == VB_SELECT_FIRMWARE_A)
			rw_area = fmap_find_area(main_fmap, "FW_MAIN_A");
		else
			rw_area = fmap_find_area(main_fmap, "FW_MAIN_B");
		uintptr_t rw_addr = rw_area->area_offset + main_rom_base;
		if (start_rw_firmware((void *)rw_addr))
			return 1;
	}
	return 0;
}

static int vboot_select_and_load_kernel(void)
{
	static char cmd_line_temp[CMD_LINE_SIZE];

	VbSelectAndLoadKernelParams kparams = {
		.kernel_buffer = (void *)0x100000,
		.kernel_buffer_size = 0x800000
	};

	printf("Calling VbSelectAndLoadKernel().\n");
	if (VbSelectAndLoadKernel(&cparams, &kparams) != VBERROR_SUCCESS) {
		printf("Doing a cold reboot.\n");
		cold_reboot();
	}

	uintptr_t params_addr =
		kparams.bootloader_address - sizeof(struct boot_params);
	struct boot_params *params = (struct boot_params *)params_addr;
	uintptr_t cmd_line_addr = params_addr - CMD_LINE_SIZE;
	strncpy(cmd_line_temp, "cros_secure ", CMD_LINE_SIZE);
	strncat(cmd_line_temp, (char *)cmd_line_addr, CMD_LINE_SIZE);

	if (commandline_subst(cmd_line_temp, 0,
			      kparams.partition_number + 1,
			      kparams.partition_guid,
			      cmd_line_buf,
			      sizeof(cmd_line_buf)))
		return 1;

	if (zboot(params, cmd_line_buf, kparams.kernel_buffer))
		return 1;

	return 0;
}

int main(void)
{
	// Let the world know we're alive.
	outb(0xaa, 0x80);

	// Initialize some consoles.
	serial_init();
	cbmem_console_init();

	printf("\n\nStarting read-only depthcharge...\n");

	get_cpu_speed();
	timestamp_init();

	if (fmap_init()) {
		printf("Problem with the FMAP.\n");
		halt();
	}

	// Set up the common param structure.
	memset(&cparams, 0, sizeof(cparams));
	memset(shared_data_blob, 0, sizeof(shared_data_blob));

	FmapArea *gbb_area = fmap_find_area(main_fmap, "GBB");
	if (!gbb_area) {
		printf("Couldn't find the GBB.\n");
		halt();
	}

	cparams.gbb_data =
		(void *)(uintptr_t)(gbb_area->area_offset + main_rom_base);
	cparams.gbb_size = gbb_area->area_size;

	cparams.shared_data_blob = shared_data_blob;
	cparams.shared_data_size = sizeof(shared_data_blob);

	// Initialize vboot.
	int dev_switch = flag_fetch(FLAG_DEVSW);
	int rec_switch = flag_fetch(FLAG_RECSW);
	int wp_switch = flag_fetch(FLAG_WPSW);
	int oprom_loaded = flag_fetch(FLAG_OPROM);
	if (dev_switch < 0 || rec_switch < 0 ||
	    wp_switch < 0 || oprom_loaded < 0) {
		// An error message should have already been printed.
		halt();
	}

	if (vboot_init(dev_switch, rec_switch, wp_switch, oprom_loaded))
		halt();

	// Select firmware.
	mkbp_init();

	if (vboot_select_firmware())
		halt();

	// Select a kernel and boot it.

	// If we got this far and the option rom is loaded, we can assume
	// vboot wants to use the display and probably also wants to use the
	// keyboard.
	if (oprom_loaded) {
		keyboard_init();
		video_init();
		video_console_cursor_enable(0);
	}

	ahci_init(PCI_DEV(0, 31, 2));
	if (vboot_select_and_load_kernel())
		halt();

	printf("Got to the end!\n");
	halt();
	return 0;
}