import struct
import random as r
import sys

if len(sys.argv) != 3:
    print sys.argv[0] + ' N C'
    exit()

N, C = int(sys.argv[1]), int(sys.argv[2])
f = open('a_' + str(N) + '_' + str(C), 'wb')
for i in range(N):
    f.write(struct.pack('i', r.randint(1, C)))
f.close()
print 'a_' + str(N) + '_' + str(C)
