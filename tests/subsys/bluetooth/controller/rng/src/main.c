/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <ztest.h>
#include <stdint.h>
#include <soc.h>
#include <logging/log.h>
#include "nrf_errno.h"
#include "ble_controller_soc.h"
#include <entropy.h>
#include <mock_ext.h>

#define LOG_MODULE_NAME test_ble_controller_rng
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

/** Mocked functions. */
void ble_controller_RNG_IRQHandler(void)
{

}

int32_t ble_controller_rand_pool_capacity_get(uint8_t * p_pool_capacity)
{
	return 0;
}

int32_t ble_controller_rand_application_bytes_available_get(uint8_t *p_bytes_avail)
{
	return 0;
}

int32_t ble_controller_rand_application_vector_get(uint8_t * p_dst, uint8_t length)
{
	return 0;
}

int32_t ble_controller_rand_prio_low_vector_get(uint8_t * p_dst, uint8_t length)
{
	return 0;
}

void ble_controller_rand_prio_low_vector_get_blocking(uint8_t * p_dst, uint8_t length)
{

}

/** Test cases. */
void test_stub(void)
{
}

void test_main(void)
{
	ztest_test_suite(test_ble_controller_rng,
			 ztest_unit_test(test_stub));
	ztest_run_test_suite(test_ble_controller_rng);
}
