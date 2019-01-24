#
# Copyright (c) 2018 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.8.2)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

MESSAGE("Current dir:" ${CMAKE_CURRENT_SOURCE_DIR})
set(app_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/mock/ble_controller_soc_mock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../subsys/bluetooth/controller/crypto.c"
   )
target_sources(app PRIVATE ${app_sources})

target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE ./mock)
target_include_directories(app PRIVATE ${NRFXLIB_DIR}/ble_controller/include)
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/include/bluetooth)
