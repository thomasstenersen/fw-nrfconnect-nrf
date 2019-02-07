/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <errno.h>

#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <flash.h>

#include "ble_controller_soc.h"

#include <logging/log.h>
#define LOG_MODULE_NAME blectrl_flash
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_DBG);

#define FLASH_DRIVER_WRITE_BLOCK_SIZE 1

static struct {
	/** Used to ensure a single ongoing operation at any time.  */
	struct k_sem sem;
	off_t addr;
	u16_t len;
	/* NOTE: Read is not async, so not a part of this enum. */
	enum {
		FLASH_OP_NONE,
		FLASH_OP_WRITE,
		FLASH_OP_ERASE
	} op;
} flash_state;

/* Forward declarations */
static int btctlr_flash_read(struct device *dev, off_t offset, void *data, size_t len);
static int btctlr_flash_write(struct device *dev, off_t offset, const void *data, size_t len);
static int btctlr_flash_erase(struct device *dev, off_t offset, size_t size);
static int btctlr_flash_write_protection_set(struct device *dev, bool enable);

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static void btctlr_flash_page_layout_get(struct device *dev,
					 const struct flash_pages_layout **layout,
					 size_t *layout_size);
#endif /* defined(CONFIG_FLASH_PAGE_LAYOUT) */

static const struct flash_driver_api btctrl_flash_api = {
	.read = btctlr_flash_read,
	.write = btctlr_flash_write,
	.erase = btctlr_flash_erase,
	.write_protection = btctlr_flash_write_protection_set,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	.page_layout = btctlr_flash_page_layout_get,
#endif  /* CONFIG_FLASH_PAGE_LAYOUT */
	.write_block_size = FLASH_DRIVER_WRITE_BLOCK_SIZE
};


/* Utility functions */
static inline bool is_addr_valid(off_t addr, size_t len)
{
	if ((addr + len) > NRF_FICR->CODEPAGESIZE * NRF_FICR->CODESIZE ||
	    addr < 0) {
		return false;
	}

	return true;
}

static inline bool is_page_aligned(off_t addr)
{
	return (addr % NRF_FICR->CODEPAGESIZE) == 0;
}

static void flash_operation_complete_callback(u32_t status)
{
	__ASSERT_NO_MSG(flash_state.op != FLASH_OP_NONE);
	flash_state.op = FLASH_OP_NONE;
	k_sem_give(&flash_state.sem);
	LOG_DBG("Flash operation complete\n");
}

/* Driver API. */

static int btctlr_flash_read(struct device *dev, off_t offset, void *data, size_t len)
{
	int err;

	if (!is_addr_valid(offset, len)) {
		return -EINVAL;
	}

	if (len == 0) {
		return 0;
	}

	/* TODO: CONFIG_MULTITHREADING */
	err = k_sem_take(&flash_state.sem, K_FOREVER);
	if (err) {
		return err;
	}

	memcpy(data, (void *)offset, len);
	k_sem_give(&flash_state.sem);

	return err;
}

static int btctlr_flash_write(struct device *dev, off_t offset, const void *data, size_t len)
{
	/* TODO: CONFIG_MULTITHREADING */
	/* TODO: Support multi-page write. */

	int err;

	err = k_sem_take(&flash_state.sem, K_FOREVER);
	if (err) {
		return err;
	}

	__ASSERT_NO_MSG(flash_state.op == FLASH_OP_NONE);
	flash_state.op = FLASH_OP_WRITE;
	err = ble_controller_flash_write((u32_t) offset, data, len,
					 flash_operation_complete_callback);
	return err;
}

static int btctlr_flash_erase(struct device *dev, off_t offset, size_t len)
{
	/* TODO: CONFIG_MULTITHREADING */
	int err;
	int page_count = len / NRF_FICR->CODEPAGESIZE;

	/* Follows the behavior of soc_flash_nrf.c */
	if (!(is_page_aligned(offset) && is_page_aligned(len)) ||
	    !is_addr_valid(offset, len)) {
		return -EINVAL;
	}

	/* TODO: Support multi-page erase. */
	if (page_count > 1) {
		return -EINVAL;
	}

	if (page_count == 0) {
		return 0;
	}

	err = k_sem_take(&flash_state.sem, K_FOREVER);
	if (err) {
		return err;
	}

	__ASSERT_NO_MSG(flash_state.op == FLASH_OP_NONE);
	flash_state.op = FLASH_OP_ERASE;
	err = ble_controller_flash_page_erase(offset,
					      flash_operation_complete_callback);
	return err;
}

static int btctlr_flash_write_protection_set(struct device *dev, bool enable)
{
	/* The BLE controller handles the write protection automatically. */
	return 0;
}

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static struct flash_pages_layout dev_layout;

static void btctlr_flash_page_layout_get(struct device *dev,
					 const struct flash_pages_layout **layout,
					 size_t *layout_size)
{
	*layout = &dev_layout;
	*layout_size = 1;
}
#endif /* defined(CONFIG_FLASH_PAGE_LAYOUT) */

static int nrf_btctrl_flash_init(struct device *dev)
{
	dev->driver_api = &btctrl_flash_api;
	k_sem_init(&flash_state.sem, 1, 1);

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	dev_layout.pages_count = NRF_FICR->CODESIZE;
	dev_layout.pages_size = NRF_FICR->CODEPAGESIZE;
#endif

	return 0;
}

DEVICE_INIT(nrf_btctrl_flash, DT_FLASH_DEV_NAME, nrf_btctrl_flash_init,
	    NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
