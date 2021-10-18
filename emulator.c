#include <stdio.h>
#include <stdlib.h>

#include "disassembler.h"

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

    // Assumption: we can address bytes in this memory map using u_int16_t values (stored in registers)
    // This is not guaranteed to be true on any given architecture. Consider using a map of u_int16_t -> u_int8_t instead.
    u_int8_t*      memory;
    ConditionCodes codes;
    u_int8_t       int_enable;
} State8080;

int emulate8080(FILE *f);
void emulateOp8080(State8080* state);

void opMOV(u_int8_t* destAddr, u_int8_t* srcAddr);
void opMVI(State8080* state, u_int8_t* dest);
void opLXI(State8080* state, u_int8_t* opPointer, u_int8_t* firstReg, u_int8_t* secondReg);
void opLXI_sp(State8080* state, u_int8_t* opPointer);
void opLDA(State8080* state);
void opSTA(State8080* state);
void opLHLD(State8080* state);
void opSHLD(State8080* state);

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
void opDAD(State8080* state, u_int8_t firstReg, u_int8_t secondReg);
void opDAD_sp(State8080* state);
void opDAA(State8080* state);

void opANA(State8080* state, u_int8_t operand);
void opANI(State8080* state);
void opXRA(State8080* state, u_int8_t operand);
void opXRI(State8080* state);
void opORA(State8080* state, u_int8_t operand);
void opORI(State8080* state);
void opCMP(State8080* state, u_int8_t operand);
void opCPI(State8080* state);

void opJMP(State8080* state, u_int8_t* opPointer);
void opJMPConditional(State8080* state, u_int8_t* opPointer, u_int8_t condition);
void opCALL(State8080* state, u_int8_t* opPointer);
void opCALLConditional(State8080* state, u_int8_t* opPointer, u_int8_t condition);
void opRET(State8080* state);
void opRETConditional(State8080* state, u_int8_t condition);
void opRST(State8080* state, u_int8_t exp);
void opPCHL(State8080* state);

