
/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <stdbool.h>
#include <inttypes.h>
#include <ztest.h>
#include "ble_controller_soc.h"

#define LOG_MODULE_NAME ble_controller_soc_mock
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

int32_t ble_controller_rand_vector_get(uint8_t *p_dst, uint8_t length)
{
	LOG_INF("%s()\n", __func__);
	ztest_check_expected_value(p_dst);
	ztest_check_expected_value(length);

	/* mock function memsets the p_dst with some data */
	memset(p_dst, 'R', length);
	return length;
}

int32_t ble_controller_ecb_block_encrypt(const uint8_t key[16],
					 const uint8_t cleartext[16],
					 uint8_t ciphertext[16])
{
	LOG_INF("%s()\n", __func__);
	ztest_check_expected_value(key);
	ztest_check_expected_value(cleartext);
	ztest_check_expected_value(ciphertext);

	/* This mock simply copies the cleartext to ciphertext */
	memcpy(ciphertext, cleartext, 16);

	return ztest_get_return_value();
}

void ll_util_revcpy(uint8_t *dest, const uint8_t *src, uint8_t size)
{
	LOG_INF("%s()\n", __func__);
	/* Don't care about reversing the array. Just copy, and use a regular memcmp to verify the contents in the test. */
	memcpy(dest, src, size);
}