#!/usr/bin/python

# Convert (Json) file to hex values, similar as 'hexdump' without lines.

import sys

with open(sys.argv[1]) as f:
    chars = f.read()

for (i, x) in enumerate(chars):
    print "0x%02X," %ord(x),
    if (i + 1) % 16 == 0:
        print
print '0x00'  # add a '\0' to the end as string terminator
