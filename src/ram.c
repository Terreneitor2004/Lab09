#include "ram.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

// Reinicia el color de la terminal.
#define ANSI_RESET "\033[0m"

// Color para frames libres.
#define ANSI_GREEN "\033[32m"

// Color para frames ocupados previamente.
#define ANSI_RED "\033[31m"

// Color para frames del proceso A.
#define ANSI_BLUE "\033[34m"

// Color para frames del proceso B.
#define ANSI_MAGENTA "\033[35m"

static char frame_symbol(PhysicalFrame frame) {
    // Si el frame está libre, mostramos F.
    if (frame.state == FRAME_FREE) {
        return FREE_SYMBOL;
    }

    // Si pertenece al proceso A, mostramos A.
    if (frame.owner_pid == PROCESS1_ID) {
        return PROCESS1_SYMBOL;
    }

    // Si pertenece al proceso B, mostramos B.
    if (frame.owner_pid == PROCESS2_ID) {
        return PROCESS2_SYMBOL;
    }

    // Si no pertenece a A ni B, estaba ocupado antes.
    return SYSTEM_SYMBOL;
}

static const char *frame_color(PhysicalFrame frame) {
    // Los frames libres van en verde.
    if (frame.state == FRAME_FREE) {
        return ANSI_GREEN;
    }

    // Los frames del proceso A van en azul.
    if (frame.owner_pid == PROCESS1_ID) {
        return ANSI_BLUE;
    }

    // Los frames del proceso B van en morado.
    if (frame.owner_pid == PROCESS2_ID) {
        return ANSI_MAGENTA;
    }

    // Los frames ocupados previamente van en rojo.
    return ANSI_RED;
}

unsigned count_free_frames(PhysicalFrame ram[]) {
    // Empezamos el contador en cero.
    unsigned free_count = 0;

    // Recorremos todos los frames.
    for (int i = 0; i < NUM_FRAMES; i++) {
        // Sumamos uno si el frame está libre.
        if (ram[i].state == FRAME_FREE) {
            free_count++;
        }
    }

    // Devolvemos la cantidad de frames libres.
    return free_count;
}

void clear_ram(PhysicalFrame ram[]) {
    // Recorremos todos los frames de RAM.
    for (int i = 0; i < NUM_FRAMES; i++) {
        // Marcamos el frame como libre.
        ram[i].state = FRAME_FREE;

        // Quitamos cualquier dueño anterior.
        ram[i].owner_pid = -1;

        // Quitamos cualquier VPN anterior.
        ram[i].owner_vpn = -1;
    }
}

void randomize_ram(PhysicalFrame ram[], unsigned total_virtual_pages_needed) {
    // Calculamos el mínimo de frames libres necesarios.
    unsigned min_free_needed =
        (total_virtual_pages_needed > 10) ? total_virtual_pages_needed : 10;

    // Calculamos el máximo de frames que podemos ocupar.
    int max_occupied_allowed = NUM_FRAMES - (int)min_free_needed;

    // Revisamos si la regla de RAM se puede cumplir.
    if (max_occupied_allowed < RANDOM_OCCUPIED_MIN) {
        fprintf(stderr,
                "ERROR: no se puede cumplir la regla de RAM. "
                "Paginas totales necesarias=%u.\n",
                total_virtual_pages_needed);
        exit(EXIT_FAILURE);
    }

    // Empezamos usando el máximo permitido por el lab.
    int effective_max_occupied = RANDOM_OCCUPIED_MAX;

    // Reducimos el máximo si hace falta dejar más frames libres.
    if (effective_max_occupied > max_occupied_allowed) {
        effective_max_occupied = max_occupied_allowed;
    }

    // Intentamos generar un mapa válido varias veces.
    for (int attempt = 1; attempt <= RANDOM_ATTEMPTS_LIMIT; attempt++) {
        // Reiniciamos toda la RAM como libre.
        clear_ram(ram);

        // Calculamos el rango de ocupados posibles.
        int range = effective_max_occupied - RANDOM_OCCUPIED_MIN + 1;

        // Elegimos cuántos frames estarán ocupados.
        int occupied_target = RANDOM_OCCUPIED_MIN + (rand() % range);

        // Inicializamos el contador de frames ocupados.
        int occupied = 0;

        // Ocupamos frames aleatorios hasta llegar al objetivo.
        while (occupied < occupied_target) {
            // Elegimos un índice de frame al azar.
            int frame = rand() % NUM_FRAMES;

            // Solo ocupamos el frame si estaba libre.
            if (ram[frame].state == FRAME_FREE) {
                ram[frame].state = FRAME_OCCUPIED;
                ram[frame].owner_pid = 0;
                ram[frame].owner_vpn = -1;
                occupied++;
            }
        }

        // Terminamos si quedaron suficientes frames libres.
        if (count_free_frames(ram) >= min_free_needed) {
            return;
        }
    }

    // Mostramos error si no se pudo generar RAM válida.
    fprintf(stderr, "ERROR: no se pudo generar RAM valida.\n");
    exit(EXIT_FAILURE);
}

