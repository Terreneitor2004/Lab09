#include "config.h"
#include "parser.h"
#include "process.h"
#include "ram.h"
#include "translator.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static unsigned parse_virtual_pages(const char *text, const char *name) {
    // Variable para guardar el número leído.
    unsigned int value = 0;

    // Convertimos el texto a número.
    if (!parse_unsigned_number(text, &value)) {
        fprintf(stderr, "ERROR: %s invalido.\n", name);
        exit(EXIT_FAILURE);
    }

    // Revisamos que V esté entre 1 y 256.
    if (value < 1 || value > MAX_VIRTUAL_PAGES) {
        fprintf(stderr, "ERROR: %s debe estar entre 1 y 256.\n", name);
        exit(EXIT_FAILURE);
    }

    // Revisamos que un proceso no pida más frames que la RAM total.
    if (value > NUM_FRAMES) {
        fprintf(stderr,
                "ERROR: %s=%u no puede cargarse completo con solo %d frames fisicos.\n",
                name,
                value,
                NUM_FRAMES);
        exit(EXIT_FAILURE);
    }

    // Devolvemos el valor validado.
    return (unsigned)value;
}

int main(int argc, char *argv[]) {
    // Revisamos que el usuario haya pasado los argumentos correctos.
    if (argc < 4 || argc > 5) {
        fprintf(stderr,
                "Uso: %s V1 V2 archivo_direcciones [seed]\n"
                "Ejemplo: %s 8 5 example_addresses.txt 12345\n",
                argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    // Leemos las páginas virtuales del proceso A.
    unsigned v1 = parse_virtual_pages(argv[1], "V1");

    // Leemos las páginas virtuales del proceso B.
    unsigned v2 = parse_virtual_pages(argv[2], "V2");

    // Guardamos el nombre del archivo de direcciones.
    const char *address_file = argv[3];

    // Revisamos que ambos procesos quepan en RAM.
    if (v1 + v2 > NUM_FRAMES) {
        fprintf(stderr,
                "ERROR: V1 + V2 = %u, pero solo hay %d frames fisicos.\n",
                v1 + v2,
                NUM_FRAMES);
        return EXIT_FAILURE;
    }

    // Creamos la variable para la seed.
    unsigned seed = 0;

    // Si el usuario escribió seed, la usamos.
    if (argc == 5) {
        // Variable temporal para leer la seed.
        unsigned int seed_value = 0;

        // Convertimos la seed a número.
        if (!parse_unsigned_number(argv[4], &seed_value)) {
            fprintf(stderr, "ERROR: seed invalida.\n");
            return EXIT_FAILURE;
        }

        // Guardamos la seed leída.
        seed = (unsigned)seed_value;
    } else {
        // Si no hay seed, usamos la hora actual.
        seed = (unsigned)time(NULL);
    }

    // Inicializamos los números aleatorios.
    srand(seed);

    // Creamos la RAM física simulada.
    PhysicalFrame ram[NUM_FRAMES];

    // Creamos el proceso A.
    Process process_a;

    // Creamos el proceso B.
    Process process_b;

    // Inicializamos el proceso A.
    init_process(&process_a, PROCESS1_ID, PROCESS1_SYMBOL, v1);

    // Inicializamos el proceso B.
    init_process(&process_b, PROCESS2_ID, PROCESS2_SYMBOL, v2);

    // Inicializamos la RAM con espacios ocupados aleatorios.
    randomize_ram(ram, v1 + v2);

    // Imprimimos el mapa básico de RAM.
    print_ram_map_basic(ram, seed);

    // Imprimimos el mapa 2-D antes de cargar procesos.
    print_ram_map_2d_color(ram, "2-D COLOR RAM MAP BEFORE LOADING PROCESSES");

    // Contamos los frames libres antes de cargar.
    unsigned free_before_load = count_free_frames(ram);

    // Revisamos que haya frames suficientes para ambos procesos.
    if (v1 + v2 > free_before_load) {
        fprintf(stderr,
                "ERROR: no hay suficientes frames libres. V1+V2=%u, FREE=%u.\n",
                v1 + v2,
                free_before_load);
        return EXIT_FAILURE;
    }

    // Cargamos el proceso A en RAM.
    if (!load_process(&process_a, ram)) {
        return EXIT_FAILURE;
    }

    // Cargamos el proceso B en la misma RAM.
    if (!load_process(&process_b, ram)) {
        return EXIT_FAILURE;
    }

    // Imprimimos
    print_process_page_table(&process_a);
    print_process_page_table(&process_b);
    print_ram_map_2d_color(ram, "2-D COLOR RAM MAP AFTER LOADING PROCESSES");
    // Traducimos las direcciones para el proceso A.
    printf("\nTRANSLATIONS FOR PROCESS %c\n", process_a.symbol);
    translate_file_for_process(&process_a, address_file);
    // Traducimos las direcciones para el proceso B.
    printf("\nTRANSLATIONS FOR PROCESS %c\n", process_b.symbol);
    translate_file_for_process(&process_b, address_file);
    // Terminamos correctamente.
    return EXIT_SUCCESS;
}
