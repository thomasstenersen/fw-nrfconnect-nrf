/*
 * Copyright (c) 2019 - 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include "multithreading_lock.h"
#include "nrf.h"

static K_SEM_DEFINE(mpsl_lock, 1, 1);

int multithreading_lock_acquire(int32_t timeout)
{
	NRF_P0->OUTSET = 1 << 3;
	return k_sem_take(&mpsl_lock,
			  ((timeout == K_FOREVER || timeout == K_NO_WAIT) ?
			   timeout : K_MSEC(timeout)));
}

void multithreading_lock_release(void)
{
	NRF_P0->OUTCLR = 1 << 3;
	k_sem_give(&mpsl_lock);
}
