#include <stdio.h>
#include "calc.h"

#ifdef _WIN32
#include <windows.h>
#endif

typedef struct {
    int id;                    // номер пункта меню
    const char *name;          // текст пункта
    operation_func operation;  // указатель на функцию (может быть NULL для служебных пунктов)
} Command;

// Безопасное чтение одного double
int read_double(double *num) {
    if (scanf("%lf", num) != 1) {
        printf("Ошибка: введено не число!\n");
        while (getchar() != '\n'); // очищаем ввод
        return 0;
    }
    return 1;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // UTF-8
    SetConsoleCP(65001);
#endif

    // Таблица команд: динамически определяет доступные операции
    Command commands[] = {
        {1, "Сложение",      add},
        {2, "Вычитание",     subtract},
        {3, "Умножение",     multiply},
        {4, "Деление",       divide},
        {0, "Выход",         NULL}
    };
    int commands_count = sizeof(commands) / sizeof(commands[0]);

    while (1) {
        printf("\nВыберите действие:\n");
        for (int i = 0; i < commands_count; ++i) {
            printf("%d: %s\n", commands[i].id, commands[i].name);
        }
        printf("Введите номер: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка: введено не число!\n");
            while (getchar() != '\n');
            continue;
        }

        // Поиск выбранной команды в таблице
        const Command *selected = NULL;
        for (int i = 0; i < commands_count; ++i) {
            if (commands[i].id == choice) {
                selected = &commands[i];
                break;
            }
        }

        if (!selected) {
            printf("Неизвестный пункт!\n");
            continue;
        }

        if (selected->id == 0) {
            printf("Выход из программы.\n");
            return 0;
        }

        // Все арифметические операции принимают два аргумента
        double a, b;
        printf("Введите два числа: ");
        if (!read_double(&a) || !read_double(&b)) {
            continue;
        }

        double result = selected->operation(a, b);
        if (check_result(result)) {
            printf("Результат: %.6lf\n", result);
        }
    }
}

