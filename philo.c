#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "philo.h"
#include <math.h>

void	philo_eat(t_philo *philo) {
	pthread_mutex_lock(philo->left->m);
	philo->left->holder = philo->id;
	philo->left_hand = philo->left;

	pthread_mutex_lock(philo->right->m);
	philo->right->holder = philo->id;
	philo->right_hand = philo->right;

	printf("%d EATING\n", philo->id);
	philo->state = eat;
	sleep(EAT_T);
	philo->life = MAX_LIFE;

	philo->left_hand = NULL;
	philo->left->holder = -1;
	pthread_mutex_unlock(philo->left->m);

	philo->right_hand = NULL;
	philo->right->holder = -1;
	pthread_mutex_unlock(philo->right->m);

	printf("%d DONE EATING\n", philo->id);
	philo->state = none;
}

void	philo_rest(t_philo *philo) {
	printf("%d SLEEPING\n", philo->id);
	philo->state = rest;
	sleep(REST_T);
	printf("%d DONE SLEEPING\n", philo->id);
	philo->state = none;
}

// TODO: THIS DEADLOCKS PROBABLY.  FIX
void *philosopher(void *arg) {
	t_philo *philo = arg;

	while (!running);
	// TODO: only run until time is up, then party
	while (running) {

		philo_eat(philo);

		philo_rest(philo);

	}
	return (NULL);
}

struct s_thing {
	int num;
	t_philo *philos;
};

// TODO: handle death better
void	*overseer(void *arg) {
	struct s_thing *thing = arg;
	t_philo *philos = thing->philos;
	int num = thing->num;
	while (!running);
	time_t old = time(NULL) + 1;
	while (running) {
		// TODO: only run until time is up, then party
		while (time(NULL) > old)
		{
			++old;
			for (int ii = 0; ii < num; ++ii) {
				if (philos[ii].state != eat)
					--philos[ii].life;
				printf("LIFE %d: %d DOING %d\n", ii, philos[ii].life, philos[ii].state);
				if (philos[ii].life <= 0) {
					philos[ii].state = dead;
					running = 0;
					printf("OH NO THREAD %d DIED\n", ii);
				}
			}
		}
		usleep(100000);
	}
	return (NULL);
}

int main(int argc, char **argv) {
	int		num;
	if (argc != 2)
		num = 7;
	else
		num = atoi(argv[1]);
	if (num > 100 || num < 2) {
		printf("Dude, chill.  %d is a ridiculous number of philosophers\n", num);
		return (1);
	}
	t_stick		sticks[num];
	pthread_t ids[num];
	pthread_mutex_t mutexes[num];
	t_philo			philos[num];

	running = 0;

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_init(&mutexes[ii], NULL);

	for (int ii = 0; ii < num; ++ii)
		sticks[ii] = (t_stick){&mutexes[ii], -1};

	printf("Begin\n");
	for (int ii = 0; ii < num; ++ii) {
		float deg = 2 * M_PI / num * ii;
		philos[ii] = (t_philo){ii, &sticks[ii], &sticks[(ii+1) % num], NULL, NULL, none, MAX_LIFE, cos(deg), sin(deg)};
		printf("%d %f %f\n", philos[ii].id, philos[ii].x, philos[ii].y);
		pthread_create(&ids[ii], NULL, philosopher, &philos[ii]);
		pthread_detach(ids[ii]);
	}
	pthread_t overseer_id;
	pthread_create(&overseer_id, NULL, overseer, &(struct s_thing){num, philos});

	display_visu(num, philos, sticks);

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_destroy(&mutexes[ii]);
	return 0;
}
