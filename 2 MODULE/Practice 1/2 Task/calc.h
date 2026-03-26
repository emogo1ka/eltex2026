#ifndef CALC_H
#define CALC_H

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

// Variadic функция: сложение n чисел
double add(int n, ...);

double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);

// Проверка результата на NaN и Infinity
int check_result(double result);

#endif