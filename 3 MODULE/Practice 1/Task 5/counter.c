#include "counter.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t g_notice_sig = 0;
static volatile sig_atomic_t g_sigint_cnt = 0;

static void on_signal(int sig) {
    if (sig == SIGINT)
        g_sigint_cnt++;
    if (sig == SIGINT || sig == SIGQUIT) // устанавливаем флаг о получении сигнала
        g_notice_sig = (sig_atomic_t)sig;
}

void setup_counter_signals(void) {
    struct sigaction sa; // структура для описания обработчика сигнала
    memset(&sa, 0, sizeof sa); // обнуляем структуру
    sa.sa_handler = on_signal; // устанавливаем функцию-обработчик сигнала
    sigemptyset(&sa.sa_mask); // не блокируем дополнительные сигналы во время обработки
    sa.sa_flags = 0; // без дополнительных флагов
    sigaction(SIGINT, &sa, NULL); // устанавливаем обработчик для SIGINT
    sigaction(SIGQUIT, &sa, NULL); // устанавливаем обработчик для SIGQUIT
}

void counter_run(const char *path) { 
    setup_counter_signals(); //настройка обработчиков сигналов для корректной работы счетчика

    FILE *f = fopen(path, "w");
    if (!f) {
        perror(path);
        return;
    }
    // отключаем буферизацию для немедленной записи в файл
    if (setvbuf(f, NULL, _IONBF, 0) != 0)
        perror("setvbuf");

    long long counter = 0;

    for (;;) {
        if (g_notice_sig != 0) {
            int s = (int)g_notice_sig; // сохраняем значение сигнала
            g_notice_sig = 0; // сбрасываем флаг сигнала
            if (s == SIGINT)
                fputs("Получен и обработан SIGINT\n", f);
            else if (s == SIGQUIT)
                fputs("Получен и обработан SIGQUIT\n", f);
            fflush(f);
        }

        if (g_sigint_cnt >= 3) {
            if (fclose(f) != 0)
                perror("fclose");
            return;
        }

        counter++;
        fprintf(f, "%lld\n", counter);
        fflush(f); //сразу записываем данные в файл
        sleep(1); // ждем 1 секунду
    }
}
