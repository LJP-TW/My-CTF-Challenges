#!/usr/bin/env python3
import struct

def _int32(x):
    return int(0xFFFFFFFF & x)

class MT19937:
    def __init__(self, seed):
        self.mt = [0] * 624
        self.mt[0] = seed
        self.mti = 0
        for i in range(1, 624):
            self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)


    def extract_number(self):
        if self.mti == 0:
            self.twist()
        y = self.mt[self.mti]
        y = y ^ y >> 11
        y = y ^ y << 7 & 2636928640
        y = y ^ y << 15 & 4022730752
        y = y ^ y >> 18
        self.mti = (self.mti + 1) % 624
        return _int32(y)


    def twist(self):
        for i in range(0, 624):
            y = _int32((self.mt[i] & 0x80000000) + (self.mt[(i + 1) % 624] & 0x7fffffff))
            self.mt[i] = (y >> 1) ^ self.mt[(i + 397) % 624]

            if y % 2 != 0:
                self.mt[i] = self.mt[i] ^ 0x9908b0df

with open('wannasleeeeeeep.txt.enc', 'rb') as f:
    content = list(f.read())

prng = MT19937(0o613)
prng2 = MT19937(struct.unpack('<I', bytes(content[-4:]))[0])

for i in range(len(content)-4):
    content[i] ^= prng.extract_number()
    content[i] ^= prng2.extract_number()
    content[i] -= 0x20
    content[i] &= 0xff
 
print(bytes(content[:-4]))
 
