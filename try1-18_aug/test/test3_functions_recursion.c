int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int add(int a, int b) {
    return a + b;
}

int main(int argc, char **argv) {
    int (*fp)(int, int) = add;
    int result = fp(3, 4);

    for (int i = 0; i < argc; i++) {
        if (i == 2) {
            continue;
        }
        if (i == 5) {
            break;
        }
    }

    int k = 0;
retry:
    k++;
    if (k < 3) goto retry;

    return factorial(5) + result;
}
