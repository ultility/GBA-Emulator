#include "include/main.h"
#include "include/testing.h"

const int screen_width = 240;
const int screen_height = 160;

int main(int argc, char *argv[])
{
    /*SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Event e;
    bool run = true;
    init(&window, &renderer);*/
    struct cpu cpu;
    cpu_init(&cpu);
    test_cpu();
    /*while (run)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                run = false;
                break;
            }
            draw_display(renderer, &cpu);
            SDL_RenderPresent(renderer);
        }
    }*/
    // terminate(window, renderer);
    return no_error;
}

void init(SDL_Window **window, SDL_Renderer **renderer)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s", SDL_GetError());
        exit(sdl_error);
    }
    *window =
        SDL_CreateWindow("gba", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         screen_width, screen_height, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow Error: %s", SDL_GetError());
        exit(sdl_error);
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        exit(sdl_error);
    }
}

void terminate(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

void draw_display(SDL_Renderer *renderer, struct cpu *cpu)
{
    if (!cpu->drawn)
    {
        for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++)
        {
            PIXEL pixel = cpu_load_word(cpu, VRAM_START + i);
            Uint8 A = pixel & UINT8_MAX;
            Uint8 R = (pixel >> __CHAR_BIT__) & UINT8_MAX;
            Uint8 G = (pixel >> (__CHAR_BIT__ * 2)) & UINT8_MAX;
            Uint8 B = (pixel >> (__CHAR_BIT__ * 3)) & UINT8_MAX;
            // printf("pixel %d, A:%d, R:%d, G:%d, B:%d\n", i, A, R, G, B);
            SDL_SetRenderDrawColor(renderer, R, G, B, A);
            SDL_RenderDrawPoint(renderer, i % SCREEN_WIDTH, i / SCREEN_WIDTH);
        }
        cpu->drawn = true;
    }
}

void draw_rect(struct cpu *cpu, int left, int top, int width, int height, int color)
{
    for (int x = left; x < left + width; x++)
    {
        for (int y = top; y < top + height; y++)
        {
            cpu_save_word(cpu, VRAM_START + x + y * SCREEN_WIDTH, color);
        }
    }
}
