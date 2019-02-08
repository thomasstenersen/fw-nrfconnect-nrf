/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include "multithreading_lock.h"

/* Semaphore with a single count functioning as a lock. */
K_SEM_DEFINE(ble_controller_lock, 1, 1);

int32_t multithreading_lock_acquire(int32_t timeout)
{
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFETY)
	return k_sem_take(&ble_controller_lock, K_MSEC(timeout));
#else
	return 0;
#endif
}

void multithreading_lock_release(void)
{
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFETY)
	k_sem_give(&ble_controller_lock);
#endif
}
