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
#include <kernel.h>

#define LOG_MODULE_NAME test_ble_controller_rng
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

/* The ble controller poll length is limited to UINT8_MAX. This should be
 * more than uint8_t to ensure it fills the whole buffer. */
#define BUFFER_LENGTH 1024
static u8_t buffer[BUFFER_LENGTH];

/** Extern declaration */
struct device *rng_driver_get(void);
struct k_sem * rng_driver_sema_sync_get(void);

/** Mocked functions */
void ble_controller_RNG_IRQHandler(void)
{
	/* Stub */
}

int32_t ble_controller_rand_pool_capacity_get(uint8_t * p_pool_capacity)
{
	mock_check_expected();

	mock_arg_get(*p_pool_capacity);

	return (int32_t)ztest_get_return_value();
}

/* FIXME: Unused, shall be removed */
int32_t ble_controller_rand_application_bytes_available_get(uint8_t *p_bytes_avail)
{
	mock_check_expected();

	mock_arg_get(*p_bytes_avail);

	return (int32_t)ztest_get_return_value();
}

int32_t ble_controller_rand_application_vector_get(uint8_t * p_dst, uint8_t length)
{
	mock_check_expected();

	ztest_check_expected_value(p_dst);
	ztest_check_expected_value(length);

	int32_t retval = (int32_t)ztest_get_return_value();
	if (retval != 0) {
		k_sem_give(rng_driver_sema_sync_get());
	}

	return retval;
}

int32_t ble_controller_rand_prio_low_vector_get(uint8_t * p_dst, uint8_t length)
{
	mock_check_expected();

	ztest_check_expected_value(p_dst);
	ztest_check_expected_value(length);

	return (int32_t)ztest_get_return_value();
}

void ble_controller_rand_prio_low_vector_get_blocking(uint8_t * p_dst, uint8_t length)
{
	mock_check_expected();

	ztest_check_expected_value(p_dst);
	ztest_check_expected_value(length);
}

/** Helper functions */
static void capacity_get_expect(uint8_t capacity, int32_t return_value)
{
	mock_expect(ble_controller_rand_pool_capacity_get, 1);
	mock_arg(ble_controller_rand_pool_capacity_get, capacity);
	ztest_returns_value(ble_controller_rand_pool_capacity_get, return_value);
}

static void app_vector_get_expect(uint8_t *p_dst, uint8_t length, int return_value)
{
	mock_expect(ble_controller_rand_application_vector_get, 1);
	ztest_expect_value(ble_controller_rand_application_vector_get, p_dst, p_dst);
	ztest_expect_value(ble_controller_rand_application_vector_get, length, length);
	ztest_returns_value(ble_controller_rand_application_vector_get, return_value);
}

static void prio_low_vector_get_expect(uint8_t *p_dst, uint8_t length, int return_value)
{
	mock_expect(ble_controller_rand_prio_low_vector_get, 1);
	ztest_expect_value(ble_controller_rand_prio_low_vector_get, p_dst, p_dst);
	ztest_expect_value(ble_controller_rand_prio_low_vector_get, length, length);
	ztest_returns_value(ble_controller_rand_prio_low_vector_get, return_value);
}

static void prio_low_vector_get_blocking_expect(uint8_t *p_dst, uint8_t length, int return_value)
{
	mock_expect(ble_controller_rand_prio_low_vector_get_blocking, 1);
	ztest_expect_value(ble_controller_rand_prio_low_vector_get_blocking, p_dst, p_dst);
	ztest_expect_value(ble_controller_rand_prio_low_vector_get_blocking, length, length);
}


/** Tests */
void test_driver_init(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	zassert_not_null(dev, "Entropy driver shouldn't be null");

	struct k_sem *p_sema_sync = rng_driver_sema_sync_get();
	zassert_not_null(p_sema_sync, "Semaphor is null");
	zassert_equal(k_sem_count_get(p_sema_sync), 0, "Initial semaphore value is wrong");
}

