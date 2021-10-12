#include <stdio.h>
#include <stdlib.h>

typedef struct {
    u_int8_t    Z:1;
    u_int8_t    S:1;
    u_int8_t    P:1;
    u_int8_t    CY:1;
    u_int8_t    AC:1;
    u_int8_t    _padding:3;
} ConditionCodes;

typedef struct {
    u_int8_t    A;
    u_int8_t    B;
    u_int8_t    C;
    u_int8_t    D;
    u_int8_t    E;
    u_int8_t    H;
    u_int8_t    L;
    u_int16_t   SP;
    u_int16_t   PC;

    u_int8_t*      memory;
    ConditionCodes codes;
    u_int8_t       int_enable;
} State8080;

int emulate8080(FILE *f);
void emulateOp8080(State8080* state);
int getFileSize(FILE *f);

u_int16_t getHLAddress(State8080* state);

void opLXI(State8080* state, u_int8_t* opPointer, u_int8_t* firstReg, u_int8_t* secondReg);
void opLXI_sp(State8080* state, u_int8_t* opPointer);
void opADD(State8080* state, u_int16_t addend);
void opADI(State8080* state);
void opADC(State8080* state, u_int16_t addend);
void opACI(State8080* state);
void opSUB(State8080* state, u_int16_t subtrahend);
void opSUI(State8080* state);
void opSBB(State8080* state, u_int16_t subtrahend);
void opSBI(State8080* state);
void opINR(State8080* state, u_int8_t* address);
void opDCR(State8080* state, u_int8_t* address);
void opINX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg);
void opDCX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg);
void opDAD(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg);
void opDAD_sp(State8080* state);
void opDAA(State8080* state);

u_int8_t getParity(u_int8_t value);
void UnimplementedInstruction(State8080* state);


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

    emulate8080(f);
}

int emulate8080(FILE *f) {
    int fsize = getFileSize(f);

    unsigned char* buffer = malloc(fsize);

    fread(buffer, fsize, 1, f);
    fclose(f);

    State8080 state = {
        .A = 0, .B = 0, .C = 0, .D = 0, .E = 0, .H = 0, .L = 0, .SP = 0, .PC = 0,
        // TODO: size memory properly. current code will lead to segfaults
        .memory = buffer,
        .codes = {}, // all codes initialized to zero
        .int_enable = 1
    };

    int ops = 0;
    while (ops < 100) {
        emulateOp8080(&state);
    }

    return 0;
}

