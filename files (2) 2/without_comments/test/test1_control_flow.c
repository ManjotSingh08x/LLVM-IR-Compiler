// test1 — control flow, operators, ternary

int classify(int n) {
    if (n < 0) {
        return -1;
    } else if (n == 0) {
        return 0;
    } else {
        return 1;
    }
}

int main() {
    int a = 10, b = 3, c;
    bool t;

    c = a + b;   c = a - b;   c = a * b;
    c = a / b;   c = a % b;

    c += a;  c -= a;  c *= a;  c /= a;  c %= a;

    a++;  a--;  ++a;  --a;

    t = (a == b);  t = (a != b);
    t = (a <  b);  t = (a >  b);
    t = (a <= b);  t = (a >= b);
    t = (a && b);  t = (a || b);  t = !a;

    c = a & b;   c = a | b;   c = a ^ b;
    c = ~a;      c = a << 2;  c = a >> 2;

    c = (a > b) ? a : b;

    int i, sum = 0;
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) continue;
        if (i > 7) break;
        sum += i;
    }

    while (sum > 0) { sum -= 1; }
    do { sum += 2; } while (sum < 10);

    i = 0;
top:
    i = i + 1;
    if (i < 3) goto top;

    return classify(sum);
}
