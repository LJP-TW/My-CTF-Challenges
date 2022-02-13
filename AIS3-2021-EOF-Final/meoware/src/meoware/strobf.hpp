#pragma once
/*
 *  Example usage:
 *    constexpr auto test_str = OBFUSCATE("TEST");
 *    printf("%s\n", test_str);
 *    DEOBFUSCA(test_str);
 *    printf("%s\n", test_str);
*/
template <unsigned int N>
struct obfuscator {
    char m_data[N] = { 0 };
    constexpr obfuscator(const char* data) {
        for (unsigned int i = 0; i < N; i++) {
            m_data[i] = data[i] ^ 0x87;
        }
    };
};

extern "C" void deobfuscate(char* data, unsigned int N);

#define OBFUSCATE(str) obfuscator<sizeof(str)>(str)
#define DEOBFUSCA(var) deobfuscate((char*)&var, sizeof(var.m_data))