void emulateOp8080(State8080* state) {
    unsigned char* opCode = &state->memory[state->PC];

    switch(*opCode) {
        // Data Transfer Group

        // LXI
        case 0x01: opLXI(state, opCode, &state->B, &state->C); break;
        case 0x11: opLXI(state, opCode, &state->D, &state->E); break;
        case 0x21: opLXI(state, opCode, &state->H, &state->L); break;
        case 0x31: opLXI_sp(state, opCode); break;


        // Arithmetic Group

        // ADD
        case 0x80: opADD(state, state->B); break;
        case 0x81: opADD(state, state->C); break;
        case 0x82: opADD(state, state->D); break;
        case 0x83: opADD(state, state->E); break;
        case 0x84: opADD(state, state->H); break;
        case 0x85: opADD(state, state->L); break;
        case 0x87: opADD(state, state->A); break;

        // ADD M
        case 0x86: opADD(state, state->memory[getHLAddress(state)]); break;

        // ADI
        case 0xc6: opADI(state); break;

        // ADC
        case 0x88: opADC(state, state->B); break;
        case 0x89: opADC(state, state->C); break;
        case 0x8a: opADC(state, state->D); break;
        case 0x8b: opADC(state, state->E); break;
        case 0x8c: opADC(state, state->H); break;
        case 0x8d: opADC(state, state->L); break;
        case 0x8f: opADC(state, state->A); break;

        // ADC M
        case 0x8e: opADC(state, state->memory[getHLAddress(state)]); break;

        // ACI
        case 0xce: opACI(state); break;

        // SUB
        case 0x90: opSUB(state, state->B); break;
        case 0x91: opSUB(state, state->C); break;
        case 0x92: opSUB(state, state->D); break;
        case 0x93: opSUB(state, state->E); break;
        case 0x94: opSUB(state, state->H); break;
        case 0x95: opSUB(state, state->L); break;
        case 0x97: opSUB(state, state->A); break;

        // SUB M
        case 0x96: opSUB(state, state->memory[getHLAddress(state)]); break;

        // SUI
        case 0xd6: opSUI(state); break;

        // SBB
        case 0x98: opSBB(state, state->B); break;
        case 0x99: opSBB(state, state->C); break;
        case 0x9a: opSBB(state, state->D); break;
        case 0x9b: opSBB(state, state->E); break;
        case 0x9c: opSBB(state, state->H); break;
        case 0x9d: opSBB(state, state->L); break;
        case 0x9f: opSBB(state, state->A); break;

        // SBB M
        case 0x9e: opSBB(state, state->memory[getHLAddress(state)]); break;

        // SBI
        case 0xde: opSBI(state); break;

        // INR
        case 0x04: opINR(state, &state->B); break;
        case 0x0c: opINR(state, &state->C); break;
        case 0x14: opINR(state, &state->D); break;
        case 0x1c: opINR(state, &state->E); break;
        case 0x24: opINR(state, &state->H); break;
        case 0x2c: opINR(state, &state->L); break;
        case 0x3c: opINR(state, &state->A); break;

        // INR M
        case 0x34: opINR(state, &state->memory[getHLAddress(state)]); break;

        // DCR
        case 0x05: opDCR(state, &state->B); break;
        case 0x0d: opDCR(state, &state->C); break;
        case 0x15: opDCR(state, &state->D); break;
        case 0x1d: opDCR(state, &state->E); break;
        case 0x25: opDCR(state, &state->H); break;
        case 0x2d: opDCR(state, &state->L); break;
        case 0x3d: opDCR(state, &state->A); break;

        // DCR M
        case 0x35: opDCR(state, &state->memory[getHLAddress(state)]); break;

        // INX
        case 0x03: opINX(state, &state->B, &state->C); break;
        case 0x13: opINX(state, &state->D, &state->E); break;
        case 0x23: opINX(state, &state->H, &state->L); break;
        case 0x33: state->SP += 1; break;

        // DCX
        case 0x0b: opDCX(state, &state->B, &state->C); break;
        case 0x1b: opDCX(state, &state->D, &state->E); break;
        case 0x2b: opDCX(state, &state->H, &state->L); break;
        case 0x3b: state->SP -= 1; break;

        // DAD
        case 0x09: opDAD(state, state->B, state->C); break;
        case 0x19: opDAD(state, state->D, state->E); break;
        case 0x29: opDAD(state, state->H, state->L); break;
        case 0x39: opDAD_sp(state); break;

        // DAA
        case 0x27: opDAA(state); break;

        // Logical Group
        case 0x02: UnimplementedInstruction(state); break;
        case 0x06: UnimplementedInstruction(state); break;
        case 0x07: UnimplementedInstruction(state); break;
        case 0x08: UnimplementedInstruction(state); break;
        case 0x0a: UnimplementedInstruction(state); break;
        case 0x0e: UnimplementedInstruction(state); break;
        case 0x0f: UnimplementedInstruction(state); break;
        case 0x10: UnimplementedInstruction(state); break;
        case 0x12: UnimplementedInstruction(state); break;
        case 0x16: UnimplementedInstruction(state); break;
        case 0x17: UnimplementedInstruction(state); break;
        case 0x18: UnimplementedInstruction(state); break;
        case 0x1a: UnimplementedInstruction(state); break;
        case 0x1e: UnimplementedInstruction(state); break;
        case 0x1f: UnimplementedInstruction(state); break;
        case 0x20: UnimplementedInstruction(state); break;
        case 0x22: UnimplementedInstruction(state); break;
        case 0x26: UnimplementedInstruction(state); break;
        case 0x28: UnimplementedInstruction(state); break;
        case 0x2a: UnimplementedInstruction(state); break;
        case 0x2e: UnimplementedInstruction(state); break;
        case 0x2f: UnimplementedInstruction(state); break;
        case 0x30: UnimplementedInstruction(state); break;
        case 0x32: UnimplementedInstruction(state); break;
        case 0x36: UnimplementedInstruction(state); break;
        case 0x37: UnimplementedInstruction(state); break;
        case 0x38: UnimplementedInstruction(state); break;
        case 0x3a: UnimplementedInstruction(state); break;
        case 0x3e: UnimplementedInstruction(state); break;
        case 0x3f: UnimplementedInstruction(state); break;
        case 0x40: UnimplementedInstruction(state); break;
        case 0x41: UnimplementedInstruction(state); break;
        case 0x42: UnimplementedInstruction(state); break;
        case 0x43: UnimplementedInstruction(state); break;
        case 0x44: UnimplementedInstruction(state); break;
        case 0x45: UnimplementedInstruction(state); break;
        case 0x46: UnimplementedInstruction(state); break;
        case 0x47: UnimplementedInstruction(state); break;
        case 0x48: UnimplementedInstruction(state); break;
        case 0x49: UnimplementedInstruction(state); break;
        case 0x4a: UnimplementedInstruction(state); break;
        case 0x4b: UnimplementedInstruction(state); break;
        case 0x4c: UnimplementedInstruction(state); break;
        case 0x4d: UnimplementedInstruction(state); break;
        case 0x4e: UnimplementedInstruction(state); break;
        case 0x4f: UnimplementedInstruction(state); break;
        case 0x50: UnimplementedInstruction(state); break;
        case 0x51: UnimplementedInstruction(state); break;
        case 0x52: UnimplementedInstruction(state); break;
        case 0x53: UnimplementedInstruction(state); break;
        case 0x54: UnimplementedInstruction(state); break;
        case 0x55: UnimplementedInstruction(state); break;
        case 0x56: UnimplementedInstruction(state); break;
        case 0x57: UnimplementedInstruction(state); break;
        case 0x58: UnimplementedInstruction(state); break;
        case 0x59: UnimplementedInstruction(state); break;
        case 0x5a: UnimplementedInstruction(state); break;
        case 0x5b: UnimplementedInstruction(state); break;
        case 0x5c: UnimplementedInstruction(state); break;
        case 0x5d: UnimplementedInstruction(state); break;
        case 0x5e: UnimplementedInstruction(state); break;
        case 0x5f: UnimplementedInstruction(state); break;
        case 0x60: UnimplementedInstruction(state); break;
        case 0x61: UnimplementedInstruction(state); break;
        case 0x62: UnimplementedInstruction(state); break;
        case 0x63: UnimplementedInstruction(state); break;
        case 0x64: UnimplementedInstruction(state); break;
        case 0x65: UnimplementedInstruction(state); break;
        case 0x66: UnimplementedInstruction(state); break;
        case 0x67: UnimplementedInstruction(state); break;
        case 0x68: UnimplementedInstruction(state); break;
        case 0x69: UnimplementedInstruction(state); break;
        case 0x6a: UnimplementedInstruction(state); break;
        case 0x6b: UnimplementedInstruction(state); break;
        case 0x6c: UnimplementedInstruction(state); break;
        case 0x6d: UnimplementedInstruction(state); break;
        case 0x6e: UnimplementedInstruction(state); break;
        case 0x6f: UnimplementedInstruction(state); break;
        case 0x70: UnimplementedInstruction(state); break;
        case 0x71: UnimplementedInstruction(state); break;
        case 0x72: UnimplementedInstruction(state); break;
        case 0x73: UnimplementedInstruction(state); break;
        case 0x74: UnimplementedInstruction(state); break;
        case 0x75: UnimplementedInstruction(state); break;
        case 0x76: UnimplementedInstruction(state); break;
        case 0x77: UnimplementedInstruction(state); break;
        case 0x78: UnimplementedInstruction(state); break;
        case 0x79: UnimplementedInstruction(state); break;
        case 0x7a: UnimplementedInstruction(state); break;
        case 0x7b: UnimplementedInstruction(state); break;
        case 0x7c: UnimplementedInstruction(state); break;
        case 0x7d: UnimplementedInstruction(state); break;
        case 0x7e: UnimplementedInstruction(state); break;
        case 0x7f: UnimplementedInstruction(state); break;
        case 0xa0: UnimplementedInstruction(state); break;
        case 0xa1: UnimplementedInstruction(state); break;
        case 0xa2: UnimplementedInstruction(state); break;
        case 0xa3: UnimplementedInstruction(state); break;
        case 0xa4: UnimplementedInstruction(state); break;
        case 0xa5: UnimplementedInstruction(state); break;
        case 0xa6: UnimplementedInstruction(state); break;
        case 0xa7: UnimplementedInstruction(state); break;
        case 0xa8: UnimplementedInstruction(state); break;
        case 0xa9: UnimplementedInstruction(state); break;
        case 0xaa: UnimplementedInstruction(state); break;
        case 0xab: UnimplementedInstruction(state); break;
        case 0xac: UnimplementedInstruction(state); break;
        case 0xad: UnimplementedInstruction(state); break;
        case 0xae: UnimplementedInstruction(state); break;
        case 0xaf: UnimplementedInstruction(state); break;
        case 0xb0: UnimplementedInstruction(state); break;
        case 0xb1: UnimplementedInstruction(state); break;
        case 0xb2: UnimplementedInstruction(state); break;
        case 0xb3: UnimplementedInstruction(state); break;
        case 0xb4: UnimplementedInstruction(state); break;
        case 0xb5: UnimplementedInstruction(state); break;
        case 0xb6: UnimplementedInstruction(state); break;
        case 0xb7: UnimplementedInstruction(state); break;
        case 0xb8: UnimplementedInstruction(state); break;
        case 0xb9: UnimplementedInstruction(state); break;
        case 0xba: UnimplementedInstruction(state); break;
        case 0xbb: UnimplementedInstruction(state); break;
        case 0xbc: UnimplementedInstruction(state); break;
        case 0xbd: UnimplementedInstruction(state); break;
        case 0xbe: UnimplementedInstruction(state); break;
        case 0xbf: UnimplementedInstruction(state); break;
        case 0xc0: UnimplementedInstruction(state); break;
        case 0xc1: UnimplementedInstruction(state); break;
        case 0xc2: UnimplementedInstruction(state); break;
        case 0xc3: UnimplementedInstruction(state); break;
        case 0xc4: UnimplementedInstruction(state); break;
        case 0xc5: UnimplementedInstruction(state); break;
        case 0xc7: UnimplementedInstruction(state); break;
        case 0xc8: UnimplementedInstruction(state); break;
        case 0xc9: UnimplementedInstruction(state); break;
        case 0xca: UnimplementedInstruction(state); break;
        case 0xcb: UnimplementedInstruction(state); break;
        case 0xcc: UnimplementedInstruction(state); break;
        case 0xcd: UnimplementedInstruction(state); break;
        case 0xcf: UnimplementedInstruction(state); break;
        case 0xd0: UnimplementedInstruction(state); break;
        case 0xd1: UnimplementedInstruction(state); break;
        case 0xd2: UnimplementedInstruction(state); break;
        case 0xd3: UnimplementedInstruction(state); break;
        case 0xd4: UnimplementedInstruction(state); break;
        case 0xd5: UnimplementedInstruction(state); break;
        case 0xd7: UnimplementedInstruction(state); break;
        case 0xd8: UnimplementedInstruction(state); break;
        case 0xd9: UnimplementedInstruction(state); break;
        case 0xda: UnimplementedInstruction(state); break;
        case 0xdb: UnimplementedInstruction(state); break;
        case 0xdc: UnimplementedInstruction(state); break;
        case 0xdd: UnimplementedInstruction(state); break;
        case 0xdf: UnimplementedInstruction(state); break;
        case 0xe0: UnimplementedInstruction(state); break;
        case 0xe1: UnimplementedInstruction(state); break;
        case 0xe2: UnimplementedInstruction(state); break;
        case 0xe3: UnimplementedInstruction(state); break;
        case 0xe4: UnimplementedInstruction(state); break;
        case 0xe5: UnimplementedInstruction(state); break;
        case 0xe6: UnimplementedInstruction(state); break;
        case 0xe7: UnimplementedInstruction(state); break;
        case 0xe8: UnimplementedInstruction(state); break;
        case 0xe9: UnimplementedInstruction(state); break;
        case 0xea: UnimplementedInstruction(state); break;
        case 0xeb: UnimplementedInstruction(state); break;
        case 0xec: UnimplementedInstruction(state); break;
        case 0xed: UnimplementedInstruction(state); break;
        case 0xee: UnimplementedInstruction(state); break;
        case 0xef: UnimplementedInstruction(state); break;
        case 0xf0: UnimplementedInstruction(state); break;
        case 0xf1: UnimplementedInstruction(state); break;
        case 0xf2: UnimplementedInstruction(state); break;
        case 0xf3: UnimplementedInstruction(state); break;
        case 0xf4: UnimplementedInstruction(state); break;
        case 0xf5: UnimplementedInstruction(state); break;
        case 0xf6: UnimplementedInstruction(state); break;
        case 0xf7: UnimplementedInstruction(state); break;
        case 0xf8: UnimplementedInstruction(state); break;
        case 0xf9: UnimplementedInstruction(state); break;
        case 0xfa: UnimplementedInstruction(state); break;
        case 0xfb: UnimplementedInstruction(state); break;
        case 0xfc: UnimplementedInstruction(state); break;
        case 0xfd: UnimplementedInstruction(state); break;
        case 0xfe: UnimplementedInstruction(state); break;
        case 0xff: UnimplementedInstruction(state); break;

        // Stack, I/O, and Machine Control group
        case 0x00: break; // NOP
    }

    // operation consumed, so advance
    state->PC++;
}

