#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# make_layout.py
# Copyright (C) 2018 xent
# Project is distributed under the terms of the GNU General Public License v3.0

import argparse
import json

parser = argparse.ArgumentParser()
parser.add_argument(dest='files', nargs='*')
options = parser.parse_args()

def buildMemoryStruct(branch):
    if 'address' not in branch.keys() or 'blocks' not in branch.keys() or 'name' not in branch.keys():
        raise Exception()

    start = int(branch['address'], 16)
    blocks = []
    for block in branch['blocks']:
        array = block['array'] if 'array' in block.keys() else 1
        aligned = block['aligned'] if 'aligned' in block.keys() else False
        blocks.append({'offset': int(block['offset'], 16), 'type': block['type'],
                'name': block['name'], 'count': array, 'aligned': aligned})

    position = 0
    reserved = 0

    print('typedef struct')
    print('{')
    for i in range(0, len(blocks)):
        if i == 0 and position != blocks[i]['offset']:
            size = blocks[i]['offset']
            print('  __ne__ uint8_t RESERVED{:d}[0x{:X}];'.format(
                    reserved, blocks[i]['offset']))
            reserved += 1
        elif i > 0 and not blocks[i]['aligned']:
            size = blocks[i]['offset'] - position
            print('  __ne__ uint8_t RESERVED{:d}[0x{:X} - sizeof({})];'.format(
                    reserved, size, blocks[i - 1]['type']))
            reserved += 1

        if blocks[i]['count'] == 1:
            print('  {:s} {:s};'.format(blocks[i]['type'], blocks[i]['name']))
        else:
            print('  {:s} {:s}[{:d}];'.format(blocks[i]['type'], blocks[i]['name'], blocks[i]['count']))
        position = blocks[i]['offset']
    print('}} {:s}_Type;'.format(branch['name']))
    print('')

def build(path):
    tree = json.load(open(path, 'rb'))

    for branch in tree:
        buildMemoryStruct(branch)

for path in options.files:
    build(path)
