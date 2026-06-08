#include "sts_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static int g_initialized = 0;
static void (*g_header_callback)(void) = NULL;

static const char* g_color_codes[] = {
    "\033[0m", "\033[30m", "\033[31m", "\033[32m", "\033[33m",
    "\033[34m", "\033[35m", "\033[36m", "\033[37m", "\033[90m",
    "\033[91m", "\033[92m", "\033[93m", "\033[94m", "\033[95m",
    "\033[96m", "\033[97m"
};

static int is_blank_line(const char* str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

static int is_valid_integer(const char* str, int* out_value) {
    char* endptr;
    long val = strtol(str, &endptr, 10);

    if (endptr == str) {
        return 0;
    }
    if (!is_blank_line(endptr)) {
        return 0;
    }

    if (val < (long)INT_MIN || val > (long)INT_MAX) {
        return 0;
    }

    if (out_value) {
        *out_value = (int)val;
    }
    return 1;
}

static int is_valid_single_char(const char* str, char* out_char) {
    const char* p = str;

    while (*p && isspace((unsigned char)*p)) {
        p++;
    }

    if (!*p) {
        return 0;
    }

    if (out_char) {
        *out_char = *p;
    }
    p++;

    while (*p && isspace((unsigned char)*p)) {
        p++;
    }

    return (*p == '\0');
}

static int str_ci_equal(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int parse_bool(const char* str) {
    char trimmed[16];
    int i = 0;

    while (*str && isspace((unsigned char)*str)) {
        str++;
    }

    while (*str && !isspace((unsigned char)*str) && i < 15) {
        trimmed[i++] = *str++;
    }
    trimmed[i] = '\0';

    if (str_ci_equal(trimmed, "y") || str_ci_equal(trimmed, "yes")) {
        return 1;
    }
    if (str_ci_equal(trimmed, "n") || str_ci_equal(trimmed, "no")) {
        return 0;
    }

    return -1;
}

static void print_error(const char* msg) {
    sts_init();
    sts_set_color(STS_COLOR_BRIGHT_RED);
    printf("  [Error] %s\n", msg);
    sts_reset_color();
    fflush(stdout);
}

void sts_init(void) {
    if (g_initialized) {
        return;
    }

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    g_initialized = 1;
}

void sts_clear_screen(void) {
    sts_init();
    printf("\033[H\033[2J");
    fflush(stdout);

    if (g_header_callback) {
        g_header_callback();
        fflush(stdout);
    }
}

void sts_set_header(void (*func)(void)) {
    g_header_callback = func;
}

void sts_clear_header(void) {
    g_header_callback = NULL;
}

void sts_set_color(StsColor color) {
    sts_init();
    if (color >= 0 && color < (int)(sizeof(g_color_codes) / sizeof(g_color_codes[0]))) {
        fputs(g_color_codes[color], stdout);
        fflush(stdout);
    }
}

void sts_reset_color(void) {
    sts_init();
    fputs(g_color_codes[0], stdout);
    fflush(stdout);
}

int sts_read_int(const char* prompt) {
    char buf[STS_IO_BUFFER_SIZE];
    int value;

    sts_init();

    while (1) {
        if (prompt) {
            fputs(prompt, stdout);
            fflush(stdout);
        }

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n");
                exit(0);
            }
            clearerr(stdin);
            continue;
        }

        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] != '\n') {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        if (is_valid_integer(buf, &value)) {
            return value;
        }

        print_error("Please enter an integer!");
    }
}

int sts_read_int_range(const char* prompt, int min_val, int max_val) {
    int value;

    while (1) {
        value = sts_read_int(prompt);

        if (value >= min_val && value <= max_val) {
            return value;
        }

        char err_msg[128];
        snprintf(err_msg, sizeof(err_msg),
                 "Please enter an integer between %d and %d!",
                 min_val, max_val);
        print_error(err_msg);
    }
}

