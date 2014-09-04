/*
 * Copyright 2014 Rockchip Electronics Co., Ltd.
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

#include <arch/io.h>

#include "base/init_funcs.h"
#include "drivers/gpio/rockchip.h"
#include "drivers/bus/spi/rockchip.h"
#include "drivers/bus/i2c/rockchip.h"
#include "drivers/flash/spi.h"
#include "drivers/ec/cros/spi.h"
#include "drivers/tpm/slb9635_i2c.h"
#include "drivers/tpm/tpm.h"
#include "drivers/gpio/sysinfo.h"
#include "vboot/util/flag.h"

static int board_setup(void)
{
	RkSpi *spi2 = new_rockchip_spi(0xff130000, 0, 0, 0);
	flash_set_ops(&new_spi_flash(&spi2->ops, 0x400000)->ops);

	RkSpi *spi0 = new_rockchip_spi(0xff110000, 0, 0, 0);
	cros_ec_set_bus(&new_cros_ec_spi_bus(&spi0->ops)->ops);

	sysinfo_install_flags();
	RkGpio *lid_switch = new_rk_gpio_input((RkGpioSpec) {.port = 7,
							     .bank = GPIO_B,
							     .idx = 5});
	RkGpio *ec_in_rw = new_rk_gpio_input((RkGpioSpec) {.port = 0,
							   .bank = GPIO_A,
							   .idx = 7});
	flag_replace(FLAG_LIDSW, &lid_switch->ops);
	flag_install(FLAG_ECINRW, &ec_in_rw->ops);

	RkI2c *i2c1 = new_rockchip_i2c((void *)0xff140000);
	tpm_set_ops(&new_slb9635_i2c(&i2c1->ops, 0x20)->base.ops);

	return 0;
}

INIT_FUNC(board_setup);