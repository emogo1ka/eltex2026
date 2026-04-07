#include <stdio.h>
#include "calc.h"

#ifdef _WIN32
#include <windows.h>
#endif

// Функция для безопасного чтения одного числа
int read_double(double *num) {
    if (scanf("%lf", num) != 1) {
        printf("Ошибка: введено не число!\n");
        while(getchar() != '\n'); // очищаем ввод
        return 0; // ошибка
    }
    return 1; // успешно
}

static int read_int(int *v) {
    if (scanf("%d", v) != 1) {
        printf("Ошибка: введено не число!\n");
        while (getchar() != '\n')
            ;
        return 0;
    }
    return 1;
}

static int add_with_count(int n, double *out) {
    double a, b, c, d, e;
    if (n < 2 || n > 5) {
        printf("Можно складывать 2..5 чисел.\n");
        return 0;
    }

    printf("Введите %d чисел: ", n);
    if (!read_double(&a) || !read_double(&b))
        return 0;
    if (n >= 3 && !read_double(&c))
        return 0;
    if (n >= 4 && !read_double(&d))
        return 0;
    if (n >= 5 && !read_double(&e))
        return 0;

    switch (n) {
        case 2: *out = add(2, a, b); break;
        case 3: *out = add(3, a, b, c); break;
        case 4: *out = add(4, a, b, c, d); break;
        case 5: *out = add(5, a, b, c, d, e); break;
        default: return 0;
    }
    return 1;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // UTF-8
    SetConsoleCP(65001);       
#endif

    int choice;
    while (1) {
        printf("\nВыберите действие:\n");
        printf("1: Сложение\n");
        printf("2: Вычитание\n");
        printf("3: Умножение\n");
        printf("4: Деление\n");
        printf("0: Выход\n");
        printf("Введите номер: ");
        
        if (!read_int(&choice))
            continue;

        double a, b, result;
        switch(choice) {
            case 1: // Сложение
            {
                int n;
                printf("Сколько чисел складывать (2-5): ");
                if (!read_int(&n))
                    break;
                if (!add_with_count(n, &result))
                    break;
                if (check_result(result)) printf("Результат: %.6lf\n", result);
                break;
            }

            case 2: // Вычитание
                printf("Введите два числа: ");
                if (!read_double(&a) || !read_double(&b)) break;
                result = subtract(a, b);
                if (check_result(result)) printf("Результат: %.6lf\n", result);
                break;

            case 3: // Умножение
                printf("Введите два числа: ");
                if (!read_double(&a) || !read_double(&b)) break;
                result = multiply(a, b);
                if (check_result(result)) printf("Результат: %.6lf\n", result);
                break;

            case 4: // Деление
                printf("Введите два числа: ");
                if (!read_double(&a) || !read_double(&b)) break;
                result = divide(a, b);
                if (check_result(result)) printf("Результат: %.6lf\n", result);
                break;

            case 0:
                printf("Выход из программы.\n");
                return 0;

            default:
                printf("Неизвестный пункт!\n");
        }
    }
}