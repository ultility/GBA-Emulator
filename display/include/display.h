#pragma once

#include <SDL2/SDL.h>

struct display
{
    int width;
    int height;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

void init_display(struct display *display, int width, int height, const char* title);

void destory_display(struct display *display);

void update_display(struct display *display);

void set_pixel(struct display *display, int x, int y, int32_t color);

void get_pixel(struct display *display, int x, int y, int32_t *color);

SDL_HitTestResult MyCallback(SDL_Window* win, const SDL_Point* area, void* data);
