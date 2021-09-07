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

        // Data Transfer Group
        case 0x40: printf("MOV    B <- B"); break;
        case 0x41: printf("MOV    B <- C"); break;
        case 0x42: printf("MOV    B <- D"); break;
        case 0x43: printf("MOV    B <- E"); break;
        case 0x44: printf("MOV    B <- H"); break;
        case 0x45: printf("MOV    B <- L"); break;
        case 0x47: printf("MOV    B <- A"); break;
        case 0x48: printf("MOV    C <- B"); break;
        case 0x49: printf("MOV    C <- C"); break;
        case 0x4a: printf("MOV    C <- D"); break;
        case 0x4b: printf("MOV    C <- E"); break;
        case 0x4c: printf("MOV    C <- H"); break;
        case 0x4d: printf("MOV    C <- L"); break;
        case 0x4f: printf("MOV    C <- A"); break;
        case 0x50: printf("MOV    D <- B"); break;
        case 0x51: printf("MOV    D <- C"); break;
        case 0x52: printf("MOV    D <- D"); break;
        case 0x53: printf("MOV    D <- E"); break;
        case 0x54: printf("MOV    D <- H"); break;
        case 0x55: printf("MOV    D <- L"); break;
        case 0x57: printf("MOV    D <- A"); break;
        case 0x58: printf("MOV    E <- B"); break;
        case 0x59: printf("MOV    E <- C"); break;
        case 0x5a: printf("MOV    E <- D"); break;
        case 0x5b: printf("MOV    E <- E"); break;
        case 0x5c: printf("MOV    E <- H"); break;
        case 0x5d: printf("MOV    E <- L"); break;
        case 0x5f: printf("MOV    E <- A"); break;
        case 0x60: printf("MOV    H <- B"); break;
        case 0x61: printf("MOV    H <- C"); break;
        case 0x62: printf("MOV    H <- D"); break;
        case 0x63: printf("MOV    H <- E"); break;
        case 0x64: printf("MOV    H <- H"); break;
        case 0x65: printf("MOV    H <- L"); break;
        case 0x67: printf("MOV    H <- A"); break;
        case 0x68: printf("MOV    L <- B"); break;
        case 0x69: printf("MOV    L <- C"); break;
        case 0x6a: printf("MOV    L <- D"); break;
        case 0x6b: printf("MOV    L <- E"); break;
        case 0x6c: printf("MOV    L <- H"); break;
        case 0x6d: printf("MOV    L <- L"); break;
        case 0x6f: printf("MOV    L <- A"); break;
        case 0x78: printf("MOV    A <- B"); break;
        case 0x79: printf("MOV    A <- C"); break;
        case 0x7a: printf("MOV    A <- D"); break;
        case 0x7b: printf("MOV    A <- E"); break;
        case 0x7c: printf("MOV    A <- H"); break;
        case 0x7d: printf("MOV    A <- L"); break;
        case 0x7f: printf("MOV    A <- A"); break;

        case 0x46: printf("MOV    B <- (H-L)"); break;
        case 0x4e: printf("MOV    C <- (H-L)"); break;
        case 0x56: printf("MOV    D <- (H-L)"); break;
        case 0x5e: printf("MOV    E <- (H-L)"); break;
        case 0x66: printf("MOV    H <- (H-L)"); break;
        case 0x6e: printf("MOV    L <- (H-L)"); break;
        case 0x7e: printf("MOV    A <- (H-L)"); break;

        case 0x70: printf("MOV    (H-L) <- B"); break;
        case 0x71: printf("MOV    (H-L) <- C"); break;
        case 0x72: printf("MOV    (H-L) <- D"); break;
        case 0x73: printf("MOV    (H-L) <- E"); break;
        case 0x74: printf("MOV    (H-L) <- H"); break;
        case 0x75: printf("MOV    (H-L) <- L"); break;
        case 0x77: printf("MOV    (H-L) <- A"); break;

        case 0x06: printf("MVI    B <- #$%02x", codeAddress[1]); opBytes=2; break;
        case 0x0e: printf("MVI    C <- #$%02x", codeAddress[1]); opBytes=2; break;
        case 0x16: printf("MVI    D <- #$%02x", codeAddress[1]); opBytes=2; break;
        case 0x1e: printf("MVI    E <- #$%02x", codeAddress[1]); opBytes=2; break;
        case 0x26: printf("MVI    H <- #$%02x", codeAddress[1]); opBytes=2; break;
        case 0x2e: printf("MVI    L <- #$%02x", codeAddress[1]); opBytes=2; break;
        case 0x3e: printf("MVI    A <- #$%02x", codeAddress[1]); opBytes=2; break;

        case 0x36: printf("MVI    (H-L) <- #$%02x", codeAddress[1]); opBytes=2; break;

        case 0x01: printf("LXI    B-C <- #$%02x%02x", codeAddress[2], codeAddress[1]); opBytes=3; break;
        case 0x11: printf("LXI    D-E <- #$%02x%02x", codeAddress[2], codeAddress[1]); opBytes=3; break;
        case 0x21: printf("LXI    H-L <- #$%02x%02x", codeAddress[2], codeAddress[1]); opBytes=3; break;
        case 0x31: printf("LXI    SP <- #$%02x%02x", codeAddress[2], codeAddress[1]); opBytes=3; break;

        case 0x3a: printf("LDA    A <- ($%02x%02x)", codeAddress[2], codeAddress[1]); opBytes=3; break;
        case 0x32: printf("STA    ($%02x%02x) <- A", codeAddress[2], codeAddress[1]); opBytes=3; break;

        case 0x2a: printf("LHLD   L <- ($%02x%02x), H <- ($%02x%02x)", codeAddress[2], codeAddress[1], codeAddress[2], codeAddress[1]+1); opBytes=3; break;
        case 0x22: printf("SHLD   ($%02x%02x) <- L, ($%02x%02x) <- H", codeAddress[2], codeAddress[1], codeAddress[2], codeAddress[1]+1); opBytes=3; break;

        case 0x0a: printf("LDAX   A <- (B-C) "); break;
        case 0x1a: printf("LDAX   A <- (D-E) "); break;

        case 0x02: printf("STAX   (B-C) <- A"); break;
        case 0x12: printf("STAX   (D-E) <- A"); break;

        case 0xeb: printf("XCHG   H-L <-> D-E"); break;

        // Arithmetic Group
        case 0x80: printf("ADD    A <- A + B"); break;
        case 0x81: printf("ADD    A <- A + C"); break;
        case 0x82: printf("ADD    A <- A + D"); break;
        case 0x83: printf("ADD    A <- A + E"); break;
        case 0x84: printf("ADD    A <- A + H"); break;
        case 0x85: printf("ADD    A <- A + L"); break;
        case 0x87: printf("ADD    A <- A + A"); break;

        case 0x86: printf("ADD    A <- A + (H-L)"); break;

        case 0xc6: printf("ADI    A <- A + #$%02x", codeAddress[1]); opBytes=2; break;

        case 0x88: printf("ADC    A <- A + B + CY"); break;
        case 0x89: printf("ADC    A <- A + C + CY"); break;
        case 0x8a: printf("ADC    A <- A + D + CY"); break;
        case 0x8b: printf("ADC    A <- A + E + CY"); break;
        case 0x8c: printf("ADC    A <- A + H + CY"); break;
        case 0x8d: printf("ADC    A <- A + L + CY"); break;
        case 0x8f: printf("ADC    A <- A + A + CY"); break;

        case 0x8e: printf("ADC    A <- A + (H-L) + CY"); break;

        case 0xce: printf("ACI    A <- A + #$%02x + CY", codeAddress[1]); opBytes=2; break;

        case 0x90: printf("SUB    A <- A - B"); break;
        case 0x91: printf("SUB    A <- A - C"); break;
        case 0x92: printf("SUB    A <- A - D"); break;
        case 0x93: printf("SUB    A <- A - E"); break;
        case 0x94: printf("SUB    A <- A - H"); break;
        case 0x95: printf("SUB    A <- A - L"); break;
        case 0x97: printf("SUB    A <- A - A"); break;

        case 0x96: printf("SUB    A <- A - (H-L)"); break;

        case 0xd6: printf("SUI    A <- A - #$%02x", codeAddress[1]); opBytes=2; break;

        case 0x98: printf("SBB    A <- A - B - CY"); break;
        case 0x99: printf("SBB    A <- A - C - CY"); break;
        case 0x9a: printf("SBB    A <- A - D - CY"); break;
        case 0x9b: printf("SBB    A <- A - E - CY"); break;
        case 0x9c: printf("SBB    A <- A - H - CY"); break;
        case 0x9d: printf("SBB    A <- A - L - CY"); break;
        case 0x9f: printf("SBB    A <- A - A - CY"); break;

        case 0x9e: printf("SBB    A <- A - (H-L) - CY"); break;

        case 0xde: printf("SBI    A <- A - #$%02x - CY", codeAddress[1]); opBytes=2; break;

        case 0x04: printf("INR    B <- B + 1"); break;
        case 0x0c: printf("INR    C <- C + 1"); break;
        case 0x14: printf("INR    D <- D + 1"); break;
        case 0x1c: printf("INR    E <- E + 1"); break;
        case 0x24: printf("INR    H <- H + 1"); break;
        case 0x2c: printf("INR    L <- L + 1"); break;
        case 0x3c: printf("INR    A <- A + 1"); break;

        case 0x34: printf("INR    (H-L) <- (H-L) + 1"); break;

        case 0x05: printf("DCR    B <- B - 1"); break;
        case 0x0d: printf("DCR    C <- C - 1"); break;
        case 0x15: printf("DCR    D <- D - 1"); break;
        case 0x1d: printf("DCR    E <- E - 1"); break;
        case 0x25: printf("DCR    H <- H - 1"); break;
        case 0x2d: printf("DCR    L <- L - 1"); break;
        case 0x3d: printf("DCR    A <- A - 1"); break;

        case 0x35: printf("DCR    (H-L) <- (H-L) - 1"); break;

        case 0x03: printf("INX    B-C <- B-C + 1"); break;
        case 0x13: printf("INX    D-E <- D-E + 1"); break;
        case 0x23: printf("INX    H-L <- H-L + 1"); break;
        case 0x33: printf("INX    SP <- SP + 1"); break;

        case 0x0b: printf("DCX    B-C <- B-C - 1"); break;
        case 0x1b: printf("DCX    D-E <- D-E - 1"); break;
        case 0x2b: printf("DCX    H-L <- H-L - 1"); break;
        case 0x3b: printf("DCX    SP <- SP - 1"); break;

        case 0x09: printf("DAD    H-L <- H-L + B-C"); break;
        case 0x19: printf("DAD    H-L <- H-L + D-E"); break;
        case 0x29: printf("DAD    H-L <- H-L + H-L"); break;
        case 0x39: printf("DAD    H-L <- H-L + SP"); break;

        case 0x27: printf("DAA"); break;

        // Logical Group
        case 0xa0: printf("ANA    A <- A ∧ B"); break;
        case 0xa1: printf("ANA    A <- A ∧ C"); break;
        case 0xa2: printf("ANA    A <- A ∧ D"); break;
        case 0xa3: printf("ANA    A <- A ∧ E"); break;
        case 0xa4: printf("ANA    A <- A ∧ H"); break;
        case 0xa5: printf("ANA    A <- A ∧ L"); break;
        case 0xa7: printf("ANA    A <- A ∧ A"); break;

        case 0xa6: printf("ANA    A <- A ∧ (H-L)"); break;
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
