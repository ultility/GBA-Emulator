#pragma once
#include "cpu.h"
#include "error.h"
#include <SDL2/SDL.h>
#include <stdio.h>

void init(SDL_Window **window, SDL_Renderer **renderer);

void terminate(SDL_Window *window, SDL_Renderer *renderer);

void draw_display(SDL_Renderer *renderer, struct cpu *cpu);

void draw_rect(struct cpu *cpu, int left, int top, int width, int height, int color);