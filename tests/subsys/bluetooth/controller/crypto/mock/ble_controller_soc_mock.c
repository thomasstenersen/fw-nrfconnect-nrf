
/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <stdbool.h>
#include <inttypes.h>
#include <ztest.h>
#include "ble_controller_soc.h"
#include "mock_ext.h"

#define LOG_MODULE_NAME ble_controller_soc_mock
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

int32_t ble_controller_ecb_block_encrypt(const uint8_t key[16],
					 const uint8_t cleartext[16],
					 uint8_t ciphertext[16])
{
	mock_check_expected();

	/* This mock simply xors the cleartext with key and copies it to ciphertext */
	for (uint32_t i = 0; i < 16; i++) {
		ciphertext[i] = key[i] ^ cleartext[i];
	}

	return ztest_get_return_value();
}

void ll_util_revcpy(uint8_t *dest, const uint8_t *src, uint8_t size)
{
	LOG_INF("%s()\n", __func__);
	/* Don't care about reversing the array. Just copy, and use a regular memcmp to verify the contents in the test. */
	memcpy(dest, src, size);
}

u32_t sys_rand32_get(void)
{
	return (123);
}
