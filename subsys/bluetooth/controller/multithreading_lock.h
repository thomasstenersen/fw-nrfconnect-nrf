/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

/**
 * @lock.h
 *
 * @brief This file defines APIs needed to lock the BLE controller APIs for threadsafe operation.
 */

#ifndef MULTITHREADING_LOCK_H__
#define MULTITHREADING_LOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>

/** Macro representation for unlock API */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
#define API_UNLOCK multithreading_lock_unlock()
#else
#define API_UNLOCK
#endif

/** Macro representation for typical usage of locking API */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
#define API_LOCK_AND_RETURN_ON_FAIL \
	do {			    \
		s32_t err = multithreading_lock();   \
		if (err != 0) {	    \
			return err; \
		}		    \
	} while (0)
#else
#define API_LOCK_AND_RETURN_ON_FAIL
#endif

/** @brief Try to take the lock while maintaining the specified blocking behavior.
 *
 * This API call will be blocked forever if @ref CONFIG_BLECTLR_THREADSAFE_BLOCKING and
 * @ref CONFIG_BLECTLR_THREADSAFE_BLOCKING_FOREVER options are selected. Otherwise API call
 * will wait for timeout with a given timeout value specified by
 * @ref CONFIG_BLECTLR_THREADSAFE_BLOCKING_WITH_TIMEOUT and return error if lock cannot be
 * obtained. If @ref CONFIG_BLECTLR_THREADSAFE_BLOCKING is not specified, API will return
 * immediately in the event of not being able to obtain a lock.
 *
 * @retval 0			Success
 * @retval - ::NRF_EBUSY	Returned without waiting.
 * @retval - ::NRF_EAGAIN	Waiting period timed out.
 */
int multithreading_lock(void);

/** @brief Try to take the lock and return immediately on failure.
 *
 * This API is useful for use cases where waiting is not desirable (e.g. calling library API
 * from IRQ).
 *
 * @retval 0			Success
 * @retval - ::NRF_EBUSY	Returned without waiting.
 */
int multithreading_lock_try(void);

/** @brief Unlock the lock.
 *
 * @note This API is must be called only after lock is obtained. Otherwise it will block
 * execution for forever.
 */
void multithreading_lock_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* MULTITHREADING_LOCK_H__ */
