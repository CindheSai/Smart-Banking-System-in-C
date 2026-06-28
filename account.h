#ifndef ACCOUNT_H
#define ACCOUNT_H

/* ============================================================
 * account.h — Account data model & interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "utils.h"
#include <time.h>

/* ---- Account type ---- */
typedef enum {
    ACCT_SAVINGS  = 1,
    ACCT_CURRENT  = 2,
    ACCT_SALARY   = 3
} AccountType;

/* ---- Account status ---- */
typedef enum {
    ACCT_ACTIVE   = 0,
    ACCT_FROZEN   = 1,
    ACCT_CLOSED   = 2,
    ACCT_LOCKED   = 3    /* PIN lockout */
} AccountStatus;

/* Interest rates (annual %) */
#define INTEREST_SAVINGS   3.5
#define INTEREST_CURRENT   0.0
#define INTEREST_SALARY    4.0

/* Transaction limits */
#define DAILY_LIMIT_SAVINGS   50000.0
#define DAILY_LIMIT_CURRENT  200000.0
#define DAILY_LIMIT_SALARY   100000.0
#define MIN_DEPOSIT            1.0
#define MAX_SINGLE_TXN      500000.0

#define MAX_ACCOUNTS         1024
#define ADMIN_ID             "ADMIN001"

/* ---- Core account record (binary-serializable) ---- */
typedef struct {
    char            account_id[ACCOUNT_ID_LEN];
    char            full_name[NAME_LEN];
    char            phone[PHONE_LEN];
    char            email[EMAIL_LEN];
    char            address[ADDRESS_LEN];

    AccountType     type;
    AccountStatus   status;

    double          balance;
    double          daily_txn_total;     /* running total for today */
    char            daily_txn_date[16];  /* YYYY-MM-DD of last txn */

    char            pin_hash[HASH_LEN];
    char            pin_salt[SALT_LEN];

    int             failed_pin_attempts;
    long            created_at;          /* epoch */
    long            last_login;          /* epoch */
    long            last_modified;       /* epoch */

    double          total_deposited;
    double          total_withdrawn;

    int             is_admin;
    unsigned long   checksum;            /* anti-tamper field */
} Account;

/* ---- In-memory array (loaded from DB) ---- */
extern Account  g_accounts[MAX_ACCOUNTS];
extern int      g_account_count;

/* ---- Account CRUD ---- */
int    acct_create(Account *a);
int    acct_find_by_id(const char *id, Account *out);
int    acct_find_index_by_id(const char *id);
int    acct_find_by_name(const char *name, Account results[], int max, int *found);
int    acct_update(const Account *a);
int    acct_delete(const char *id);

/* ---- Business logic ---- */
int    acct_deposit(const char *id, double amount, const char *note);
int    acct_withdraw(const char *id, double amount, const char *note);
int    acct_transfer(const char *from_id, const char *to_id, double amount);
int    acct_apply_interest(const char *id);
int    acct_apply_interest_all(void);

/* ---- Status management ---- */
int    acct_freeze(const char *id);
int    acct_unfreeze(const char *id);
int    acct_lock(const char *id);
int    acct_unlock(const char *id);
int    acct_increment_failed_pin(const char *id);
int    acct_reset_failed_pin(const char *id);

/* ---- Daily limit ---- */
int    acct_check_daily_limit(const Account *a, double amount);
void   acct_reset_daily_if_needed(Account *a);

/* ---- Validation ---- */
int    acct_validate(const Account *a);
void   acct_recompute_checksum(Account *a);
int    acct_verify_checksum(const Account *a);

/* ---- Display ---- */
void   acct_print_summary(const Account *a);
void   acct_print_full(const Account *a);
void   acct_print_table_header(void);
void   acct_print_table_row(const Account *a);

/* ---- Sorting / analytics ---- */
void   acct_sort_by_balance(Account arr[], int n);
double acct_total_deposits_all(void);
double acct_total_withdrawals_all(void);
int    acct_count_active(void);

/* ---- Type helpers ---- */
const char *acct_type_str(AccountType t);
const char *acct_status_str(AccountStatus s);
void        acct_generate_id(char *out);

#endif /* ACCOUNT_H */
