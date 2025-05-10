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

void printInt(int value) {
    if (value < 0) {
        printChar('-');

        value *= -1;
    }

    char buffer[8];
    unsigned int i = sizeof(buffer) - 2;

    buffer[sizeof(buffer) - 1] = '\0';

    do {
        if (i == 0) {
            break;
        }

        buffer[i--] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    print(buffer + i + 1);
}

class A {
    private:
        const char* _value;

    public:
        A(const char* value) {
            _value = value;
        }

        virtual void describe() {
            print("This is from class A\n");
            print(_value);
        }
};

class B : public A {
    public:
        B(const char* value) : A(value) {}

        void describe() override {
            print("This is from class B\n");

            A::describe();
        }
};

WASM_EXPORT_AS("start") inline void start() {
    print("Hello, world!\n");
    print("This is a demo of wasmu!\n");

    A a("Class A value\n");

    a.describe();

    B b("Class B value\n");

    b.describe();

    print("Sum: ");
    printInt(123 + 456);
    print("\n");

    float floatA = 10.6;
    float floatB = 7;
    float floatC = floatA / floatB;

    print("Float div: ");
    printInt(floatC * 1000);
    print("\n");
}