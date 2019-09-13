#include "philo.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define WINDOWWIDTH 750
#define WINDOWHEIGHT 750

#define TABLE_POS_LEFT (WINDOWWIDTH / 4)
#define TABLE_POS_TOP (WINDOWHEIGHT / 4)

#define TABLE_WIDTH (WINDOWWIDTH / 2)
#define TABLE_HEIGHT (WINDOWHEIGHT / 2)

#define HBAR_WIDTH 25
#define HBAR_HEIGHT 75

#define PHILO_SIZE (75)
#define PHILO_CENTER_OFF (TABLE_WIDTH / 2.)

#define PLATE_CENTER_OFF (TABLE_WIDTH / 3.3)

#define VISU_ERR 1

#define HBAR_COLOR 141, 8, 1
#define HBAR_BKG_COLOR 231, 46, 43
#define BKG_COLOR 244, 213, 141
#define TABLE_COLOR 239, 235, 206
#define STICK_COLOR 163, 163, 128
#define PLATE_COLOR 255, 255, 255

#define STICK_ROT 0
#define STICK_WIDTH 30
#define STICK_HEIGHT PHILO_SIZE

void draw_hbar(SDL_Renderer *renderer, t_philo *philo, float x, float y) {
  SDL_SetRenderDrawColor(renderer, HBAR_BKG_COLOR, 255);
  SDL_RenderFillRect(renderer, &(SDL_Rect){x, y, HBAR_WIDTH, HBAR_HEIGHT});
  SDL_SetRenderDrawColor(renderer, HBAR_COLOR, 255);

  float height = (float)philo->life / (float)MAX_LIFE * (float)HBAR_HEIGHT;

  SDL_RenderFillRect(
      renderer, &(SDL_Rect){x, y + (HBAR_HEIGHT - height), HBAR_WIDTH, height});
}

#define SHAKE_MAX 20
#define SHAKE_MIN -SHAKE_MAX

void draw_state(SDL_Renderer *renderer, SDL_Texture *circle_texture,
                SDL_Texture *philo_texture, SDL_Texture *stick_texture, int num,
                t_philo *philos, t_stick *sticks) {
  static int shake = 0;
  static int shake_direction = 1;

  // table
  SDL_SetTextureColorMod(circle_texture, TABLE_COLOR);
  SDL_RenderCopy(
      renderer, circle_texture, NULL,
      &(SDL_Rect){TABLE_POS_LEFT, TABLE_POS_TOP, TABLE_WIDTH, TABLE_HEIGHT});

  if (running == run_done) {
    if (shake >= SHAKE_MAX || shake <= SHAKE_MIN)
      shake_direction *= -1;
    shake += shake_direction;
  }
  for (int ii = 0; ii < num; ++ii) {
    // philo
    float x = philos[ii].x * PHILO_CENTER_OFF + WINDOWWIDTH / 2;
    float y = philos[ii].y * PHILO_CENTER_OFF + WINDOWHEIGHT / 2;
    if (philos[ii].dead)
      SDL_SetTextureColorMod(philo_texture, 0, 0, 0);
    SDL_RenderCopyEx(renderer, philo_texture, NULL,
                     &(SDL_Rect){x - (PHILO_SIZE / 2), y - (PHILO_SIZE / 2),
                                 PHILO_SIZE, PHILO_SIZE},
                     philos[ii].dead ? 0 : shake, NULL, SDL_FLIP_NONE);
    SDL_SetTextureColorMod(philo_texture, 255, 255, 255);

    // health
    draw_hbar(renderer, &philos[ii],
              (philos[ii].x < 0 ? x - HBAR_WIDTH * 2 - PHILO_SIZE / 2
                                : x + HBAR_WIDTH + PHILO_SIZE / 2),
              y - (PHILO_SIZE / 2));

    // plate
    SDL_SetTextureColorMod(circle_texture, PLATE_COLOR);
    float plate_x =
        philos[ii].x * PLATE_CENTER_OFF + WINDOWWIDTH / 2 - (PHILO_SIZE / 2);
    float plate_y =
        philos[ii].y * PLATE_CENTER_OFF + WINDOWHEIGHT / 2 - (PHILO_SIZE / 2);
    SDL_RenderCopy(renderer, circle_texture, NULL,
                   &(SDL_Rect){plate_x, plate_y, PHILO_SIZE, PHILO_SIZE});
    SDL_SetTextureColorMod(circle_texture, 255, 255, 255);

    // sticks
    if (sticks[ii].holder == -1) {
      x = ((philos[ii].x + philos[(ii - 1 + num) % num].x) / 2) *
              PHILO_CENTER_OFF +
          WINDOWWIDTH / 2 - (PHILO_SIZE / 2) + (STICK_WIDTH / 2);
      y = ((philos[ii].y + philos[(ii - 1 + num) % num].y) / 2) *
              PHILO_CENTER_OFF +
          WINDOWHEIGHT / 2 - (PHILO_SIZE / 2) + (STICK_WIDTH / 2);
      float vec_x = -(WINDOWWIDTH / 2) + (x + STICK_WIDTH / 2);
      float vec_y = -(WINDOWHEIGHT / 2) + (y + STICK_HEIGHT / 2);
      SDL_RenderCopyEx(renderer, stick_texture, NULL,
                       &(SDL_Rect){x, y, STICK_WIDTH, STICK_HEIGHT},
                       STICK_ROT + (atan2(vec_y, vec_x) * 180 / M_PI - 90),
                       NULL, SDL_FLIP_NONE);

    } else if (sticks[ii].holder == philos[ii].id) {
      SDL_RenderCopyEx(renderer, stick_texture, NULL,
                       &(SDL_Rect){x - PHILO_SIZE + STICK_WIDTH / 2,
                                   y - PHILO_SIZE / 2, STICK_WIDTH,
                                   STICK_HEIGHT},
                       STICK_ROT, NULL, SDL_FLIP_NONE);
    }
    if (sticks[(ii + 1) % num].holder == philos[ii].id) {
      SDL_RenderCopyEx(renderer, stick_texture, NULL,
                       &(SDL_Rect){x + STICK_WIDTH / 2, y - PHILO_SIZE / 2,
                                   STICK_WIDTH, STICK_HEIGHT},
                       STICK_ROT, NULL, SDL_FLIP_NONE);
    }
  }
}

