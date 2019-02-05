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

#define SEM_GIVE_ERROR (-0xFF)
#define RANDOM_POOL_CAPACITY (100)

/* The ble controller poll length is limited to UINT8_MAX. This should be
 * more than uint8_t to ensure it fills the whole buffer. */
#define BUFFER_LENGTH 1024
static u8_t buffer[BUFFER_LENGTH];
static uint8_t rand_pool[BUFFER_LENGTH];

/* This could only be a macro because ztest_returns_value() uses __func__ inside
 * to get the stored value. */
#define rand_pool_to_buffer_copy(p_dst, length)								 \
	{												 \
		uint8_t *p_rand_pool;									 \
		uint8_t rand_pool_length;								 \
		mock_arg_array_get(p_rand_pool, rand_pool_length);					 \
		if (p_rand_pool != NULL) {								 \
			zassert_equal(rand_pool_length, length,						 \
				      "Rand pool length should be equal to the provided buffer lenght"); \
			if (rand_pool_length > 0) {							 \
				memcpy(p_dst, p_rand_pool, rand_pool_length);				 \
			}										 \
		}											 \
	} while (0)

/** Extern declaration */
struct device *rng_driver_get(void);
struct k_sem *rng_driver_sema_sync_get(void);

/** Mocked functions */
void ble_controller_RNG_IRQHandler(void)
{
	/* Stub */
}

int32_t ble_controller_rand_pool_capacity_get(uint8_t *p_pool_capacity)
{
	zassert_not_null(p_pool_capacity, "NULL pointer argument provided");

	*p_pool_capacity = RANDOM_POOL_CAPACITY;
	return 0;
}

uint8_t ble_controller_rand_pool_available_get(void)
{
	mock_check_expected();

	return (uint8_t)ztest_get_return_value();
}

int32_t ble_controller_rand_vector_get(uint8_t *p_dst, uint8_t length)
{
	mock_check_expected();

	ztest_check_expected_value(p_dst);
	ztest_check_expected_value(length);

	rand_pool_to_buffer_copy(p_dst, length);

	int32_t retval = (int32_t)ztest_get_return_value();
	if (retval == SEM_GIVE_ERROR) {
		k_sem_give(rng_driver_sema_sync_get());
	}

	return retval;
}

void ble_controller_rand_vector_get_blocking(uint8_t *p_dst, uint8_t length)
{
	mock_check_expected();

	ztest_check_expected_value(p_dst);
	ztest_check_expected_value(length);

	rand_pool_to_buffer_copy(p_dst, length);
}

/** Helper functions */
static void rand_pool_fill(uint8_t *p_rand_pool, uint16_t length)
{
	static uint8_t shift; /**< To "randomize" values between tests. */

	for (uint16_t i = 0; i < length; i++) {
		p_rand_pool[i] = (uint8_t)i + shift;
	}
	shift++;
}

static void available_get_expect(uint8_t bytes_available)
{
	mock_expect(ble_controller_rand_pool_available_get);

	ztest_returns_value(ble_controller_rand_pool_available_get, bytes_available);
}

static void vector_get_expect(uint8_t *p_dst, uint8_t length,
				       uint8_t *p_rand_pool, uint8_t rand_pool_length,
				       int return_value)
{
	mock_expect(ble_controller_rand_vector_get);

	ztest_expect_value(ble_controller_rand_vector_get, p_dst, p_dst);
	ztest_expect_value(ble_controller_rand_vector_get, length, length);

	mock_arg_array(ble_controller_rand_vector_get, p_rand_pool, rand_pool_length);

	ztest_returns_value(ble_controller_rand_vector_get, return_value);
}

static void vector_get_blocking_expect(uint8_t *p_dst, uint8_t length,
						uint8_t *p_rand_pool, uint8_t rand_pool_length)
{
	mock_expect(ble_controller_rand_vector_get_blocking);

	ztest_expect_value(ble_controller_rand_vector_get_blocking, p_dst, p_dst);
	ztest_expect_value(ble_controller_rand_vector_get_blocking, length, length);

	mock_arg_array(ble_controller_rand_vector_get_blocking, p_rand_pool, rand_pool_length);
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

	available_get_expect(64);
	vector_get_expect(buffer, 64, NULL, 0, -1);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 64, 0), -EINVAL, "Failed to get rand vector");
}

