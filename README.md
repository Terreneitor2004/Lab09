# Lab009 - Virtual Address Translation and Page Tables

## 1. ¿De qué trata este laboratorio?

Este laboratorio simula cómo un sistema operativo traduce direcciones virtuales a direcciones físicas usando paginación.

El programa trabaja con:

- Una RAM física simulada de 100 frames.
- Páginas virtuales de 256 bytes.
- Frames físicos de 256 bytes.
- Una tabla de páginas por proceso.
- Un archivo de direcciones virtuales que se traducen una por una.

La idea principal es entender cómo una dirección virtual se divide en dos partes:

```text
[ VPN ][ OFFSET ]
```

Donde:

```text
VPN    = número de página virtual
OFFSET = posición dentro de la página
```

---

## 2. Objetivo del programa

El programa debe:

1. Leer direcciones virtuales desde un archivo.
2. Revisar que cada dirección esté dentro del rango permitido.
3. Separar cada dirección en VPN y offset.
4. Usar la tabla de páginas para encontrar el PFN.
5. Calcular la dirección física.
6. Mostrar errores claros cuando no se pueda traducir.
7. Simular RAM física con frames libres y ocupados.
8. Cargar uno o más procesos en memoria.
9. Mostrar un mapa visible de la RAM.
10. Implementar los extras: mapa 2-D/color y segundo proceso.

---

## 3. Formato de dirección virtual

El laboratorio usa direcciones virtuales de 16 bits.

La dirección se divide así:

```text
Bits [15:8] = VPN
Bits [7:0]  = OFFSET
```

Como el offset usa 8 bits:

```text
2^8 = 256 bytes
```

Por eso cada página mide 256 bytes.

En el código, esta separación se hace así:

```c
unsigned offset = va16 & 0xFF;
unsigned vpn = (va16 >> 8) & 0xFF;
```

Ejemplo:

```text
VA = 512
VA = 0x0200

VPN = 0x02
OFFSET = 0x00
```

---

## 4. Fórmula para traducir a dirección física

Cuando ya tenemos el VPN, buscamos su PFN en la tabla de páginas.

Luego calculamos:

```text
PA = PFN * PAGE_SIZE + OFFSET
```

Como `PAGE_SIZE = 256`, la fórmula queda:

```text
PA = PFN * 256 + OFFSET
```

Ejemplo:

```text
PFN = 4
OFFSET = 255

PA = 4 * 256 + 255
PA = 1279
```

---

## 5. Estructura del proyecto

El proyecto está organizado así:

```text
Lab009/
├── Makefile
├── README.md
├── example_addresses.txt
├── bin/
├── include/
│   ├── config.h
│   ├── parser.h
│   ├── process.h
│   ├── ram.h
│   └── translator.h
└── src/
    ├── main.c
    ├── parser.c
    ├── process.c
    ├── ram.c
    └── translator.c
```

---

## 6. ¿Qué hace cada archivo?

### `include/config.h`

Contiene las constantes principales del programa.

Aquí se definen valores como:

```c
#define NUM_FRAMES 100
#define PAGE_SIZE 256
#define MAX_VIRTUAL_PAGES 256
```

También se definen los símbolos usados en el mapa de RAM:

```c
#define PROCESS1_SYMBOL 'A'
#define PROCESS2_SYMBOL 'B'
#define SYSTEM_SYMBOL 'X'
#define FREE_SYMBOL 'F'
```

Significado:

```text
F = frame libre
X = frame ocupado antes de cargar procesos
A = frame usado por proceso A
B = frame usado por proceso B
```

---

### `src/parser.c`

Se encarga de leer números desde texto.

Este archivo se usa para leer:

- V1
- V2
- seed
- direcciones virtuales del archivo

La función principal es:

```c
parse_unsigned_number()
```

Acepta números en decimal:

```text
256
```

Y también en hexadecimal:

```text
0x0100
```

La línea clave es:

```c
unsigned int result = (unsigned int)strtoul(text, &end, 0);
```

El `0` al final permite que C detecte automáticamente si el número está en decimal o hexadecimal.

---

### `src/ram.c`

Este archivo maneja la RAM física simulada.

Contiene funciones para:

- Limpiar la RAM.
- Contar frames libres.
- Inicializar frames ocupados aleatoriamente.
- Imprimir el mapa de RAM.
- Asignar frames libres a procesos.

Funciones importantes:

```c
clear_ram()
count_free_frames()
randomize_ram()
print_ram_map_basic()
print_ram_map_2d_color()
allocate_frame()
```

La función más importante para asignar memoria es:

```c
allocate_frame()
```

Esta función busca el primer frame libre, lo marca como ocupado y devuelve su índice.

Ese índice es el PFN.

