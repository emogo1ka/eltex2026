#include "calc.h"

double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0) {
        printf("Ошибка: деление на 0!\n");
        return NAN;
    }
    return a / b;
}

int check_result(double result) {
    if (isnan(result)) {
        printf("Результат: NaN\n");
        return 0;
    }
    if (isinf(result)) {
        printf("Результат: Infinity\n");
        return 0;
    }
    return 1;
}

