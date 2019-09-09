#include <unistd.h>
#include <time.h>
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

int running;

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

void	philo_eat(t_philo *philo) {
	pthread_mutex_lock(philo->left);
	pthread_mutex_lock(philo->right);
	philo->right_hand = philo->right;
	philo->left_hand = philo->left;

	printf("%d EATING\n", philo->id);
	philo->state = eat;
	sleep(EAT_T);
	philo->life = MAX_LIFE;

	philo->right_hand = NULL;
	philo->left_hand = NULL;
	pthread_mutex_unlock(philo->left);
	pthread_mutex_unlock(philo->right);

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

void	*overseer(void *arg) {
	struct s_thing *thing = arg;
	t_philo *philos = thing->philos;
	int num = thing->num;
	while (!running);
	time_t old = time(NULL);
	while (running) {
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

#define WINDOWWIDTH 1000
#define WINDOWHEIGHT 1000

#define PHILO_SIZE 75

#include <SDL2/SDL.h>

int main(int argc, char **argv) {
	int		num;
	if (argc != 2)
		num = 7;
	else
		num = atoi(argv[1]);
	pthread_t ids[num];
	pthread_mutex_t mutexes[num];
	t_philo			philos[num];

	running = 0;

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_init(&mutexes[ii], NULL);

	printf("Begin\n");
	for (int ii = 0; ii < num; ++ii) {
		philos[ii] = (t_philo){ii, &mutexes[ii], &mutexes[(ii+1) % num], NULL, NULL, none, MAX_LIFE};
		pthread_create(&ids[ii], NULL, philosopher, &philos[ii]);
		pthread_detach(ids[ii]);
	}
	pthread_t overseer_id;
	pthread_create(&overseer_id, NULL, overseer, &(struct s_thing){num, philos});

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *circle;
	SDL_Texture *circle_texture;
	SDL_Event event;

	// SDL stuff
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return (1);
	}

	if (SDL_CreateWindowAndRenderer(WINDOWWIDTH, WINDOWHEIGHT, SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return (1);
	}
	// TODO: validate that circle was created
	circle = SDL_LoadBMP("assets/circle.bmp");
	circle_texture = SDL_CreateTextureFromSurface(renderer, circle);

	running = 1;
	while (1) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			break;
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, circle_texture, NULL, &(SDL_Rect){WINDOWWIDTH/4,WINDOWHEIGHT/4,WINDOWWIDTH/2,WINDOWWIDTH/2});
		for (int ii = 0; ii < num; ++ii) {
			float	deg = 2 * M_PI / num * ii;
			// TODO: generalize this.  Use macros.  Base size on WINDOWWIDTH or circle dimensions or smth
			float	x = cos(deg) * 225;
			float	y = sin(deg) * 225;
			SDL_RenderCopy(renderer, circle_texture, NULL, &(SDL_Rect){x + WINDOWWIDTH/2 - (PHILO_SIZE / 2),y + WINDOWHEIGHT / 2 - (PHILO_SIZE / 2),PHILO_SIZE,PHILO_SIZE});
		}
		// draw_state(renderer, state);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	// TODO: probably need to kill circle and circle_texture
	// SDL_DestroySurface(circle);
	SDL_DestroyWindow(window);

	SDL_Quit();

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_destroy(&mutexes[ii]);
	return 0;

}

/*
philosopher to rad
rad to deg
y = sin(deg); x = cos(deg)
*/
