/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr/types.h>
#include <misc/byteorder.h>
#include <stddef.h>
#include <stdint.h>
#include <soc.h>
#include <logging/log.h>

#include "nrf_error.h"

#include "ble_controller_soc.h"
#include "blectlr_util.h"

#define LOG_MODULE_NAME ble_controller_crypto
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);


int bt_rand(void *buf, size_t len)
{
  //@todo: support CONFIG_MULTITHREADING

  if (len > UINT8_MAX || buf == NULL)
  {
    return -NRF_EINVAL;
  }

  uint32_t errcode = ble_controller_rand_vector_get(buf, (uint8_t) len);

  LOG_DBG("rand bytes %d\n", errcode);

  return errcode;
}

int bt_encrypt_le(const u8_t key[16], const u8_t plaintext[16], u8_t enc_data[16])
{
  //@todo: support CONFIG_MULTITHREADING

  LOG_HEXDUMP_DBG(key, 16, "key");
  LOG_HEXDUMP_DBG(plaintext, 16, "plaintext");

  uint32_t errcode = ble_controller_ecb_block_encrypt(key, plaintext, enc_data);

  LOG_HEXDUMP_DBG(enc_data, 16, "enc_data");

  return errcode;
}

int bt_encrypt_be(const u8_t key[16], const u8_t plaintext[16], u8_t enc_data[16])
{
  //@todo: support CONFIG_MULTITHREADING

  uint8_t key_le[16], plaintext_le[16], enc_data_le[16];

  LOG_HEXDUMP_DBG(key, 16, "key");
  LOG_HEXDUMP_DBG(plaintext, 16, "plaintext");

  sys_memcpy_swap(key_le, key, 16);              //?? use zephyr function
  sys_memcpy_swap(plaintext_le, plaintext, 16);
  uint32_t errcode = ble_controller_ecb_block_encrypt(key_le, plaintext_le, enc_data_le);
  sys_memcpy_swap(enc_data, enc_data_le, 16);

  LOG_HEXDUMP_DBG(enc_data, 16, "enc_data");

  return errcode;
}
