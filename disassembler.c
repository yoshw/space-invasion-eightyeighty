int main(int argc, char** argv) {
    FILE *f = fopen(argv[1], "rb");

    if (f == NULL) {
        printf("Error: Couldn't open %s\n", argv[1]);
        exit(1);
    }

    disassemble8080Code(f);
}

void disassemble8080Code(FILE *f) {
    int fsize = getFileSize(f);
}

int getFileSize(FILE *f) {
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    return fsize;
}