#
# Copyright (c) 2019 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.8.2)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

MESSAGE("Current dir:" ${CMAKE_CURRENT_SOURCE_DIR})
set(app_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../subsys/bluetooth/controller/rng_driver.c"
   )

set(OPTIONS
    -DUNIT_TEST
    -DNRF5_IRQ_RNG_IRQn=5
    -DCONFIG_ENTROPY_NAME="Entropy"
    )
target_sources(app PRIVATE ${app_sources})

target_compile_options(app PRIVATE ${OPTIONS})

target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE ../include)
target_include_directories(app PRIVATE ${NRFXLIB_DIR}/ble_controller/include)
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/include/bluetooth)
