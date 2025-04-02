#ifndef HELPER_H_
#define HELPER_H_

#include <stdio.h>
#include <stdbool.h>

#define TEST bool test()
#define PART(name) printf("\n# %s\n", name);
#define PASS() printf("PASS\n"); return true;
#define FAIL(reason) printf("FAIL: %s\n", reason); return false;
#define ASSERT(condition, failReason) if (!(condition)) {FAIL(failReason);}

bool test();

int main(int argc, char* argv[]) {
    return test() ? 0 : 1;
}

#endif