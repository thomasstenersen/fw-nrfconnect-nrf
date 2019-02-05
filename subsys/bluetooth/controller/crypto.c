/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr/types.h>
#include <misc/byteorder.h>
#include <stddef.h>
#include <stdint.h>
#include <soc.h>
#include <logging/log.h>

#include <ble_controller_soc.h>

#include "nrf_error.h"
#include "multithreading_lock.h"

#define LOG_MODULE_NAME ble_controller_crypto
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

int bt_rand(void *buf, size_t len)
{
	if (len > UINT8_MAX || buf == NULL) {
		return -NRF_EINVAL;
	}

	int errcode = MULTITHREADING_LOCK_ACQUIRE();
	if (!errcode) {
		errcode = ble_controller_rand_vector_get(buf, (uint8_t)len);
		MULTITHREADING_LOCK_RELEASE();
	}

	LOG_INF("rand bytes %d\n", errcode);

	return errcode;
}

int bt_encrypt_le(const u8_t key[16], const u8_t plaintext[16],
		  u8_t enc_data[16])
{
	LOG_HEXDUMP_DBG(key, 16, "key");
	LOG_HEXDUMP_DBG(plaintext, 16, "plaintext");

	int errcode = MULTITHREADING_LOCK_ACQUIRE();
	if (!errcode) {
		errcode = ble_controller_ecb_block_encrypt(key, plaintext, enc_data);
		MULTITHREADING_LOCK_RELEASE();
	}

	LOG_HEXDUMP_DBG(enc_data, 16, "enc_data");

	return errcode;
}

int bt_encrypt_be(const u8_t key[16], const u8_t plaintext[16],
		  u8_t enc_data[16])
{
	uint8_t key_le[16], plaintext_le[16], enc_data_le[16];

	LOG_HEXDUMP_DBG(key, 16, "key");
	LOG_HEXDUMP_DBG(plaintext, 16, "plaintext");

	sys_memcpy_swap(key_le, key, 16);
	sys_memcpy_swap(plaintext_le, plaintext, 16);

	int errcode = MULTITHREADING_LOCK_ACQUIRE();
	if (!errcode) {
		errcode = ble_controller_ecb_block_encrypt(key_le, plaintext_le, enc_data_le);
		MULTITHREADING_LOCK_RELEASE();
	}

	sys_memcpy_swap(enc_data, enc_data_le, 16);

	LOG_HEXDUMP_DBG(enc_data, 16, "enc_data");

	return errcode;
}
