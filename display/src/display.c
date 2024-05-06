#include "display.h"

void init_display(struct display *display, int width, int height, const char* title)
{
    display->width = width;
    display->height = height;
    display->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (display->window == NULL)
    {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
    if (display->renderer == NULL)
    {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void destory_display(struct display *display)
{
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
}

void update_display(struct display *display)
{
    SDL_RenderPresent(display->renderer);
}

void set_pixel(struct display *display, int x, int y, int32_t color)
{
    SDL_SetRenderDrawColor(display->renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, (color >> 24) & 0xFF);
    SDL_RenderDrawPoint(display->renderer, x, y);
}

void get_pixel(struct display *display, int x, int y, int32_t *color)
{
    SDL_RenderReadPixels(display->renderer, &(SDL_Rect){x, y, 1, 1}, SDL_PIXELFORMAT_RGBA8888, color, display->width);
}