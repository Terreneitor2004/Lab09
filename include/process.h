#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

#include "config.h"
#include "ram.h"

// Representa una entrada de la tabla de páginas.
typedef struct {
    bool valid;
    int pfn;
} PageTableEntry;

// Representa un proceso con su propia tabla de páginas.
typedef struct {
    int pid;
    char symbol;
    unsigned virtual_pages;
    PageTableEntry page_table[MAX_VIRTUAL_PAGES];
} Process;

// Inicializa un proceso.
void init_process(Process *process,
                  int pid,
                  char symbol,
                  unsigned virtual_pages);

// Carga un proceso en la RAM física.
bool load_process(Process *process,
                  PhysicalFrame ram[]);

// Imprime la tabla de páginas del proceso.
void print_process_page_table(const Process *process);

#endif
