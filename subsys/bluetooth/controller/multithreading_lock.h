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

/** Macro for calling ble controller API with a return value in threadsafe manner */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
#define THREADSAFE_CALL_WITH_RETCODE(ERROR_CODE_VAR, FUNC_CALL)                 \
	do {                                                                   \
		ERROR_CODE_VAR = multithreading_lock_acquire();                \
		if (ERROR_CODE_VAR == 0) {                                     \
			ERROR_CODE_VAR = FUNC_CALL;                            \
			multithreading_lock_release();                         \
		}                                                              \
	} while (0)
#else
#define THREADSAFE_CALL_WITH_RETCODE(ERROR_CODE_VAR, FUNC_CALL)                 \
	do {                                                                   \
		ERROR_CODE_VAR = FUNC_CALL;                                    \
	} while (0)
#endif

/** Macro for calling ble controller API without a return value in threadsafe manner */
#if IS_ENABLED(CONFIG_BLECTLR_THREADSAFE_BLOCKING)
#define THREADSAFE_CALL_WITHOUT_RETCODE(ERROR_CODE_VAR, FUNC_CALL)              \
	do {                                                                   \
		ERROR_CODE_VAR = multithreading_lock_acquire();                \
		if (ERROR_CODE_VAR == 0) {                                     \
			FUNC_CALL;                                             \
			multithreading_lock_release();                         \
		}                                                              \
	} while (0)
#else
#define THREADSAFE_CALL_WITHOUT_RETCODE(ERROR_CODE_VAR, FUNC_CALL)              \
	do {                                                                   \
		FUNC_CALL;                                                     \
		ERROR_CODE_VAR = 0;                                            \
	} while (0)
#endif

/** @brief Try to take the lock while maintaining the specified blocking behavior.
 *
 * This API call will be blocked for the time specified by @ref
 * CONFIG_BLECTLR_THREADSAFE_BLOCKING_TIMEOUT and then return error code.
 *
 * @retval 0			Success
 * @retval - ::NRF_EBUSY	Returned without waiting.
 * @retval - ::NRF_EAGAIN	Waiting period timed out.
 */
int multithreading_lock_acquire(void);

/** @brief Try to take the lock and return immediately on failure.
 *
 * This API is useful for use cases where waiting is not desirable (e.g. calling library API
 * from IRQ).
 *
 * @retval 0			Success
 * @retval - ::NRF_EBUSY	Returned without waiting.
 */
int multithreading_lock_acquire_try(void);

/** @brief Unlock the lock.
 *
 * @note This API is must be called only after lock is obtained.
 */
void multithreading_lock_release(void);

#ifdef __cplusplus
}
#endif

#endif /* MULTITHREADING_LOCK_H__ */
