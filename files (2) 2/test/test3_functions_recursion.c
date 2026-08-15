// test3_functions_recursion.c
// Covers: function calls with arguments, recursive function calls,
//         function pointers, lambda functions, printf and scanf,
//         command line input, dynamic memory allocation.

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int add(int a, int b) {
    return a + b;
}

int twice(int x) {
    return x * 2;
}

int main(int argc, char *argv[]) {
    int input;
    scanf("%d", &input);

    int f = factorial(input);
    int s = add(f, 10);

    int (*fp)(int) = twice;
    s = fp(s);

    auto square = [](int x) { return x * x; };
    s = square(s);

    int *buffer = new int[10];
    buffer[0] = s;
    delete buffer;

    double *d = new double;
    delete d;

    printf("%d %d %s\n", s, argc, argv[0]);
    return 0;
}