u_int16_t getHLAddress(State8080* state) {
    return (state->H << 8) | (state->L);
}

void opLXI(State8080* state, u_int8_t* opPointer, u_int8_t* firstReg, u_int8_t* secondReg) {
    *firstReg = opPointer[2];
    *secondReg = opPointer[1];
    state->PC += 2;
}

void opLXI_sp(State8080* state, u_int8_t* opPointer) {
    u_int16_t combinedValue = (opPointer[2] << 8) | opPointer[1];
    state->SP = combinedValue;
    state->PC += 2;
}

/**
 * Implicitly converts `addend` to uint16 if uint8
 */
void opADD(State8080* state, u_int16_t addend) {
    u_int16_t answer = (u_int16_t) state->A + addend;
    u_int8_t ansTruncated = answer & 0xff;

    state->codes.Z = (ansTruncated == 0);
    state->codes.S = ((answer & 0x80) != 0);
    state->codes.P = getParity(ansTruncated);
    state->codes.CY = answer > 0xff;

    state->A = ansTruncated;
}

void opADI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opADD(state, nextByte);
    state->PC += 1;
}

void opADC(State8080* state, u_int16_t addend) {
    opADD(state, addend + state->codes.CY);
}

void opACI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opADD(state, nextByte + state->codes.CY);
    state->PC += 1;
}

