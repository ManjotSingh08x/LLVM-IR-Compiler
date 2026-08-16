// Tests: structure
struct Point {
    int x;
    int y;
};

// Tests: class and object; public/private/protected keywords
class Foo {
public:
    int  m_id;
    void setId(int v)      { this->m_id = v; }
    int  getId()           { return this->m_id; }
    void setFilled(bool f) { this->m_filled = f; }
    bool isFilled()        { return this->m_filled; }
protected:
    int  m_level;
    bool m_filled;
private:
    int  m_serial;
    int  checksum()        { return this->m_serial + this->m_level; }
};

// Tests: inheritance
class Bar : public Foo {
private:
    int m_width;
    int m_height;
public:
    void init(int a, int b) { this->m_width = a; this->m_height = b; }
    int  area()             { return this->m_width * this->m_height; }
    int  perimeter()        { return 2 * (this->m_width + this->m_height); }
};

// Tests: function call with arguments
int add(int a, int b) {
    return a + b;
}

// Tests: recursive function call
int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

// Tests: recursive function call
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// Tests: reference
void swap(int &a, int &b) {
    int t = a;
    a = b;
    b = t;
}

// Tests: pointers; function call with arguments
int sumArray(int *data, int count) {
    int total = 0;
    int i;
    for (i = 0; i < count; i++) {
        total += data[i];
    }
    return total;
}

// Tests: function pointer targets
int addOp(int a, int b) { return a + b; }
int mulOp(int a, int b) { return a * b; }

