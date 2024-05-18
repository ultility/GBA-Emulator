#pragma once
#include <SDL_ttf.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef __linux__
#include "/usr/include/linux/limits.h"
#include <libelf.h>
#include <gelf.h>
#endif
#ifdef _WIN32
    #include <windows.h>
#endif
#include "display.h"
#include "cpu.h"

#define MAX_FPS 60
#define SECOND 1000

const char* open_rom();
void load_bios(struct cpu* cpu);