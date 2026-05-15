# Compilador usado.
CC = gcc

# Opciones de compilación.
CFLAGS = -std=c11 -Wall -Wextra -Iinclude

# Ejecutable final.
TARGET = bin/Lab009

# Archivos fuente.
SRC = src/main.c src/parser.c src/ram.c src/process.c src/translator.c

# Archivos objeto guardados en bin.
OBJ = bin/main.o bin/parser.o bin/ram.o bin/process.o bin/translator.o

# Regla principal.
all: $(TARGET)

# Enlaza los archivos objeto.
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compila main.c.
bin/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o bin/main.o

# Compila parser.c.
bin/parser.o: src/parser.c
	$(CC) $(CFLAGS) -c src/parser.c -o bin/parser.o

# Compila ram.c.
bin/ram.o: src/ram.c
	$(CC) $(CFLAGS) -c src/ram.c -o bin/ram.o

# Compila process.c.
bin/process.o: src/process.c
	$(CC) $(CFLAGS) -c src/process.c -o bin/process.o

# Compila translator.c.
bin/translator.o: src/translator.c
	$(CC) $(CFLAGS) -c src/translator.c -o bin/translator.o

# Compila y ejecuta una prueba.
run: all
	./$(TARGET) 8 5 example_addresses.txt 12345

# Borra ejecutable y archivos objeto.
clean:
	rm -f bin/Lab009 bin/*.o
