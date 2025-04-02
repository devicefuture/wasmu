#ifndef HELPER_H_
#define HELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define _STR(value) #value
#define TEST bool test()
#define PART(name) printf("\n# %s\n", name)
#define PASS() printf("PASS\n"); return true;
#define FAIL(reason) printf("FAIL: %s\n", reason); return false;
#define ASSERT(condition, failReason) if (!(condition)) {FAIL(failReason);}
#define LOAD(filename, dataPtr) loadFile("test/" TEST_NAME "/" filename, dataPtr)

bool test();

int main(int argc, char* argv[]) {
    return test() ? 0 : 1;
}

unsigned int loadFile(char* path, char** data) {
    FILE* fp = fopen(path, "r");

    if (!fp) {
        printf("WARNING: Couldn't read file: %s\n", path);

        return 0;
    }

    fseek(fp, 0, SEEK_END);

    unsigned int size = ftell(fp);

    *data = (char*)malloc(size);

    fseek(fp, 0, SEEK_SET);

    if (fread(*data, sizeof(char), size, fp) != size) {
        printf("WARNING: Couldn't read contents of file: %s\n", path);

        return 0;
    }

    fclose(fp);

    return size;
}

#endif