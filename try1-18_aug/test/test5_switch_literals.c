/* test5: switch statement and literal forms */
int main() {
    int day = 3;
    float pi = 3.14159;
    char grade = 'A';
    char *msg = "hello world"; // string literal

    switch (day) {
        case 1:
            grade = 'M';
            break;
        case 3:
            grade = 'W';
            break;
        default:
            grade = 'X';
            break;
    }

    return 0;
}