void opSUB(State8080* state, u_int16_t subtrahend) {
    u_int16_t answer = (u_int16_t) state->A - subtrahend;
    u_int8_t ansTruncated = answer & 0xff;

    state->codes.Z = (ansTruncated == 0);
    state->codes.S = ((answer & 0x80) != 0);
    state->codes.P = getParity(ansTruncated);
    // carry out of high-order bit indicates no borrow occurred, meaning Carry is reset
    state->codes.CY = answer <= 0xff;

    state->A = ansTruncated;
}

void opSUI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opSUB(state, nextByte);
    state->PC += 1;
}

void opSBB(State8080* state, u_int16_t subtrahend) {
    opSUB(state, subtrahend + state->codes.CY);
}

void opSBI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opSUB(state, nextByte + state->codes.CY);
    state->PC += 1;
}

void opINR(State8080* state, u_int8_t* address) {
    *address += 1;
    state->codes.Z = (*address == 0);
    state->codes.S = ((*address & 0x80) != 0);
    state->codes.P = getParity(*address);
}

void opDCR(State8080* state, u_int8_t* address) {
    *address -= 1;
    state->codes.Z = (*address == 0);
    state->codes.S = ((*address & 0x80) != 0);
    state->codes.P = getParity(*address);
}

void opINX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg) {
    u_int16_t word = (*firstReg << 8) | *secondReg;
    word += 1;
    *firstReg = (word & 0xff00) >> 8;
    *secondReg = word & 0xff;
}

