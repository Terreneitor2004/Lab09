#include "translator.h"
#include "config.h"
#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void translate_address_for_process(const Process *process,
                                   unsigned int virtual_address) {
    // Rechazamos direcciones mayores a 16 bits.
    if (virtual_address > 0xFFFF) {
        printf("P%c VA=%u ERROR=VA_OUT_OF_RANGE\n",
               process->symbol,
               virtual_address);
        return;
    }

    // Convertimos la dirección a 16 bits.
    uint16_t va16 = (uint16_t)virtual_address;

    // Extraemos el offset con los 8 bits bajos.
    unsigned offset = va16 & 0xFF;

    // Extraemos el VPN con los 8 bits altos.
    unsigned vpn = (va16 >> 8) & 0xFF;

    // Revisamos si el VPN existe para este proceso.
    if (vpn >= process->virtual_pages) {
        printf("P%c VA=%u (0x%04X) ERROR=VPN_OUT_OF_RANGE (vpn=%u, V=%u)\n",
               process->symbol,
               virtual_address,
               virtual_address,
               vpn,
               process->virtual_pages);
        return;
    }

    // Revisamos si la página está mapeada.
    if (!process->page_table[vpn].valid) {
        printf("P%c VA=0x%04X (%u) VPN=0x%02X OFF=0x%02X ERROR=PAGE_NOT_MAPPED\n",
               process->symbol,
               virtual_address,
               virtual_address,
               vpn,
               offset);
        return;
    }

    // Obtenemos el PFN desde la tabla de páginas.
    int pfn = process->page_table[vpn].pfn;

    // Calculamos la dirección física.
    unsigned physical_address = (unsigned)(pfn * PAGE_SIZE + offset);

    // Imprimimos la traducción completa.
    printf("P%c VA=0x%04X (%u) VPN=0x%02X OFF=0x%02X PFN=%d PA=%u\n",
           process->symbol,
           virtual_address,
           virtual_address,
           vpn,
           offset,
           pfn,
           physical_address);
}

void translate_file_for_process(const Process *process,
                                const char *filename) {
    // Abrimos el archivo de direcciones.
    FILE *file = fopen(filename, "r");

    // Revisamos si el archivo abrió correctamente.
    if (file == NULL) {
        fprintf(stderr, "ERROR: no se pudo abrir el archivo: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Reservamos espacio para leer una línea.
    char line[256];

    // Leemos el archivo línea por línea.
    while (fgets(line, sizeof(line), file) != NULL) {
        // Variable para guardar la dirección leída.
        unsigned int virtual_address = 0;

        // Convertimos la línea a número.
        if (parse_unsigned_number(line, &virtual_address)) {
            // Traducimos la dirección si la línea era válida.
            translate_address_for_process(process, virtual_address);
        }
    }

    // Cerramos el archivo.
    fclose(file);
}