int display_visu(int num, t_philo *philos, t_stick *sticks) {
  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_Surface *circle;
  SDL_Texture *circle_texture;

  SDL_Surface *philo;
  SDL_Texture *philo_texture;

  SDL_Surface *stick;
  SDL_Texture *stick_texture;

  // SDL stuff
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s",
                 SDL_GetError());
    return (VISU_ERR);
  }

  if (SDL_CreateWindowAndRenderer(WINDOWWIDTH, WINDOWHEIGHT, SDL_WINDOW_SHOWN,
                                  &window, &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Couldn't create window and renderer: %s", SDL_GetError());
    return (VISU_ERR);
  }
  circle = SDL_LoadBMP("assets/circle.bmp");
  if (!circle) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load bmp: %s",
                 SDL_GetError());
    return (VISU_ERR);
  }
  circle_texture = SDL_CreateTextureFromSurface(renderer, circle);

  philo = SDL_LoadBMP("assets/philo.bmp");
  if (!philo) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load bmp: %s",
                 SDL_GetError());
    return (VISU_ERR);
  }
  philo_texture = SDL_CreateTextureFromSurface(renderer, philo);

  stick = SDL_LoadBMP("assets/stick.bmp");
  if (!stick) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load bmp: %s",
                 SDL_GetError());
    return (VISU_ERR);
  }
  stick_texture = SDL_CreateTextureFromSurface(renderer, stick);
  SDL_SetTextureColorMod(stick_texture, STICK_COLOR);

  running = run_go;
  int start = 0;
  int old = 0;
  int delta = 0;
  int msPerFrame = 1000 / 30; // ms / fps
  int done = 0;
  SDL_Event event;
  while (!done) {
    if (!start)
      start = SDL_GetTicks();
    else
      delta = old - start;
    if (delta < msPerFrame)
      SDL_Delay(msPerFrame - delta);

    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      done = 1;
    } else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
      case SDLK_q:
        done = 1;
        break;
      default:
        // quit if time up and any non-special (ctrl alt...) key pressed
        if (!(event.key.keysym.sym & (1 << 30)))
          done = (running == run_done);
      }
    }
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, BKG_COLOR, 255);
    SDL_RenderFillRect(renderer, NULL);
    draw_state(renderer, circle_texture, philo_texture, stick_texture, num,
               philos, sticks);
    SDL_RenderPresent(renderer);

    start = old;
    old = SDL_GetTicks();
  }

  // teardown
  SDL_DestroyRenderer(renderer);
  SDL_FreeSurface(circle);
  SDL_DestroyTexture(circle_texture);
  SDL_FreeSurface(philo);
  SDL_DestroyTexture(philo_texture);
  SDL_FreeSurface(stick);
  SDL_DestroyTexture(stick_texture);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return (0);
}
