#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "philo.h"

enum e_state {
	none,
	eat,
	rest,
	think,
	dead,
	dance,
};

int start;

typedef struct s_philo {
	int				id;
	pthread_mutex_t *left;
	pthread_mutex_t *right;
	pthread_mutex_t *left_hand;
	pthread_mutex_t *right_hand;
	enum e_state	state;
	unsigned		life;
} t_philo;

pthread_mutex_t lock;

void *philosopher(void *arg) {
	t_philo *philo = arg;

	while (!start);
	while (1) {
		pthread_mutex_lock(philo->left);
		pthread_mutex_lock(philo->right);

		printf("%d EATING\n", philo->id);
		sleep(EAT_T);

		pthread_mutex_unlock(philo->left);
		pthread_mutex_unlock(philo->right);

		printf("%d SLEEPING\n", philo->id);
		sleep(REST_T);

	}
	return (NULL);
}

int main(int argc, char **argv) {
	int		num;
	if (argc != 2)
		num = 7;
	else
		num = atoi(argv[1]);
	pthread_t ids[num];
	pthread_mutex_t mutexes[num];
	t_philo			philos[num];

	start = 0;

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_init(&mutexes[ii], NULL);

	printf("Begin\n");
	for (int ii = 0; ii < num; ++ii) {
		philos[ii] = (t_philo){ii, &mutexes[ii], &mutexes[(ii+1) % num], NULL, NULL, none, MAX_LIFE};
		pthread_create(&ids[ii], NULL, philosopher, &philos[ii]);
		pthread_detach(ids[ii]);
	}
	start = 1;
	sleep(TIMEOUT);

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_destroy(&mutexes[ii]);
}
