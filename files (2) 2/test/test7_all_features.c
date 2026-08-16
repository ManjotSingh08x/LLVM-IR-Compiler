/* test7_all_features.c
 *
 * THE SINGLE FEATURE TEST FOR THE WHOLE PROJECT.
 *
 * Every feature ticked on the project proposal form is exercised exactly
 * once below, in a labelled section. Nothing that was left unticked appears
 * anywhere. If a compiler phase handles this one file, it handles the whole
 * agreed feature set.
 *
 *   Assignment 1  tokenize it, zero lexical errors
 *   Assignment 2  parse it into an AST
 *   Assignment 3  type-check it, build the symbol table
 *   Assignment 4  lower it to LLVM IR, then to x86
 *
 * NO STANDARD LIBRARY. This language has no preprocessor, so there is no
 * #include anywhere. printf and scanf are built-in functions supplied by the
 * compiler and declared implicitly. A '#' in a source file is a lexical
 * error.
 *
 * Ticked on the form, and covered here:
 *   BASIC     arithmetic and logical operators, if-else, for loop,
 *             while loop, do while loop, switch cases, array (integer and
 *             char), pointers, structure, printf and scanf, function call
 *             with arguments, goto/break/continue
 *   ADVANCED  recursive function call, class and object, inheritance,
 *             dynamic memory allocation, lambda function, function pointer,
 *             command line input, public/private/protected keywords,
 *             reference, multi-dimensional arrays
 *
 * NOT ticked, and deliberately absent:
 *   static keywords, function call with variable arguments, typedef,
 *   enum, union, file manipulation, until loop, multi-level pointers
 *
 * Also absent because this language does not define them: the compound
 * bitwise assignments <<= >>= &= |= ^=. Written out longhand instead.
 */


/* =========================================================================
 * SECTION 1 — BASIC 9: structure
 * ====================================================================== */

struct Point {
    int x;
    int y;
};


/* =========================================================================
 * SECTION 2 — ADVANCED 2: class and object
 *             ADVANCED 9: public, private and protected keywords
 * ====================================================================== */

class Shape {
public:
    int  id;
    void setId(int v)      { this->id = v; }
    int  getId()           { return this->id; }
    void setFilled(bool f) { this->filled = f; }
    bool isFilled()        { return this->filled; }
protected:
    int  level;
    bool filled;
private:
    int  serial;
    int  checksum()        { return this->serial + this->level; }
};


/* =========================================================================
 * SECTION 3 — ADVANCED 3: inheritance
 * ====================================================================== */

class Rect : public Shape {
private:
    int w;
    int h;
public:
    void init(int a, int b) { this->w = a; this->h = b; }
    int  area()             { return this->w * this->h; }
    int  perimeter()        { return 2 * (this->w + this->h); }
};


/* =========================================================================
 * SECTION 4 — BASIC 11: function call with arguments
 *             ADVANCED 1: recursive function call
 *             ADVANCED 11: reference (as a parameter)
 * ====================================================================== */

int add(int a, int b) {
    return a + b;
}

