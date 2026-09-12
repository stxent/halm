#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# linter_defs.py
# Copyright (C) 2026 xent
# Project is distributed under the terms of the MIT License

import os
import re
import sys


class StyleLinter:
    def __init__(self, filepath: str):
        self.filepath = filepath
        with open(filepath, 'r', encoding='utf-8') as f:
            self.lines = f.readlines()
        self.errors = []

    def report(self, line_num: int, rule_str: str, desc: str):
        '''Appends a style violation to the errors report list.'''
        self.errors.append(f'Line {line_num} [{rule_str}]: {desc}')

    def check_line_lengths(self):
        '''Rule 2: Strictly 80 characters maximum per line.'''
        for idx, line in enumerate(self.lines, 1):
            length = len(line.rstrip('\r\n'))
            if length > 80:
                self.report(idx, 'Rule 2', f'Line exceeds 80 characters ({length} chars)')

    def check_file_header(self):
        '''Rule 1.1: File Header & License block format and spacing.'''
        if not self.lines:
            self.report(1, 'Rule 1.1', 'File is completely empty')
            return

        if len(self.lines) < 5:
            self.report(1, 'Rule 1.1', 'File too short for valid header block')
            return

        if self.lines[0].strip() != '/*':
            self.report(1, 'Rule 1.1', 'File must start exactly with "/*"')

        if not any(re.search(r'Copyright\s+\(C\)\s+\d{4}', l) for l in self.lines[:5]):
            self.report(3, 'Rule 1.1', 'Missing or invalid "Copyright (C) <YEAR>"')

        if not any(re.search(r'MIT License', l) for l in self.lines[:5]):
            self.report(4, 'Rule 1.1', 'Missing expected MIT License text statement')

    def check_visual_dividers(self):
        '''Rule 1.2: No empty lines around dividers, and strict maximum line length.'''
        for idx, line in enumerate(self.lines):
            line_stripped = line.strip()
            if line_stripped.startswith('/*---') and line_stripped.endswith('*/'):
                line_num = idx + 1

                # Check visual divider line length
                length = len(line.rstrip('\r\n'))
                if length != 80:
                    self.report(line_num, 'Rule 1.2',
                        f'Visual divider must be strictly 80 characters ({length} chars)')

                # Check for consecutive/double visual dividers
                if (idx > 0 and self.lines[idx - 1].strip().startswith('/*---')
                    and self.lines[idx - 1].strip().endswith('*/')):
                    self.report(line_num, 'Rule 1.2', 'Consecutive visual dividers')

                # Check line before the divider
                if idx > 0 and self.lines[idx - 1].strip() == '':
                    self.report(line_num, 'Rule 1.2', 'Empty line directly before visual divider')

                # Check line after the divider
                if idx < len(self.lines) - 1 and self.lines[idx + 1].strip() == '':
                    self.report(line_num, 'Rule 1.2', 'Empty line directly after visual divider')

    def check_bit_field_spacing(self):
        '''Rule 3.4: Single-bit field definitions must not be divided by empty lines.'''
        is_bit_field = False
        last_bit_field_idx = -1

        for idx, line in enumerate(self.lines):
            line_stripped = line.strip()

            # Identify if the line is a single-bit macro definition (e.g., BIT(x))
            # It matches lines ending with BIT(<digits>) or BIT(<digits>)\r\n
            has_bit = '#define' in line and re.search(r'BIT\(\d+\)', line_stripped)

            if has_bit:
                # If we were already in a bit-field block and an empty line occurred
                # between the last bit-field macro and this one
                if is_bit_field and last_bit_field_idx != -1:
                    # Scan the lines between the last bit field and this one
                    for check_idx in range(last_bit_field_idx + 1, idx):
                        if self.lines[check_idx].strip() == '':
                            self.report(idx + 1, 'Rule 3.4',
                                'Empty line between sequential single-bit field definitions')
                            break # Only report once per block separation

                is_bit_field = True
                last_bit_field_idx = idx
            elif (line_stripped != '' and not line_stripped.startswith('/*')
                and not line_stripped.startswith('*') and not line_stripped.endswith('*/')):
                # Reset tracking if we encounter an actual code line
                is_bit_field = False
                last_bit_field_idx = -1

    def check_macro_alignments(self):
        '''Rule 3: Value alignment, multi-line splits and naming restrictions.'''
        guard_token = re.sub(r'[^a-zA-Z0-9]', '_', self.filepath).upper() + '_'

        filename = os.path.basename(self.filepath)
        periph_match = re.match(r'^([a-zA-Z0-9]+)_', filename)
        periph_prefix = (periph_match.group(1).upper() + '_') if periph_match else None

        for idx, line in enumerate(self.lines, 1):
            if line.startswith('#define'):
                name_match = re.match(r'^#define\s+([A-Za-z0-9_]+)', line)
                if not name_match:
                    continue
                macro_name = name_match.group(1)

                # Skip 40-column strict alignment check for include guards and header path
                if (guard_token in macro_name or macro_name.endswith('_H_')
                    or macro_name == 'HEADER_PATH'):
                    continue

                if not macro_name.isupper() and not macro_name.startswith('_'):
                    self.report(idx, 'Rule 3.1', f'Macro "{macro_name}" must be in UPPER_CASE')

                if periph_prefix and macro_name.startswith(periph_prefix):
                    self.report(idx, 'Rule 3.1',
                        f'Macro contains prohibited peripheral prefix "{periph_prefix}"')

                # Check backslash line continuation rules
                if line.rstrip().endswith('\\'):
                    # Rule 3.3: Must have exactly a single space before the backslash
                    if not re.search(r'(?<!\s)\s\\$', line.rstrip('\r\n')):
                        self.report(idx, 'Rule 3.3',
                            'Multi-line macro must have exactly one space before the backslash')

                    # Check for incorrect long line breaking
                    match_regexp = r'^#define\s+[A-Za-z0-9_]+\([^)]*\)|^#define\s+[A-Za-z0-9_]+'
                    match = re.match(match_regexp, line)
                    if match:
                        line_end_idx = line.rstrip().rfind('\\')
                        leftover_content = line[match.end():line_end_idx].strip()
                        if leftover_content:
                            self.report(idx, 'Rule 3.3',
                                'Incorrect line breaking, full macro value must be on a new line')

                match_regexp = r'^#define\s+[A-Za-z0-9_]+\([^)]*\)|^#define\s+[A-Za-z0-9_]+'
                match = re.match(match_regexp, line)
                if match:
                    combined_len = len(match.group(0))

                    if combined_len >= 40:
                        if not line.rstrip().endswith('\\'):
                            self.report(idx, 'Rule 3.3',
                                'Macro reaches or exceeds column 40, line must be split')
                        if idx < len(self.lines) and not self.lines[idx].startswith('    '):
                            self.report(idx + 1, 'Rule 3.3',
                                'Multi-line macro split value must start with 4 spaces indentation')
                    else:
                        remaining = line[combined_len:]
                        required = 40 - combined_len

                        if (not remaining.startswith(' ' * required) or (len(remaining) > required
                            and remaining[required] == ' ')):
                            if remaining.strip() and not line.rstrip().endswith('\\'):
                                self.report(idx, 'Rule 3.2',
                                    'Macro value must begin exactly at column index 40')

    def check_enum_alignment(self):
        '''Rule 4: Enumeration formatting and odd-numbered column index alignment constraint.'''
        in_enum = False
        for idx, line in enumerate(self.lines, 1):
            if 'enum ' in line and '{' in line:
                in_enum = True
                continue
            if in_enum and '};' in line:
                in_enum = False
                continue

            if in_enum and '=' in line:
                pos = line.find('=')
                if pos % 2 == 0:
                    self.report(idx, 'Rule 4',
                       '"=" operator must sit at an odd-numbered column index')

    def check_prohibited_structs(self):
        '''Rule 6: Prohibited Structural Composite Register Declarations.'''
        for idx, line in enumerate(self.lines, 1):
            if 'typedef struct' in line or (line.strip().startswith('struct') and '{' in line):
                self.report(idx, 'Rule 6',
                    'Registers must use independent preprocessor macros, structs are banned')

    def run_all(self):
        self.check_line_lengths()
        self.check_file_header()
        self.check_visual_dividers()
        self.check_bit_field_spacing()
        self.check_macro_alignments()
        self.check_enum_alignment()
        self.check_prohibited_structs()
        return self.errors


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: linter_defs.py <path_to_header.h>')
        sys.exit(1)

    linter = StyleLinter(sys.argv[1])
    violations = linter.run_all()

    if violations:
        print(f'❌ Found {len(violations)} style guide violations:')
        for error in violations:
            print(error)
        sys.exit(1)
    else:
        print('✅ Header conforms perfectly to the style guide')
        sys.exit(0)
