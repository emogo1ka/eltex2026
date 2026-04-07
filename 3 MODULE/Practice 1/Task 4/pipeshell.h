#ifndef PIPESHELL_H
#define PIPESHELL_H

#define MAX_LINE 1024

/**
 * Читает строки из stdin в цикле, разбирает конвейер (|), перенаправления < и >
 * для первой и последней команды соответственно, создаёт pipe/fork/exec.
 */
void pipeshell_loop(void);

#endif
