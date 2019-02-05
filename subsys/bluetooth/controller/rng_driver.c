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

#define DEV_DATA(dev) \
	((struct rng_driver_data *)(dev)->driver_data)

/* The RNG driver data. */
struct rng_driver_data {
	struct k_sem sem_sync; /* Used to fill the pending client buffer with new
	                        * random values after RNG IRQ is triggered. */
	uint8_t pool_capacity;
};

static struct rng_driver_data rng_data;

static int rng_driver_get_entropy(struct device *dev, u8_t *buf, u16_t len)
{
	__ASSERT_NO_MSG(&rng_data == DEV_DATA(dev));

	u16_t bytes_to_read;
	u8_t *p_dst = buf;
	u16_t bytes_left = len;
	while (bytes_left > 0) {
		bytes_to_read = min(bytes_left, (u16_t)rng_data.pool_capacity);

		while (ble_controller_rand_vector_get(p_dst, (uint8_t)bytes_to_read) != 0) {
			/* Put the thread on wait until next interrupt to get more
			 * random values. */
			k_sem_take(&rng_data.sem_sync, K_FOREVER);
		}

		p_dst += bytes_to_read;
		bytes_left -= bytes_to_read;
	}

	return 0;
}

static int rng_driver_get_entropy_isr(struct device *dev, u8_t *buf, u16_t len,
				      u32_t flags)
{
	__ASSERT_NO_MSG(&rng_data == DEV_DATA(dev));

	if (likely((flags & ENTROPY_BUSYWAIT) == 0)) {
		uint8_t bytes_available = ble_controller_rand_pool_available_get();
		int32_t errcode = ble_controller_rand_vector_get(buf, bytes_available);
		return errcode == 0 ? bytes_available : -EINVAL;
	}

	u16_t bytes_to_read;
	u16_t bytes_left = len;
	u8_t *p_dst = buf;
	while (bytes_left > 0) {
		bytes_to_read = min(bytes_left, (u16_t)rng_data.pool_capacity);

		ble_controller_rand_vector_get_blocking(p_dst, (uint8_t) bytes_to_read);

		p_dst += bytes_to_read;
		bytes_left -= bytes_to_read;
	}

	return len;
}

static void rng_driver_isr(void *param)
{
	ARG_UNUSED(param);

	ble_controller_RNG_IRQHandler();

	/* This sema wakes up the pending client buffer to fill it with new random
	 * values. */
	k_sem_give(&rng_data.sem_sync);
}

static int rng_driver_init(struct device *dev)
{
	ARG_UNUSED(dev);

	k_sem_init(&rng_data.sem_sync, 0, 1);

	if (ble_controller_rand_pool_capacity_get(&rng_data.pool_capacity) != 0) {
		__ASSERT_NO_MSG(false);
	}

	IRQ_CONNECT(NRF5_IRQ_RNG_IRQn, CONFIG_ENTROPY_NRF5_PRI, rng_driver_isr, NULL, 0);

	return 0;
}

static const struct entropy_driver_api rng_driver_api_funcs = {
	.get_entropy = rng_driver_get_entropy,
	.get_entropy_isr = rng_driver_get_entropy_isr
};

DEVICE_AND_API_INIT(rng_driver, CONFIG_ENTROPY_NAME,
		    rng_driver_init, &rng_data, NULL,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &rng_driver_api_funcs);

#ifdef UNIT_TEST

struct device *rng_driver_get(void)
{
	return &__device_rng_driver;
}

struct k_sem *rng_driver_sema_sync_get(void)
{
	return &rng_data.sem_sync;
}

#endif