void pushPCToStack(State8080* state);
u_int8_t dereferenceHL(State8080* state);
u_int16_t getHLValue(State8080* state);
u_int16_t wordFromBytes(u_int8_t left, u_int8_t right);
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
    printf("[ PC %04x ]\n", state->PC);
    printf("-----------\n");
    printDisassembledOp(state->memory, state->PC);
    printf("-----------\n");

    unsigned char* opCode = &state->memory[state->PC];

    switch(*opCode) {
        // Data Transfer Group

        // MOV
        case 0x40: opMOV(&state->B, &state->B); break;
        case 0x41: opMOV(&state->B, &state->C); break;
        case 0x42: opMOV(&state->B, &state->D); break;
        case 0x43: opMOV(&state->B, &state->E); break;
        case 0x44: opMOV(&state->B, &state->H); break;
        case 0x45: opMOV(&state->B, &state->L); break;
        case 0x47: opMOV(&state->B, &state->A); break;
        case 0x48: opMOV(&state->C, &state->B); break;
        case 0x49: opMOV(&state->C, &state->C); break;
        case 0x4a: opMOV(&state->C, &state->D); break;
        case 0x4b: opMOV(&state->C, &state->E); break;
        case 0x4c: opMOV(&state->C, &state->H); break;
        case 0x4d: opMOV(&state->C, &state->L); break;
        case 0x4f: opMOV(&state->C, &state->A); break;
        case 0x50: opMOV(&state->D, &state->B); break;
        case 0x51: opMOV(&state->D, &state->C); break;
        case 0x52: opMOV(&state->D, &state->D); break;
        case 0x53: opMOV(&state->D, &state->E); break;
        case 0x54: opMOV(&state->D, &state->H); break;
        case 0x55: opMOV(&state->D, &state->L); break;
        case 0x57: opMOV(&state->D, &state->A); break;
        case 0x58: opMOV(&state->E, &state->B); break;
        case 0x59: opMOV(&state->E, &state->C); break;
        case 0x5a: opMOV(&state->E, &state->D); break;
        case 0x5b: opMOV(&state->E, &state->E); break;
        case 0x5c: opMOV(&state->E, &state->H); break;
        case 0x5d: opMOV(&state->E, &state->L); break;
        case 0x5f: opMOV(&state->E, &state->A); break;
        case 0x60: opMOV(&state->H, &state->B); break;
        case 0x61: opMOV(&state->H, &state->C); break;
        case 0x62: opMOV(&state->H, &state->D); break;
        case 0x63: opMOV(&state->H, &state->E); break;
        case 0x64: opMOV(&state->H, &state->H); break;
        case 0x65: opMOV(&state->H, &state->L); break;
        case 0x67: opMOV(&state->H, &state->A); break;
        case 0x68: opMOV(&state->L, &state->B); break;
        case 0x69: opMOV(&state->L, &state->C); break;
        case 0x6a: opMOV(&state->L, &state->D); break;
        case 0x6b: opMOV(&state->L, &state->E); break;
        case 0x6c: opMOV(&state->L, &state->H); break;
        case 0x6d: opMOV(&state->L, &state->L); break;
        case 0x6f: opMOV(&state->L, &state->A); break;
        case 0x78: opMOV(&state->A, &state->B); break;
        case 0x79: opMOV(&state->A, &state->C); break;
        case 0x7a: opMOV(&state->A, &state->D); break;
        case 0x7b: opMOV(&state->A, &state->E); break;
        case 0x7c: opMOV(&state->A, &state->H); break;
        case 0x7d: opMOV(&state->A, &state->L); break;
        case 0x7f: opMOV(&state->A, &state->A); break;

        case 0x46: opMOV(&state->B, (u_int8_t*) getHLValue(state)); break;
        case 0x4e: opMOV(&state->C, (u_int8_t*) getHLValue(state)); break;
        case 0x56: opMOV(&state->D, (u_int8_t*) getHLValue(state)); break;
        case 0x5e: opMOV(&state->E, (u_int8_t*) getHLValue(state)); break;
        case 0x66: opMOV(&state->H, (u_int8_t*) getHLValue(state)); break;
        case 0x6e: opMOV(&state->L, (u_int8_t*) getHLValue(state)); break;
        case 0x7e: opMOV(&state->A, (u_int8_t*) getHLValue(state)); break;

        case 0x70: opMOV((u_int8_t*) getHLValue(state), &state->B); break;
        case 0x71: opMOV((u_int8_t*) getHLValue(state), &state->C); break;
        case 0x72: opMOV((u_int8_t*) getHLValue(state), &state->D); break;
        case 0x73: opMOV((u_int8_t*) getHLValue(state), &state->E); break;
        case 0x74: opMOV((u_int8_t*) getHLValue(state), &state->H); break;
        case 0x75: opMOV((u_int8_t*) getHLValue(state), &state->L); break;
        case 0x77: opMOV((u_int8_t*) getHLValue(state), &state->A); break;

        case 0x06: opMVI(state, &state->B); break;
        case 0x0e: opMVI(state, &state->C); break;
        case 0x16: opMVI(state, &state->D); break;
        case 0x1e: opMVI(state, &state->E); break;
        case 0x26: opMVI(state, &state->H); break;
        case 0x2e: opMVI(state, &state->L); break;
        case 0x3e: opMVI(state, &state->A); break;

        case 0x36: opMVI(state, (u_int8_t*) getHLValue(state)); break;

        // LXI
        case 0x01: opLXI(state, opCode, &state->B, &state->C); break;
        case 0x11: opLXI(state, opCode, &state->D, &state->E); break;
        case 0x21: opLXI(state, opCode, &state->H, &state->L); break;
        case 0x31: opLXI_sp(state, opCode); break;

        case 0x3a: opLDA(state); break;
        case 0x32: opSTA(state); break;
        case 0x2a: opLHLD(state); break;
        case 0x22: opSHLD(state); break;

        // case 0x0a: printf("LDAX   A <- (B-C) "); break;
        // case 0x1a: printf("LDAX   A <- (D-E) "); break;
        // case 0x02: printf("STAX   (B-C) <- A"); break;
        // case 0x12: printf("STAX   (D-E) <- A"); break;

        // case 0xeb: printf("XCHG   H-L <-> D-E"); break;


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
        case 0x86: opADD(state, dereferenceHL(state)); break;

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
        case 0x8e: opADC(state, dereferenceHL(state)); break;

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
        case 0x96: opSUB(state, dereferenceHL(state)); break;

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
        case 0x9e: opSBB(state, dereferenceHL(state)); break;

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
        case 0x34: opINR(state, &state->memory[getHLValue(state)]); break;

        // DCR
        case 0x05: opDCR(state, &state->B); break;
        case 0x0d: opDCR(state, &state->C); break;
        case 0x15: opDCR(state, &state->D); break;
        case 0x1d: opDCR(state, &state->E); break;
        case 0x25: opDCR(state, &state->H); break;
        case 0x2d: opDCR(state, &state->L); break;
        case 0x3d: opDCR(state, &state->A); break;

        // DCR M
        case 0x35: opDCR(state, &state->memory[getHLValue(state)]); break;

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

        // ANA
        case 0xa0: opANA(state, state->B); break;
        case 0xa1: opANA(state, state->C); break;
        case 0xa2: opANA(state, state->D); break;
        case 0xa3: opANA(state, state->E); break;
        case 0xa4: opANA(state, state->H); break;
        case 0xa5: opANA(state, state->L); break;
        case 0xa7: opANA(state, state->A); break;

        // ANA M
        case 0xa6: opANA(state, dereferenceHL(state)); break;

        // ANI
        case 0xe6: opANI(state); break;


        // XRA
        case 0xa8: opXRA(state, state->B); break;
        case 0xa9: opXRA(state, state->C); break;
        case 0xaa: opXRA(state, state->D); break;
        case 0xab: opXRA(state, state->E); break;
        case 0xac: opXRA(state, state->H); break;
        case 0xad: opXRA(state, state->L); break;
        case 0xaf: opXRA(state, state->A); break;

        // XRA M
        case 0xae: opXRA(state, dereferenceHL(state)); break;

        // XRI
        case 0xee: opXRI(state); break;


        // ORA
        case 0xb0: opORA(state, state->B); break;
        case 0xb1: opORA(state, state->C); break;
        case 0xb2: opORA(state, state->D); break;
        case 0xb3: opORA(state, state->E); break;
        case 0xb4: opORA(state, state->H); break;
        case 0xb5: opORA(state, state->L); break;
        case 0xb7: opORA(state, state->A); break;

        // ORA M
        case 0xb6: opORA(state, dereferenceHL(state)); break;

        // ORI
        case 0xf6: opORI(state); break;


        // CMP
        case 0xb8: opCMP(state, state->B); break;
        case 0xb9: opCMP(state, state->C); break;
        case 0xba: opCMP(state, state->D); break;
        case 0xbb: opCMP(state, state->E); break;
        case 0xbc: opCMP(state, state->H); break;
        case 0xbd: opCMP(state, state->L); break;
        case 0xbf: opCMP(state, state->A); break;

        // CMP M
        case 0xbe: opCMP(state, dereferenceHL(state)); break;

        // CPI
        case 0xfe: opCPI(state); break;

        // RLC
        case 0x07: {
            u_int8_t tmp = state->A;
            state->A = (tmp << 1 | tmp >> 7);
            state->codes.CY = tmp >> 7;
            break;
        }

        // RRC
        case 0x0f: {
            u_int8_t tmp = state->A;
            state->A = (tmp >> 1 | tmp << 7);
            state->codes.CY = tmp & 1;
            break;
        }

        // RAL
        case 0x17: {
            u_int8_t tmp = state->A;
            state->A = (tmp << 1 | state->codes.CY);
            state->codes.CY = tmp >> 7;
            break;
        }

        // RAR
        case 0x1f: {
            u_int8_t tmp = state->A;
            state->A = (tmp >> 1 | state->codes.CY << 7);
            state->codes.CY = tmp & 1;
            break;
        }

        // CMA
        case 0x2f: {
            state->A = ~state->A;
            break;
        }
        // CMC
        case 0x3f: {
            state->codes.CY = ~state->codes.CY;
            break;
        }
        // STC
        case 0x37: {
            state->codes.CY = 1;
            break;
        }
        
        // Branch Group

        // JMP etc
        case 0xc3: opJMP(state, opCode); break;
        case 0xc2: opJMPConditional(state, opCode, !state->codes.Z); break;
        case 0xca: opJMPConditional(state, opCode, state->codes.Z); break;
        case 0xd2: opJMPConditional(state, opCode, !state->codes.CY); break;
        case 0xda: opJMPConditional(state, opCode, state->codes.CY); break;
        case 0xe2: opJMPConditional(state, opCode, !state->codes.P); break;
        case 0xea: opJMPConditional(state, opCode, state->codes.P); break;
        case 0xf2: opJMPConditional(state, opCode, !state->codes.S); break;
        case 0xfa: opJMPConditional(state, opCode, state->codes.S); break;

        // CALL etc
        case 0xcd: opCALL(state, opCode); break;
        case 0xc4: opCALLConditional(state, opCode, !state->codes.Z); break;
        case 0xcc: opCALLConditional(state, opCode, state->codes.Z); break;
        case 0xd4: opCALLConditional(state, opCode, !state->codes.CY); break;
        case 0xdc: opCALLConditional(state, opCode, state->codes.CY); break;
        case 0xe4: opCALLConditional(state, opCode, !state->codes.P); break;
        case 0xec: opCALLConditional(state, opCode, state->codes.P); break;
        case 0xf4: opCALLConditional(state, opCode, !state->codes.S); break;
        case 0xfc: opCALLConditional(state, opCode, state->codes.S); break;

        // RET etc
        case 0xc9: opRET(state); break;
        case 0xc0: opRETConditional(state, !state->codes.Z); break;
        case 0xc8: opRETConditional(state, state->codes.Z); break;
        case 0xd0: opRETConditional(state, !state->codes.CY); break;
        case 0xd8: opRETConditional(state, state->codes.CY); break;
        case 0xe0: opRETConditional(state, !state->codes.P); break;
        case 0xe8: opRETConditional(state, state->codes.P); break;
        case 0xf0: opRETConditional(state, !state->codes.S); break;
        case 0xf8: opRETConditional(state, state->codes.S); break;

        // RST
        case 0xc7: opRST(state, 0); break;
        case 0xcf: opRST(state, 1); break;
        case 0xd7: opRST(state, 2); break;
        case 0xdf: opRST(state, 3); break;
        case 0xe7: opRST(state, 4); break;
        case 0xef: opRST(state, 5); break;
        case 0xf7: opRST(state, 6); break;
        case 0xff: opRST(state, 7); break;

        case 0xe9: opPCHL(state); break;


        case 0x02: UnimplementedInstruction(state); break;
        case 0x08: UnimplementedInstruction(state); break;
        case 0x0a: UnimplementedInstruction(state); break;
        case 0x10: UnimplementedInstruction(state); break;
        case 0x12: UnimplementedInstruction(state); break;
        case 0x18: UnimplementedInstruction(state); break;
        case 0x1a: UnimplementedInstruction(state); break;
        case 0x20: UnimplementedInstruction(state); break;
        case 0x28: UnimplementedInstruction(state); break;
        case 0x30: UnimplementedInstruction(state); break;
        case 0x38: UnimplementedInstruction(state); break;
        case 0x76: UnimplementedInstruction(state); break;
        case 0xc1: UnimplementedInstruction(state); break;
        case 0xc5: UnimplementedInstruction(state); break;
        case 0xcb: UnimplementedInstruction(state); break;
        case 0xd1: UnimplementedInstruction(state); break;
        case 0xd3: UnimplementedInstruction(state); break;
        case 0xd5: UnimplementedInstruction(state); break;
        case 0xd9: UnimplementedInstruction(state); break;
        case 0xdb: UnimplementedInstruction(state); break;
        case 0xdd: UnimplementedInstruction(state); break;
        case 0xe1: UnimplementedInstruction(state); break;
        case 0xe3: UnimplementedInstruction(state); break;
        case 0xe5: UnimplementedInstruction(state); break;
        case 0xeb: UnimplementedInstruction(state); break;
        case 0xed: UnimplementedInstruction(state); break;
        case 0xf1: UnimplementedInstruction(state); break;
        case 0xf3: UnimplementedInstruction(state); break;
        case 0xf5: UnimplementedInstruction(state); break;
        case 0xf9: UnimplementedInstruction(state); break;
        case 0xfb: UnimplementedInstruction(state); break;
        case 0xfd: UnimplementedInstruction(state); break;

        // Stack, I/O, and Machine Control group
        case 0x00: break; // NOP
    }

    // operation consumed, so advance
    state->PC++;

    printf("==> Resulting processor state\n");
    printf("    C=%d,P=%d,S=%d,Z=%d,AC=%d\n",
        state->codes.CY, state->codes.P, state->codes.S, state->codes.Z, state->codes.AC);
    printf("    A $%02x | B $%02x C $%02x | D $%02x E $%02x | H $%02x L $%02x\n",
        state->A, state->B, state->C, state->D, state->E, state->H, state->L);
    printf("    SP $%04x\n\n", state->SP);
}

