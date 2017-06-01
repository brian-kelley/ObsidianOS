#!/usr/bin/python

import sys
import os

# print 256 bytes of disassembly in system image at given address - 128
# address given in exception handler

if len(sys.argv) != 2:
  print("Provide just the faulting address")
  quit()

n = 256
breakAddr = int(sys.argv[1], 16)
addr = breakAddr - 0x500 - (n / 2)
bytes = open("temp/system.bin", "rb").read()[addr:(addr + n)]
open("temp/debug.bin", "wb").write(bytes)

os.system("ndisasm -u -o " + hex(breakAddr - (n / 2)) + " temp/debug.bin")

