#!/usr/bin/env python3


import sys
import json
import re
import os
import subprocess


if '-h' in sys.argv or \
   '--help' in sys.argv or \
   len(sys.argv) < 2 or \
   not os.path.isfile(sys.argv[1]):
    print('''USAGE
1. Run the rom_report target for your application:
   > west build --build-dir $BUILD_DIRECTORY --target rom_report

2. Run the script with the rom.json as an input file:
   > python bt_rom_summary.py $BUILD_DIRECTORY/rom.json
''')
    sys.exit(1)

with open(sys.argv[1], 'r', encoding='utf8') as fd:
    data = json.load(fd)

# flattened_list = [{'identifier': data['identifier'], 'name': data['name'], 'size': data['size']}]
flattened_list = []
def walk_children(children, flattened_list=None):
    for child in children:
        if 'identifier' in child and child['identifier'].endswith('.h') or child['identifier'].endswith('.c'):
            flattened_list.append(dict(identifier=child['identifier'],
                                       name=child['name'],
                                       size=child['size']))
        elif 'children' in child:
            walk_children(child['children'], flattened_list)
        else:
            flattened_list.append(child)

walk_children(data['children'], flattened_list)


regex = re.compile('|'.join(
    ['.*sym_[A-Z0-9]+',         # Obfuscated symbols in SDC and MPSL
     '.*subsys/bluetooth.*',
     '.*mpsl.*',
     '.*MPSL.*',
     '.*sdc.*',
     'kernel/.*',
     'subsys/net/.*',
     'include/sys/.*',
     'include/bluetooth/.*',
     'include/kernel.*',
     ':/events.*',
     ':/CSWTCH.*',
     ':/hci_vendor_event',
     'lib/os/.*',
     'lib/libc/.*',
     ':/__compound_literal.*',
     'drivers/clock_control/.*',
     'drivers/entropy/.*',
     'modules/crypto/tinycrypt/.*',
     'modules/hal.*',
     'arch/arm/core/.*',
     'soc/arm/.*',
     '.*l2cap.*',
     '.*att_.*',
     '.*pub_key_cb.*',
     '.*salt.*',
     ':/rand_prio_high_vector_get',
     '.*levels.*',
     '.*device_dts.*',
     ':/SystemCoreClock',
     '.*bt_.*',
     ':/_sw_isr_table',
     ':/__aeabi_idiv0',
     'drivers.*',
     'subsys/fs.*'
     ]
))

flattened_list.sort(key=lambda e: e['identifier'])

sum = 0
remaining = 0
included = [v for v in flattened_list if regex.match(v['identifier'])]
excluded = [v for v in flattened_list if not regex.match(v['identifier'])]

def printer(l):
    sum = 0
    for v in l:
        sum += v['size']
        print(f'{v["identifier"]+":"!s:70}{v["size"]}')

    print(f'{"SUM:"!s:70}{sum} ={sum/1024:.2f} kB')
    return sum


print('EXCLUDED')
sum_excluded = printer(excluded)
print('-'*80)

print('INCLUDED')
sum_included = printer(included)
print('-'*80)

print(f'SUM EXCLUDED+INCLUDED: {sum_excluded+sum_included}')
size = ['arm-none-eabi-size', 'build/zephyr/zephyr.elf']
print(' '.join(size) + ':\n', subprocess.run(args=size, capture_output=True).stdout.decode('utf-8'))
