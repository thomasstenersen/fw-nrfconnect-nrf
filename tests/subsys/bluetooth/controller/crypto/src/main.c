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

#include "crypto.h"
#include "ble_controller_soc.h"

#define LOG_MODULE_NAME tests_crypto
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#define KEYSIZE (16)

void test_bt_rand(void)
{
	u8_t rand_num[5];

	LOG_INF("bt_rand() @ 0x%08x\n", (uintptr_t) bt_rand);

	/* Test invalid params */
	zassert_equal(bt_rand(NULL, sizeof(rand_num)), -NRF_EINVAL, "Failed");
	zassert_equal(bt_rand(rand_num, UINT8_MAX + 1), -NRF_EINVAL, "Failed");

	/* Test function call */
	ztest_expect_value(ble_controller_rand_vector_get, p_dst, rand_num);
	ztest_expect_value(ble_controller_rand_vector_get, length, sizeof(rand_num));
	zassert_equal(bt_rand(rand_num, sizeof(rand_num)), 5, "Failed");
}

void test_bt_encrypt_le(void)
{
	u8_t key_in[KEYSIZE];
	u8_t cleartext_in[KEYSIZE] = { 'C' };
	u8_t ciphertext_out[KEYSIZE] = { 0 };

	memset(key_in, 'K', KEYSIZE);
	memset(cleartext_in, 'C', KEYSIZE);
	memset(ciphertext_out, 0, KEYSIZE);

	ztest_expect_value(ble_controller_ecb_block_encrypt, key, key_in);
	ztest_expect_value(ble_controller_ecb_block_encrypt, cleartext, cleartext_in);
	ztest_expect_value(ble_controller_ecb_block_encrypt, ciphertext, ciphertext_out);
	ztest_returns_value(ble_controller_ecb_block_encrypt, 0);
	zassert_equal(bt_encrypt_le(key_in, cleartext_in, ciphertext_out), 0, "Failed");

	/* Test custom mock behavior, this ensures that API's internal transformation on inputs (if any)
	   works correctly. */
	for (u8_t i = 0; i < KEYSIZE; i++) {
		zassert_equal(cleartext_in[i], ciphertext_out[i], "Failed");
	}
}

void test_bt_encrypt_be(void)
{
	u8_t key_in[KEYSIZE];
	u8_t cleartext_in[KEYSIZE] = { 'C' };
	u8_t ciphertext_out[KEYSIZE] = { 0 };

	memset(key_in, 'K', KEYSIZE);
	memset(cleartext_in, 'C', KEYSIZE);
	memset(ciphertext_out, 0, KEYSIZE);

	ztest_ignore_param(ble_controller_ecb_block_encrypt, key);
	ztest_ignore_param(ble_controller_ecb_block_encrypt, cleartext);
	ztest_ignore_param(ble_controller_ecb_block_encrypt, ciphertext);
	ztest_returns_value(ble_controller_ecb_block_encrypt, 0);
	zassert_equal(bt_encrypt_be(key_in, cleartext_in, ciphertext_out), 0, "Failed");

	/* Test custom mock behavior, this ensures that API's internal transformation on inputs (if any)
	   works correctly. */
	for (u8_t i = 0; i < KEYSIZE; i++) {
		zassert_equal(cleartext_in[i], ciphertext_out[i], "Failed");
	}
}

void test_main(void)
{
	ztest_test_suite(test_crypto,
			 ztest_unit_test(test_bt_rand),
			 ztest_unit_test(test_bt_encrypt_le),
			 ztest_unit_test(test_bt_encrypt_be));
	ztest_run_test_suite(test_crypto);
}
