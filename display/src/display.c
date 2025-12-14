#include "display.h"

void init_display(struct display *display, int width, int height, const char *title)
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
    display->last_update = 0;
}

void destory_display(struct display *display)
{
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
}

void update_display(struct display *display, int current_tick)
{
    SDL_RenderPresent(display->renderer);
    display->last_update = current_tick;
}

void set_pixel(struct display *display, int x, int y, int32_t color)
{
    uint8_t r,g,b;
    r = color & 0b11111;
    g = (color & 0b11111 << 5) >> 5;
    b = (color & 0b11111 << 10) >> 10;
    float lerp = 0xFF / 0b11111;
    r = (int)(r * lerp);
    g = (int)(g * lerp);
    b = (int)(b * lerp);
    SDL_SetRenderDrawColor(display->renderer, r, g, b, 0xFF);
    SDL_RenderDrawPoint(display->renderer, x, y);
    //update_display(display, SDL_GetTicks());
}

void get_pixel(struct display *display, int x, int y, int32_t *color)
{
    SDL_Rect dst = {.x = x, .y = y, .w = 1, .h = 1};
    if (SDL_RenderReadPixels(display->renderer, &(SDL_Rect){x, y, 1, 1}, SDL_PIXELFORMAT_RGBA8888, color, display->width) != 0)
    {
        SDL_Log("Failed to read pixel, %s", SDL_GetError());
        exit(1);
    }
}