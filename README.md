# HALM

![C21](https://img.shields.io/badge/C-21-blue)
![CMake](https://img.shields.io/badge/CMake-3.21+-blue)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A library of basic drivers for microcontrollers. It is written using
the C21 standard and CMake.

## Supported Platforms

Library supports the following platforms:

* **x86** — minimal implementation provided for testing other projects
  and for development/debugging purposes on desktop systems
* **NXP LPC82x, LPC11xx, LPC11Exx, LPC13xx, LPC13Uxx, LPC17xx, LPC43xx**;
  for **LPC43xx**, both the main **M4 core** and auxiliary **M0 cores**
  are supported
* **NXP i.MX RT106x**, currently has initial support
* **STMicro STM32F0, STM32F1, STM32F4**
* **Nuvoton M03x, M48x**
* **Bouffalo Lab BL602/BL604**, currently has initial support

> **Note:** Platform support levels may vary, platforms with "initial support"
have limited driver availability.

## Required Packages

To build and use the library, you need the following software installed
on your system:

* **GCC 13 or newer** — the GNU Compiler Collection, required for
  building the x86 version.
* **ARM GCC 13 or newer** — Arm GNU Toolchain, required for
  Cortex-M embedded targets.
* **RISC-V GCC 13 or newer** — RISC-V GNU Toolchain, required for
  RISC-V embedded targets.
* **CMake 3.21 or newer** — used for configuring and generating build
  systems across platforms.
* **Kconfiglib** or **kconfig‑frontends** — tools for interactive library
  configuration via `menuconfig`.
* **Xcore** — can be built separately or included as CMake submodule
  in the project.

## Library Contents

The library includes the following major components:

### Peripheral Device Drivers

Provides low‑level access to hardware peripherals, with a consistent API
across platforms. Supports the following peripherals:

* ADC and DAC
* DMA
* GPIO
* Timers and watchdog
* External interrupts
* UART, SPI, I2C, I2S, CAN, SDIO (in blocking and zero‑copy modes)
* QSPI and specialized peripherals with XIP
* USB FS and HS
* Ethernet
* Built‑in Flash and EEPROM
* External parallel SRAM and SDRAM
* Clock management and power saving
* Specialized peripherals

### Generic Drivers

Higher‑level software components that abstract common patterns:

* Software timers
* Task queues
* SDIO via SPI and SD/MMC

### USB Device Drivers

Support for USB device functionality:

* Platform‑dependent parts
* Platform‑independent drivers for CDC, HID, MSC and UAC
* Composite devices

## Usage Examples

Below are common scenarios for using the library in your projects.

1. Including in a CMake Project

To include the library as a subdirectory in your CMake project,
use `add_subdirectory` and specify the configuration file via
the `HALM_CONFIG_FILE` variable:

```cmake
add_subdirectory("/path/to/halm" halm)
```

Make sure to set the HALM_CONFIG_FILE variable to point to your configuration
file before including the library. This file defines which drivers and features
are compiled in.

Example in CMakeLists.txt:

```cmake
set(HALM_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/halm.config")
add_subdirectory("/path/to/halm" halm)
```

2. Building for x86 Outside the Project Tree

When building for x86 outside the project tree, configure the library
using menuconfig and build:

```sh
make menuconfig
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/output/dir/ \
  -DCMAKE_PREFIX_PATH=/path/to/output/dir/
make
make install
```

3. Building for LPC175x Outside the Project Tree

For building under LPC175x outside the project tree, use the following commands:

```sh
make menuconfig
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/output/dir/ \
  -DCMAKE_PREFIX_PATH=/path/to/output/dir/ \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/xcore/toolchains/cortex-m3.cmake \
  -DPLATFORM=LPC17XX
make
make install
```

The toolchain file sets up the cross‑compiler, and the PLATFORM variable tells
which MCU‑specific code to include.

## Build Options

The following build options are available to customize your build:

* **PLATFORM** — specifies the target platform for the build. Possible values:

      IMXRT106X — NXP i.MX RT106x series
      LPC82X — NXP LPC82x series
      LPC11XX — NXP LPC11xx series
      LPC11EXX — NXP LPC11Exx series
      LPC13XX — NXP LPC13xx series
      LPC13UXX — NXP LPC13Uxx series
      LPC43XX — NXP LPC43xx series (main M4 core)
      LPC43XX_M0APP — NXP LPC43xx series, M0 application core
      LPC43XX_M0SUB — NXP LPC43xx series, M0 subcore
      M03X — Nuvoton M03x series
      M48X — Nuvoton M48x series
      STM32F0XX — STMicro STM32F0 series
      STM32F1XX — STMicro STM32F1 series
      STM32F4XX — STMicro STM32F4 series
      BL602 — Bouffalo Lab BL602/BL604 series

  If the PLATFORM option is omitted, x86 target is used.

* **HALM_CONFIG_FILE** — path to the library configuration file (.config).
  This file can be created or edited using menuconfig. It defines the features,
  peripherals, and drivers to be included in the build.
