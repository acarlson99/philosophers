FLAGS = -Wall -Wextra -Werror
CFLAGS = $(FLAGS) -I ~/.brew/include/ -D_THREAD_SAFE 
LDFLAGS = $(FLAGS) -lpthread -lSDL2 -lm

CFILES = philo.c visu.c
OFILES = $(addprefix cmd/philo_c/, $(CFILES:.c=.o))
CNAME = philo_c

GONAME = philo_go

all: $(CNAME) $(GONAME)

$(CNAME): $(OFILES)
	$(CC) $(LDFLAGS) -o $(CNAME) $(OFILES)

$(GONAME):
	go build -o $(GONAME) ./cmd/philo_go/

clean:
	rm -f $(OFILES)

fclean: clean
	rm -f $(CNAME) $(GONAME) Go/philosopher
	rm -rf *.dSYM

re: fclean all

dbg: FLAGS = -Wall -Wextra -g -DDEBUG
dbg: re
