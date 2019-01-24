
/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#ifndef BLE_CONTROLLER_SOC_MOCK_H_
#define BLE_CONTROLLER_SOC_MOCK_H_

#define KEYSIZE (16)

int32_t ble_controller_rand_vector_get(uint8_t * p_dst, uint8_t length);

int32_t ble_controller_ecb_block_encrypt(const uint8_t key[16],
                                         const uint8_t cleartext[16],
                                         uint8_t ciphertext[16]);


#endif /* BLE_CONTROLLER_SOC_MOCK_H_ */
