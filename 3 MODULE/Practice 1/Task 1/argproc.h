#ifndef ARGPROC_H
#define ARGPROC_H

/**
 * Обработка одного аргумента командной строки:
 * если строка — целое или вещественное число, печатает значение и удвоенное значение;
 * иначе печатает строку как есть.
 */
void process_argument(const char *arg);

/**
 * Обрабатывает argv[start] .. argv[end] включительно (индексы в массиве argv).
 */
void process_argument_range(char **argv, int start, int end);

#endif
