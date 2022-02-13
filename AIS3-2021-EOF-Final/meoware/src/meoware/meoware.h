#pragma once
#include <winsock2.h>

#include <vector>

struct meow_context;

typedef void (*meow_fp)(meow_context*);

struct meow_context {
    SOCKET sock;
    char port[0x8];
    char site[0x20];
    meow_fp meow_connect;
    meow_fp meow_interactive;
    meow_fp meow_setenc;
    std::vector<unsigned char> aes_enc;
    std::vector<unsigned char> aes_key;
};

meow_context* meow_init(void);
