#include "../helper.h"
#include "../../dist/wasmu.h"

TEST {
    PART("Create new context");

    wasmu_Context* context = wasmu_newContext();

    ASSERT(context, "Context is null");

    PASS();
}