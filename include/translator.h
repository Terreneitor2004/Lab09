#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "process.h"

// Traduce una dirección virtual para un proceso.
void translate_address_for_process(const Process *process,
                                   unsigned int virtual_address);

// Traduce todas las direcciones de un archivo para un proceso.
void translate_file_for_process(const Process *process,
                                const char *filename);

#endif
