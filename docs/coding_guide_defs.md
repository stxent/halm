# C Coding Style Guide: Embedded Peripheral Definitions

Use this guide to generate header files and peripheral definitions. Adhere strictly to the following formatting, layout, and naming rules.

---

## 1. File Structure & Layout

### 1.1 File Header & License
* Every file must start with a file header containing the full repository file path, the **current year**, and the license statement wrapped in a multi-line comment block.
* **Author Field Rule:** Include the author's full name. If the author is unknown or the file is generated globally, use **"Author"** as the fallback name.
* **Format Requirement:** Match this exact block template:
  ```c
  /*
   * halm/platform/lpc/pin_defs.h
   * Copyright (C) <YEAR> <AUTHOR>
   * Project is distributed under the terms of the MIT License
   */
  ```
* A single empty line must be placed immediately after this file header block, before the opening include guard directive.

### 1.2 Visual Dividers & Block Headers
* Divide the include guard, inclusion block, and each block of definitions using a line-wide comment filled with hyphens. Do **not** place empty lines before or after this divider:
  ```c
  /*----------------------------------------------------------------------------*/
  ```
* For groups of register definitions, place the register name inside the comment line. The register name must start exactly at column 20 (the 21st character index). Do **not** place empty lines before or after this divider:
  ```c
  /*------------------USB Control register--------------------------------------*/
  ```

### 1.3 Include Guards
* **Format:** `UPPER_CASE`, matching the full repository path.
* **Structure:** `[LIBRARY]_[DIRECTORY_PATH]_[FILE_NAME_WITH_EXTENSION]_`
* **Example:**
  ```c
  #ifndef HALM_PLATFORM_LPC_PIN_DEFS_H_
  #define HALM_PLATFORM_LPC_PIN_DEFS_H_
  ```

### 1.4 Header Inclusion Order
* **Grouping Rules:** Group headers into exactly three sections, sorted **alphabetically** within each group:
  1. Current project headers
  2. Third-party project headers
  3. System headers
* **Grouping Layout Rules:**
  - By default, do **not** use visual dividers or empty lines between these three groups.
  - **Exception:** If any single group contains **more than 4 elements**, separate the groups using a single **empty line** instead of a commented hyphen line.
* **Standard Dependencies:** To support the fundamental bit manipulation operations outlined in Section 5, you must include the following library dependency inside the system headers section by default:
  ```c
  #include <xcore/bits.h>
  ```

---

## 2. Indentation & Spacing

* **Indentation Unit:** Spaces only. 1 unit = 2 spaces.
* **Line Width Limit:** Strictly 80 characters maximum.
* **Block Indentation:** 2 spaces inside functions, structures, and enumerations.
* **Expression Indentation:** 4 spaces by default for multi-line expressions.

---

## 3. Macro Formatting & Alignment

### 3.1 Value Alignment & Multi-line Splitting
* **Default Alignment:** The macro value expression must begin exactly at column index **40** (the 41st character of the line). This means the cumulative length of `#define`, the macro name, any parameter lists, and the trailing spacing must total exactly 40 characters before the value text starts.
  - Correct layout reference visual guide:
  ```
  Column Index:                        40 V
  1234567890123456789012345678901234567890|
  #define REG_CONSTANT                    VALUE
  ```
