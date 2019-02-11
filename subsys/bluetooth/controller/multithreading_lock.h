/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/**
 * @multithreading_lock.h
 *
 * @brief This file defines APIs needed to lock the BLE controller APIs for threadsafe operation.
 */

#ifndef MULTITHREADING_LOCK_H__
#define MULTITHREADING_LOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>

/** Macro for acquiring a lock */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFETY)
#define MULTITHREADING_LOCK_ACQUIRE() \
	multithreading_lock_acquire(CONFIG_BLECTLR_THREADSAFETY_LOCK_TIMEOUT_MSEC)
#else
#define MULTITHREADING_LOCK_ACQUIRE() (0)
#endif

/** Macro for acquiring a lock without waiting for use in special cases. */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFETY)
#define MULTITHREADING_LOCK_ACQUIRE_NO_WAIT() \
	multithreading_lock_acquire(K_NO_WAIT)
#else
#define MULTITHREADING_LOCK_ACQUIRE_NO_WAIT() (0)
#endif

/** Macro for acquiring a lock while waiting forever for use in special cases. */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFETY)
#define MULTITHREADING_LOCK_ACQUIRE_FOREVER_WAIT() \
	multithreading_lock_acquire(K_FOREVER)
#else
#define MULTITHREADING_LOCK_ACQUIRE_FOREVER_WAIT() (0)
#endif

/** Macro for releasing a lock */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFETY)
#define MULTITHREADING_LOCK_RELEASE() multithreading_lock_release()
#else
#define MULTITHREADING_LOCK_RELEASE()
#endif

/** @brief Try to take the lock while maintaining the specified blocking behavior.
 *
 * This API call will be blocked for the time specified by @ref
 * CONFIG_BLECTLR_THREADSAFETY_LOCK_TIMEOUT_MSEC and then return error code.
 *
 * @param  timeout		Timeout value (in milliseconds) for the locking API.
 *
 * @retval 0			Success
 * @retval - ::NRF_EBUSY	Returned without waiting.
 * @retval - ::NRF_EAGAIN	Waiting period timed out.
 */
int32_t multithreading_lock_acquire(int timeout);

/** @brief Unlock the lock.
 *
 * @note This API is must be called only after lock is obtained.
 */
void multithreading_lock_release(void);

#ifdef __cplusplus
}
#endif

#endif /* MULTITHREADING_LOCK_H__ */