void test_thread_mode_entropy_less_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);

	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, RANDOM_POOL_CAPACITY - 1);

	/* Request vector of randoms that lesser than the pool capacity. */
	vector_get_expect(buffer, RANDOM_POOL_CAPACITY - 1,
					  rand_pool, RANDOM_POOL_CAPACITY - 1, 0);
	zassert_equal(entropy_get_entropy(dev, buffer, RANDOM_POOL_CAPACITY - 1), 0,
				  "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, RANDOM_POOL_CAPACITY - 1),
				  "Rand data mismatch");
}

void test_thread_mode_entropy_greater_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);

	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, BUFFER_LENGTH);

	/* Request vector of randoms that bigger than the pool capacity. */
	for (size_t i = 0; i < BUFFER_LENGTH / RANDOM_POOL_CAPACITY; i++) {
		vector_get_expect(&buffer[i * RANDOM_POOL_CAPACITY], RANDOM_POOL_CAPACITY,
				&rand_pool[i * RANDOM_POOL_CAPACITY], RANDOM_POOL_CAPACITY, 0);
	}

	uint8_t bytes_left = BUFFER_LENGTH % RANDOM_POOL_CAPACITY;
	vector_get_expect(&buffer[BUFFER_LENGTH - bytes_left], bytes_left,
				  &rand_pool[BUFFER_LENGTH - bytes_left], bytes_left, 0);
	zassert_equal(entropy_get_entropy(dev, buffer, BUFFER_LENGTH), 0,
				  "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, BUFFER_LENGTH), "Rand data mismatch");
}

void test_thread_mode_not_enough_bytes(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);

	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, 64);

	/* Simulate the case when the entropy_get_entropy() call gets blocked until
	 * more bytes generated . */
	vector_get_expect(buffer, 64, rand_pool, 64, SEM_GIVE_ERROR);
	vector_get_expect(buffer, 64, rand_pool, 64, 0);
	zassert_equal(entropy_get_entropy(dev, buffer, 64), 0, "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, 64), "Rand data mismatch");
}

void test_isr_mode_no_wait_less_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);

	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, 32);

	/* Request vector of randoms that lesser than the pool capacity. */
	available_get_expect(32);
	vector_get_expect(buffer, 32, rand_pool, 32, 0);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 64, 0), 32, "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, 32), "Rand data mismatch");
}

void test_isr_mode_no_wait_greater_than_capacity(void)
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);

	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, RANDOM_POOL_CAPACITY);

	/* Request vector of randoms that greater than the pool capacity. */
	available_get_expect(RANDOM_POOL_CAPACITY);
	vector_get_expect(buffer, RANDOM_POOL_CAPACITY, rand_pool, RANDOM_POOL_CAPACITY, 0);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, 200, 0), RANDOM_POOL_CAPACITY, "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, RANDOM_POOL_CAPACITY), "Rand data mismatch");
}

void test_isr_mode_busywait_less_than_capacity()
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, RANDOM_POOL_CAPACITY - 1);

	/* Request vector of randoms that lesser than the pool capacity. */
	vector_get_blocking_expect(buffer, RANDOM_POOL_CAPACITY - 1, rand_pool, RANDOM_POOL_CAPACITY - 1);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, RANDOM_POOL_CAPACITY - 1, ENTROPY_BUSYWAIT), RANDOM_POOL_CAPACITY - 1, "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, RANDOM_POOL_CAPACITY - 1), "Rand data mismatch");
}

void test_isr_mode_busywait_greater_than_capacity()
{
	struct device *dev = device_get_binding(CONFIG_ENTROPY_NAME);
	memset(buffer, 0, sizeof(buffer));

	rand_pool_fill(rand_pool, BUFFER_LENGTH);

	/* Request vector of randoms that greater than the pool capacity. */
	for (size_t i = 0; i < BUFFER_LENGTH / RANDOM_POOL_CAPACITY; i++) {
		vector_get_blocking_expect(&buffer[i * RANDOM_POOL_CAPACITY], RANDOM_POOL_CAPACITY,
							&rand_pool[i * RANDOM_POOL_CAPACITY], RANDOM_POOL_CAPACITY);
	}

	uint8_t bytes_left = BUFFER_LENGTH % RANDOM_POOL_CAPACITY;
	vector_get_blocking_expect(&buffer[BUFFER_LENGTH - bytes_left], bytes_left,
						&rand_pool[BUFFER_LENGTH - bytes_left], bytes_left);
	zassert_equal(entropy_get_entropy_isr(dev, buffer, BUFFER_LENGTH, ENTROPY_BUSYWAIT),
				  BUFFER_LENGTH, "Failed to get rand vector");
	zassert_false(memcmp(buffer, rand_pool, BUFFER_LENGTH), "Rand data mismatch");
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
