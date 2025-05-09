#define WASM_EXPORT extern "C" __attribute__((used)) __attribute__((visibility ("default")))
#define WASM_EXPORT_AS(name) WASM_EXPORT __attribute__((export_name(name)))
#define WASM_IMPORT(module, name) __attribute__((import_module(module))) __attribute__((import_name(name)))
#define WASM_CONSTRUCTOR __attribute__((constructor))

extern "C" {
    WASM_IMPORT("native", "printChar") void printChar(char c);
}

void print(const char* message) {
    while (*message) {
        printChar(*message);

        message++;
    }
}

WASM_EXPORT_AS("start") inline void start() {
    print("Hello, world!\n");
    print("This is a demo of wasmu!\n");
}