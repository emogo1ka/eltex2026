#ifndef CALC_H
#define CALC_H

#include <stdio.h>
#include <math.h>

// Тип указателя на функцию операции (два аргумента double)
typedef double (*operation_func)(double a, double b);

// Арифметические операции
double add(double a, double b);
double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);

// Проверка результата на NaN и Infinity
int check_result(double result);

#endif
