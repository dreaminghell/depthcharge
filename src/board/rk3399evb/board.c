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
#include "config.h"
#include "drivers/bus/spi/rockchip.h"
#include "drivers/bus/usb/usb.h"
#include "drivers/ec/cros/spi.h"
#include "drivers/flash/spi.h"
#include "drivers/flash/spi.h"
#include "drivers/gpio/rockchip.h"
#include "drivers/gpio/sysinfo.h"
#include "drivers/storage/dw_mmc.h"
#include "drivers/storage/rk_dwmmc.h"
#include "drivers/storage/sdhci.h"
#include "vboot/util/flag.h"

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

	RkGpio *card_detect = new_rk_gpio_input(GPIO(4, D, 0));
	GpioOps *card_detect_ops = &card_detect->ops;

	card_detect_ops = new_gpio_not(card_detect_ops);

	DwmciHost *sd_card = new_rkdwmci_host(0xfe320000, 594000000, 4, 1,
					      card_detect_ops);

	list_insert_after(&sd_card->mmc.ctrlr.list_node,
			  &removable_block_dev_controllers);

	/* Support USB20_HOST1 in firmware and USB20_HOST0 not needed. */
	/* USB2.0-EHCI*/
	UsbHostController *uhst1_ehci = new_usb_hc(EHCI, 0xfe3c0000);

	list_insert_after(&uhst1_ehci->list_node, &usb_host_controllers);

	/* USB2.0-OHCI */
	UsbHostController *uhst1_ohci = new_usb_hc(OHCI, 0xfe3e0000);

	list_insert_after(&uhst1_ohci->list_node, &usb_host_controllers);

	/* Support both USB3.0 XHCI controllers in firmware. */
	UsbHostController *uhst0_xhci = new_usb_hc(XHCI, 0xfe800000);
	UsbHostController *uhst1_xhci = new_usb_hc(XHCI, 0xfe900000);

	list_insert_after(&uhst0_xhci->list_node, &usb_host_controllers);
	list_insert_after(&uhst1_xhci->list_node, &usb_host_controllers);

	return 0;
}

INIT_FUNC(board_setup);
