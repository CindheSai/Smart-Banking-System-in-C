/* ============================================================
 * utils.c — General-purpose utility implementations
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
  #include <conio.h>
  #define CLEAR_CMD "cls"
#else
  #include <termios.h>
  #include <unistd.h>
  #define CLEAR_CMD "clear"
#endif

/* ============================================================
 * String utilities
 * ============================================================ */

void str_trim(char *s) {
    if (!s) return;
    /* trim leading */
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    /* trim trailing */
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) {
        s[--len] = '\0';
    }
}

int str_is_numeric(const char *s) {
    if (!s || !*s) return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

int str_is_alpha(const char *s) {
    if (!s || !*s) return 0;
    while (*s) {
        if (!isalpha((unsigned char)*s) && *s != ' ') return 0;
        s++;
    }
    return 1;
}

int str_is_alphanumeric(const char *s) {
    if (!s || !*s) return 0;
    while (*s) {
        if (!isalnum((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

void str_sanitize(char *s, size_t max_len) {
    if (!s) return;
    size_t len = strlen(s);
    if (len > max_len - 1) {
        s[max_len - 1] = '\0';
        len = max_len - 1;
    }
    /* Replace control chars except space */
    for (size_t i = 0; i < len; i++) {
        if (iscntrl((unsigned char)s[i]) && s[i] != '\n') {
            s[i] = '?';
        }
    }
    str_trim(s);
}

void str_upper(char *s) {
    while (*s) { *s = (char)toupper((unsigned char)*s); s++; }
}

void str_lower(char *s) {
    while (*s) { *s = (char)tolower((unsigned char)*s); s++; }
}

/* ============================================================
 * Secure input
 * ============================================================ */

void safe_read_line(char *buf, size_t max_len) {
    if (!buf || max_len == 0) return;
    if (!fgets(buf, (int)max_len, stdin)) {
        buf[0] = '\0';
        return;
    }
    /* strip trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[--len] = '\0';
    if (len > 0 && buf[len-1] == '\r') buf[--len] = '\0';
    /* if input was truncated, flush stdin */
    if (len == max_len - 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    str_sanitize(buf, max_len);
}

void read_pin_hidden(char *pin, size_t max_len) {
#ifndef _WIN32
    struct termios old_t, new_t;
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    new_t.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

    size_t i = 0;
    int c;
    while (i < max_len - 1) {
        c = getchar();
        if (c == '\n' || c == '\r' || c == EOF) break;
        if (c == 127 || c == '\b') {   /* backspace */
            if (i > 0) { i--; printf("\b \b"); fflush(stdout); }
        } else {
            pin[i++] = (char)c;
            printf("*"); fflush(stdout);
        }
    }
    pin[i] = '\0';
    printf("\n");

    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
#else
    size_t i = 0;
    int c;
    while (i < max_len - 1) {
        c = _getch();
        if (c == '\r' || c == '\n') break;
        if (c == '\b') {
            if (i > 0) { i--; printf("\b \b"); fflush(stdout); }
        } else {
            pin[i++] = (char)c;
            printf("*"); fflush(stdout);
        }
    }
    pin[i] = '\0';
    printf("\n");
#endif
}

double read_amount(void) {
    char buf[64];
    safe_read_line(buf, sizeof(buf));
    char *end = NULL;
    double v = strtod(buf, &end);
    if (end == buf || *end != '\0') return -1.0;
    return v;
}

int read_int(int min, int max) {
    char buf[32];
    safe_read_line(buf, sizeof(buf));
    if (!str_is_numeric(buf) && !(buf[0] == '-' && str_is_numeric(buf+1))) return min - 1;
    int v = atoi(buf);
    if (v < min || v > max) return min - 1;
    return v;
}

/* ============================================================
 * Time utilities
 * ============================================================ */

void get_timestamp(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

long get_epoch(void) {
    return (long)time(NULL);
}

/* ============================================================
 * UI helpers
 * ============================================================ */

void print_banner(void) {
    printf(CLR_CYAN);
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════════╗\n");
    printf("  ║         SECURBANK — CORE BANKING SYSTEM v1.0        ║\n");
    printf("  ║      Bank-Grade Financial Management Platform        ║\n");
    printf("  ╚══════════════════════════════════════════════════════╝\n");
    printf(CLR_RESET "\n");
}

void print_separator(void) {
    printf(CLR_BLUE "  ──────────────────────────────────────────────────────\n" CLR_RESET);
}

void print_header(const char *title) {
    printf(CLR_BOLD CLR_BLUE);
    printf("\n  ┌─────────────────────────────────────────────────────┐\n");
    printf("  │  %-51s  │\n", title);
    printf("  └─────────────────────────────────────────────────────┘\n");
    printf(CLR_RESET);
}

void print_success(const char *msg) {
    printf(CLR_GREEN "  ✔  %s\n" CLR_RESET, msg);
}

void print_error(const char *msg) {
    printf(CLR_RED "  ✘  ERROR: %s\n" CLR_RESET, msg);
}

void print_warning(const char *msg) {
    printf(CLR_YELLOW "  ⚠  WARNING: %s\n" CLR_RESET, msg);
}

void print_info(const char *msg) {
    printf(CLR_CYAN "  ℹ  %s\n" CLR_RESET, msg);
}

void clear_screen(void) {
    system(CLEAR_CMD);
}

void pause_prompt(void) {
    printf(CLR_YELLOW "\n  Press ENTER to continue..." CLR_RESET);
    getchar();
}

int confirm_action(const char *msg) {
    printf(CLR_YELLOW "  ⚠  %s [y/N]: " CLR_RESET, msg);
    char buf[8];
    safe_read_line(buf, sizeof(buf));
    str_lower(buf);
    return (strcmp(buf, "y") == 0 || strcmp(buf, "yes") == 0);
}

/* ============================================================
 * Math
 * ============================================================ */

double round2(double val) {
    return floor(val * 100.0 + 0.5) / 100.0;
}
