#include "phonebook.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

// читаем строку с учётом размера буфера.
// возвращает 1, если строка закончилась переводом строки в пределах буфера,
// и 0, если строка была обрезана (т.е. \n не попали в буфер) — остаток строки выкидываем.
static int read_line_internal(char *buf, size_t size, const char *prompt) {
    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
        return 1;
    }

    int has_nl = (strchr(buf, '\n') != NULL) || (strchr(buf, '\r') != NULL);

    // обрезаем \r\n в конце строки
    buf[strcspn(buf, "\r\n")] = '\0';

    // если строка не поместилась, выкидываем остаток до следующего перевода строки
    if (!has_nl) {
        int ch;
        do {
            ch = fgetc(stdin);
        } while (ch != '\n' && ch != '\r' && ch != EOF);
    }

    return has_nl;
}

// чтение строки (без статуса усечения)
void read_line(char *buf, size_t size, const char *prompt) {
    (void)read_line_internal(buf, size, prompt);
}

// чтение строки с повтором при усечении
// для необязательных полей: пустой ввод всё равно ок
static void read_line_checked(char *buf, size_t size, const char *prompt) {
    while (1) {
        int ok = read_line_internal(buf, size, prompt);
        if (ok) return;
        puts("Слишком длинный ввод. Повторите ввод.");
    }
}

// проверка строки на непустоту
int is_non_empty(const char *s) {
    if (!s) return 0;
    while (*s) {
        unsigned char ch = (unsigned char)*s;
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\v' || ch == '\f') {
            s++;
            continue;
        }
        return 1;
    }
    return 0;
}

// проверка на обязательное поле ввода
void read_required(char *buf, size_t size, const char *prompt) {
    do {
        int ok = read_line_internal(buf, size, prompt);
        if (!ok) {
            puts("Слишком длинный ввод. Повторите ввод.");
            continue;
        }
        if (is_non_empty(buf)) return;
        puts("Это поле обязательно. Повторите ввод.");
    } while (1);
}

// методы описанные для структуры pb

void init_phonebook(PhoneBook *pb) {
    pb->count = 0;
    pb->capacity = INITIAL_CAP;
    pb->next_id = 1;
    pb->contacts = malloc(pb->capacity * sizeof(Contact));
    if (!pb->contacts) {
        fputs("Ошибка выделения памяти\n", stderr);
        exit(1);
    }
}

void free_phonebook(PhoneBook *pb) {
    free(pb->contacts);
    pb->contacts = NULL;
    pb->count = pb->capacity = 0;
}

// функция добавления контакта
void add_contact(PhoneBook *pb) {
    if (pb->count >= pb->capacity) {
        int new_cap = pb->capacity * 2;
        Contact *tmp = realloc(pb->contacts, new_cap * sizeof(Contact));
        if (!tmp) {
            puts("Не удалось добавить контакт. Нет места. Ошибка расширения списка контактов");
            return;
        }
        pb->contacts = tmp;
        pb->capacity = new_cap;
    }

    Contact *c = &pb->contacts[pb->count]; //эуказатель на место в массиве куда мы будем записывать новый контакт
    memset(c, 0, sizeof(*c)); //обнуление содержимого структуы
    c->id = pb->next_id++;

    read_required(c->last_name,  MAX_FIO, "Фамилия (обязательное): ");
    read_required(c->first_name, MAX_FIO, "Имя (обязательное): ");
    read_line(c->patronymic,     MAX_FIO, "Отчество: ");
    read_line_checked(c->company,  MAX_LINE, "Компания: ");
    read_line_checked(c->position, MAX_LINE, "Должность: ");

    // телефоны
    puts("\nТелефоны (до 4 шт.)");
    c->phone_count = 0;
    for (int i = 0; i < MAX_PHONE; i++) {
        char code[16];
        char number[32];

        read_line(code, sizeof code, "  Код страны (Enter для пропуска ввода): ");
        if (!is_non_empty(code)) break;
        read_required(number, sizeof number, "  Номер: ");

        strncpy(c->phones[i].code, code, sizeof(c->phones[i].code) - 1);
        c->phones[i].code[sizeof(c->phones[i].code) - 1] = '\0';
        strncpy(c->phones[i].number, number, sizeof(c->phones[i].number) - 1);
        c->phones[i].number[sizeof(c->phones[i].number) - 1] = '\0';
        c->phone_count++;
    }

    // email
    puts("\nEmail (до 5 шт.) (Enter для пропуска ввода):");
    c->email_count = 0;
    for (int i = 0; i < MAX_EMAIL; i++) {
        read_line(c->emails[i], sizeof(c->emails[i]), "  ");
        if (!is_non_empty(c->emails[i])) break;
        c->email_count++;
    }

    // соцсети
    read_line(c->social.vk,        sizeof(c->social.vk),        "VK (Enter для пропуска ввода):        ");
    read_line(c->social.telegram,  sizeof(c->social.telegram),  "Telegram (Enter для пропуска ввода):  ");
    read_line(c->social.instagram, sizeof(c->social.instagram), "Instagram (Enter для пропуска ввода): ");

    pb->count++;
    printf("Контакт добавлен → ID %d\n", c->id);
}