// Tests: command line input
int main(int argc, char *argv[]) {

    int mode = 1;
    if (argc > 1) {
        mode = 2;
    }
    printf("argc = %d, program = %s\n", argc, argv[0]);

    // Tests: arithmetic operators
    int a = 10;
    int b = 3;
    int c;

    c = a + b;
    c = a - b;
    c = a * b;
    c = a / b;
    c = a % b;
    c = -a;
    c = +a;

    a++;
    a--;
    ++a;
    --a;

    c += a;
    c -= a;
    c *= a;
    c /= a;
    c %= a;

    // Tests: logical and relational operators
    bool t;
    t = (a == b);
    t = (a != b);
    t = (a <  b);
    t = (a >  b);
    t = (a <= b);
    t = (a >= b);
    t = (a && b);
    t = (a || b);
    t = !t;
    t = true;
    t = false;

    int biggest = (a > b) ? a : b;

    // Tests: additional bitwise operators
    int mask;
    mask = a & b;
    mask = a | b;
    mask = a ^ b;
    mask = ~a;
    mask = a << 2;
    mask = a >> 2;

    // Tests: if-else
    if (biggest < 0) {
        c = 0;
    } else if (biggest == 0) {
        c = 1;
    } else {
        c = 2;
    }

    // Tests: for loop
    int sum = 0;
    int i;
    for (i = 0; i < 10; i++) {
        sum += i;
    }

    // Tests: while loop
    while (sum > 20) {
        sum -= 1;
    }

    // Tests: do-while loop
    int countdown = 3;
    do {
        countdown--;
    } while (countdown > 0);

    // Tests: switch cases
    switch (mode) {
        case 1:
            printf("mode: default run\n");
            break;
        case 2:
            printf("mode: arguments supplied\n");
            break;
        default:
            printf("mode: unknown\n");
    }

    // Tests: goto, break and continue
    int evens = 0;
    for (i = 0; i < 20; i++) {
        if (i % 2 != 0) continue;
        if (i > 12) break;
        evens += i;
    }

    int tries = 0;
retry:
    tries++;
    if (tries < 3) goto retry;

    // Tests: integer and char arrays
    int values[8];
    for (i = 0; i < 8; i++) {
        values[i] = (i + 1) * (i + 1);
    }
    int squares = sumArray(values, 8);

    char label[8];
    label[0] = 'R';
    label[1] = 'E';
    label[2] = 'S';
    label[3] = 'U';
    label[4] = 'L';
    label[5] = 'T';
    label[6] = ':';
    label[7] = '\0';

    // Tests: multi-dimensional arrays
    int  grid[3][3];
    char board[2][2];
    int  row;
    int  col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            grid[row][col] = row * 3 + col;
        }
    }
    board[0][0] = 'x';
    board[1][1] = 'o';
    int trace = grid[0][0] + grid[1][1] + grid[2][2];

    // Tests: pointers
    int  value = 99;
    int *ptr = &value;
    int  read = *ptr;
    ptr = null;
    if (ptr == null) {
        printf("null pointer check ok\n");
    }

    // Tests: reference
    int &alias = value;
    alias = read;

    int left  = 11;
    int right = 22;
    swap(left, right);

    // Tests: structure
    struct Point origin;
    origin.x = 0;
    origin.y = 0;

    struct Point *pointPtr = &origin;
    pointPtr->x = 3;
    pointPtr->y = 4;

    // Tests: class and object; inheritance
    Bar bar;
    bar.setId(1);
    bar.setFilled(true);
    bar.init(6, 7);
    int rectArea = bar.area();
    int rectPerim = bar.perimeter();

    // Tests: dynamic memory allocation
    Bar *barPtr = new Bar;
    barPtr->setId(2);
    barPtr->init(10, 20);
    int heapArea = barPtr->area();
    delete barPtr;

    int *heapData = new int[5];
    for (i = 0; i < 5; i++) {
        heapData[i] = i * 3;
    }
    int heapSum = sumArray(heapData, 5);
    delete heapData;

    // Tests: lambda function
    auto cube = [](int v) { return v * v * v; };
    int cubed = cube(4);

    // Tests: function pointer
    int (*binaryOp)(int, int) = addOp;
    int combined = binaryOp(left, right);
    binaryOp = mulOp;
    combined = combined + binaryOp(2, 5);

    // Tests: recursive function calls; function calls with arguments
    int g = gcd(48, 18);
    int f = factorial(6);
    int both = add(g, f);

    // Tests: printf and scanf
    int typed = 0;
    printf("enter a number: ");
    scanf("%d", &typed);
    printf("you typed %d, doubled %d\n", typed, typed * 2);

    // Tests: supported literal forms
    int    decNum   = 42;
    int    hexNum   = 0x1F;
    int    octNum   = 0755;
    int    binNum   = 0b1010;

    float  pi       = 3.14;
    float  tiny     = 1.5e-3;
    double half     = .5;
    double two      = 2.;
    double huge     = 1e10;

    char   letter   = 'a';
    char   newline  = '\n';
    char   tab      = '\t';
    char   quote    = '\'';
    char   slash    = '\\';
    char   hexChar  = '\x41';
    char   nul      = '\0';

    char  *greeting = "hello world";
    char  *escaped  = "tab\there, newline\n, quote \", slash \\";
    char  *tricky   = "not a comment: /* nor this */ // nor this";
    char  *empty    = "";

    bool   yes      = true;
    bool   no       = false;

    // Tests: printf output using values produced above
    printf("%s squares=%d trace=%d evens=%d tries=%d\n",
           label, squares, trace, evens, tries);
    printf("swapped=%d,%d alias=%d point=(%d,%d)\n",
           left, right, alias, origin.x, origin.y);
    printf("rect area=%d perim=%d heap area=%d heap sum=%d\n",
           rectArea, rectPerim, heapArea, heapSum);
    printf("cubed=%d combined=%d gcd=%d fact=%d both=%d\n",
           cubed, combined, g, f, both);
    printf("mask=%d biggest=%d countdown=%d c=%d t=%d\n",
           mask, biggest, countdown, c, t);
    printf("literals %d %d %d %d %f %f %f %f %f\n",
           decNum, hexNum, octNum, binNum, pi, tiny, half, two, huge);
    printf("chars %c%c%c%c%c%c%c\n",
           letter, newline, tab, quote, slash, hexChar, nul);
    printf("strings %s %s %s %s\n", greeting, escaped, tricky, empty);
    printf("bools %d %d board %c%c\n", yes, no, board[0][0], board[1][1]);

    return 0;
}
