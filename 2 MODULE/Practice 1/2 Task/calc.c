#include "calc.h"

double add(int n, ...) {
    va_list args;
    va_start(args, n);
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += va_arg(args, double);
    }
    va_end(args);
    return sum;
}

double subtract(double a, double b) { return a - b; }
double multiply(double a, double b) { return a * b; }

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
        printf("Результат: Inf\n"); 
        return 0; 
    }
    return 1;
}