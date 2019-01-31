/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <ztest.h>
#include <stdint.h>

/**
 * @brief Expect the mocked function to be called
 *
 * @param fn Mocked function
 */
#define mock_expect(fn, times)			       \
	do {					       \
		ztest_expect_value(fn, called, times); \
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
 * @brief Obtain stored value in the mocked function
 *
 * Multiple values are obtained in a queue order.
 *
 * @param value
 */
#define mock_arg_get(value)			  \
	do {					  \
		value = ztest_get_return_value(); \
	} while (0);
