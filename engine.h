#ifndef ENGINE_H
#define ENGINE_H

#ifdef _WIN32
#error "No support for windows"
#else
#include <curses.h>
#endif

#define invoke(func, ...) do {            \
    if (func != NULL) func(__VA_ARGS__);  \
} while (0)

#endif
