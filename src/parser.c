#include "parser.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

bool parse_unsigned_number(const char *text, unsigned int *value) {
    // Saltamos espacios al inicio de la línea.
    while (isspace((unsigned char)*text)) {
        text++;
    }
    // Ignoramos líneas vacías o comentarios.
    if (*text == '\0' || *text == '\n' || *text == '#') {
        return false;
    }

    // Rechazamos números negativos porque las direcciones son sin signo.
    if (*text == '-') {
        fprintf(stderr, "ERROR: direccion negativa no permitida: %s", text);
        return false;
    }
    // Reiniciamos errno antes de convertir.
    errno = 0;

    // Guardamos dónde termina la conversión.
    char *end = NULL;

    // Convertimos el texto usando base automática.
    unsigned int result = (unsigned int)strtoul(text, &end, 0);

    // Revisamos si la conversión falló.
    if (errno != 0 || end == text) {
        fprintf(stderr, "ERROR: no se pudo leer direccion: %s", text);
        return false;
    }

    // Saltamos espacios después del número.
    while (isspace((unsigned char)*end)) {
        end++;
    }

    // Rechazamos texto extra que no sea comentario.
    if (*end != '\0' && *end != '\n' && *end != '#') {
        fprintf(stderr, "ERROR: texto extra despues de la direccion: %s", text);
        return false;
    }

    // Guardamos el número convertido.
    *value = result;

    // Indicamos que sí se leyó un número.
    return true;
}
