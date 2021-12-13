#!/usr/bin/python

# Convert (Json) file to hex values, similar as 'hexdump' without lines.

import os
import sys

with open(sys.argv[1]) as f:
    chars = f.read()

print("// dump from file: %s" %os.path.basename(sys.argv[1]))

print("static const char webpage_str[] = {\n   "),
for (i, x) in enumerate(chars, 1):
    print("0x%02X," %ord(x)),
    if i % 16 == 0:
        print('\n   '),

print('0x00')  # add a '\0' to the end as string terminator
print('};')
