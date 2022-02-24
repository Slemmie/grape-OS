// video memory address writing

#pragma once

#include "../util/types.h"

// put this define before including vidmem header to enable simplified function names as well
#ifdef VIDMEM_SIMPLIFIED_API

#define set_cursor         vidmem_set_cursor
#define clear_screen       vidmem_clear_screen
#define clear_screen_color vidmem_clear_screen_color

#endif

// set cursor position
void vidmem_set_cursor(uint_8 row, uint_8 col);

// clear the screen
void vidmem_clear_screen();
// clear screen to a specific color
void vidmem_clear_screen_color(uint_8 color);
