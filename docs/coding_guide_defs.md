# C Coding Style Guide: Embedded Peripheral Definitions

Use this guide to generate peripheral definition header files. Adhere strictly to the following formatting, layout, and naming rules.

## 1. File Structure & Layout

### 1.1 File Header & License
* **Header:** Every header file must begin with a structured multi-line comment block. This block identifies the file's repository location, its copyright year, and its licensing terms.
  ```c
  /*
   * halm/platform/lpc/pin_defs.h
   * Copyright (C) <YEAR> <AUTHOR>
   * Project is distributed under the terms of the MIT License
   */
  ```
* **Year:** Use the **current calendar year**.
* **Author:** The full name of the author. If the file is auto-generated globally or the author is not known, fall back to **"Author"**.
* **Spacing:** A single empty line must be placed immediately after this file header block, before the opening include guard directive.

### 1.2 Visual Dividers & Block Headers
* Divide the include guard, inclusion block, and each block of definitions using a line-wide comment filled with hyphens. Do **not** place empty lines before or after this divider:
  ```c
  /*----------------------------------------------------------------------------*/
  ```
* For groups of register definitions, place the register name inside the visual divider comment block. The register name must start exactly at column 20 (the 21st character index). Do **not** place empty lines before or after this divider:
  ```c
  /*------------------USB Control register--------------------------------------*/
  ```

### 1.3 Include Guards
* **Naming:** Use `UPPER_CASE` and reflect the full repository path, replacing folder slashes and file extensions with underscores.
* **Format:** Ensure there is a trailing underscore.
  ```c
  #ifndef HALM_PLATFORM_LPC_PIN_DEFS_H_
  #define HALM_PLATFORM_LPC_PIN_DEFS_H_

  /* File contents go here */

  #endif /* HALM_PLATFORM_LPC_PIN_DEFS_H_ */
  ```

### 1.4 Header Inclusion Order
* **Grouping Rules:** Group headers into exactly three sections, sorted **alphabetically** within each group:
  1. Current project headers
  2. Third-party project headers
  3. System headers
* **Grouping Layout Rules:**
  * By default, do **not** use visual dividers or empty lines between these three groups.
  * **Exception:** If any single group contains more than 4 elements, break the default compression rule and separate all three groups from one another using a single empty line.
* **Standard Dependencies:** To support the fundamental bit manipulation operations outlined in Section 5, you must include the following library dependency inside the system headers section by default:
  ```c
  #include <xcore/bits.h>
  ```

## 2. Indentation & Spacing
* **Indentation Unit:** Spaces only. 1 unit = 2 spaces.
* **Block Indentation:** 2 spaces inside functions, structures, and enumerations.
* **Line Length:** Keep all lines strictly within a **maximum of 80 characters**.
* **Line Wrapping:** If an expression is too long and must span multiple lines, use a **4-space indent** for the continuation lines.

## 3. Macro Formatting & Alignment

### 3.1 Naming Conventions
* **Constants & Macros:** Written in `UPPER_CASE`. Macro names should not be prepended with the name of the peripheral.
* **Register Fields:** Constant names must be prefixed with the register name (e.g., `REG_FIELD_NAME`).

### 3.2 Default Alignment
The macro value expression must begin exactly at column index **40** (the 41st character of the line). This means the text of `#define`, the macro name, and its parameters must be padded out with spaces so that the value starts exactly at the same vertical column.

* Correct layout reference visual guide:
  ```
  Column Index:                        40 V
  1234567890123456789012345678901234567890|
  #define REG_CONSTANT                    VALUE
  ```

