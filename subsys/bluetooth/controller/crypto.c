/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <soc.h>

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BT_DEBUG_HCI_DRIVER)
#define LOG_MODULE_NAME bt_ctlr_crypto
#include "common/log.h"

int bt_rand(void *buf, size_t len)
{
	return 0;
}

int bt_encrypt_le(const u8_t key[16], const u8_t plaintext[16],
		  u8_t enc_data[16])
{


	return 0;
}

int bt_encrypt_be(const u8_t key[16], const u8_t plaintext[16],
		  u8_t enc_data[16])
{


	return 0;
}
