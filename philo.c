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

	// GUI stuff
	float			x;
	float			y;
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

void	*overseer(void *arg) {
	struct s_thing *thing = arg;
	t_philo *philos = thing->philos;
	int num = thing->num;
	while (!running);
	time_t old = time(NULL);
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

#define WINDOWWIDTH 1000
#define WINDOWHEIGHT 1000

#define TABLE_POS_LEFT (WINDOWWIDTH/4)
#define TABLE_POS_TOP (WINDOWHEIGHT/4)

#define TABLE_WIDTH (WINDOWWIDTH/2)
#define TABLE_HEIGHT (WINDOWHEIGHT/2)

#define HBAR_WIDTH 25
#define HBAR_HEIGHT 50

#define PHILO_SIZE (75)
#define PHILO_CENTER_OFF (TABLE_WIDTH/2.1)

#define VISU_ERR 1

#include <SDL2/SDL.h>

// TODO: draw health bar
void	draw_hbar(SDL_Renderer *renderer, t_philo *philo, float x, float y) {
	(void)philo;
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderFillRect(renderer, &(SDL_Rect){x - HBAR_WIDTH, y - HBAR_HEIGHT, HBAR_WIDTH, HBAR_HEIGHT});
}

void	draw_philos(SDL_Renderer *renderer, SDL_Texture *philo_texture, int num, t_philo *philos) {
	for (int ii = 0; ii < num; ++ii) {
		// TODO: draw health bars, add beards
		float x = philos[ii].x * PHILO_CENTER_OFF + WINDOWWIDTH/2 - (PHILO_SIZE / 2);
		float y = philos[ii].y * PHILO_CENTER_OFF + WINDOWHEIGHT / 2 - (PHILO_SIZE / 2);
		SDL_RenderCopy(renderer, philo_texture, NULL, &(SDL_Rect){x, y, PHILO_SIZE, PHILO_SIZE});
		draw_hbar(renderer, &philos[ii], x, y);
	}
}

int	display_visu(int num, t_philo *philos) {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *circle;
	SDL_Texture *circle_texture;
	SDL_Event event;

	// SDL stuff
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return (VISU_ERR);
	}

	if (SDL_CreateWindowAndRenderer(WINDOWWIDTH, WINDOWHEIGHT, SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return (VISU_ERR);
	}
	circle = SDL_LoadBMP("assets/circle.bmp");
	if (!circle) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load bmp: %s", SDL_GetError());
		return (VISU_ERR);
	}
	circle_texture = SDL_CreateTextureFromSurface(renderer, circle);

	running = 1;
	time_t start = time(NULL);
	int done = 0;
	while (!done) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			done = 1;
		}
		else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_q:
				done = 1;
			default:
				// quit if time up and any key pressed
				done = !(time(NULL) - start < TIMEOUT);
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, circle_texture, NULL, &(SDL_Rect){TABLE_POS_LEFT, TABLE_POS_TOP, TABLE_WIDTH, TABLE_HEIGHT});
		draw_philos(renderer, circle_texture, num, philos);
		// TODO: draw chopsticks
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_FreeSurface(circle);
	SDL_DestroyTexture(circle_texture);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return (0);
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

	running = 0;

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_init(&mutexes[ii], NULL);

	printf("Begin\n");
	for (int ii = 0; ii < num; ++ii) {
		float deg = 2 * M_PI / num * ii;
		philos[ii] = (t_philo){ii, &mutexes[ii], &mutexes[(ii+1) % num], NULL, NULL, none, MAX_LIFE, cos(deg), sin(deg)};
		pthread_create(&ids[ii], NULL, philosopher, &philos[ii]);
		pthread_detach(ids[ii]);
	}
	pthread_t overseer_id;
	pthread_create(&overseer_id, NULL, overseer, &(struct s_thing){num, philos});

	display_visu(num, philos);

	for (int ii = 0; ii < num; ++ii)
		pthread_mutex_destroy(&mutexes[ii]);
	return 0;
}
