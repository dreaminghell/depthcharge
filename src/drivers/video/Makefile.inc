##
## Copyright 2013 Google Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

depthcharge-y += display.c
depthcharge-y += coreboot_fb.c
depthcharge-$(CONFIG_DRIVER_VIDEO_EXYNOS5) += exynos5.c
depthcharge-$(CONFIG_DRIVER_VIDEO_INTEL_I915) += intel_i915.c
depthcharge-$(CONFIG_DRIVER_VIDEO_ROCKCHIP) += rockchip.c
depthcharge-$(CONFIG_DRIVER_VIDEO_TEGRA132) += tegra132.c
depthcharge-$(CONFIG_DRIVER_VIDEO_TEGRA210) += tegra132.c
depthcharge-$(CONFIG_DRIVER_VIDEO_WW_RING) += ww_ring.c
depthcharge-$(CONFIG_DRIVER_VIDEO_WW_RING) += ww_ring_programs.c
depthcharge-$(CONFIG_DRIVER_VIDEO_MEDIATEK) += mt8173_ddp.c
