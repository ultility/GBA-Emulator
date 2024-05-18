#include "main.h"

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        SDL_Log("Failed to initialize SDL, %s", SDL_GetError());
        exit(-1);
    }
    bool run = true;    
    TTF_Init();
    struct display emulator;
    init_display(&emulator, 640, 480, "Gameboy Advance");
    struct display debug;
    //init_display(&debug, 100, 800, "DEBUG");
    struct cpu cpu;
    cpu_init(&cpu);
    load_bios(&cpu);
    struct request_channel channel;
    channel.id = 258;
    channel.memory_address = VIRTUAL_PALLETTE_RAM;
    channel.memory_range = 0x06017FFF - 0x06000000;
    void func(struct request_data * data)
    { // error can be ignored this syntax is for the gcc nested function declaration
        if (data->request_type == input)
        {
            data->data_type = word;
            get_pixel(&emulator, data->address % emulator.width, data->address / emulator.width, &(data->data.word));
        }
        else
        {
            set_pixel(&emulator, data->address % emulator.width, data->address / emulator.width, data->data.word);
        }
    }
    channel.push_to_channel = func;
    add_request_channel(&cpu, channel);
    while (run)
    {
        if (SDL_GetTicks() - emulator.last_update > (SECOND / MAX_FPS))
        {
            update_display(&emulator, SDL_GetTicks());
        }
        cpu_loop(&cpu);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                run = false;
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
                        run = false;
                        break;
                    }
                }
            }
        }
    }
    destory_display(&emulator);
    destory_display(&debug);
    free_cpu(&cpu);
    SDL_Quit();
    return 0;
}

const char* open_rom()
{
    char *buffer = calloc(PATH_MAX, sizeof(char));
    buffer[PATH_MAX - 1] = '\0';
    #ifdef __linux__
    const char zenityP[] = "/usr/bin/zenity";
    char Call[2048];

    sprintf(Call, "%s  --file-selection --modal --title=\"%s\" ", zenityP, "Select file");

    FILE *f = popen(Call, "r");

    fgets(buffer, PATH_MAX-1, f);
    
    int ret = pclose(f);
    if (ret < 0)
    {
        perror("file_name_dialog()");
        return "\0";
    }
    if (strncmp(buffer + strlen(buffer) - 4 - 1, ".rom", 4) != 0)
    {
        printf("Error: File must be a .rom file.\n");
        return "\0";
    }
    ret == 0; // return true if all is OK
    return buffer;
    #elif _WIN32
    OPENFILENAME  ofn;        
    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = NULL;
    ofn.hInstance       = NULL;
    ofn.lpstrFilter     = "ROM Files\0*.rom\0\0";    
    ofn.lpstrFile       = buffer;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrTitle      = "Please Select A File To Open";
    ofn.Flags           = OFN_NONETWORKBUTTON |
                          OFN_FILEMUSTEXIST |
                          OFN_HIDEREADONLY;
    if (!GetOpenFileName(&ofn))
    {
        return "\0";
    }
    return buffer;
    #endif
}

void load_bios(struct cpu* cpu)
{
    FILE *f = fopen(BIOS_PATH, "rb");
    if (f == NULL)
    {
        printf("Error: failed to open bios.elf, errno: %d", errno);
        exit(1);
    }
    int fd = open(BIOS_PATH, O_RDONLY, 0);
    if (fd < 0)
    {
        printf("Error: Could not open bios.elf, errno: %d\n", errno);
        exit(1);
    }
    if (elf_version(EV_CURRENT) == EV_NONE)
    {
        printf("elf library initialization failed: %s\n", elf_errmsg(-1));
    }
    Elf *e = elf_begin(fd, ELF_C_READ, NULL);
    if (e == NULL)
    {
        printf("Error: bios.elf is not an elf file, err: %s\n", elf_errmsg(-1));
        exit(1);
    }
    if (elf_kind(e)!= ELF_K_ELF)
    {
        printf("Error: bios.elf is not an object file\n");
        exit(1);
    }
    GElf_Ehdr ehdr;
    if (gelf_getehdr(e, &ehdr) == NULL)
    {
        printf("Error: elf_getehdr() failed: %s\n", elf_errmsg(-1));
        exit(1);
    }
    if (ehdr.e_type!= ET_EXEC)
    {
        printf("Error: bios.elf isnt an executeable file\n");
        exit(1);
    }
    if (ehdr.e_phnum == 0)
    {
        printf("Error: elf header missing program header\n");
    }
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    while ((scn = elf_nextscn(e, scn)) != NULL)
    {
        if (gelf_getshdr(scn, &shdr) != &shdr)
        {
            printf("Error: failed to get shdr, %s", elf_errmsg(-1));
        }
        if (shdr.sh_flags & SHF_EXECINSTR)
        {
            if (shdr.sh_size > (BIOS_SIZE - 0xFF))
            {
                printf("Error: section is bigger that max bios size");
                exit(-1);
            }
            fseek(f, shdr.sh_offset, SEEK_SET);
            BYTE file_buffer[shdr.sh_size];
            fread(file_buffer, shdr.sh_size, 1, f);
            memcpy(cpu->memory, file_buffer, shdr.sh_size);
            if (ehdr.e_ident[EI_DATA] == ELFDATA2MSB)
            {
                cpu->registers[CPSR] |= E_MASK;
            }
            GElf_Phdr phdr;
            cpu->registers[PC] = ehdr.e_entry - shdr.sh_addr;
            break;
        }
    }
    elf_end(e);
    close(fd);
    fclose(f);
}