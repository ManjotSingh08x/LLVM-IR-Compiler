struct Point {
    int x;
    int y;
};

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    int matrix[3][3];
    char name[4] = "abc";

    int *p = &arr[0];
    *p = 42;

    matrix[0][0] = arr[1] + *p;

    struct Point pt;
    pt.x = 1;
    pt.y = 2;

    return 0;
}