void print_ram_map_basic(PhysicalFrame ram[], unsigned seed) {
    // Contamos frames libres.
    unsigned free_count = count_free_frames(ram);

    // Calculamos frames ocupados.
    unsigned occupied_count = NUM_FRAMES - free_count;

    // Imprimimos el título del mapa.
    printf("PHYSICAL RAM (%d frames) after random init (seed=%u):\n",
           NUM_FRAMES, seed);

    // Imprimimos el resumen de RAM.
    printf("FREE=%u OCCUPIED=%u\n", free_count, occupied_count);

    // Imprimimos todos los frames.
    for (int i = 0; i < NUM_FRAMES; i++) {
        // Imprimimos índice y símbolo del frame.
        printf("%2d:%c", i, frame_symbol(ram[i]));

        // Cambiamos de línea cada 10 frames.
        if ((i + 1) % 10 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}

void print_ram_map_2d_color(PhysicalFrame ram[], const char *title) {
    // Contamos frames libres.
    unsigned free_count = count_free_frames(ram);

    // Calculamos frames ocupados.
    unsigned occupied_count = NUM_FRAMES - free_count;

    // Imprimimos el título del mapa.
    printf("\n%s\n", title);

    // Imprimimos el resumen de RAM.
    printf("FREE=%u OCCUPIED=%u\n", free_count, occupied_count);

    // Imprimimos la leyenda del mapa.
    printf("Legend: " ANSI_GREEN "F=free" ANSI_RESET " "
           ANSI_RED "X=system/pre-used" ANSI_RESET " "
           ANSI_BLUE "A=process 1" ANSI_RESET " "
           ANSI_MAGENTA "B=process 2" ANSI_RESET "\n\n");

    // Recorremos las 10 filas del mapa.
    for (int row = 0; row < 10; row++) {
        // Recorremos las 10 columnas del mapa.
        for (int col = 0; col < 10; col++) {
            // Convertimos fila y columna a índice de frame.
            int frame_index = row * 10 + col;

            // Obtenemos el símbolo del frame.
            char symbol = frame_symbol(ram[frame_index]);

            // Imprimimos frame con color.
            printf("%s%2d:%c%s ",
                   frame_color(ram[frame_index]),
                   frame_index,
                   symbol,
                   ANSI_RESET);
        }

        // Terminamos la fila actual.
        printf("\n");
    }
}

int allocate_frame(PhysicalFrame ram[], int owner_pid, int owner_vpn) {
    // Buscamos el primer frame libre.
    for (int i = 0; i < NUM_FRAMES; i++) {
        // Revisamos si el frame actual está libre.
        if (ram[i].state == FRAME_FREE) {
            // Marcamos el frame como ocupado.
            ram[i].state = FRAME_OCCUPIED;

            // Guardamos el proceso dueño.
            ram[i].owner_pid = owner_pid;

            // Guardamos la VPN dueña.
            ram[i].owner_vpn = owner_vpn;

            // Devolvemos el índice del frame como PFN.
            return i;
        }
    }

    // Devolvemos -1 si no hay frames libres.
    return -1;
}