int gcd(int a, int b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

void swap(int &a, int &b) {
    int t = a;
    a = b;
    b = t;
}

/* BASIC 8: pointer as a parameter */
int sumArray(int *data, int count) {
    int total = 0;
    int i;
    for (i = 0; i < count; i++) {
        total += data[i];
    }
    return total;
}

/* ADVANCED 7: targets for a function pointer */
int addOp(int a, int b) { return a + b; }
int mulOp(int a, int b) { return a * b; }


/* =========================================================================
 * SECTION 5 — ADVANCED 8: command line input
 * ====================================================================== */

int main(int argc, char *argv[]) {

    int mode = 1;
    if (argc > 1) {
        mode = 2;
    }
    printf("argc = %d, program = %s\n", argc, argv[0]);


    /* =====================================================================
     * SECTION 6 — BASIC 1: all arithmetic operators
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 7 — BASIC 1: all logical and relational operators
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 8 — BASIC 1: bitwise operators
     * ================================================================== */

    int mask;
    mask = a & b;
    mask = a | b;
    mask = a ^ b;
    mask = ~a;
    mask = a << 2;
    mask = a >> 2;


    /* =====================================================================
     * SECTION 9 — BASIC 2: if-else
     * ================================================================== */

    if (biggest < 0) {
        c = 0;
    } else if (biggest == 0) {
        c = 1;
    } else {
        c = 2;
    }


    /* =====================================================================
     * SECTION 10 — BASIC 3: for loop
     * ================================================================== */

    int sum = 0;
    int i;
    for (i = 0; i < 10; i++) {
        sum += i;
    }


    /* =====================================================================
     * SECTION 11 — BASIC 4: while loop
     * ================================================================== */

    while (sum > 20) {
        sum -= 1;
    }


    /* =====================================================================
     * SECTION 12 — BASIC 5: do while loop
     * ================================================================== */

    int countdown = 3;
    do {
        countdown--;
    } while (countdown > 0);


    /* =====================================================================
     * SECTION 13 — BASIC 6: switch cases
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 14 — BASIC 12: goto, break and continue
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 15 — BASIC 7: array, integer and char
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 16 — ADVANCED 16: multi-dimensional arrays
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 17 — BASIC 8: pointers
     * ================================================================== */

    int  value = 99;
    int *p = &value;
    int  read = *p;
    p = null;
    if (p == null) {
        printf("null pointer check ok\n");
    }


    /* =====================================================================
     * SECTION 18 — ADVANCED 11: reference
     * ================================================================== */

    int &alias = value;
    alias = read;

    int left  = 11;
    int right = 22;
    swap(left, right);


    /* =====================================================================
     * SECTION 19 — BASIC 9: structure use, both access operators
     * ================================================================== */

    struct Point origin;
    origin.x = 0;
    origin.y = 0;

    struct Point *op = &origin;
    op->x = 3;
    op->y = 4;


    /* =====================================================================
     * SECTION 20 — ADVANCED 2 and 3: object use, inherited members
     * ================================================================== */

    Rect r;
    r.setId(1);
    r.setFilled(true);
    r.init(6, 7);
    int rectArea = r.area();
    int rectPerim = r.perimeter();


    /* =====================================================================
     * SECTION 21 — ADVANCED 5: dynamic memory allocation
     * ================================================================== */

    Rect *big = new Rect;
    big->setId(2);
    big->init(10, 20);
    int heapArea = big->area();
    delete big;

    int *heapData = new int[5];
    for (i = 0; i < 5; i++) {
        heapData[i] = i * 3;
    }
    int heapSum = sumArray(heapData, 5);
    delete heapData;


    /* =====================================================================
     * SECTION 22 — ADVANCED 6: lambda function
     * ================================================================== */

    auto cube = [](int v) { return v * v * v; };
    int cubed = cube(4);


    /* =====================================================================
     * SECTION 23 — ADVANCED 7: function pointer
     * ================================================================== */

    int (*binaryOp)(int, int) = addOp;
    int combined = binaryOp(left, right);
    binaryOp = mulOp;
    combined = combined + binaryOp(2, 5);


    /* =====================================================================
     * SECTION 24 — ADVANCED 1: recursive calls, BASIC 11: calls with args
     * ================================================================== */

    int g = gcd(48, 18);
    int f = factorial(6);
    int both = add(g, f);


    /* =====================================================================
     * SECTION 25 — BASIC 10: printf and scanf
     * ================================================================== */

    int typed = 0;
    printf("enter a number: ");
    scanf("%d", &typed);
    printf("you typed %d, doubled %d\n", typed, typed * 2);


    /* =====================================================================
     * SECTION 26 — every literal form the language accepts
     * ================================================================== */

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


    /* =====================================================================
     * SECTION 27 — report, so nothing above is dead code
     * ================================================================== */

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
