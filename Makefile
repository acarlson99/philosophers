FLAGS = -Wall -Wextra -Werror
CFLAGS = $(FLAGS) -I inc/
LDFLAGS = $(FLAGS) -lpthread

SRC = src/
CFILES = main.c
CSRCS = $(addprefix $(SRC), $(CFILES))
OFILES = $(CSRCS:.c=.o)
NAME = philo

all: $(NAME)

$(NAME): $(OFILES)
	$(CC) $(LDFLAGS) $(OFILES) -o $(NAME)

clean:
	rm -f $(OFILES)

fclean: clean
	rm -f $(BIN) $(NAME)

re: fclean all

dbg: FLAGS = -Wall -Wextra -g
dbg: re
