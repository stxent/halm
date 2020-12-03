#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# make_layout.py
# Copyright (C) 2018 xent
# Project is distributed under the terms of the MIT License

'''Convert peripheral descriptions to C/C++ structures.

This module converts JSON files, containing descriptions of microcontroller
peripheral blocks and passed as arguments, to C/C++ structures.
Each file should have a list of memory domain descriptors with a specific structure.

Example:
    [
      {
        "address": "0x50000000",
        "name": "AHB_DOMAIN",
        "blocks":
        [
          {"offset": "0x00000", "name": "ETHERNET", "type": "ETHERNET_Type"}
        ]
      }
    ]

Domain descriptor:
    address (int): An absolute address of the domain.
    blocks (list): List of peripheral descriptors.
    name (str): Prefix of the generated structure.

Peripheral descriptor:
    aligned (bool): Disable generation of a padding before this peripheral.
    array (int): Number of blocks if the peripheral consists of multiple similar blocks.
    name (str): Name of the peripheral.
    offset (int): Offset from the beginning of the domain.
    type (str): Type of the peripheral.
'''

import argparse
import json

def build_memory_struct(branch):
    '''Prepare C structure of a peripheral from a JSON item.'''

    if 'address' not in branch or 'blocks' not in branch or 'name' not in branch:
        raise Exception()

    blocks = []
    for descriptor in branch['blocks']:
        array = descriptor['array'] if 'array' in descriptor else 1
        aligned = descriptor['aligned'] if 'aligned' in descriptor else False
        block = {
            'offset': int(descriptor['offset'], 16),
            'type': descriptor['type'],
            'name': descriptor['name'],
            'count': array,
            'aligned': aligned}
        blocks.append(block)

    position = 0
    reserved = 0
    output = 'typedef struct\n{\n'

    for i, block in enumerate(blocks):
        if i == 0 and position != block['offset']:
            size = block['offset']
            output += ('  __ne__ uint8_t RESERVED{:d}[0x{:X}];\n'.format(
                reserved, block['offset']))
            reserved += 1
        elif i > 0 and not block['aligned']:
            size = block['offset'] - position
            output += '  __ne__ uint8_t RESERVED{:d}[0x{:X} - sizeof({})];\n'.format(
                reserved, size, blocks[i - 1]['type'])
            reserved += 1

        if block['count'] == 1:
            output += '  {:s} {:s};\n'.format(block['type'], block['name'])
        else:
            output += '  {:s} {:s}[{:d}];\n'.format(
                block['type'], block['name'], block['count'])
        position = block['offset']

    output += '}} {:s}_Type;\n'.format(branch['name'])
    return output

def main():
    '''Process list of JSON files and print C structures, generated from descriptions.'''
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='files', nargs='*')
    options = parser.parse_args()

    for filepath in options.files:
        for branch in json.load(open(filepath, 'rb')):
            print(build_memory_struct(branch))

if __name__ == '__main__':
    main()
