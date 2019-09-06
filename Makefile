FLAGS = -Wall -Wextra -Werror
CFLAGS = $(FLAGS) -I inc/
LDFLAGS = $(FLAGS)

SRC = src/
CFILES = main.c
CSRCS = $(addprefix $(SRC), $(CFILES))
OFILES = $(CSRCS:.c=.o)
NAME = philo

all: $(NAME)

$(NAME): $(OFILES)
	$(CC) $(OFILES) -o $(NAME)

clean:
	rm -f $(OFILES)

fclean: clean
	rm -f $(BIN) $(NAME)

re: fclean all
