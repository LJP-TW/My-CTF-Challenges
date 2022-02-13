#include "strobf.hpp"

void deobfuscate(char* data, unsigned int N) {
    for (unsigned int i = 0; i < N; i++) {
        data[i] = data[i] ^ 0x87;
    }
}
