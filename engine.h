#ifndef ENGINE_H
#define ENGINE_H

#ifdef _WIN32
#error "No support for windows"
#else
#include <curses.h>
#endif

#define SECONDS 1000
#define MICRO_SECONDS 1000000

#ifdef DEBUG
#define DEBUG_PRINT(level, message, ...) \
    fprintf(stderr,                      \
        "[%-5s: %s:%d]: " message "\n",  \
        #level, __FILE__, __LINE__,      \
        __VA_ARGS__                      \
    );
#else
#define DEBUG_PRINT(...)
#endif

#define LOG(...)   DEBUG_PRINT(LOG  , __VA_ARGS__)
#define INFO(...)  DEBUG_PRINT(INFO , __VA_ARGS__)
#define WARN(...)  DEBUG_PRINT(WARN , __VA_ARGS__)
#define ERROR(...) DEBUG_PRINT(ERROR, __VA_ARGS__)


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

#define linear_scale_rgb(x) linear_scale(x, 0, 255, 0, 1000)

#define linear_scale(x, min_in, max_in, min_out, max_out) (                 \
    (((x - min_in) * (max_out - min_out)) / (max_in - min_in)) + min_out    \
)

#define HEX_TO_RGB(hex) \
    (hex & 0xFF0000) >> 16, (hex & 0x00FF00) >> 8, (hex & 0x0000FF)

#endif
