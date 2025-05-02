
cmake_minimum_required(VERSION 3.20.0)
set(SHIELD ssd1306_128x64)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(LGVL4)
include_directories(
    include
)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_sources(app PRIVATE
    src/bluetooth/ble_hrs.c
    src/bluetooth/ble_nus.c
    src/bluetooth/ble_service.c
)
target_sources(app PRIVATE src/watchdog.c)

