#include <stdio.h>
#include <stdlib.h>

#include "emulator.h"
#include "../../albert-yu/emulator-101/include/cpu.h"

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

    diff8080s(f);
}

int diff8080s(FILE *f) {
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

    // State8080AY ayState = initialized state;

    int ops = 0;
    int OP_LIMIT = 1000000;
    while (ops < OP_LIMIT) {
        printf("[ Ops executed: %8d ]\n", ops);
        emulateOp8080(&state);
        // cpu_emulate_op(ayState, <io stuff>);

        // assert state equality
        ops += 1;
    }

    return 0;
}
