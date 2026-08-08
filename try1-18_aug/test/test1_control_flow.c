// test1: control flow + basic operators
int main() {
    int a = 5, b = 10, sum = 0;

    if (a < b && b != 0) {
        sum = a + b;
    } else {
        sum = a - b;
    }

    for (int i = 0; i < 10; i++) {
        sum += i;
    }

    int j = 0;
    while (j <= 5) {
        sum *= 2;
        j++;
    }

    do {
        sum--;
    } while (sum > 100);

    printf("%d\n", sum);
    scanf("%d", &a);
    return 0;
}