// Operation functions

void opMOV(u_int8_t* destAddr, u_int8_t* srcAddr) {
    *destAddr = *srcAddr;
}

void opMVI(State8080* state, u_int8_t* dest) {
    u_int8_t nextByte = state->memory[state->PC+1];
    *dest = nextByte;
}

void opLXI(State8080* state, u_int8_t* opPointer, u_int8_t* firstReg, u_int8_t* secondReg) {
    *firstReg = opPointer[2];
    *secondReg = opPointer[1];
    state->PC += 2;
}

void opLXI_sp(State8080* state, u_int8_t* opPointer) {
    u_int16_t combinedValue = wordFromBytes(opPointer[2], opPointer[1]);
    state->SP = combinedValue;
    state->PC += 2;
}

void opLDA(State8080* state) {
    u_int16_t addr = wordFromBytes(state->PC+2, state->PC+1);
    state->A = state->memory[addr];
    state->PC += 2;
}

void opSTA(State8080* state) {
    u_int16_t addr = wordFromBytes(state->PC+2, state->PC+1);
    state->memory[addr] = state->A;
    state->PC += 2;
}

void opLHLD(State8080* state) {
    u_int16_t firstAddr = wordFromBytes(state->PC+2, state->PC+1);
    state->L = state->memory[firstAddr];
    state->H = state->memory[firstAddr+1];
    state->PC += 2;
}