* **Value Exceeds 80 Characters:** If the definition starts at column 40 but overflows the 80-character line limit, add a space and a backslash (`\`) at the end of the macro name, and place the definition on the next line starting at **4 spaces**.
* **Macro Name Exceeds 40 Characters:** If the `#define` keyword combined with the macro name stretches to or beyond column 40, add a space and a backslash (`\`) at the actual end of the name, and place the value on the next line starting at **4 spaces**.

---

## 4. Naming Conventions & Enumerations

### 4.1 Basic Conventions
* **Constants & Macros:** Written in `UPPER_CASE`. Macro names should not be prepended with the name of the peripheral.
* **Register Fields:** Constant names must be prefixed with the register name (e.g., `REG_FIELD_NAME`).
* **Enumeration Names:** Written in `CamelCase` (e.g., `CtrlMode`).
* **Enumeration Values:** Written in `UPPER_CASE` and prefixed with the specific register field name (e.g., `FIELD_VALUE_NAME`).
* **Enumeration Formatting:** Horizontally align all `=` operators at the nearest odd-numbered column index that leaves at least one space after the longest enum name. Enforce this programmatically by ensuring the string length from the start of the line to the `=` symbol is an odd number.

### 4.2 Ordering Rule
* **Numerical Sorting:** All register field macro descriptions must be strictly sorted in numerical order from the **lowest bit number to the highest bit number**.

### 4.3 Multi-bit Register Fields
For every register field consisting of **more than 1 bit**, you must strictly define a set of three specific macros matching these naming and function patterns:

1. **Value Shift Macro:** Uses the raw field name without a suffix. Accepts a `value` parameter. Implemented using: `BIT_FIELD(value, offset)`
2. **Mask Macro:** Appends a `_MASK` suffix to the name. Implemented using: `BIT_FIELD(MASK(width), offset)`
3. **Value Extraction Macro:** Appends a `_VALUE` suffix to the name. Accepts a `reg` or field source parameter. Implemented using: `FIELD_VALUE(reg, mask, offset)`

**Separation & Layout Rule:** The macro definition group for a multi-bit field must be separated from neighboring macro groups or single-bit field definitions by a single empty line.

---

## 5. Standard Bitfield Manipulation Macros

Use these exact functional macro patterns for all bit-level operations (provided by `<xcore/bits.h>`):

| Operation | Macro Syntax |
| :--- | :--- |
| **Single Bit Value** | `BIT(n)` |
| **Unshifted Mask** | `MASK(width)` |
| **Shifted Value** | `BIT_FIELD(value, shift)` |
| **Shifted Mask** | `BIT_FIELD(MASK(width), shift)` |
| **Value Extraction** | `FIELD_VALUE(source, mask, shift)` |

---

## 6. Prohibited Structures

* All registers, offsets, and masks must be declared explicitly using independent preprocessor macros (`#define`). Do not group them into structural composite layouts.

---

## 7. Comprehensive Example Header

```c
/*
 * myproject/drivers/periph_defs.h
 * Copyright (C) 2026
 * Project is distributed under the terms of the MIT License
 */

#ifndef MYPROJECT_DRIVERS_PERIPH_DEFS_H_
#define MYPROJECT_DRIVERS_PERIPH_DEFS_H_
/*----------------------------------------------------------------------------*/
#include "myproject/core.h"
#include "myproject/drivers/periph.h"
#include <thirdparty/lib.h>
#include <xcore/bits.h>
#include <stdint.h>
/*------------------Control Register------------------------------------------*/
/* Control Enable */
#define CTRL_ENABLE                     BIT(0)

/* Control Mode (bits 1-2) */
#define CTRL_MODE_MASK                  BIT_FIELD(MASK(2), 1)
#define CTRL_MODE(value)                BIT_FIELD(value, 1)
#define CTRL_MODE_VALUE(reg)            FIELD_VALUE(reg, CTRL_MODE_MASK, 1)

/* Control Mode enumeration (CamelCase name and even char alignment) */
enum CtrlMode
{
  CTRL_MODE_IDLE    = 0,
  CTRL_MODE_ACTIVE  = 1,
  CTRL_MODE_BYPASS  = 2
};
/*------------------Status Register-------------------------------------------*/
/* Status Register fields */
#define STAT_COMPLEX_EXPRESSION_VALUE_MASK \
    BIT_FIELD(MASK(8), 16) | BIT_FIELD(MASK(4), 24)
#define STAT_GET_ERROR(reg) \
    FIELD_VALUE(reg, STAT_COMPLEX_EXPRESSION_VALUE_MASK, 16)

/* Long Definition name (Exceeds column 40 -> Next line uses 4 spaces) */
#define STAT_CRITICAL_ERROR_INTERRUPT_ENABLE_MASK \
    BIT(31)
/*----------------------------------------------------------------------------*/
#endif /* MYPROJECT_DRIVERS_PERIPH_DEFS_H_ */
```
