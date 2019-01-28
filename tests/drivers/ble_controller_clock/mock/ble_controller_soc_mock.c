/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <stdbool.h>
#include <inttypes.h>
#include <ztest.h>
#include <logging/log.h>
#include "ble_controller_soc.h"

#define LOG_MODULE_NAME ble_controller_soc_mock
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

int32_t ble_controller_hf_clock_request(ble_controller_hf_clock_callback_t on_started)
{
	return 0;
}

int32_t ble_controller_hf_clock_release(void)
{
	return 0;
}

int32_t ble_controller_hf_clock_is_running(bool * p_is_running)
{
	return 0;
}
