#!/usr/bin/python

#
# License: Revised BSD License, see LICENSE.TXT file including in the project
#

import sys
import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client_socket.settimeout(1.0)
port = int(sys.argv[1])
message = sys.argv[2]
addr = ("127.0.0.1", port)

client_socket.sendto(message, addr)
try:
    data, server = client_socket.recvfrom(1024)
    print 'got:', data
except socket.timeout:
    print('REQUEST TIMED OUT')
