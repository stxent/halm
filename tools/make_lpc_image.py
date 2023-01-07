#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# make_lpc_image.py
# Copyright (C) 2021 xent
# Project is distributed under the terms of the MIT License

'''Calculate checksum of the firmware image.

This module calculates checksum of the first 7 words of the firmware
and inserts it in the vector table.
'''

import argparse
import struct
import sys

def main():
    '''Calculate checksum of the firmware image.'''
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='files', nargs='*')
    options = parser.parse_args()

    if len(options.files) != 2:
        print('Usage: make_lpc_image.py INPUT OUTPUT')
        sys.exit()

    input_data = bytearray(open(options.files[0], 'rb').read())
    words = struct.unpack('<7I', input_data[0:28])
    checksum = (~sum(words) + 1) & 0xFFFFFFFF
    output_data = input_data[0:28] + struct.pack('<I', checksum) + input_data[32:]

    open(options.files[1], 'wb').write(output_data)

if __name__ == '__main__':
    main()