Ejemplo conceptual:

```text
Frame 3 está libre.
Proceso A necesita cargar VPN 0.

Entonces:
VPN 0 -> PFN 3
```

---

### `include/ram.h`

Define la estructura de un frame físico:

```c
typedef struct {
    FrameState state;
    int owner_pid;
    int owner_vpn;
} PhysicalFrame;
```

Cada frame guarda:

```text
state      indica si está libre u ocupado
owner_pid  indica qué proceso lo ocupa
owner_vpn  indica qué VPN usa ese frame
```

---

### `src/process.c`

Este archivo maneja los procesos y sus tablas de páginas.

Funciones principales:

```c
init_process()
load_process()
print_process_page_table()
```

La función:

```c
init_process()
```

Inicializa un proceso y pone todas sus entradas de tabla como inválidas.

La función:

```c
load_process()
```

Carga un proceso en RAM. Para cada VPN del proceso, pide un frame libre y guarda el PFN en la tabla de páginas.

La parte más importante es:

```c
for (unsigned vpn = 0; vpn < process->virtual_pages; vpn++) {
    int pfn = allocate_frame(ram, process->pid, (int)vpn);

    process->page_table[vpn].valid = true;
    process->page_table[vpn].pfn = pfn;
}
```

Eso significa:

```text
VPN 0 -> PFN asignado
VPN 1 -> PFN asignado
VPN 2 -> PFN asignado
...
```

---

### `include/process.h`

Define una entrada de la tabla de páginas:

```c
typedef struct {
    bool valid;
    int pfn;
} PageTableEntry;
```

Cada entrada tiene:

```text
valid = indica si la página está mapeada
pfn   = indica el frame físico asignado
```

También define la estructura del proceso:

```c
typedef struct {
    int pid;
    char symbol;
    unsigned virtual_pages;
    PageTableEntry page_table[MAX_VIRTUAL_PAGES];
} Process;
```

Eso significa que cada proceso tiene su propia tabla de páginas.

---

### `src/translator.c`

Este archivo hace la traducción de direcciones virtuales a direcciones físicas.

Funciones principales:

```c
translate_address_for_process()
translate_file_for_process()
```

La función:

```c
translate_address_for_process()
```

Traduce una sola dirección virtual.

Primero revisa si la dirección cabe en 16 bits:

```c
if (virtual_address > 0xFFFF)
```

Si la dirección es mayor, imprime:

```text
VA_OUT_OF_RANGE
```

Después separa VPN y offset:

```c
unsigned offset = va16 & 0xFF;
unsigned vpn = (va16 >> 8) & 0xFF;
```

Luego revisa si el VPN existe dentro del proceso:

```c
if (vpn >= process->virtual_pages)
```

Si el VPN no existe, imprime:

```text
VPN_OUT_OF_RANGE
```

Después busca el PFN:

```c
int pfn = process->page_table[vpn].pfn;
```

Finalmente calcula la dirección física:

```c
unsigned physical_address = (unsigned)(pfn * PAGE_SIZE + offset);
```

---

### `src/main.c`

Es el archivo principal.

Aquí se conectan todas las partes del programa.

El orden general es:

1. Leer argumentos desde la terminal.
2. Leer V1 y V2.
3. Leer el archivo de direcciones.
4. Leer la seed.
5. Crear la RAM simulada.
6. Crear proceso A.
7. Crear proceso B.
8. Inicializar la RAM aleatoria.
9. Imprimir el mapa de RAM.
10. Cargar proceso A.
11. Cargar proceso B.
12. Imprimir las tablas de páginas.
13. Imprimir el mapa 2-D con color.
14. Traducir direcciones para el proceso A.
15. Traducir direcciones para el proceso B.

Partes importantes:

```c
randomize_ram(ram, v1 + v2);
```

Inicializa la RAM con frames ocupados aleatoriamente.

```c
load_process(&process_a, ram);
load_process(&process_b, ram);
```

Carga ambos procesos en la misma RAM física.

```c
translate_file_for_process(&process_a, address_file);
translate_file_for_process(&process_b, address_file);
```

Traduce las direcciones usando la tabla de páginas de cada proceso.

---

## 7. Tareas del laboratorio y dónde están

### Task 1 - Virtual address format

Está en:

```text
src/translator.c
```

Función:

```c
translate_address_for_process()
```

Hace:

```text
1. Verifica que la VA esté entre 0x0000 y 0xFFFF.
2. Extrae el VPN.
3. Extrae el offset.
```

Código clave:

```c
if (virtual_address > 0xFFFF)

unsigned offset = va16 & 0xFF;
unsigned vpn = (va16 >> 8) & 0xFF;
```

---

### Task 2 - Parse and display

Está en:

```text
src/parser.c
src/translator.c
```