// вывод контакта
void print_contact(const Contact *c) {
    printf("\n%d. %s %s %s\n", c->id, c->last_name, c->first_name, c->patronymic[0] ? c->patronymic : "");
    if (c->company[0] || c->position[0])
        printf("   %s — %s\n", c->company[0]?c->company:"—", c->position[0]?c->position:"—");

    if (c->phone_count > 0) {
        puts("   Телефоны:");
        for (int i = 0; i < c->phone_count; i++)
            printf("     +%s %s\n", c->phones[i].code, c->phones[i].number);
    }

    if (c->email_count > 0) {
        puts("   Email:");
        for (int i = 0; i < c->email_count; i++)
            printf("     %s\n", c->emails[i]);
    }

    if (c->social.vk[0] || c->social.telegram[0] || c->social.instagram[0]) {
        puts("   Соцсети:");
        if (c->social.vk[0])        printf("     VK: %s\n", c->social.vk);
        if (c->social.telegram[0])  printf("     Telegram: %s\n", c->social.telegram);
        if (c->social.instagram[0]) printf("     Instagram: %s\n", c->social.instagram);
    }
    puts("─");
}

// показать все контакты
void show_all(const PhoneBook *pb) {
    if (pb->count == 0) {
        puts("Список контактов пуст.");
        return;
    }
    printf("\nКонтактов в книге: %d\n\n", pb->count);
    for (int i = 0; i < pb->count; i++)
        print_contact(&pb->contacts[i]);
}

// поиск по имени или фамилии (без учета регистра)
static int contains(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    if (!*needle) return 1;

#ifdef _WIN32
    UINT cp = CP_UTF8;
    int wh = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, haystack, -1, NULL, 0);
    int wn = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, needle, -1, NULL, 0);
    if (wh <= 0 || wn <= 0) {
        cp = CP_ACP;
        wh = MultiByteToWideChar(cp, 0, haystack, -1, NULL, 0);
        wn = MultiByteToWideChar(cp, 0, needle, -1, NULL, 0);
        if (wh <= 0 || wn <= 0) return 0;
    }

    wchar_t *wH = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)wh);
    wchar_t *wN = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)wn);
    if (!wH || !wN) {
        free(wH);
        free(wN);
        return 0;
    }

    MultiByteToWideChar(cp, 0, haystack, -1, wH, wh);
    MultiByteToWideChar(cp, 0, needle, -1, wN, wn);

    int lowH = LCMapStringW(LOCALE_INVARIANT, LCMAP_LOWERCASE, wH, -1, NULL, 0);
    int lowN = LCMapStringW(LOCALE_INVARIANT, LCMAP_LOWERCASE, wN, -1, NULL, 0);
    if (lowH <= 0 || lowN <= 0) {
        free(wH);
        free(wN);
        return 0;
    }

    wchar_t *lH = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)lowH);
    wchar_t *lN = (wchar_t *)malloc(sizeof(wchar_t) * (size_t)lowN);
    if (!lH || !lN) {
        free(wH);
        free(wN);
        free(lH);
        free(lN);
        return 0;
    }

    LCMapStringW(LOCALE_INVARIANT, LCMAP_LOWERCASE, wH, -1, lH, lowH);
    LCMapStringW(LOCALE_INVARIANT, LCMAP_LOWERCASE, wN, -1, lN, lowN);

    int res = (wcsstr(lH, lN) != NULL);

    free(wH);
    free(wN);
    free(lH);
    free(lN);
    return res;
#else
    // fallback для ascii
    char hbuf[256];
    char nbuf[256];
    strncpy(hbuf, haystack, sizeof(hbuf) - 1);
    hbuf[sizeof(hbuf) - 1] = '\0';
    strncpy(nbuf, needle, sizeof(nbuf) - 1);
    nbuf[sizeof(nbuf) - 1] = '\0';
    for (char *p = hbuf; *p; ++p) {
        unsigned char ch = (unsigned char)*p;
        if (ch >= 'A' && ch <= 'Z') *p = (char)(ch - 'A' + 'a');
    }
    for (char *p = nbuf; *p; ++p) {
        unsigned char ch = (unsigned char)*p;
        if (ch >= 'A' && ch <= 'Z') *p = (char)(ch - 'A' + 'a');
    }
    return strstr(hbuf, nbuf) != NULL;
#endif
}