void opSHLD(State8080* state) {
    u_int16_t firstAddr = wordFromBytes(state->PC+2, state->PC+1);
    state->memory[firstAddr] = state->L;
    state->memory[firstAddr+1] = state->H;
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
    u_int16_t word = wordFromBytes(*firstReg, *secondReg);
    word += 1;
    *firstReg = (word & 0xff00) >> 8;
    *secondReg = word & 0xff;
}

void opDCX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg) {
    u_int16_t word = wordFromBytes(*firstReg, *secondReg);
    word -= 1;
    *firstReg = (word & 0xff00) >> 8;
    *secondReg = word & 0xff;
}

void opDAD(State8080* state, u_int8_t firstReg, u_int8_t secondReg) {
    u_int16_t hlValue = getHLValue(state);
    u_int16_t addend = wordFromBytes(firstReg, secondReg);

    u_int32_t sum = (u_int32_t) hlValue + (u_int32_t) addend;
    u_int16_t sumTruncated = sum & 0x0000ffff;

    state->codes.CY = sum > 0xffff;

    state->H = (sumTruncated & 0xff00) >> 8;
    state->L = sumTruncated & 0xff;
}

void opDAD_sp(State8080* state) {
    u_int16_t hlValue = getHLValue(state);

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

void opJMP(State8080* state, u_int8_t* opPointer) {
    state->PC = wordFromBytes(opPointer[2], opPointer[1]);

    // account for PC auto-advance
    state->PC -= 1;
}

void opJMPConditional(State8080* state, u_int8_t* opPointer, u_int8_t condition) {
    if (condition) {
        opJMP(state, opPointer);
    } else {
        state->PC += 2;
    }
}

void opCALL(State8080* state, u_int8_t* opPointer) {
    pushPCToStack(state);

    state->PC = wordFromBytes(opPointer[2], opPointer[1]);

    // account for PC auto-advance
    state->PC -= 1;
}

void opCALLConditional(State8080* state, u_int8_t* opPointer, u_int8_t condition) {
    if (condition) {
        opCALL(state, opPointer);
    } else {
        state->PC += 2;
    }
}

void opRET(State8080* state) {
    state->PC = wordFromBytes(state->memory[state->SP+1], state->memory[state->SP]);
    state->SP += 2;

    // account for PC auto-advance
    state->PC -= 1;
}

void opRETConditional(State8080* state, u_int8_t condition) {
    if (condition) {
        opRET(state);
    }
}

void opRST(State8080* state, u_int8_t exp) {
    pushPCToStack(state);

    state->PC = exp * 8;

    // account for PC auto-advance
    state->PC -= 1;
}

void opPCHL(State8080* state) {
    state->PC = getHLValue(state);
    state->PC -= 1;
}

void opANA(State8080* state, u_int8_t operand) {
    u_int8_t result = state->A & operand;

    state->codes.Z = (result == 0);
    state->codes.S = ((result & 0x80) != 0);
    state->codes.P = getParity(result);
    state->codes.CY = 0;

    state->A = result;
}

void opANI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opANA(state, nextByte);
    state->PC += 1;
}

