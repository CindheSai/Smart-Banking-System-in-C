#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <time.h>

/* ============================================================
 * utils.h — General-purpose utility interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#define MAX_INPUT_LEN       256
#define ACCOUNT_ID_LEN      12
#define NAME_LEN            64
#define PIN_LEN             6
#define HASH_LEN            65
#define SALT_LEN            17
#define PHONE_LEN           16
#define EMAIL_LEN           64
#define ADDRESS_LEN         128
#define TIMESTAMP_LEN       32

/* ANSI color codes for terminal UI */
#define CLR_RESET   "\033[0m"
#define CLR_RED     "\033[1;31m"
#define CLR_GREEN   "\033[1;32m"
#define CLR_YELLOW  "\033[1;33m"
#define CLR_BLUE    "\033[1;34m"
#define CLR_CYAN    "\033[1;36m"
#define CLR_WHITE   "\033[1;37m"
#define CLR_BOLD    "\033[1m"

/* Return codes */
typedef enum {
    RC_OK               =  0,
    RC_ERR_GENERIC      = -1,
    RC_ERR_NOT_FOUND    = -2,
    RC_ERR_AUTH         = -3,
    RC_ERR_LOCKED       = -4,
    RC_ERR_DUPLICATE    = -5,
    RC_ERR_LIMIT        = -6,
    RC_ERR_FROZEN       = -7,
    RC_ERR_INVALID      = -8,
    RC_ERR_IO           = -9,
    RC_ERR_BALANCE      = -10,
    RC_ERR_TAMPER       = -11
} ReturnCode;

/* ---- String utilities ---- */
void   str_trim(char *s);
int    str_is_numeric(const char *s);
int    str_is_alpha(const char *s);
int    str_is_alphanumeric(const char *s);
void   str_sanitize(char *s, size_t max_len);
void   str_upper(char *s);
void   str_lower(char *s);

/* ---- Secure input ---- */
void   safe_read_line(char *buf, size_t max_len);
void   read_pin_hidden(char *pin, size_t max_len);
double read_amount(void);
int    read_int(int min, int max);

/* ---- Time utilities ---- */
void   get_timestamp(char *buf, size_t len);
long   get_epoch(void);

/* ---- UI helpers ---- */
void   print_banner(void);
void   print_separator(void);
void   print_header(const char *title);
void   print_success(const char *msg);
void   print_error(const char *msg);
void   print_warning(const char *msg);
void   print_info(const char *msg);
void   clear_screen(void);
void   pause_prompt(void);
int    confirm_action(const char *msg);

/* ---- Math ---- */
double round2(double val);

#endif /* UTILS_H */
