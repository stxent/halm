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

    def _iter_comment_blocks(self):
        '''Yield (start_idx, end_idx) for every C-style comment block in the file.

        Indexes are 0-based and inclusive. A visual divider (a single-line
        block such as ``/*---*/``) is not yielded, as it is handled by the
        dedicated divider check. A block whose opening line is the very first
        line of the file is the file header.
        '''
        num_lines = len(self.lines)
        idx = 0
        while idx < num_lines:
            line_stripped = self.lines[idx].strip()

            # Skip non-comment lines
            if not line_stripped.startswith('/*'):
                idx += 1
                continue

            # Skip visual dividers (handled by check_visual_dividers)
            if line_stripped.startswith('/*---') and line_stripped.endswith('*/'):
                idx += 1
                continue

            start = idx
            end = start
            while end < num_lines and '*/' not in self.lines[end]:
                end += 1
            # If the closing marker is never found, treat the rest of the file
            # as the (unterminated) block so that it is still inspected.
            if end >= num_lines:
                end = num_lines - 1

            yield start, end
            idx = end + 1

    def _comment_content(self, start, end):
        '''Return (first_line_idx, last_line_idx) that hold the actual text.

        For a well-formed multi-line comment the first and last lines carry
        only the delimiters (``/*`` and ``*/``) and the text lives on the
        middle lines. For a single-line comment the text lives on the single
        line itself. Malformed layouts may place text on the delimiter lines;
        those lines are still returned so the caller can inspect them.
        '''
        first = start
        last = end

        # Multi-line: first line holds only the opening delimiter
        if end > start and self.lines[start].strip() == '/*':
            first = start + 1

        # Multi-line: last line holds only the closing delimiter
        if end > start and self.lines[end].strip() == '*/':
            last = end - 1

        return first, last

    @staticmethod
    def _strip_comment_marker(text):
        '''Return the text of a comment line without the ``/*``, ``*`` or ``*/`` markers.'''
        stripped = text.strip()
        if stripped.startswith('/*'):
            stripped = stripped[2:]
        if stripped.startswith('*'):
            stripped = stripped[1:]
        if stripped.endswith('*/'):
            stripped = stripped[:-2]
        return stripped.strip()

    def check_comment_formatting(self):
        '''Style Rule 11: Formatting that applies to every comment (single and multi-line).

        For all comments:
          1) the first letter of the sentence must be capitalized,
          2) there must be exactly one space after ``/*`` and exactly one
             space before ``*/`` (for single-line comments).
        '''
        for start, end in self._iter_comment_blocks():
            # The file header has its own dedicated format (Rule 1.1)
            if start == 0:
                continue

            first, last = self._comment_content(start, end)

            # Style Rule 11: first letter of the first text line must be capitalized
            first_content_idx = None
            for candidate in range(first, last + 1):
                content = self._strip_comment_marker(self.lines[candidate])
                if content == '':
                    continue
                first_content_idx = candidate
                break

            if first_content_idx is not None:
                content = self._strip_comment_marker(
                    self.lines[first_content_idx])
                if content[0].islower():
                    self.report(first_content_idx + 1, 'Style Rule 11',
                        'Comment must start with a capitalized letter')

            # Style Rule 11: single-line comments need exactly one space after
            # "/*" and exactly one space before "*/"
            if end == start:
                stripped = self.lines[start].strip()
                if not stripped.startswith('/* ') or stripped.startswith('/*  '):
                    self.report(start + 1, 'Style Rule 11',
                        'Single-line comment must have exactly one space after "/*"')
                if not stripped.endswith(' */') or stripped.endswith('  */'):
                    self.report(start + 1, 'Style Rule 11',
                        'Single-line comment must have exactly one space before "*/"')

    def check_multiline_comments(self):
        '''Style Rule 11: Well-formed multi-line comment structure.

        A multi-line comment must:
          1) have its first line contain only the ``/*`` marker,
          2) end with a period (a final sentence that is terminated),
          3) have its last line contain only the ``*/`` marker.
        '''
        for start, end in self._iter_comment_blocks():
            # A single-line block cannot be a (malformed) multi-line comment
            if end == start:
                continue

            # The file header has its own dedicated format (Rule 1.1)
            if start == 0:
                continue

            line_num = start + 1
            first_line = self.lines[start]
            last_line = self.lines[end]

            # Style Rule 11: first line must contain only "/*"
            if first_line.strip() != '/*':
                self.report(line_num, 'Style Rule 11',
                    'Multi-line comment first line must contain only "/*"')

            # Style Rule 11: last line must contain only "*/"
            if last_line.strip() != '*/':
                self.report(end + 1, 'Style Rule 11',
                    'Multi-line comment last line must contain only "*/"')

            # Style Rule 11: the comment must end with a period. Locate the last
            # line that carries actual text (skip empty lines and the bare
            # closing-delimiter line).
            text_idx = None
            for candidate in range(end, start - 1, -1):
                candidate_stripped = self.lines[candidate].strip()
                if candidate_stripped in ('', '*/'):
                    continue
                content = self._strip_comment_marker(self.lines[candidate])
                if content == '':
                    continue
                text_idx = candidate
                break

            if text_idx is None:
                self.report(line_num, 'Style Rule 11', 'Multi-line comment is empty')
                continue

            final_text = self._strip_comment_marker(self.lines[text_idx])
            if not final_text.endswith('.'):
                self.report(text_idx + 1, 'Style Rule 11',
                    'Multi-line comment must end with a period')

    def check_bit_field_comments(self):
        '''Rule 3.6: Macro definitions must be preceded by a field description comment.'''
        for idx, line in enumerate(self.lines):
            line_num = idx + 1
            line_stripped = line.strip()

            # Process single-bit field macros or the start of a multi-bit triplet (_MASK)
            is_single_bit = '#define' in line and re.search(r'BIT\(\d+\)', line_stripped)
            is_multi_bit_start = re.match(r'^#define\s+\w+_MASK\b', line_stripped)

            if is_single_bit or is_multi_bit_start:
                # Look back to find a valid comment block preceding this line
                has_comment = False

                # Check the immediate lines above for a C-style comment string
                for check_idx in range(idx - 1, -1, -1):
                    prev_line = self.lines[check_idx].strip()

                    # Stop searching if we hit an empty line or a divider
                    if prev_line == '' or prev_line.startswith('/*---'):
                        break

                    # Valid comments look like: /* comment */ or * comment or // comment
                    if (prev_line.startswith('/*') or prev_line.startswith('*')
                        or prev_line.startswith('//')):
                        has_comment = True
                        break

                if not has_comment:
                    macro_name = re.search(r'#define\s+([A-Za-z0-9_]+)', line_stripped).group(1)
                    self.report(
                        line_num,
                        'Rule 3.6',
                        f'Missing description comment before macro "{macro_name}"'
                    )

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

    def check_macro_sorting_by_offset(self):
        '''Rule 3.4: Macros inside register descriptions must be sorted by field offset.'''
        current_register_block = []

        for idx, line in enumerate(self.lines, 1):
            line_stripped = line.strip()

            # Detect visual divider indicating a new register block
            if line_stripped.startswith('/*---') and line_stripped.endswith('*/'):
                self._validate_block_sorting(current_register_block)
                current_register_block = []
                continue

            if line.startswith('#define'):
                # Extract the last integer value inside parentheses, which represents the bit offset
                offset_match = re.findall(r'\b\d+\s*\)', line_stripped)
                if offset_match:
                    try:
                        # Extract the numeric string and convert to integer
                        offset_value = int(re.sub(r'[^0-9]', '', offset_match[-1]))
                        current_register_block.append({'line_num': idx, 'offset': offset_value})
                    except ValueError:
                        pass

        # Validate the last block in case the file doesn't end with a divider
        self._validate_block_sorting(current_register_block)

    def _validate_block_sorting(self, block):
        '''Helper function to check if the collected offsets in a block are increasing.'''
        if len(block) < 2:
            return

        for i in range(len(block) - 1):
            current_item = block[i]
            next_item = block[i + 1]

            # If a subsequent macro has a lower offset than the current macro, report a violation
            if next_item['offset'] < current_item['offset']:
                self.report(
                    next_item['line_num'],
                    'Rule 3.4',
                    f'Macro offsets out of order, '
                    f'{next_item['offset']} follows {current_item['offset']}'
                )
                # Break after first violation in a block to prevent redundant error cascading
                break

    def check_macro_triplet_naming(self):
        '''Rule 3.5: The _VALUE macro's second argument must match its own field name base.'''
        num_lines = len(self.lines)

        for idx in range(num_lines):
            line_num = idx + 1
            line_stripped = self.lines[idx].strip()

            if re.match(r'^#define\s+\w+_VALUE\s*\(', line_stripped):
                signature_match = re.match(r'^#define\s+(\w+)_VALUE\s*\([^)]*\)', line_stripped)
                if not signature_match:
                    continue

                value_base = signature_match.group(1)
                macro_body = line_stripped[signature_match.end():].strip()

                if macro_body.endswith('\\') and (idx + 1) < num_lines:
                    next_line_stripped = self.lines[idx + 1].strip()
                    macro_body = macro_body.rstrip('\\').strip() + ' ' + next_line_stripped

                body_args_match = re.search(r'\((.*)\)', macro_body)
                if not body_args_match:
                    continue

                # Clean arguments and ensure that stray backslashes are stripped
                body_args = body_args_match.group(1)
                clean_args = [arg.strip('() ').rstrip('\\').strip() for arg in body_args.split(',')]

                # Explicitly extract the second string element (index 1)
                if len(clean_args) >= 2:
                    mask_argument = clean_args[1]
                    expected_mask = f'{value_base}_MASK'

                    if mask_argument != expected_mask:
                        self.report(
                            line_num,
                            'Rule 3.5',
                            f'Macro "{value_base}_VALUE" argument mismatch, '
                            f'uses "{mask_argument}" instead of "{expected_mask}"'
                        )

    def check_macro_triplet_ordering(self):
        '''Rule 3.5: A _MASK macro must be preceded by a comment or empty line.'''
        for idx, line in enumerate(self.lines):
            line_stripped = line.strip()

            # Identify if the current line defines a _MASK macro
            if (line_stripped.startswith('#define')
                and re.match(r'^#define\s+\w+_MASK\b', line_stripped)):
                line_num = idx + 1

                # If it's the first line of the file, it is automatically validly placed
                if idx == 0:
                    continue

                # Inspect the immediate preceding line
                prev_line_stripped = self.lines[idx - 1].strip()

                # Check if the previous line is an empty line
                is_empty_line = prev_line_stripped == ''

                # Check if the previous line is a valid comment line or divider closing
                is_comment_line = (
                    prev_line_stripped.startswith('/*') or
                    prev_line_stripped.startswith('*') or
                    prev_line_stripped.endswith('*/')
                )

                # If it's neither, then it's misplaced (e.g., following another macro directly)
                if not (is_empty_line or is_comment_line):
                    macro_name = re.match(r'^#define\s+(\w+)\b', line_stripped).group(1)
                    self.report(
                        line_num,
                        'Rule 3.5',
                        f'Macro "{macro_name}" is misplaced. The _MASK macro must be '
                        f'the first macro in the block'
                    )

    def check_enum_alignment(self):
        '''Rule 4: Enumeration formatting and alignment constraints.'''
        in_enum = False
        inside_braces = False
        enum_block_assignments = []

        for idx, line in enumerate(self.lines, 1):
            line_stripped = line.strip()

            # Detect entering an enum statement block
            if 'enum ' in line:
                in_enum = True
                enum_block_assignments = []
                # Fall through to check if brace is on the same line

            # Track when we actually cross the opening brace barrier
            if in_enum and not inside_braces:
                if '{' in line:
                    inside_braces = True
                continue # Skip processing elements until inside the brace body

            # Detect leaving the enum block completely
            if inside_braces and '};' in line:
                self._validate_current_enum_block(enum_block_assignments)
                in_enum = False
                inside_braces = False
                continue

            # Process assignments and inner blocks when inside the enum body
            if inside_braces:
                # Empty lines or internal comments break sequential alignment blocks
                if (line_stripped == '' or line_stripped.startswith('/*')
                    or line_stripped.startswith('//') or line_stripped.startswith('*')):
                    self._validate_current_enum_block(enum_block_assignments)
                    enum_block_assignments = []
                    continue

                if '=' in line:
                    # Find the exact column index of '=' using the raw, unstripped line
                    pos = line.find('=')
                    enum_block_assignments.append({'line_num': idx, 'pos': pos})

    def _validate_current_enum_block(self, assignments):
        if not assignments:
            return

        # Find the expected column index based on the first item in the block
        expected_pos = assignments[0]['pos']
        for item in assignments:
            # Check if it matches the group alignment
            if item['pos'] != expected_pos:
                self.report(
                    item['line_num'],
                    'Rule 4',
                    f'Enum "=" operator alignment mismatch, '
                    f'expected column {expected_pos}, found {item['pos']}'
                )
                continue
            # Check the existing odd-numbered column rule
            if item['pos'] % 2 == 0:
                self.report(
                    item['line_num'],
                    'Rule 4',
                    f'Enum "=" operator sits at column {item['pos']} '
                    f'instead of odd-numbered column'
                )

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
        self.check_comment_formatting()
        self.check_multiline_comments()
        self.check_bit_field_comments()
        self.check_bit_field_spacing()
        self.check_macro_alignments()
        self.check_macro_sorting_by_offset()
        self.check_macro_triplet_naming()
        self.check_macro_triplet_ordering()
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
