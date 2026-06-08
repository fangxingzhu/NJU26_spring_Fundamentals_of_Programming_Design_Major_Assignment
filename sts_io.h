/*
 * STS IO Library
 *
 * Simple I/O library for command line programs.
 *
 * Features:
 *   - Safe integer input with validation
 *   - Manual screen clearing
 *   - ANSI color support (Windows compatible)
 *   - Cross-platform (Windows/Linux/Mac)
 *   - Header callback for persistent HUD display
 */

#ifndef STS_IO_H
#define STS_IO_H

#define STS_IO_BUFFER_SIZE 4096
#define STS_DEFAULT_WIDTH 80

typedef enum {
    STS_COLOR_DEFAULT,
    STS_COLOR_BLACK,
    STS_COLOR_RED,
    STS_COLOR_GREEN,
    STS_COLOR_YELLOW,
    STS_COLOR_BLUE,
    STS_COLOR_MAGENTA,
    STS_COLOR_CYAN,
    STS_COLOR_WHITE,
    STS_COLOR_BRIGHT_BLACK,
    STS_COLOR_BRIGHT_RED,
    STS_COLOR_BRIGHT_GREEN,
    STS_COLOR_BRIGHT_YELLOW,
    STS_COLOR_BRIGHT_BLUE,
    STS_COLOR_BRIGHT_MAGENTA,
    STS_COLOR_BRIGHT_CYAN,
    STS_COLOR_BRIGHT_WHITE
} StsColor;

void sts_init(void);

#if defined(_MSC_VER) && (_MSC_VER >= 1000)
#include <sal.h>
#endif

#if defined(__has_c_attribute) && __has_c_attribute(nodiscard)
#define NODISCARD [[nodiscard]]
#elif defined(_Check_return_)
#define NODISCARD _Check_return_
#elif defined(__GNUC__) || defined(__clang__)
#define NODISCARD __attribute__((warn_unused_result))
#else
#define NODISCARD
#endif

NODISCARD int sts_read_int(const char* prompt);
NODISCARD int sts_read_int_range(const char* prompt, int min_val, int max_val);
NODISCARD char sts_read_char(const char* prompt);
NODISCARD int sts_read_bool(const char* prompt);
void sts_read_line(const char* prompt, char* buf, int buf_size);
void sts_pause(const char* prompt);

void sts_set_color(StsColor color);
void sts_reset_color(void);

void sts_printf(const char* format, ...);
void sts_puts(const char* line);
void sts_separator(void);
void sts_title(const char* title);

void sts_clear_screen(void);
void sts_set_header(void (*func)(void));
void sts_clear_header(void);

#undef NODISCARD

/* ==================== Type-safe print (sts_print) ==================== */

/* Helper functions for different types */
void sts_print_str(const char* s);
void sts_print_int(int n);
void sts_print_long(long n);
void sts_print_long_long(long long n);
void sts_print_uint(unsigned int n);
void sts_print_ulong(unsigned long n);
void sts_print_ulong_long(unsigned long long n);
void sts_print_double(double d);
void sts_print_float(float f);
void sts_print_char(char c);
void sts_print_bool(int b);

/* _Generic selector - picks the right function for each type */
#define sts_print_selector(x) _Generic((x), \
    char*: sts_print_str, \
    const char*: sts_print_str, \
    int: sts_print_int, \
    long: sts_print_long, \
    long long: sts_print_long_long, \
    unsigned int: sts_print_uint, \
    unsigned long: sts_print_ulong, \
    unsigned long long: sts_print_ulong_long, \
    double: sts_print_double, \
    float: sts_print_float, \
    char: sts_print_char, \
    _Bool: sts_print_bool, \
    default: sts_print_int \
)

/* Single argument print */
#define sts_print1(x) sts_print_selector(x)(x)

/* Two arguments */
#define sts_print2(a, b) do { \
    sts_print1(a); \
    sts_print1(b); \
} while(0)

/* Three arguments */
#define sts_print3(a, b, c) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
} while(0)

/* Four arguments */
#define sts_print4(a, b, c, d) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
} while(0)

/* Five arguments */
#define sts_print5(a, b, c, d, e) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
    sts_print1(e); \
} while(0)

/* Six arguments */
#define sts_print6(a, b, c, d, e, f) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
    sts_print1(e); \
    sts_print1(f); \
} while(0)

/* Seven arguments */
#define sts_print7(a, b, c, d, e, f, g) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
    sts_print1(e); \
    sts_print1(f); \
    sts_print1(g); \
} while(0)

/* Eight arguments */
#define sts_print8(a, b, c, d, e, f, g, h) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
    sts_print1(e); \
    sts_print1(f); \
    sts_print1(g); \
    sts_print1(h); \
} while(0)

/* Nine arguments */
#define sts_print9(a, b, c, d, e, f, g, h, i) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
    sts_print1(e); \
    sts_print1(f); \
    sts_print1(g); \
    sts_print1(h); \
    sts_print1(i); \
} while(0)

/* Ten arguments */
#define sts_print10(a, b, c, d, e, f, g, h, i, j) do { \
    sts_print1(a); \
    sts_print1(b); \
    sts_print1(c); \
    sts_print1(d); \
    sts_print1(e); \
    sts_print1(f); \
    sts_print1(g); \
    sts_print1(h); \
    sts_print1(i); \
    sts_print1(j); \
} while(0)

/* Print with newline */
#define sts_println1(x) do { sts_print1(x); sts_puts(""); } while(0)
#define sts_println2(a, b) do { sts_print2(a, b); sts_puts(""); } while(0)
#define sts_println3(a, b, c) do { sts_print3(a, b, c); sts_puts(""); } while(0)
#define sts_println4(a, b, c, d) do { sts_print4(a, b, c, d); sts_puts(""); } while(0)
#define sts_println5(a, b, c, d, e) do { sts_print5(a, b, c, d, e); sts_puts(""); } while(0)
#define sts_println6(a, b, c, d, e, f) do { sts_print6(a, b, c, d, e, f); sts_puts(""); } while(0)
#define sts_println7(a, b, c, d, e, f, g) do { sts_print7(a, b, c, d, e, f, g); sts_puts(""); } while(0)
#define sts_println8(a, b, c, d, e, f, g, h) do { sts_print8(a, b, c, d, e, f, g, h); sts_puts(""); } while(0)
#define sts_println9(a, b, c, d, e, f, g, h, i) do { sts_print9(a, b, c, d, e, f, g, h, i); sts_puts(""); } while(0)
#define sts_println10(a, b, c, d, e, f, g, h, i, j) do { sts_print10(a, b, c, d, e, f, g, h, i, j); sts_puts(""); } while(0)

/* sts_print - dispatch based on number of arguments */
#define sts_print_get_macro(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME
#define sts_print(...) sts_print_get_macro(__VA_ARGS__, \
    sts_print10, sts_print9, sts_print8, sts_print7, sts_print6, \
    sts_print5, sts_print4, sts_print3, sts_print2, sts_print1)(__VA_ARGS__)

#define sts_println(...) sts_print_get_macro(__VA_ARGS__, \
    sts_println10, sts_println9, sts_println8, sts_println7, sts_println6, \
    sts_println5, sts_println4, sts_println3, sts_println2, sts_println1)(__VA_ARGS__)

#endif
