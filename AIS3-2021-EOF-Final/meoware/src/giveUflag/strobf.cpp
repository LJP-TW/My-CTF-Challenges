#include "strobf.hpp"

void deobfuscate(char* data, unsigned int N) {
    for (unsigned int i = 0; i < N; i++) {
        data[i] = (data[i] ^ 0x5a) - 20;
    }
    for (unsigned int i = 0; i < N / 2; i++) {
        char c = data[i];
        data[i] = data[N - i - 1];
        data[N - i - 1] = c;
    }
}
