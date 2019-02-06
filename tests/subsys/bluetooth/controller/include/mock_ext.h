/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#ifndef __MOCK_EXT_H
#define __MOCK_EXT_H

#include <ztest.h>
#include <stdint.h>
#include <misc/util.h>

/**
 * @brief Expect the mocked function to be called
 *
 * @param fn Mocked function
 */
#define mock_expect(fn)				   \
	do {					   \
		ztest_expect_value(fn, called, 1); \
	} while (0);

/**
 * @brief Check that the mocked function is expected
 */
#define mock_check_expected()			    \
	do {					    \
		size_t called = 1;		    \
		ztest_check_expected_value(called); \
	} while (0);

/**
 * @brief Store the specified value to use it in the mocked function
 *
 * Multiple values are stored in a queue order.
 *
 * @param fn Mocked function
 * @param value Value to store
 */
#define mock_arg(fn, value)			\
	do {					\
		ztest_returns_value(fn, value);	\
	} while (0);

/**
 * @brief Obtain the stored value in the mocked function
 *
 * Multiple values are obtained in a queue order.
 *
 * @param var Name of the variable where the obtained value is to be stored
 */
#define mock_arg_get(var)			\
	do {					\
		var = ztest_get_return_value();	\
	} while (0);

/**
 * @brief Store the specified array to use it in the mocked function
 *
 * Multiple values are stored in a queue order.
 *
 * @param fn Mocked function
 * @param ptr Pointer to the array
 * @param len The arry length
 */
#define mock_arg_array(fn, ptr, len)			       \
	do {						       \
		ztest_returns_value(fn, len);		       \
		ztest_returns_value(fn, POINTER_TO_UINT(ptr)); \
	} while (0);

/**
 * @brief Obtain stored array in the mocked function
 *
 * Multiple arrays are obtained in a queue order.
 *
 * @param ptr Pointer to the obtained array
 * @param len Length of the obrained array
 */
#define mock_arg_array_get(ptr, len)				 \
	do {							 \
		len = ztest_get_return_value();			 \
		ptr = UINT_TO_POINTER(ztest_get_return_value()); \
	} while (0);

#endif /* __MOCK_EXT_H */
