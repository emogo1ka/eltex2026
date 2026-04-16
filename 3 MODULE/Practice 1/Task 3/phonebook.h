#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <stddef.h>
#include <stdio.h>

#define MAX_FIO       32
#define MAX_LINE      80
#define MAX_PHONE     4
#define MAX_EMAIL     5
#define INITIAL_CAP   8
#define DEFAULT_DATA_FILE "contacts.bin"

typedef struct {
    char code[8];
    char number[24];
} Phone;

typedef struct {
    char vk[48];
    char telegram[48];
    char instagram[48];
} SocialLinks;

typedef struct {
    int id;
    char first_name[MAX_FIO];
    char last_name[MAX_FIO];
    char patronymic[MAX_FIO];
    char company[MAX_LINE];
    char position[MAX_LINE];
    Phone phones[MAX_PHONE];
    int phone_count;
    char emails[MAX_EMAIL][64];
    int email_count;
    SocialLinks social;
} Contact;

typedef struct {
    Contact *contacts;
    int count;
    int capacity;
    int next_id;
} PhoneBook;

void init_phonebook(PhoneBook *pb);
void free_phonebook(PhoneBook *pb);

void pb_load(PhoneBook *pb, const char *path);

void pb_save(const PhoneBook *pb, const char *path);

void add_contact(PhoneBook *pb);
void show_all(const PhoneBook *pb);
void search_and_show(const PhoneBook *pb);
void edit_contact(PhoneBook *pb);
void delete_contact(PhoneBook *pb);

void read_line(char *buf, size_t size, const char *prompt);
int  is_non_empty(const char *s);
void read_required(char *buf, size_t size, const char *prompt);

#endif
