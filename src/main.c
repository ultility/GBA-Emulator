#include "main.h"
#define font_path TIMES_TTF_PATH
enum bool
{
    False,
    True
};

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    struct display emulator;
    init_display(&emulator, 640, 480, "Gameboy Advance");
    struct display debug;
    init_display(&debug, 100, 800, "DEBUG");
    SDL_SetRenderDrawColor(debug.renderer, 255, 255, 255, 255);
    for (int i = R0; i < R12; i++)
    {
        SDL_Rect rect = {.x = 0, .y = (i - R0) * (800 / 15), .w = 100, .h = (800 / 15) - 1};
        SDL_RenderDrawRect(debug.renderer, &rect);
        TTF_Font *font = TTF_OpenFont(font_path, 12);
        if (font == NULL)
        {
            SDL_Log("Failed to load font, %s", SDL_GetError());
            exit(1);
        }
        SDL_Color color = {255, 255, 255, 255};
        char reg_name[4] = {'R', '\0', '\0', '\0'};
        if ((i - R0) < 10)
        {
            reg_name[1] = (i - R0) + '0';
        }
        else
        {
            reg_name[1] = (i - R0) / 10 + '0';
            reg_name[2] = (i - R0) % 10 + '0';
        }
        SDL_Surface *text = TTF_RenderText_Solid(font, reg_name, color);
        if (text == NULL)
        {
            SDL_Log("Failed to render text, %s", SDL_GetError());
            exit(1);
        }
        SDL_Texture *texture = SDL_CreateTextureFromSurface(debug.renderer, text);
        SDL_RenderCopy(debug.renderer, texture, NULL, &rect);
        SDL_FreeSurface(text);
        SDL_DestroyTexture(texture);
    }
    struct cpu cpu;
    cpu_init(&cpu);
    struct request_channel channel;
    channel.id = 258;
    channel.memory_address = 0x06000000;
    channel.memory_range = 0x06017FFF - 0x06000000;
    auto void func(struct request_data *data);
    void func(struct request_data *data)
    {
        if (data->request_type == input)
        {
            data->data_type = word;
            get_pixel(&emulator, data->address % emulator.width, data->address / emulator.width, &(data->data.word));
        }
        else
        {
            set_pixel(&emulator, data->address % emulator.width, data->address / emulator.width, &(data->data.word));
        }
    }
    channel.push_to_channel = func;
    add_request_channel(&cpu, channel);
    set_pixel(&emulator, 100, 50, 0xFF0000);
    cpu.registers[CPSR] &= ~E_MASK;
    cpu.registers[R0] = channel.memory_address + emulator.width * 50 + 100;
    uint32_t str = SINGLE_DATA_TRANSFER_OPCODE | 0b1 << 24 | 0b1 << 21 | 0b1 << 20 | R0 << 16 | R1 << 12;
    cpu.memory[0] = str & 0xFF;
    str >>= 8;
    cpu.memory[1] = str & 0xFF;
    str >>= 8;
    cpu.memory[2] = str & 0xFF;
    str >>= 8;
    cpu.memory[3] = str & 0xFF;

    cpu_print_instruction(&cpu);
    arm_single_data_transfer(&cpu, cpu_fetch_arm_instruction(&cpu));
    cpu_print_registers(&cpu);
    int color;
    get_pixel(&emulator, 100, 50, &color);
    printf("%08x\n", color);
    cpu.registers[CPSR] |= T_MASK;
    cpu_print_instruction(&cpu);
    enum bool run = True;
    while (run)
    {
        update_display(&emulator);
        update_display(&debug);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                run = False;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = event.button.x;
                int y = event.button.y;
                int color = 0xFFFF0000;
                set_pixel(&emulator, x, y, color);
            }
            if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    if (SDL_GetWindowFromID(event.window.windowID) == NULL)
                    {
                        SDL_Log("window not found: %s", SDL_GetError());
                        exit(1);
                    }
                    SDL_HideWindow(SDL_GetWindowFromID(event.window.windowID));
                    if (event.window.windowID == SDL_GetWindowID(emulator.window))
                    {
                        run = False;
                    }
                }
            }
        }
    }
    destory_display(&emulator);
    destory_display(&debug);
    SDL_Quit();
    return 0;
}