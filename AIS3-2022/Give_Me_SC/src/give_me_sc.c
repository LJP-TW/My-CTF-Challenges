#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

typedef void (*funcp)(char *);

char buf[0x1000];

int main(int argc, char **argv)
{
    char name[0x100];

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    mprotect(buf, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);

    printf("What's your name:\n");

    read(0, name, 0x100);

    printf("Give me shellcode:\n");
    
    read(0, buf, 0x1000);

    printf("Run your shellcode:\n");

    ((funcp)buf)(name);
}