void opXRA(State8080* state, u_int8_t operand) {
    u_int8_t result = state->A ^ operand;

    state->codes.Z = (result == 0);
    state->codes.S = ((result & 0x80) != 0);
    state->codes.P = getParity(result);
    state->codes.CY = 0;

    state->A = result;
}

void opXRI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opXRA(state, nextByte);
    state->PC += 1;
}

void opORA(State8080* state, u_int8_t operand) {
    u_int8_t result = state->A | operand;

    state->codes.Z = (result == 0);
    state->codes.S = ((result & 0x80) != 0);
    state->codes.P = getParity(result);
    state->codes.CY = 0;

    state->A = result;
}

void opORI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opORA(state, nextByte);
    state->PC += 1;
}

/**
 * Identical to SUB, except result is not stored
 */
void opCMP(State8080* state, u_int8_t operand) {
    u_int16_t result = (u_int16_t) state->A - (u_int16_t) operand;
    u_int8_t resultTruncated = result & 0xff;

    state->codes.Z = (resultTruncated == 0);
    state->codes.S = ((result & 0x80) != 0);
    state->codes.P = getParity(resultTruncated);
    // carry out of high-order bit indicates no borrow occurred, meaning Carry is reset
    state->codes.CY = result <= 0xff;
}

void opCPI(State8080* state) {
    u_int8_t nextByte = state->memory[state->PC+1];
    opCMP(state, nextByte);
    state->PC += 1;
}

// Utilities

void pushPCToStack(State8080* state) {
    state->memory[state->SP-1] = (state->PC >> 8);
    state->memory[state->SP-2] = (state->PC & 0xff);
    state->SP -= 2;
}

u_int8_t dereferenceHL(State8080* state) {
    return state->memory[getHLValue(state)];
}

u_int16_t getHLValue(State8080* state) {
    return wordFromBytes(state->H, state->L);
}

u_int16_t wordFromBytes(u_int8_t left, u_int8_t right) {
    return (left << 8) | right;
}

u_int8_t getParity(u_int8_t value) {
    u_int8_t temp = value ^ (value >> 4);
    temp = temp ^ (temp >> 2);
    temp = temp ^ (temp >> 1);
    return !(temp & 1);
}

void UnimplementedInstruction(State8080* state) {
    u_int8_t opCode = state->memory[state->PC];
    printf("Error: Unimplemented instruction (0x%02x at offset 0x%02x)\n", opCode, state->PC);
    state->PC -= 1;
    exit(1);
}