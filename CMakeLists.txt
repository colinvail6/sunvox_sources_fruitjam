cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PICO_PLATFORM rp2350)
set(PICO_BOARD adafruit_fruit_jam CACHE STRING "Board type")
set(PICO_PIO_USB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Pico-PIO-USB)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

include(pico_sdk_import.cmake)

project(sundog_fruitjam C CXX ASM)

pico_sdk_init()

# --- FatFS library (SD card) - vendored directly, same layout as wili8jam ---
add_library(fatfs STATIC
    fatfs/ff.c
    fatfs/ffunicode.c
    fatfs/ffsystem.c
    fatfs/diskio.c
    fatfs/sdcard.c
)
target_include_directories(fatfs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs
)
target_link_libraries(fatfs PUBLIC
    pico_stdlib
    hardware_spi
    hardware_gpio
)

# --- USB Host (reuse wili8jam's usb-host/ tree wholesale if you copy it in) ---
add_subdirectory(usb-host)

# --- Sundog engine sources ---
set(SUNDOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sundog_engine)

add_executable(sundog_fruitjam
    ${SUNDOG_DIR}/main/code/main.cpp
    ${SUNDOG_DIR}/core/code/debug.cpp
    ${SUNDOG_DIR}/memory/code/memory.cpp
    ${SUNDOG_DIR}/filesystem/code/v3nus_fs.cpp
    ${SUNDOG_DIR}/time/code/timemanager.cpp
    ${SUNDOG_DIR}/sound/code/sound_player.cpp
    ${SUNDOG_DIR}/sound/code/sound_sndout.cpp
    ${SUNDOG_DIR}/window_manager/code/wmanager.cpp
    ${SUNDOG_DIR}/utils/code/utils.cpp
    # TinyUSB host stack — must be compiled into the executable, same reasoning
    # as wili8jam's comment: avoids duplicate symbols with the device stack.
    ${FWUSBHOST_TINYUSB_HOST_SOURCES}
)

target_compile_definitions(sundog_fruitjam PRIVATE
    FRUITJAM
    COLOR32BITS
    ARCH_ARM
)

# Generate PIO header if you follow wili8jam's PIO-I2S approach for audio
# (recommended - see src/audio_i2s.pio and src/audio.c in wili8jam for reference)
# pico_generate_pio_header(sundog_fruitjam ${CMAKE_CURRENT_SOURCE_DIR}/sundog_engine/sound/code/audio_i2s.pio)

target_include_directories(sundog_fruitjam PRIVATE
    ${SUNDOG_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/fatfs
)

target_link_libraries(sundog_fruitjam PRIVATE
    fatfs
    fwUSBHost
    pico_stdlib
    hardware_pio
    hardware_spi
    hardware_sync
    hardware_dma
    hardware_clocks
    hardware_i2c
)

target_compile_definitions(sundog_fruitjam PRIVATE PICO_STACK_SIZE=0x2000)

pico_enable_stdio_usb(sundog_fruitjam 1)
pico_enable_stdio_uart(sundog_fruitjam 0)

pico_add_extra_outputs(sundog_fruitjam)
