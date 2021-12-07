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
void opLDAX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg);
void opSTAX(State8080* state, u_int8_t* firstReg, u_int8_t* secondReg);
void opXCHG(State8080* state);

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

void opXTHL(State8080* state);

void pushPC(State8080* state);
void push(State8080* state, u_int8_t highOrderVal, u_int8_t lowOrderVal);
void pop(State8080* state, u_int8_t* highOrderReg, u_int8_t* lowOrderReg);

u_int8_t makePSW(State8080* state);
void setStateFromPsw(State8080* state, u_int8_t psw);

u_int8_t dereferenceHL(State8080* state);
u_int16_t getHLValue(State8080* state);
u_int16_t wordFromBytes(u_int8_t left, u_int8_t right);
u_int8_t getParity(u_int8_t value);

void UnimplementedInstruction(State8080* state);


