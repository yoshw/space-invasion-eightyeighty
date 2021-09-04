#include <stdio.h>
#include <stdlib.h>

int disassemble8080Code(FILE *f);
int printDisassembledOp(unsigned char* buffer, int pc);
int getFileSize(FILE *f);


int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s PATH\n", argv[0]);
        exit(1);
    }

    FILE *f = fopen(argv[1], "rb");

    if (f == NULL) {
        printf("Error: Couldn't open `%s`\n", argv[1]);
        exit(1);
    }

    disassemble8080Code(f);
}

int disassemble8080Code(FILE *f) {
    int fsize = getFileSize(f);

    unsigned char* buffer = malloc(fsize);

    fread(buffer, fsize, 1, f);
    fclose(f);

    int pc = 0;
    while (pc < fsize) {
        pc += printDisassembledOp(buffer, pc);
    }

    return 0;
}

int printDisassembledOp(unsigned char* codebuffer, int pc) {
    unsigned char* codeAddress = &codebuffer[pc];
    int opBytes = 1;

    printf("%04x ", pc);
    switch(*codeAddress) {
        case 0x00: printf("NOP"); break;
        case 0x01: printf("NOP"); break;
        case 0x02: printf("NOP"); break;
        case 0x03: printf("NOP"); break;
        case 0x04: printf("NOP"); break;
        case 0x05: printf("NOP"); break;
        case 0x06: printf("NOP"); break;
        case 0x07: printf("NOP"); break;
        case 0x08: printf("NOP"); break;
        case 0x09: printf("NOP"); break;
    }
    printf("\n");

    return opBytes;
}

int getFileSize(FILE *f) {
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    return fsize;
}
