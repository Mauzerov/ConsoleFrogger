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

#define max(a, b) (((a) < (b)) ? (b) : (a))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define clamp(x_min, x, x_max) ( \
    ((x) < (x_min)) ?            \
        (x_min)     :            \
        (((x) > (x_max)) ?       \
            (x_max)      : (x))  \
)

#endif
