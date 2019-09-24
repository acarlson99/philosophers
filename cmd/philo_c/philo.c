#include "philo.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifdef DEBUG
#define dbg_printf(...) printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

void philo_eat(t_philo *philo) {
	philo->state = hold;
	pthread_mutex_lock(philo->left->m);
	philo->left->holder = philo->id;
	philo->left_hand = philo->left;

	pthread_mutex_lock(philo->right->m);
	philo->right->holder = philo->id;
	philo->right_hand = philo->right;

	if (running) {
		dbg_printf("%d EATING\n", philo->id);
		philo->state = eat;
		sleep(EAT_T);
	}

	philo->left_hand = NULL;
	philo->left->holder = -1;
	pthread_mutex_unlock(philo->left->m);

	philo->right_hand = NULL;
	philo->right->holder = -1;
	pthread_mutex_unlock(philo->right->m);

	dbg_printf("%d DONE EATING\n", philo->id);
	philo->state = none;
}

void philo_rest(t_philo *philo) {
	dbg_printf("%d SLEEPING\n", philo->id);
	philo->state = rest;
	sleep(REST_T);
	dbg_printf("%d DONE SLEEPING\n", philo->id);
	philo->state = none;
}

void *philosopher(void *arg) {
	t_philo *philo = arg;
	register int action = 0;

	void (*funcs[])(t_philo *) = {
		philo_eat,
		philo_rest,
	};

	if (philo->left_neighbor->id < philo->id) {
		action = 1;
	}

	while (running == run_wait)
		;
	while (running == run_go) {
		funcs[action](philo);
		action = (action + 1) % (sizeof(funcs) / sizeof(*funcs));
	}
	return (NULL);
}

struct s_arg {
	int num;
	t_philo *philos;
};

void *overseer(void *arg) {
	struct s_arg *thing = arg;
	t_philo *philos = thing->philos;
	int num = thing->num;
	while (running == run_wait)
		;
	time_t start = time(NULL);
	time_t old = start + 1;
	while (running == run_go) {
		// make sure to run for every second
		while (running == run_go && time(NULL) > old) {
			++old;
			for (int ii = 0; ii < num; ++ii) {
				if (philos[ii].state != eat)
					--philos[ii].life;
				else
					philos[ii].life = MAX_LIFE;
				dbg_printf("LIFE %d: %d DOING %d\n", ii, philos[ii].life,
						   philos[ii].state);
				if (philos[ii].life <= 0) {
					philos[ii].dead = 1;
					running = run_done;
					dbg_printf("OH NO THREAD %d DIED\n", ii);
				}
			}
			if (time(NULL) - start > TIMEOUT) {
				running = run_done;
				dbg_printf("TIMEOUT\n");
			}
			else if (running == run_done)
				return (NULL);
		}
		usleep(100000);
	}
	return (NULL);
}

int main(int argc, char **argv) {
	register int num;

	srand(time(NULL));
	if (argc != 2)
		num = 7;
	else
		num = atoi(argv[1]);
	if (num > 100 || num < 2) {
		fprintf(stderr,
				"Dude, chill. %d is a ridiculous number of philosophers\n",
				num);
		return (1);
	}

	t_stick sticks[num];
	pthread_t ids[num];
	pthread_mutex_t mutexes[num];
	t_philo philos[num];

	running = run_wait;

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_init(&mutexes[ii], NULL);

	for (int ii = 0; ii < num; ++ii)
		sticks[ii] = (t_stick){&mutexes[ii], -1};

	dbg_printf("Begin\n");
	for (int ii = 0; ii < num; ++ii) {
		float deg = 2 * M_PI / num * ii;
		int rnum = rand();
		while (rnum == -1)
			rnum = rand();
		philos[ii] = (t_philo){rnum,
							   0,
							   &sticks[ii],
							   &sticks[(ii + 1) % num],
							   NULL,
							   NULL,
							   none,
							   MAX_LIFE,
							   &philos[(ii - 1 + num) % num],
							   &philos[(ii + 1) % num],
							   cos(deg),
							   sin(deg)};
		dbg_printf("%d %f %f\n", philos[ii].id, philos[ii].x, philos[ii].y);
		pthread_create(&ids[ii], NULL, philosopher, &philos[ii]);
		pthread_detach(ids[ii]);
	}

	pthread_t overseer_id;
	pthread_create(&overseer_id, NULL, overseer, &(struct s_arg){num, philos});
	pthread_detach(overseer_id);

	display_visu(num, philos, sticks);

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_destroy(&mutexes[ii]);
	return 0;
}
