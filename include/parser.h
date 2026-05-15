#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

// Convierte texto decimal o hexadecimal a número sin signo.
bool parse_unsigned_number(const char *text, unsigned int *value);

#endif
