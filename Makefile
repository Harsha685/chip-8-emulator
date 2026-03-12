CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lSDL2

TARGET = chip8
SRC = main.c chip8.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

rebuild: 
	clean $(TARGET)