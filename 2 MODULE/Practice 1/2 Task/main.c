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

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // UTF-8 вывод
    SetConsoleCP(65001);       // UTF-8 ввод
#endif

    int choice;
    while (1) {
        printf("\nВыберите действие:\n");
        printf("1: Сложение (3 числа, variadic)\n");
        printf("2: Вычитание\n");
        printf("3: Умножение\n");
        printf("4: Деление\n");
        printf("0: Выход\n");
        printf("Введите номер: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка: введено не число!\n");
            while(getchar() != '\n');
            continue;
        }

        double a, b, c, result;
        switch(choice) {
            case 1: // Сложение (variadic)
                printf("Введите три числа для сложения: ");
                if (!read_double(&a) || !read_double(&b) || !read_double(&c)) break;
                result = add(3, a, b, c);
                if (check_result(result)) printf("Результат: %.6lf\n", result);
                break;

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