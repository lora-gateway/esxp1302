#!/usr/bin/python2
# -*- coding: utf-8 -*-

import socket
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

srv_addr = '0.0.0.0'
srv_port = 1680

if len(sys.argv) > 1:
    srv_port = int(sys.argv[1])
print 'port:', srv_port

server = (srv_addr, srv_port)
sock.bind(server)

print("Listening on " + srv_addr + ":" + str(srv_port))

while True:
    payload, client_addr = sock.recvfrom(1024)
    print "Got: ", payload
    print("Echoing data back to " + str(client_addr))
    sent = sock.sendto(payload, client_addr)
