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

#define LOG_MODULE_NAME test_ble_controller_clock
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

void test_stub(void)
{
    /* No-op. */
}

void test_main(void)
{
    ztest_test_suite(test_ble_controller_clock,
	                 ztest_unit_test(test_stub));
    ztest_run_test_suite(test_ble_controller_clock);
}
