#ifndef CONFIG_H
#define CONFIG_H

// Cantidad total de frames físicos en la RAM simulada.
#define NUM_FRAMES 100

// Tamaño de cada página y frame en bytes.
#define PAGE_SIZE 256

// Cantidad máxima de páginas virtuales por el formato de 8 bits.
#define MAX_VIRTUAL_PAGES 256

// Mínimo de frames ocupados al inicializar RAM aleatoria.
#define RANDOM_OCCUPIED_MIN 10

// Máximo de frames ocupados al inicializar RAM aleatoria.
#define RANDOM_OCCUPIED_MAX 60

// Límite de intentos para generar una RAM válida.
#define RANDOM_ATTEMPTS_LIMIT 10000

// Identificador interno del proceso A.
#define PROCESS1_ID 1

// Identificador interno del proceso B.
#define PROCESS2_ID 2

// Símbolo usado para mostrar frames del proceso A.
#define PROCESS1_SYMBOL 'A'

// Símbolo usado para mostrar frames del proceso B.
#define PROCESS2_SYMBOL 'B'

// Símbolo usado para frames ocupados antes de cargar procesos.
#define SYSTEM_SYMBOL 'X'

// Símbolo usado para frames libres.
#define FREE_SYMBOL 'F'

#endif
