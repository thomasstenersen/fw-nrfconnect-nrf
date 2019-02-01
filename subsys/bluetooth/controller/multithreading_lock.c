/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include "multithreading_lock.h"

#include <soc.h>
#include <gpio.h>

/* Semaphore with a single count functioning as a lock. */
K_SEM_DEFINE(ble_controller_lock, 1, 1);

int api_lock(void)
{
	//printk("locking\n");
#if defined(CONFIG_BLECTLR_SHARED_API_BLOCKING)
#if defined(CONFIG_BLECTLR_SHARED_API_BLOCKING_WITH_TIMEOUT)
	return k_sem_take(&ble_controller_lock,
			  CONFIG_BLECTLR_API_BLOCKING_TIMEOUT_VALUE);
#elif defined(CONFIG_BLECTLR_SHARED_API_BLOCKING_FOREVER)
	return k_sem_take(&ble_controller_lock, K_FOREVER);
#else
	return k_sem_take(&ble_controller_lock, K_FOREVER);
#endif
#else
	return k_sem_take(&ble_controller_lock, K_NO_WAIT);
#endif
}

int api_try_lock(void)
{
	return k_sem_take(&ble_controller_lock, K_NO_WAIT);
}

void api_unlock(void)
{
	k_sem_give(&ble_controller_lock);
}
