#include <SDL2/SDL.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "philo.h"

#define WINDOWWIDTH 1000
#define WINDOWHEIGHT 1000

#define TABLE_POS_LEFT (WINDOWWIDTH/4)
#define TABLE_POS_TOP (WINDOWHEIGHT/4)

#define TABLE_WIDTH (WINDOWWIDTH/2)
#define TABLE_HEIGHT (WINDOWHEIGHT/2)

#define HBAR_WIDTH 25
#define HBAR_HEIGHT 75

#define PHILO_SIZE (75)
#define PHILO_CENTER_OFF (TABLE_WIDTH/2.1)

#define PLATE_CENTER_OFF (TABLE_WIDTH/3.3)

#define VISU_ERR 1

void	draw_hbar(SDL_Renderer *renderer, t_philo *philo, float x, float y) {
	(void)philo;
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderFillRect(renderer, &(SDL_Rect){x - HBAR_WIDTH, y - HBAR_HEIGHT, HBAR_WIDTH, HBAR_HEIGHT});
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	float height = (float)philo->life / (float)MAX_LIFE * (float)HBAR_HEIGHT;
	SDL_RenderFillRect(renderer, &(SDL_Rect){x - HBAR_WIDTH, y - HBAR_HEIGHT + (HBAR_HEIGHT - height), HBAR_WIDTH, height});
}

void	draw_state(SDL_Renderer *renderer, SDL_Texture *philo_texture, int num, t_philo *philos, t_stick *sticks) {
	for (int ii = 0; ii < num; ++ii) {
		// TODO: add beards
		float x = philos[ii].x * PHILO_CENTER_OFF + WINDOWWIDTH/2 - (PHILO_SIZE / 2);
		float y = philos[ii].y * PHILO_CENTER_OFF + WINDOWHEIGHT / 2 - (PHILO_SIZE / 2);
		SDL_RenderCopy(renderer, philo_texture, NULL, &(SDL_Rect){x, y, PHILO_SIZE, PHILO_SIZE});

		// health
		SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
		draw_hbar(renderer, &philos[ii], philos[ii].x < 0 ? x : x + HBAR_WIDTH + PHILO_SIZE, y);

		// chopsticks
		SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
		if (sticks[ii].holder == ii) {
			SDL_RenderDrawLine(renderer, x, y, x+100, y+100);
		}
		else if (sticks[ii].holder == -1) {
			// TODO: draw sticks better
			x = ((philos[ii].x + philos[(ii + 1) % num].x) / 2) * PHILO_CENTER_OFF + WINDOWWIDTH/2 - (PHILO_SIZE / 2);
			y = ((philos[ii].y + philos[(ii + 1) % num].y) / 2) * PHILO_CENTER_OFF + WINDOWHEIGHT / 2 - (PHILO_SIZE / 2);
			SDL_RenderDrawLine(renderer, x, y, WINDOWWIDTH/2, WINDOWHEIGHT/2);
		}
		if (sticks[(ii + 1) % num].holder == ii) {
			SDL_RenderDrawLine(renderer, x, y, x+100, y-100);
		}
		
		// place plate in front of philosopher
		SDL_SetTextureColorMod(philo_texture, 100, 0, 0);
		x = philos[ii].x * PLATE_CENTER_OFF + WINDOWWIDTH/2 - (PHILO_SIZE / 2);
		y = philos[ii].y * PLATE_CENTER_OFF + WINDOWHEIGHT / 2 - (PHILO_SIZE / 2);
		SDL_RenderCopy(renderer, philo_texture, NULL, &(SDL_Rect){x, y, PHILO_SIZE, PHILO_SIZE});
		SDL_SetTextureColorMod(philo_texture, 255, 255, 255);

		// draw chopsticks
	}
}

int	display_visu(int num, t_philo *philos, t_stick *sticks) {
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
				break ;
			default:
				// quit if time up and any key pressed
				done = !(time(NULL) - start < TIMEOUT);
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, circle_texture, NULL, &(SDL_Rect){TABLE_POS_LEFT, TABLE_POS_TOP, TABLE_WIDTH, TABLE_HEIGHT});
		draw_state(renderer, circle_texture, num, philos, sticks);
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