int find_contacts(const PhoneBook *pb, const char *query, int *results, int max_results) {
    if (!is_non_empty(query)) return 0;

    int found = 0;
    for (int i = 0; i < pb->count && found < max_results; i++) {
        if (contains(pb->contacts[i].last_name, query) ||
            contains(pb->contacts[i].first_name, query))
            results[found++] = i;
    }
    return found;
}

// поиск и печать результата
void search_and_show(const PhoneBook *pb) {
    char query[100];
    puts("Поиск (фамилия или имя):");
    read_line(query, sizeof query, "");
    if (!is_non_empty(query)) {
        puts("Поиск отменён.");
        return;
    }

    int indices[12];
    int count = find_contacts(pb, query, indices, 12);
    if (count == 0) {
        puts("Совпадений не найдено.");
        return;
    }

    printf("\nНайдено %d совпадение(ий):\n", count);
    for (int i = 0; i < count; i++)
        print_contact(&pb->contacts[indices[i]]);
}

void edit_contact(PhoneBook *pb) {
    char query[100];
    puts("Поиск контакта для редактирования: ");
    read_line(query, sizeof query, "");
    if (!is_non_empty(query)) return;

    int indices[8];
    int found = find_contacts(pb, query, indices, 8);
    if (found == 0) {
        puts("Контакт не найден.");
        return;
    }

    printf("\nНайдено %d:\n", found);
    for (int i = 0; i < found; i++) {
        const Contact *c = &pb->contacts[indices[i]];
        printf("  %d. %s %s %s\n", i + 1,
               c->last_name,
               c->first_name,
               c->patronymic[0] ? c->patronymic : "");
    }

    int selected = 0;
    if (found > 1) {
        char buf[32];
        printf("\nВыберите номер (1–%d): ", found);
        read_line(buf, sizeof buf, "");
        if (sscanf(buf, "%d", &selected) != 1 || selected < 1 || selected > found) {
            puts("Некорректный выбор.");
            return;
        }
    }
    selected--;

    Contact *c = &pb->contacts[indices[selected]];

    puts("\nРедактирование (Enter = оставить без изменений):");
    char buf[128];

#define UPDATE_FIELD(fld) \
    read_line(buf, sizeof buf, #fld ": "); \
    if (is_non_empty(buf)) strncpy(c->fld, buf, sizeof(c->fld)-1)

#define UPDATE_FIELD_CHECKED(fld) \
    read_line_checked(buf, sizeof buf, #fld ": "); \
    if (is_non_empty(buf)) strncpy(c->fld, buf, sizeof(c->fld)-1)

    UPDATE_FIELD(last_name);
    UPDATE_FIELD(first_name);
    UPDATE_FIELD(patronymic);
    UPDATE_FIELD_CHECKED(company);
    UPDATE_FIELD_CHECKED(position);

    puts("\nНовые номера телефонов (Enter = закончить):");
    c->phone_count = 0;
    for (int i = 0; i < MAX_PHONE; i++) {
        char code[16];
        char number[32];

        read_line(code, sizeof code, "  Код страны (Enter = закончить): ");
        if (!is_non_empty(code)) break;
        read_required(number, sizeof number, "  Номер: ");

        strncpy(c->phones[i].code, code, sizeof(c->phones[i].code) - 1);
        c->phones[i].code[sizeof(c->phones[i].code) - 1] = '\0';
        strncpy(c->phones[i].number, number, sizeof(c->phones[i].number) - 1);
        c->phones[i].number[sizeof(c->phones[i].number) - 1] = '\0';
        c->phone_count++;
    }

    puts("Контакт обновлён.");
}

void delete_contact(PhoneBook *pb) {
    char query[100];
    puts("Поиск контакта для удаления: ");
    read_line(query, sizeof query, "");
    if (!is_non_empty(query)) return;

    int indices[8];
    int found = find_contacts(pb, query, indices, 8);
    if (found == 0) {
        puts("Контакт не найден.");
        return;
    }

    printf("\nНайдено %d:\n", found);
    for (int i = 0; i < found; i++) {
        const Contact *c = &pb->contacts[indices[i]];
        printf("  %d. %s %s %s\n", i + 1,
               c->last_name,
               c->first_name,
               c->patronymic[0] ? c->patronymic : "");
    }

    char buf[32];
    printf("\nУдалить № (1–%d, 0 = отмена): ", found);
    read_line(buf, sizeof buf, "");
    int num;
    if (sscanf(buf, "%d", &num) != 1 || num == 0) {
        puts("Отмена.");
        return;
    }
    if (num < 1 || num > found) {
        puts("Неверный номер.");
        return;
    }

    int pos = indices[num - 1];
    memmove(&pb->contacts[pos], &pb->contacts[pos + 1],
            (pb->count - pos - 1) * sizeof(Contact));
    pb->count--;
    puts("Контакт удалён.");
}