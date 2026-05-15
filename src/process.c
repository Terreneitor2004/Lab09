#include "process.h"

#include <stdio.h>

void init_process(Process *process,
                  int pid,
                  char symbol,
                  unsigned virtual_pages) {
    // Guardamos el identificador del proceso.
    process->pid = pid;

    // Guardamos el símbolo visual del proceso.
    process->symbol = symbol;

    // Guardamos la cantidad de páginas virtuales.
    process->virtual_pages = virtual_pages;

    // Inicializamos todas las entradas de la tabla.
    for (int i = 0; i < MAX_VIRTUAL_PAGES; i++) {
        // Marcamos la entrada como inválida.
        process->page_table[i].valid = false;

        // Dejamos el PFN en -1 porque todavía no hay frame asignado.
        process->page_table[i].pfn = -1;
    }
}

bool load_process(Process *process,
                  PhysicalFrame ram[]) {
    // Guardamos los frames asignados por si hay que deshacer la carga.
    int allocated_frames[MAX_VIRTUAL_PAGES];

    // Inicializamos la lista de frames asignados.
    for (unsigned i = 0; i < process->virtual_pages; i++) {
        allocated_frames[i] = -1;
    }

    // Recorremos cada VPN válida del proceso.
    for (unsigned vpn = 0; vpn < process->virtual_pages; vpn++) {
        // Pedimos un frame libre para esta VPN.
        int pfn = allocate_frame(ram, process->pid, (int)vpn);

        // Revisamos si la asignación falló.
        if (pfn == -1) {
            // Mostramos dónde falló la carga.
            fprintf(stderr,
                    "ERROR: fallo de asignacion al cargar proceso %c, VPN %u.\n",
                    process->symbol, vpn);

            // Liberamos los frames que este proceso ya había tomado.
            for (unsigned j = 0; j < vpn; j++) {
                // Revisamos si había un frame asignado.
                if (allocated_frames[j] != -1) {
                    ram[allocated_frames[j]].state = FRAME_FREE;
                    ram[allocated_frames[j]].owner_pid = -1;
                    ram[allocated_frames[j]].owner_vpn = -1;
                }

                // Invalidamos la entrada de la tabla.
                process->page_table[j].valid = false;

                // Quitamos el PFN asignado.
                process->page_table[j].pfn = -1;
            }

            // Indicamos que la carga falló.
            return false;
        }

        // Guardamos el PFN asignado.
        allocated_frames[vpn] = pfn;

        // Marcamos la VPN como válida.
        process->page_table[vpn].valid = true;

        // Guardamos el PFN en la tabla de páginas.
        process->page_table[vpn].pfn = pfn;
    }

    // Indicamos que el proceso cargó bien.
    return true;
}

void print_process_page_table(const Process *process) {
    // Imprimimos el resumen de la tabla.
    printf("\nProcess %c page table: V=%u -> VPN 0..%u mapped to PFNs [",
           process->symbol,
           process->virtual_pages,
           process->virtual_pages - 1);

    // Imprimimos los PFN asignados.
    for (unsigned vpn = 0; vpn < process->virtual_pages; vpn++) {
        // Imprimimos el PFN de la VPN actual.
        printf("%d", process->page_table[vpn].pfn);

        // Agregamos coma si no es el último.
        if (vpn + 1 < process->virtual_pages) {
            printf(", ");
        }
    }

    // Cerramos el resumen.
    printf("]\n");

    // Imprimimos una línea por cada VPN.
    for (unsigned vpn = 0; vpn < process->virtual_pages; vpn++) {
        // Mostramos la relación VPN -> PFN.
        printf("  VPN=%u -> PFN=%d\n",
               vpn,
               process->page_table[vpn].pfn);
    }
}
