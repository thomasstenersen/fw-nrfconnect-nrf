/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <soc.h>
#include <errno.h>
#include <device.h>
#include <kernel_includes.h>
#include <clock_control.h>
#include <ble_controller_soc.h>

static int hf_clock_start(struct device *dev, clock_control_subsys_t sub_system)
{
	ARG_UNUSED(dev);

	if (!ble_controller_hf_clock_request(NULL)) {
		bool blocking = POINTER_TO_UINT(sub_system);
		if (blocking)
		{
			bool is_running = false;
			while (!is_running) {
				if (ble_controller_hf_clock_is_running(&is_running)) {
					return -EFAULT;
				}
			}
		}
		return 0;
	} else {
		return -EFAULT;
	}
}

static int hf_clock_stop(struct device *dev, clock_control_subsys_t sub_system)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(sub_system);

	if (!ble_controller_hf_clock_release()) {
		return 0;
	} else {
		return -EFAULT;
	}
}

static int lf_clock_start(struct device *dev, clock_control_subsys_t sub_system)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(sub_system);

	/* No-op. LFCLFK is started by default. */

	return 0;
}

static int clock_control_init(struct device *dev)
{
	/* No-op. Initialized by hci_driver_init() in subsys/bluetooth/controller/hci_driver.c */

	return 0;
}

static const struct clock_control_driver_api hf_clock_control_api = {
	.on = hf_clock_start,
	.off = hf_clock_stop,
	.get_rate = NULL,
};

DEVICE_AND_API_INIT(hf_clock,
			CONFIG_CLOCK_CONTROL_NRF5_M16SRC_DRV_NAME,
			clock_control_init, NULL, NULL, PRE_KERNEL_1,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
			&hf_clock_control_api);

/* LFCLK doesn't have stop function to replicate the nRF5 Power Clock driver
 * behavior. */
static const struct clock_control_driver_api lf_clock_control_api = {
	.on = lf_clock_start,
	.off = NULL,
	.get_rate = NULL,
};

DEVICE_AND_API_INIT(lf_clock,
			CONFIG_CLOCK_CONTROL_NRF5_K32SRC_DRV_NAME,
			clock_control_init, NULL, NULL, PRE_KERNEL_1,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
			&lf_clock_control_api);

#ifdef UNIT_TEST
struct device * lf_clock_device_get(void)
{
	return &__device_lf_clock;
}

struct device * hf_clock_device_get(void)
{
	return &__device_hf_clock;
}
#endif
