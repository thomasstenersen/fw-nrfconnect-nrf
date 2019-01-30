/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <entropy.h>
#include <soc.h>
#include <device.h>
#include <irq.h>
#include <kernel.h>
#include <kernel_includes.h>
#include <ble_controller.h>
#include <ble_controller_soc.h>
#include <nrfx.h>
#include <nrf_soc.h>

/* The RNG driver data. */
struct rng_driver_data {
	struct k_sem sem_sync; /* Used to fill the pending client buffer with new
							* random values after RNG IRQ is triggered. */
};

static struct rng_driver_data rng_data;

static int rng_driver_get_entropy(struct device *device, u8_t *buf, u16_t len)
{
	__ASSERT_NO_MSG(&entropy_nrf5_data == DEV_DATA(device));

	uint8_t pool_capacity;
	if (ble_controller_rand_pool_capacity_get(&pool_capacity) != 0) {
		return -EINVAL;
	}

	u16_t bread;
	while (len > 0) {
		bread = min(len, (u16_t)pool_capacity);

		__ASSERT_NO_MSG(bread <= (uint8_t)(-1));

		while (ble_controller_rand_application_vector_get(buf, (uint8_t)bread) != 0) {
			/* Put the thread on wait until next interrupt to get more
			 * random values. */
			k_sem_take(&rng_data.sem_sync, K_FOREVER);
		}

		buf += bread;
		len -= bread;
	}

	return 0;
}

static int rng_driver_get_entropy_isr(struct device *dev, u8_t *buf, u16_t len,
									  u32_t flags)
{
	__ASSERT_NO_MSG(&entropy_nrf5_data == DEV_DATA(device));

	u16_t bread;
	uint8_t pool_capacity;
	if (ble_controller_rand_pool_capacity_get(&pool_capacity) != 0) {
		return -EINVAL;
	}

	if (likely((flags & ENTROPY_BUSYWAIT) == 0)) {
		bread = min(len, (u16_t)pool_capacity);

		__ASSERT_NO_MSG(bread <= (uint8_t)(-1));

		int32_t errcode = ble_controller_rand_prio_low_vector_get(buf, (uint8_t) bread);
		return errcode == 0 ? bread : errcode;
	}

	u16_t bleft = len;
	while (bleft > 0) {
		bread = min(bleft, (u16_t)pool_capacity);

		__ASSERT_NO_MSG(bread <= (uint8_t)(-1));

		ble_controller_rand_prio_low_vector_get_blocking(buf, (uint8_t) bread);

		buf += bread;
		bleft -= bread;
	}

	return len;
}

static void rng_driver_isr(void)
{
	ble_controller_RNG_IRQHandler();

	/* This sema wakes up the pending client buffer to fill it with new random
	 * values. */
	k_sem_give(&rng_data.sem_sync);
}

static int rng_driver_init(struct device *dev)
{
	ARG_UNUSED(dev);

	k_sem_init(&rng_data.sem_sync, 0, 1);

	IRQ_CONNECT(NRF5_IRQ_RNG_IRQn, 4, rng_driver_isr, NULL, 0);

	return 0;
}

static const struct entropy_driver_api rng_driver_api_funcs = {
	.get_entropy = rng_driver_get_entropy,
	.get_entropy_isr = rng_driver_get_entropy_isr
};

DEVICE_AND_API_INIT(entropy_nrf5, CONFIG_ENTROPY_NAME,
			rng_driver_init, &rng_data, NULL,
			PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
			&rng_driver_api_funcs);