void test_error_cases(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	zassert_not_null(dev, "Device is null");
	memset(buffer, 0, sizeof(buffer));

	zassert_equal_ptr(rng_driver_get(), dev, "Wrong pointer");

	capacity_get_expect(100, -1);
	zassert_equal(entropy_get_entropy(dev, buffer, 64), -EINVAL, "Wrong error returned");

	capacity_get_expect(100, -1);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 64, 0), -EINVAL, "Wrong error code returned");

	capacity_get_expect(100, 0);
	prio_low_vector_get_expect(buffer, 64, -1);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 64, 0), -EINVAL, "Failed to get rand vector");
}

void test_thread_mode_entropy_less_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Request vector of randoms that lesser than the pool capacity. */
	capacity_get_expect(100, 0);
	app_vector_get_expect(buffer, 64, 0);
	zassert_equal(entropy_get_entropy(dev, buffer, 64), 0, "Failed to get rand vector");
}

void test_thread_mode_entropy_greater_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Request vector of randoms that bigger than the pool capacity. */
	capacity_get_expect(100, 0);
	for (size_t i = 0; i < BUFFER_LENGTH / 100; i++)
	{
		app_vector_get_expect(&buffer[i * 100], 100, 0);
	}
	app_vector_get_expect(&buffer[BUFFER_LENGTH - BUFFER_LENGTH % 100], BUFFER_LENGTH % 100, 0);
	zassert_equal(entropy_get_entropy(dev, buffer, BUFFER_LENGTH), 0, "Failed to get rand vector");
}

void test_thread_mode_not_enough_bytes(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Simulate the case when the entropy_get_entropy() call gets blocked until
	 * more bytes generated . */
	capacity_get_expect(100, 0);
	app_vector_get_expect(buffer, 64, -1);
	app_vector_get_expect(buffer, 64, 0);
	zassert_equal(entropy_get_entropy(dev, buffer, 64), 0, "Failed to get rand vector");
}

void test_isr_mode_no_wait_less_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Request vector of randoms that lesser than the pool capacity. */
	capacity_get_expect(100, 0);
	prio_low_vector_get_expect(buffer, 64, 0);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 64, 0), 64, "Failed to get rand vector");
}

void test_isr_mode_no_wait_greater_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Request vector of randoms that greater than the pool capacity. */
	capacity_get_expect(100, 0);
	prio_low_vector_get_expect(buffer, 100, 0);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 200, 0), 100, "Failed to get rand vector");
}

void test_isr_mode_busywait_less_than_capacity()
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Request vector of randoms that lesser than the pool capacity. */
	capacity_get_expect(100, 0);
	prio_low_vector_get_blocking_expect(buffer, 64, 0);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 64, ENTROPY_BUSYWAIT), 64, "Failed to get rand vector");
}

void test_isr_mode_busywait_greater_than_capacity()
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	/* Request vector of randoms that greater than the pool capacity. */
	capacity_get_expect(100, 0);
	for (size_t i = 0; i < BUFFER_LENGTH / 100; i++)
	{
		prio_low_vector_get_blocking_expect(&buffer[i * 100], 100, 0);
	}
	prio_low_vector_get_blocking_expect(&buffer[BUFFER_LENGTH - BUFFER_LENGTH % 100], BUFFER_LENGTH % 100, 0);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, BUFFER_LENGTH, ENTROPY_BUSYWAIT), BUFFER_LENGTH, "Failed to get rand vector");
}

void test_main(void)
{
	ztest_test_suite(test_ble_controller_rng,
			 ztest_unit_test(test_driver_init),
			 ztest_unit_test(test_error_cases),
			 ztest_unit_test(test_thread_mode_entropy_less_than_capacity),
			 ztest_unit_test(test_thread_mode_entropy_greater_than_capacity),
			 ztest_unit_test(test_thread_mode_not_enough_bytes),
			 ztest_unit_test(test_isr_mode_no_wait_less_than_capacity),
			 ztest_unit_test(test_isr_mode_no_wait_greater_than_capacity),
			 ztest_unit_test(test_isr_mode_busywait_less_than_capacity),
			 ztest_unit_test(test_isr_mode_busywait_greater_than_capacity));
	ztest_run_test_suite(test_ble_controller_rng);
}
