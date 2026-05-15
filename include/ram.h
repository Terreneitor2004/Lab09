#ifndef RAM_H
#define RAM_H

// Estado posible de un frame físico.
typedef enum {
    FRAME_FREE,
    FRAME_OCCUPIED
} FrameState;

// Representa un frame físico de RAM.
typedef struct {
    FrameState state;
    int owner_pid;
    int owner_vpn;
} PhysicalFrame;

// Cuenta los frames libres de la RAM.
unsigned count_free_frames(PhysicalFrame ram[]);

// Marca toda la RAM como libre.
void clear_ram(PhysicalFrame ram[]);

// Inicializa RAM con frames ocupados aleatoriamente.
void randomize_ram(PhysicalFrame ram[], unsigned total_virtual_pages_needed);

// Imprime el mapa básico de RAM.
void print_ram_map_basic(PhysicalFrame ram[], unsigned seed);

// Imprime el mapa 2-D con color.
void print_ram_map_2d_color(PhysicalFrame ram[], const char *title);

// Asigna un frame libre y devuelve su PFN.
int allocate_frame(PhysicalFrame ram[], int owner_pid, int owner_vpn);

#endif