char sts_read_char(const char* prompt) {
    char buf[STS_IO_BUFFER_SIZE];
    char result;

    sts_init();

    while (1) {
        if (prompt) {
            fputs(prompt, stdout);
            fflush(stdout);
        }

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n");
                exit(0);
            }
            clearerr(stdin);
            continue;
        }

        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] != '\n') {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        if (is_valid_single_char(buf, &result)) {
            return result;
        }

        print_error("Please enter exactly one character!");
    }
}

int sts_read_bool(const char* prompt) {
    char buf[STS_IO_BUFFER_SIZE];
    int result;

    sts_init();

    while (1) {
        if (prompt) {
            fputs(prompt, stdout);
            fflush(stdout);
        }

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n");
                exit(0);
            }
            clearerr(stdin);
            continue;
        }

        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] != '\n') {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        result = parse_bool(buf);
        if (result >= 0) {
            return result;
        }

        print_error("Please answer y/n or yes/no!");
    }
}

void sts_read_line(const char* prompt, char* buf, int buf_size) {
    sts_init();

    if (prompt) {
        fputs(prompt, stdout);
        fflush(stdout);
    }

    char temp_buf[STS_IO_BUFFER_SIZE];
    char* use_buf = buf ? buf : temp_buf;
    int use_size = buf ? buf_size : (int)sizeof(temp_buf);

    if (fgets(use_buf, use_size, stdin) == NULL) {
        if (feof(stdin)) {
            printf("\n");
            exit(0);
        }
        if (buf) buf[0] = '\0';
        return;
    }

    size_t len = strlen(use_buf);
    if (len > 0 && use_buf[len - 1] != '\n') {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    } else if (len > 0 && use_buf[len - 1] == '\n') {
        use_buf[len - 1] = '\0';
    }
}

void sts_pause(const char* prompt) {
    if (prompt == NULL) {
        prompt = "Press Enter to continue...";
    }
    sts_read_line(prompt, NULL, 0);
}

void sts_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

void sts_puts(const char* line) {
    printf("%s\n", line);
    fflush(stdout);
}

void sts_separator(void) {
    int width = STS_DEFAULT_WIDTH;
    char buf[128];
    int i;
    for (i = 0; i < width - 1 && i < 126; i++) {
        buf[i] = '-';
    }
    buf[i] = '\0';
    sts_puts(buf);
}

void sts_title(const char* title) {
    int width = STS_DEFAULT_WIDTH;

    sts_separator();

    if (!title) {
        title = "";
    }

    const char* p = title;
    do {
        const char* end = strchr(p, '\n');
        if (!end) {
            end = p + strlen(p);
        }
        
        int line_len = (int)(end - p);

        if (line_len > width) {
            int skip = (line_len - width) / 2;
            printf("%.*s\n", width, p + skip);
        } else {
            int padding = (width - line_len) / 2;
            for (int i = 0; i < padding; i++) {
                putchar(' ');
            }
            printf("%.*s\n", line_len, p);
        }

        p = end;
        if (*p == '\n') {
            p++;
        } else {
            break;
        }
    } while (1);

    sts_separator();
    fflush(stdout);
}

/* ==================== Type-safe print functions ==================== */

void sts_print_str(const char* s) {
    if(s) fputs(s, stdout);
    fflush(stdout);
}

void sts_print_int(int n) {
    printf("%d", n);
    fflush(stdout);
}

void sts_print_long(long n) {
    printf("%ld", n);
    fflush(stdout);
}

void sts_print_long_long(long long n) {
    printf("%lld", n);
    fflush(stdout);
}

void sts_print_uint(unsigned int n) {
    printf("%u", n);
    fflush(stdout);
}

void sts_print_ulong(unsigned long n) {
    printf("%lu", n);
    fflush(stdout);
}

void sts_print_ulong_long(unsigned long long n) {
    printf("%llu", n);
    fflush(stdout);
}

void sts_print_double(double d) {
    printf("%g", d);
    fflush(stdout);
}

void sts_print_float(float f) {
    printf("%g", (double)f);
    fflush(stdout);
}

void sts_print_char(char c) {
    putchar(c);
    fflush(stdout);
}

void sts_print_bool(int b) {
    fputs(b ? "true" : "false", stdout);
    fflush(stdout);
}