En `parser.c` se leen las direcciones.

En `translator.c` se imprimen:

```text
VA
VPN
OFF
PFN
PA
```

Código clave:

```c
parse_unsigned_number()
```

Y:

```c
printf("P%c VA=0x%04X (%u) VPN=0x%02X OFF=0x%02X PFN=%d PA=%u\n",
       process->symbol,
       virtual_address,
       virtual_address,
       vpn,
       offset,
       pfn,
       physical_address);
```

---

### Task 3 - RAM, free frames, load y page table

Está en:

```text
src/ram.c
src/process.c
include/ram.h
include/process.h
```

Partes:

```text
RAM física              -> ram.c / ram.h
Inicialización aleatoria -> randomize_ram()
Allocator              -> allocate_frame()
Tabla de páginas       -> process.h
Cargar proceso         -> load_process()
```

Funciones clave:

```c
randomize_ram()
allocate_frame()
load_process()
```

---

### Task 4 - Batch translation

Está en:

```text
src/translator.c
```

Función:

```c
translate_file_for_process()
```

Hace:

```text
1. Abre el archivo de direcciones.
2. Lee línea por línea.
3. Convierte cada línea a número.
4. Traduce cada dirección.
```

Código clave:

```c
while (fgets(line, sizeof(line), file) != NULL)
```

Y:

```c
translate_address_for_process(process, virtual_address);
```

---

## 8. Extras implementados

### Extra 1 - Mapa 2-D o color RAM

Está en:

```text
src/ram.c
```

Función:

```c
print_ram_map_2d_color()
```

Muestra los 100 frames como una matriz de 10x10.

Leyenda:

```text
F = libre
X = ocupado previamente
A = proceso A
B = proceso B
```

---

### Extra 2 - Segundo proceso

Está en:

```text
src/main.c
src/process.c
include/process.h
```

El programa crea dos procesos:

```c
Process process_a;
Process process_b;
```

Luego los inicializa:

```c
init_process(&process_a, PROCESS1_ID, PROCESS1_SYMBOL, v1);
init_process(&process_b, PROCESS2_ID, PROCESS2_SYMBOL, v2);
```

Después los carga en la misma RAM:

```c
load_process(&process_a, ram);
load_process(&process_b, ram);
```

Cada proceso tiene su propia tabla de páginas.

Por eso la misma dirección virtual puede traducirse a diferentes direcciones físicas según el proceso.

---

## 9. Archivo de direcciones

El archivo usado para probar es:

```text
example_addresses.txt
```

Contiene:

```text
0
255
256
512
3840
70000
```

Resultados esperados lógicamente:

```text
0      -> VPN 0, offset 0
255    -> VPN 0, offset 255
256    -> VPN 1, offset 0
512    -> VPN 2, offset 0
3840   -> VPN 15, offset 0
70000  -> VA_OUT_OF_RANGE
```

Si el proceso tiene V = 8, entonces VPN 15 no es válido y debe dar:

```text
VPN_OUT_OF_RANGE
```

---

## 10. Cómo compilar en WSL

Desde la carpeta del proyecto:

```bash
cd Lab009
make
```

El ejecutable queda en:

```text
bin/Lab009
```

Los archivos `.o` también quedan en:

```text
bin/
```

Ejemplo:

```text
bin/main.o
bin/parser.o
bin/ram.o
bin/process.o
bin/translator.o
bin/Lab009
```

---

## 11. Cómo ejecutar

Ejemplo recomendado:

```bash
./bin/Lab009 8 5 example_addresses.txt 12345
```

Significado:

```text
8                       páginas virtuales del proceso A
5                       páginas virtuales del proceso B
example_addresses.txt   archivo de direcciones
12345                   seed para repetir el mismo mapa de RAM
```

También se puede ejecutar con:

```bash
make run
```

---

## 12. Cómo limpiar archivos compilados

```bash
make clean
```

Esto borra:

```text
bin/Lab009
bin/*.o
```

---

## 13. Cómo explicar el laboratorio

Este programa simula la traducción de direcciones virtuales a direcciones físicas usando paginación. Primero crea una RAM física de 100 frames, donde algunos frames ya están ocupados aleatoriamente. Luego crea dos procesos, A y B, cada uno con su propia tabla de páginas. Al cargar cada proceso, el programa asigna frames libres a sus páginas virtuales. Después lee direcciones virtuales desde un archivo, separa cada dirección en VPN y offset, consulta la tabla de páginas del proceso y calcula la dirección física usando la fórmula `PA = PFN * 256 + offset`. Como extras, el programa muestra la RAM en formato 2-D con colores y permite comparar cómo dos procesos distintos pueden traducir la misma dirección virtual a diferentes direcciones físicas.
