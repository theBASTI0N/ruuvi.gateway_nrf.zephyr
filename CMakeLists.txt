cmake_minimum_required(VERSION 3.13.1)
# Re-direct the directory where the 'boards' directory is found from
# $ZEPHYR_BASE to this directory.
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})

set(BOARD nrf52_pca10040)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(ruuvi.gateway_nrf.zephyr)

target_sources(app PRIVATE src/main.c)