/*
 * Copyright 2013 Google Inc.
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

#include <assert.h>
#include <libpayload.h>

#include "base/init_funcs.h"
#include "boot/fit.h"
#include "boot/ramoops.h"
#include "config.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/flash/spi.h"
#include "drivers/gpio/sysinfo.h"
#include "drivers/sound/i2s.h"
#include "drivers/sound/max98090.h"
#include "drivers/tpm/slb9635_i2c.h"
#include "drivers/tpm/tpm.h"
#include "vboot/util/flag.h"
#include "drivers/storage/sdhci.h"
#include "drivers/storage/dw_mmc.h"
#include "drivers/storage/rk_mmc.h"
#include "drivers/flash/spi.h"
#include "drivers/bus/spi/rockchip.h"

static const int emmc_sd_clock_min = 400 * 1000;
static const int emmc_clock_max = 200 * 1000 * 1000;

static int board_setup(void)
{
	// Claim that we have an open lid to satisfy vboot.
	flag_replace(FLAG_LIDSW, new_gpio_high());

	// Claim that we have an power key to satisfy vboot.
	flag_replace(FLAG_PWRSW, new_gpio_low());

	RkSpi *spi1 = new_rockchip_spi(0xff1d0000);
	flash_set_ops(&new_spi_flash(&spi1->ops)->ops);

	SdhciHost *emmc = new_mem_sdhci_host((void *)0xfe330000,
					     SDHCI_PLATFORM_NO_EMMC_HS200 |
					     SDHCI_PLATFORM_NO_CLK_BASE,
					     emmc_sd_clock_min,
					     emmc_clock_max, 198);

	list_insert_after(&emmc->mmc_ctrlr.ctrlr.list_node,
			  &fixed_block_dev_controllers);


	/* USB2.0-EHCI */
	UsbHostController *uhst0_ehci = new_usb_hc(EHCI, 0xfe380000);
	UsbHostController *uhst1_ehci = new_usb_hc(EHCI, 0xfe3c0000);

	list_insert_after(&uhst0_ehci->list_node, &usb_host_controllers);
	list_insert_after(&uhst1_ehci->list_node, &usb_host_controllers);

	/* USB2.0-OHCI */
	UsbHostController *uhst0_ohci = new_usb_hc(OHCI, 0xfe3a0000);
	UsbHostController *uhst1_ohci = new_usb_hc(OHCI, 0xfe3e0000);

	list_insert_after(&uhst0_ohci->list_node, &usb_host_controllers);
	list_insert_after(&uhst1_ohci->list_node, &usb_host_controllers);

	return 0;
}

INIT_FUNC(board_setup);