void opDCX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg) {
    u_int16_t word = (*firstReg << 8) | *secondReg;
    word -= 1;
    *firstReg = (word & 0xff00) >> 8;
    *secondReg = word & 0xff;
}

void opDAD(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg) {
    u_int16_t hlValue = (state->H << 8) | state->L;
    u_int16_t addend = (*firstReg << 8) | *secondReg;

    u_int32_t sum = (u_int32_t) hlValue + (u_int32_t) addend;
    u_int16_t sumTruncated = sum & 0x0000ffff;

    state->codes.CY = sum > 0xffff;

    state->H = (sumTruncated & 0xff00) >> 8;
    state->L = sumTruncated & 0xff;
}

void opDAD_sp(State8080* state) {
    u_int16_t hlValue = (state->H << 8) | state->L;

    u_int32_t sum = (u_int32_t) hlValue + (u_int32_t) state->SP;
    u_int16_t sumTruncated = sum & 0x0000ffff;

    state->codes.CY = sum > 0xffff;

    state->H = (sumTruncated & 0xff00) >> 8;
    state->L = sumTruncated & 0xff;
}

void opDAA(State8080* state) {
    u_int8_t lsb = state->A & 0xf0;
    u_int8_t originalMsb = (state->A & 0xf0) >> 4;
    if ((lsb > 9) || state->codes.AC) {
        state->A += 6;
    }

    u_int8_t msb = (state->A & 0xf0) >> 4;
    state->codes.AC = msb > originalMsb;

    u_int16_t sum = (u_int16_t) state->A;
    if ((msb > 9) || state->codes.CY) {
        sum += 0x0060;
        state->A = sum & 0xff;
    }
    state->codes.CY = sum > 0xff;

    state->codes.Z = (state->A == 0);
    state->codes.S = ((state->A & 0x80) != 0);
    state->codes.P = getParity(state->A);
}

u_int8_t getParity(u_int8_t value) {
    u_int8_t temp = value ^ (value >> 4);
    temp = temp ^ (temp >> 2);
    temp = temp ^ (temp >> 1);
    return temp & 1;
}

void UnimplementedInstruction(State8080* state) {
    u_int8_t opCode = state->memory[state->PC];
    printf("Error: Unimplemented instruction (0x%02x at offset 0x%02x)\n", opCode, state->PC);
    state->PC -= 1;
    exit(1);
}

int getFileSize(FILE *f) {
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    return fsize;
}