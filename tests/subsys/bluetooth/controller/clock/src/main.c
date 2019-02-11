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
#include <clock_control.h>
#include <mock_ext.h>

#define LOG_MODULE_NAME test_ble_controller_clock
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

/** Extern declaration */
struct device *lf_clock_device_get(void);
struct device *hf_clock_device_get(void);

/** Mocked functions */
int32_t ble_controller_hf_clock_request(ble_controller_hf_clock_callback_t on_started)
{
	mock_check_expected();
	ztest_check_expected_value(on_started);
	return (int32_t) ztest_get_return_value();
}

int32_t ble_controller_hf_clock_release(void)
{
	mock_check_expected();
	return (int32_t) ztest_get_return_value();
}

int32_t ble_controller_hf_clock_is_running(bool *p_is_running)
{
	mock_check_expected();
	mock_arg_get(*p_is_running);
	return (int32_t) ztest_get_return_value();
}

/** Helper functions */
static void clock_request_expect(int32_t retval)
{
	mock_expect(ble_controller_hf_clock_request);
	ztest_expect_value(ble_controller_hf_clock_request, on_started, NULL);
	ztest_returns_value(ble_controller_hf_clock_request, retval);
}

static void clock_release_expect(int32_t retval)
{
	mock_expect(ble_controller_hf_clock_release);
	ztest_returns_value(ble_controller_hf_clock_release, retval);
}

static void clock_is_running_expect(bool is_running, int32_t retval)
{
	mock_expect(ble_controller_hf_clock_is_running);
	mock_arg(ble_controller_hf_clock_is_running, is_running);
	ztest_returns_value(ble_controller_hf_clock_is_running, retval);
}

/** Tests */
void test_device_get(void)
{
	struct device *p_hf_clock = hf_clock_device_get();
	struct device *p_lf_clock = lf_clock_device_get();

	zassert_not_null(p_hf_clock, "HFCLK device is NULL");
	zassert_not_null(p_lf_clock, "LFCLK device is NULL");

	struct device *p_clock;

	p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_M16SRC_DRV_NAME);
	zassert_not_null(p_clock, "Couldn't get HFCLK through API");
	zassert_equal_ptr(p_hf_clock, p_clock, "p_hf_clock != p_clock");

	p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_K32SRC_DRV_NAME);
	zassert_not_null(p_clock, "Couldn't get LFCLK through API");
	zassert_equal_ptr(p_lf_clock, p_clock, "p_lf_clock != p_clock");
}

void test_error_cases(void)
{
	struct device *p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_M16SRC_DRV_NAME);
	int error;

	clock_request_expect(1);
	error = clock_control_on(p_clock, (void *)0);
	zassert_equal(-EFAULT, error, "Unexpected error: %d", error);

	clock_request_expect(0);
	clock_is_running_expect(true, 1);
	error = clock_control_on(p_clock, (void *)1);
	zassert_equal(-EFAULT, error, "Unexpected error: %d", error);

	clock_release_expect(1);
	error = clock_control_off(p_clock, (void *)0);
	zassert_equal(-EFAULT, error, "Unexpected error: %d", error);
}

void test_hf_clock_request_non_blocking(void)
{
	struct device *p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_M16SRC_DRV_NAME);
	int error;

	clock_request_expect(0);

	error = clock_control_on(p_clock, (void *)0);
	zassert_true(!error, "!error %d", error);
}

void test_hf_clock_request_blocking(void)
{
	struct device *p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_M16SRC_DRV_NAME);
	int error;

	/* Case 1: Already running. */
	clock_request_expect(0);
	clock_is_running_expect(true, 0);

	error = clock_control_on(p_clock, (void *)1);
	zassert_true(!error, "!error %d", error);

	/* Case 2: Wait until clock run */
	clock_request_expect(0);

	for (size_t i = 0; i < 5; i++) {
		clock_is_running_expect(false, 0);
	}
	clock_is_running_expect(true, 0);

	error = clock_control_on(p_clock, (void *)1);
	zassert_true(!error, "!error %d", error);
}

void test_hf_clock_get_rate(void)
{
	struct device *p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_M16SRC_DRV_NAME);
	int error;
	u32_t rate;

	error = clock_control_get_rate(p_clock, (void *)0, &rate);
	zassert_true(!error, "!error %d", error);
	zassert_equal(MHZ(16), rate, "HFCLK frequency is wrong: %d", rate);
}

void test_lf_clock(void)
{
	struct device *p_clock = device_get_binding(CONFIG_CLOCK_CONTROL_NRF5_K32SRC_DRV_NAME);
	int error;

	/* On, non-blocking */
	error = clock_control_on(p_clock, (void *)0);
	zassert_true(!error, "!error %d", error);

	/* On, blocking */
	error = clock_control_on(p_clock, (void *)1);
	zassert_true(!error, "!error %d", error);

	/* Check frequency */
	u32_t rate;
	error = clock_control_get_rate(p_clock, (void *)0, &rate);
	zassert_true(!error, "!error %d", error);
	zassert_equal(32768, rate, "LFCLK frequency is wrong: %d", rate);
}

void test_lf_clock_not_implemented_api(void)
{
	struct device *p_clock = lf_clock_device_get();
	const struct clock_control_driver_api *api = p_clock->driver_api;

	zassert_is_null(api->off, "LFCLK get_rate should not be implemented");
}

void test_main(void)
{
	ztest_test_suite(test_ble_controller_clock,
			 ztest_unit_test(test_device_get),
			 ztest_unit_test(test_error_cases),
			 ztest_unit_test(test_hf_clock_request_non_blocking),
			 ztest_unit_test(test_hf_clock_request_blocking),
			 ztest_unit_test(test_hf_clock_get_rate),
			 ztest_unit_test(test_lf_clock),
			 ztest_unit_test(test_lf_clock_not_implemented_api));
	ztest_run_test_suite(test_ble_controller_clock);
}
