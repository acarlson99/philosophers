FLAGS = -Wall -Wextra -Werror
CFLAGS = $(FLAGS) -I ~/.brew/include/ -D_THREAD_SAFE 
LDFLAGS = $(FLAGS) -lpthread -L ~/.brew/lib/ -lSDL2

CFILES = philo.c visu.c
OFILES = $(addprefix C/, $(CFILES:.c=.o))
CNAME = philo_c

GONAME = philo_go

all: $(CNAME) $(GONAME)

$(CNAME): $(OFILES)
	$(CC) $(LDFLAGS) -o $(CNAME) $(OFILES)

$(GONAME):
	cd Go/ && go build
	- ln -s Go/philosopher $(GONAME)

clean:
	rm -f $(OFILES)

fclean: clean
	rm -f $(CNAME) $(GONAME) Go/philosopher
	rm -rf *.dSYM

re: fclean all

dbg: FLAGS = -Wall -Wextra -g
dbg: re
