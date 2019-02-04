/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include "multithreading_lock.h"

/* Semaphore with a single count functioning as a lock. */
K_SEM_DEFINE(ble_controller_lock, 1, 1);

int multithreading_lock_acquire(void)
{
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
	return k_sem_take(&ble_controller_lock,
			  CONFIG_BLECTLR_THREADSAFE_BLOCKING_TIMEOUT);
#else
	return 0;
#endif
}

int multithreading_lock_acquire_try(void)
{
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
	return k_sem_take(&ble_controller_lock, K_NO_WAIT);
#else
	return 0;
#endif
}

void multithreading_lock_release(void)
{
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
	k_sem_give(&ble_controller_lock);
#endif
}
