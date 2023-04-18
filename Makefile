CC=gcc
CFLAGS=-Wall -Wextra -g 
ENDFLAGS=-lm 
SRC=main.c parse.c set_sock.c sigfunction.c
OBJ=$(SRC:.c=.o)
EXE=ft_ping

all: $(EXE)

$(EXE) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -lm -o $@

clean: 
	rm $(OBJ)

fclean: clean
	rm $(EXE)

re : fclean all