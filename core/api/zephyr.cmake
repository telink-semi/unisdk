# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module — API layer
#
# This file provides Zephyr build integration for the optional UniSDK
# API modules (sleep API, time API).
#
# Variables expected from the caller (set in zephyr/CMakeLists.txt):
#   UNISDK_BASE  - path to the unisdk root directory

zephyr_sources_ifdef(CONFIG_TLK_API_SLEEP ${UNISDK_BASE}/api/src/tlk_sleep.c)
zephyr_sources_ifdef(CONFIG_TLK_API_TIME  ${UNISDK_BASE}/api/src/tlk_time.c)
