#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>

// NOTE: should be able to trim MAX_LIFE down to 5 and run fine
# define MAX_LIFE 5
# define EAT_T 1
# define REST_T 1
# define THINK_T 1
# define TIMEOUT 15

enum e_state {
	none,
	eat,
	rest,
	think,
	dead,
	dance,
};

int running;

typedef struct s_stick {
	pthread_mutex_t *m;
	int				holder;
}				t_stick;

typedef struct s_philo {
	int				id;
	t_stick *left;
	t_stick *right;
	t_stick *left_hand;
	t_stick *right_hand;
	enum e_state	state;
	unsigned		life;

	// GUI stuff
	float			x;
	float			y;
} t_philo;

int	display_visu(int num, t_philo *philos, t_stick *sticks);

#endif
