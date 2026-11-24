CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -g -MMD -MP
LDFLAGS = -lsqlite3
SRC     = main.c magasin.c article.c client.c vente.c db.c ui.c utils.c
OBJ     = $(SRC:.c=.o)
DEP     = $(OBJ:.o=.d)
BIN     = magasin

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

valgrind: $(BIN)
	valgrind --leak-check=full --show-leak-kinds=all ./$(BIN)

clean:
	rm -f $(OBJ) $(DEP) $(BIN)

.PHONY: all run valgrind clean

-include $(DEP)
