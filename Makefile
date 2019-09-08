FLAGS = -Wall -Wextra -Werror
CFLAGS = $(FLAGS) -I ~/.brew/include/ -D_THREAD_SAFE 
LDFLAGS = $(FLAGS) -lpthread -L ~/.brew/lib/ -lSDL2

CFILES = philo.c
OFILES = $(CFILES:.c=.o)
NAME = philo

all: $(NAME)

$(NAME): $(OFILES)

clean:
	rm -f $(OFILES)

fclean: clean
	rm -f $(BIN) $(NAME)

re: fclean all

dbg: FLAGS = -Wall -Wextra -g
dbg: re
