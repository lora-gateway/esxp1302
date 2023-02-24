#!/usr/bin/python

# Convert (Json) file to hex values, similar as 'hexdump' without lines.
#
# License: Revised BSD License, see LICENSE.TXT file including in the project
#

import sys

with open(sys.argv[1]) as f:
    chars = f.read()

# print the length as the first 2 bytes
n = len(chars)
if n > 0xffff:
    print("File size excel 65535. Abort")
    sys.exit()

print("0x%02X," %(n // 256)),
print("0x%02X," %(n % 256)),

for (i, x) in enumerate(chars, 3):
    print("0x%02X," %ord(x)),
    if i % 16 == 0:
        print
print('0x00')  # add a '\0' to the end as string terminator