### 3.3 Handling Edge Cases with Line Wraps
* **Value Overflows Line:** If the macro starts at column 40 but its definition pushes past the 80-character margin, append a space and a backslash (`\`) to the macro name and put the entire value on the next line, indented by **4 spaces**.
* **Name is Too Long:** If the combined length of `#define` and macro name stretches to or past column 40 on its own, append a space and a backslash (`\`) at the end of the complete macro name, and drop the value down to the next line with a **4-space indent**.

### 3.4 Bit Ordering & Paragraph Spacing
* **Numerical Order:** Always sort your register bit definitions sequentially from the **lowest bit number to the highest bit number** (Bit 0 up to Bit 31).
* **Single-Bit Fields:** Keep single-bit flags packed closely together. **Do not** put empty lines between consecutive single-bit definitions.
* **Multi-Bit Fields:** Every time you define a multi-bit field, separate that entire macro block from surrounding fields using a single **empty line**.

### 3.5 Multi-Bit Field Blueprint
Whenever a register field spans **more than 1 bit**, you must provide a consistent set of three companion macros following this exact pattern:

1. **The Mask Macro:** Appends a `_MASK` suffix to the field name. Built using: `BIT_FIELD(MASK(width), offset)`
2. **The Shift Macro:** Uses the raw field name and accepts a `value` argument. Built using: `BIT_FIELD(value, offset)`
3. **The Value Extraction Macro:** Appends a `_VALUE` suffix to the name and accepts a register container variable (`reg`). Built using: `FIELD_VALUE(reg, mask, offset)`

### 3.6 Register Field Descriptions
A compact description must accompany every single-bit and multi-bit field block.

* **Placement:** The description must be a single-line or multi-line C-style comment (/* ... */) placed immediately above the field definition block (above the first single-bit flag or the `_MASK` macro of a multi-bit field).
* **Content:** Keep descriptions technical and concise.

## 4. Enumerations
* **Naming Style:** Use `CamelCase` for the enum type name (e.g., `CtrlMode`).
* **Value Elements:** Write enum elements in `UPPER_CASE`, always prefixing them with the name of the register field they belong to (e.g., `FIELD_VALUE_NAME`).
* **Equal Sign Alignment:** Horizontally align all `=` assignment operators in the block. Find the longest element name, add at least one space, and align the `=` operators to the next available **even column index** (e.g., column 20, 22, 24).

## 5. Standard Bitfield Manipulation Macros
Do not manually write bit-shifts (`<<` or `>>`) when defining fields. Instead, use these standard bit-level macros provided by `<xcore/bits.h>`:

| Intended Action | Macro Architecture Syntax |
| :--- | :--- |
| **Get a single bit mask** | `BIT(n)` |
| **Get an unshifted field mask** | `MASK(width)` |
| **Shift a value into a field position** | `BIT_FIELD(value, shift)` |
| **Shift a field mask into position** | `BIT_FIELD(MASK(width), shift)` |
| **Extract a field value** | `FIELD_VALUE(source, mask, shift)` |

## 6. Prohibited Practices
* **No Composite Structures:** Do not parse or emit hardware registers as C structures (`typedef struct { ... }`). Every register field must map exclusively to flat `#define` preprocessor statements.
* **No Address and Offset Definitions:** Memory addresses (`0x40001000`) and register offsets (`0x04`) are forbidden. This guide covers field masks and definitions only.

## 7. Comprehensive Example Header
Here is an example demonstrating all layout, wrapping, and alignment guidelines in practice:

```c
/*
 * myproject/drivers/periph_defs.h
 * Copyright (C) 2026 Author
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
/* Control Enable (Single bits grouped continuously with no blank lines) */
#define CTRL_ENABLE                     BIT(0)
/* Interrupt Flags */
#define CTRL_INT_MODE                   BIT(1)

/* Control Mode (Multi-bit field surrounded by blank lines) */
#define CTRL_MODE_MASK                  BIT_FIELD(MASK(2), 2)
#define CTRL_MODE(value)                BIT_FIELD(value, 2)
#define CTRL_MODE_VALUE(reg)            FIELD_VALUE(reg, CTRL_MODE_MASK, 2)

/* Control Mode enumeration (CamelCase type, odd-column aligned assignment) */
enum CtrlMode
{
  CTRL_MODE_IDLE    = 0,
  CTRL_MODE_ACTIVE  = 1,
  CTRL_MODE_BYPASS  = 2
};
/*------------------Status Register-------------------------------------------*/
/* Status Register fields (Over-length value expression wrap example) */
#define STAT_COMPLEX_EXPRESSION_VALUE_MASK \
    (BIT_FIELD(MASK(8), 16) | BIT_FIELD(MASK(4), 24))
#define STAT_GET_ERROR(reg) \
    FIELD_VALUE(reg, STAT_COMPLEX_EXPRESSION_VALUE_MASK, 16)

/* Over-length macro name wrap example (reaches past column 40) */
#define STAT_CRITICAL_ERROR_INTERRUPT_ENABLE_MASK \
    BIT(31)
/*----------------------------------------------------------------------------*/
#endif /* MYPROJECT_DRIVERS_PERIPH_DEFS_H_ */
```
