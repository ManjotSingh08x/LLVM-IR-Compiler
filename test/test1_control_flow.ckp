// Tests: if-else
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
    // Tests: arithmetic operators
    int a = 10, b = 3, c;
    bool t;

    c= a+b;   c=a -b;
    c=a* b;
    c = a / b;   c = a % b;

    c += a;  c -= a;  c *= a;  c /= a;  c %= a;

    a++;  a--;  ++a;  --a;

    // Tests: logical and relational operators
    t = (a == b);  t = (a != b);
    t = (a <  b);  t = (a >  b);
    t = (a <= b);  t = (a >= b);
    t = (a && b);  t = (a || b);  t = !a;

    // Tests: additional bitwise operators
    c = a & b;   c = a | b;   c = a ^ b;
    c = ~a;      c = a << 2;  c = a >> 2;

    // Tests: additional ternary operator
    c = (a > b) ? a : b;

    // Tests: for loop; break and continue
    int i, sum = 0;
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) continue;
        if (i > 7) break;
        sum += i;
    }

    // Tests: while loop
    while (sum > 0) { sum -= 1; }

    // Tests: do-while loop
    do { sum += 2; } while (sum < 10);

    // Tests: goto
    i = 0;
top:
    i = i + 1;
    if (i < 3) goto top;

    // Tests: function call with arguments
    return classify(sum);
}
