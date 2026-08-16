/* test2_arrays_pointers_struct.c
   Covers: integer and char arrays, multi-dimensional arrays, pointers,
           references, structures, member access with . and ->
   This is also a multi-line block comment, which the lexer must skip. */

struct Point {
    int x;
    int y;
};

int main() {
    int  arr[5];
    char name[20];
    int  grid[3][4];
    int  cube[2][3][4];

    arr[0] = 1;
    name[0] = 'A';
    grid[1][2] = 7;
    cube[1][2][3] = 9;

    int value = 42;
    int *p = &value;
    int v = *p;

    int &ref = value;   // reference
    ref = v;

    struct Point pt;
    pt.x = 1;
    pt.y = 2;

    struct Point *pp = &pt;
    pp->x = 3;
    pp->y = 4;

    void *nothing = null;
    return 0;
}
