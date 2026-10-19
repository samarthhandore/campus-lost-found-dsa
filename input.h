#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

int input_line(const char *prompt, char *buffer, size_t capacity);
int input_int(const char *prompt, int *value);
int input_date(const char *prompt, char date[11]);

#endif
