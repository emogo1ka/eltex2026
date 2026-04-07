#include "phonebook.h"
#include <stdio.h>

static int show_menu(void) {
    puts("\n------Телефонная книга------");
    puts(" 1. Добавить контакт");
    puts(" 2. Показать все контакты");
    puts(" 3. Найти контакт(ы)");
    puts(" 4. Редактировать контакт");
    puts(" 5. Удалить контакт");
    puts(" 0. Выход");
    printf("→ ");

    char buf[32];
    read_line(buf, sizeof buf, "");

    int choice = -1;
    sscanf(buf, "%d", &choice);
    return choice;
}

static void run_phonebook(PhoneBook *pb) {
    int choice;
    do {
        choice = show_menu();
        switch (choice) {
        case 1:
            add_contact(pb);
            break;
        case 2:
            show_all(pb);
            break;
        case 3:
            search_and_show(pb);
            break;
        case 4:
            edit_contact(pb);
            break;
        case 5:
            delete_contact(pb);
            break;
        case 0:
            puts("Выход");
            break;
        default:
            puts("Неверный выбор.");
        }
    } while (choice != 0);
}

int main(void) {
    PhoneBook pb;
    init_phonebook(&pb);
    pb_load(&pb, DEFAULT_DATA_FILE);
    run_phonebook(&pb);
    pb_save(&pb, DEFAULT_DATA_FILE);
    free_phonebook(&pb);
    return 0;
}
