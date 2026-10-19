// Tests: recursive function call
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// Tests: function call with arguments
int add(int a, int b) {
    return a + b;
}

int twice(int x) {
    return x * 2;
}

// Tests: command line input
int main(int argc, char *argv[]) {
    // Tests: printf and scanf
    int input;
    scanf("%d", &input);

    // Tests: recursive function call; function call with arguments
    int f = factorial(input);
    int s = add(f, 10);

    // Tests: function pointer
    int (*fp)(int) = twice;
    s = fp(s);

    // Tests: lambda function
    auto square = [](int x) { return x * x; };
    s = square(s);

    // Tests: dynamic memory allocation
    int *buffer = new int[10];
    buffer[0] = s;
    delete buffer;

    double *d = new double;
    delete d;

    // Tests: printf; command line input
    printf("%d %d %s\n", s, argc, argv[0]);
    return 0;
}
