/* ============================================================
 * account.c — Account business logic implementation
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "account.h"
#include "security.h"
#include "logger.h"
#include "transaction.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * Global state
 * ============================================================ */

Account g_accounts[MAX_ACCOUNTS];
int     g_account_count = 0;

/* ============================================================
 * ID generation
 * ============================================================ */

void acct_generate_id(char *out) {
    /* Format: SBKxxxxxxxn — epoch low bits + sequence */
    static unsigned int seq = 0;
    unsigned long epoch_low = (unsigned long)time(NULL) % 100000UL;
    unsigned int s = (seq++) % 99U;
    /* Guaranteed <= 11 chars: "SBK" + 5 + 2 + NUL = 11 */
    out[0]  = 'S'; out[1]  = 'B'; out[2]  = 'K';
    out[3]  = (char)('0' + (epoch_low / 10000) % 10);
    out[4]  = (char)('0' + (epoch_low / 1000)  % 10);
    out[5]  = (char)('0' + (epoch_low / 100)   % 10);
    out[6]  = (char)('0' + (epoch_low / 10)    % 10);
    out[7]  = (char)('0' + (epoch_low)         % 10);
    out[8]  = (char)('0' + (s / 10)            % 10);
    out[9]  = (char)('0' + s % 10);
    out[10] = (char)0x00;
}

/* ============================================================
 * CRUD
 * ============================================================ */

int acct_create(Account *a) {
    if (!a) return RC_ERR_INVALID;
    if (g_account_count >= MAX_ACCOUNTS) return RC_ERR_LIMIT;
    if (acct_find_index_by_id(a->account_id) >= 0) return RC_ERR_DUPLICATE;

    a->last_modified = get_epoch();
    a->created_at    = get_epoch();
    acct_recompute_checksum(a);
    g_accounts[g_account_count++] = *a;
    log_audit("SYSTEM", "ACCOUNT_CREATED", a->account_id, a->full_name, LOG_AUDIT);
    return RC_OK;
}

int acct_find_by_id(const char *id, Account *out) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    if (out) *out = g_accounts[idx];
    return RC_OK;
}

int acct_find_index_by_id(const char *id) {
    if (!id) return -1;
    for (int i = 0; i < g_account_count; i++) {
        if (strcmp(g_accounts[i].account_id, id) == 0) return i;
    }
    return -1;
}

int acct_find_by_name(const char *name, Account results[], int max, int *found) {
    if (!name || !results || !found) return RC_ERR_INVALID;
    *found = 0;
    char lname[NAME_LEN];
    strncpy(lname, name, sizeof(lname) - 1);
    lname[sizeof(lname) - 1] = '\0';
    str_lower(lname);

    for (int i = 0; i < g_account_count && *found < max; i++) {
        char cname[NAME_LEN];
        strncpy(cname, g_accounts[i].full_name, sizeof(cname) - 1);
        cname[sizeof(cname) - 1] = '\0';
        str_lower(cname);
        if (strstr(cname, lname)) {
            results[(*found)++] = g_accounts[i];
        }
    }
    return RC_OK;
}

int acct_update(const Account *a) {
    if (!a) return RC_ERR_INVALID;
    int idx = acct_find_index_by_id(a->account_id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    Account upd = *a;
    upd.last_modified = get_epoch();
    acct_recompute_checksum(&upd);
    g_accounts[idx] = upd;
    return RC_OK;
}

int acct_delete(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    log_audit("ADMIN", "ACCOUNT_DELETED", g_accounts[idx].account_id,
              g_accounts[idx].full_name, LOG_AUDIT);
    /* Shift array */
    for (int i = idx; i < g_account_count - 1; i++) {
        g_accounts[i] = g_accounts[i + 1];
    }
    g_account_count--;
    return RC_OK;
}

/* ============================================================
 * Business logic — Deposit / Withdraw / Transfer
 * ============================================================ */

int acct_deposit(const char *id, double amount, const char *note) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;

    Account *a = &g_accounts[idx];
    if (a->status == ACCT_FROZEN) return RC_ERR_FROZEN;
    if (a->status == ACCT_CLOSED) return RC_ERR_INVALID;
    if (!sec_validate_amount(amount)) return RC_ERR_INVALID;

    acct_reset_daily_if_needed(a);
    if (acct_check_daily_limit(a, amount) != RC_OK) return RC_ERR_LIMIT;

    a->balance         = round2(a->balance + amount);
    a->total_deposited = round2(a->total_deposited + amount);
    a->daily_txn_total = round2(a->daily_txn_total + amount);
    a->last_modified   = get_epoch();
    acct_recompute_checksum(a);

    txn_record(id, "", TXN_DEPOSIT, TXN_STATUS_SUCCESS,
               amount, a->balance, note ? note : "Deposit");
    log_audit(id, "DEPOSIT", id, note ? note : "", LOG_AUDIT);
    return RC_OK;
}

int acct_withdraw(const char *id, double amount, const char *note) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;

    Account *a = &g_accounts[idx];
    if (a->status == ACCT_FROZEN) return RC_ERR_FROZEN;
    if (a->status == ACCT_CLOSED) return RC_ERR_INVALID;
    if (!sec_validate_amount(amount)) return RC_ERR_INVALID;

    acct_reset_daily_if_needed(a);
    if (acct_check_daily_limit(a, amount) != RC_OK) return RC_ERR_LIMIT;

    if (a->balance < amount) return RC_ERR_BALANCE;

    a->balance          = round2(a->balance - amount);
    a->total_withdrawn  = round2(a->total_withdrawn + amount);
    a->daily_txn_total  = round2(a->daily_txn_total + amount);
    a->last_modified    = get_epoch();
    acct_recompute_checksum(a);

    txn_record(id, "", TXN_WITHDRAW, TXN_STATUS_SUCCESS,
               amount, a->balance, note ? note : "Withdrawal");
    log_audit(id, "WITHDRAWAL", id, note ? note : "", LOG_AUDIT);
    return RC_OK;
}

int acct_transfer(const char *from_id, const char *to_id, double amount) {
    if (!from_id || !to_id || strcmp(from_id, to_id) == 0) return RC_ERR_INVALID;

    int fi = acct_find_index_by_id(from_id);
    int ti = acct_find_index_by_id(to_id);
    if (fi < 0 || ti < 0) return RC_ERR_NOT_FOUND;

    Account *fa = &g_accounts[fi];
    Account *ta = &g_accounts[ti];

    if (fa->status == ACCT_FROZEN) return RC_ERR_FROZEN;
    if (ta->status == ACCT_FROZEN) return RC_ERR_FROZEN;
    if (fa->status == ACCT_CLOSED || ta->status == ACCT_CLOSED) return RC_ERR_INVALID;
    if (!sec_validate_amount(amount)) return RC_ERR_INVALID;

    acct_reset_daily_if_needed(fa);
    if (acct_check_daily_limit(fa, amount) != RC_OK) return RC_ERR_LIMIT;
    if (fa->balance < amount) return RC_ERR_BALANCE;

    fa->balance         = round2(fa->balance - amount);
    fa->total_withdrawn = round2(fa->total_withdrawn + amount);
    fa->daily_txn_total = round2(fa->daily_txn_total + amount);
    fa->last_modified   = get_epoch();
    acct_recompute_checksum(fa);

    ta->balance         = round2(ta->balance + amount);
    ta->total_deposited = round2(ta->total_deposited + amount);
    ta->last_modified   = get_epoch();
    acct_recompute_checksum(ta);

    char detail[128];
    snprintf(detail, sizeof(detail), "Transfer to %s", to_id);
    txn_record(from_id, to_id, TXN_TRANSFER, TXN_STATUS_SUCCESS,
               amount, fa->balance, detail);

    snprintf(detail, sizeof(detail), "Transfer from %s", from_id);
    txn_record(to_id, from_id, TXN_TRANSFER, TXN_STATUS_SUCCESS,
               amount, ta->balance, detail);

    log_audit(from_id, "TRANSFER_OUT", to_id, "", LOG_AUDIT);
    log_audit(to_id,   "TRANSFER_IN",  from_id, "", LOG_AUDIT);
    return RC_OK;
}

/* ============================================================
 * Interest calculation
 * ============================================================ */

int acct_apply_interest(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;

    Account *a = &g_accounts[idx];
    if (a->status != ACCT_ACTIVE) return RC_ERR_FROZEN;
    if (a->type == ACCT_CURRENT) return RC_OK; /* no interest on current */

    double rate = (a->type == ACCT_SAVINGS) ? INTEREST_SAVINGS : INTEREST_SALARY;
    double monthly_interest = round2(a->balance * (rate / 100.0) / 12.0);
    if (monthly_interest <= 0.0) return RC_OK;

    a->balance         = round2(a->balance + monthly_interest);
    a->total_deposited = round2(a->total_deposited + monthly_interest);
    a->last_modified   = get_epoch();
    acct_recompute_checksum(a);

    char note[64];
    snprintf(note, sizeof(note), "Monthly interest @%.2f%%p.a.", rate);
    txn_record(id, "", TXN_INTEREST, TXN_STATUS_SUCCESS,
               monthly_interest, a->balance, note);
    log_audit(id, "INTEREST_APPLIED", id, note, LOG_AUDIT);
    return RC_OK;
}

int acct_apply_interest_all(void) {
    int applied = 0;
    for (int i = 0; i < g_account_count; i++) {
        if (g_accounts[i].status == ACCT_ACTIVE &&
            g_accounts[i].type != ACCT_CURRENT) {
            acct_apply_interest(g_accounts[i].account_id);
            applied++;
        }
    }
    return applied;
}

/* ============================================================
 * Status management
 * ============================================================ */

int acct_freeze(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    if (g_accounts[idx].is_admin) return RC_ERR_INVALID;
    g_accounts[idx].status = ACCT_FROZEN;
    g_accounts[idx].last_modified = get_epoch();
    acct_recompute_checksum(&g_accounts[idx]);
    log_audit("ADMIN", "ACCOUNT_FROZEN", id, "", LOG_AUDIT);
    return RC_OK;
}

int acct_unfreeze(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    g_accounts[idx].status = ACCT_ACTIVE;
    g_accounts[idx].last_modified = get_epoch();
    acct_recompute_checksum(&g_accounts[idx]);
    log_audit("ADMIN", "ACCOUNT_UNFROZEN", id, "", LOG_AUDIT);
    return RC_OK;
}

int acct_lock(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    g_accounts[idx].status = ACCT_LOCKED;
    g_accounts[idx].last_modified = get_epoch();
    acct_recompute_checksum(&g_accounts[idx]);
    log_audit("SYSTEM", "ACCOUNT_LOCKED_PIN", id, "3 failed attempts", LOG_AUDIT);
    return RC_OK;
}

int acct_unlock(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    g_accounts[idx].status = ACCT_ACTIVE;
    g_accounts[idx].failed_pin_attempts = 0;
    g_accounts[idx].last_modified = get_epoch();
    acct_recompute_checksum(&g_accounts[idx]);
    log_audit("ADMIN", "ACCOUNT_UNLOCKED", id, "", LOG_AUDIT);
    return RC_OK;
}

int acct_increment_failed_pin(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    g_accounts[idx].failed_pin_attempts++;
    if (g_accounts[idx].failed_pin_attempts >= 3) {
        acct_lock(id);
    }
    acct_recompute_checksum(&g_accounts[idx]);
    return RC_OK;
}

int acct_reset_failed_pin(const char *id) {
    int idx = acct_find_index_by_id(id);
    if (idx < 0) return RC_ERR_NOT_FOUND;
    g_accounts[idx].failed_pin_attempts = 0;
    acct_recompute_checksum(&g_accounts[idx]);
    return RC_OK;
}

/* ============================================================
 * Daily limit
 * ============================================================ */

int acct_check_daily_limit(const Account *a, double amount) {
    double limit;
    switch (a->type) {
        case ACCT_SAVINGS:  limit = DAILY_LIMIT_SAVINGS;  break;
        case ACCT_CURRENT:  limit = DAILY_LIMIT_CURRENT;  break;
        case ACCT_SALARY:   limit = DAILY_LIMIT_SALARY;   break;
        default:            limit = DAILY_LIMIT_SAVINGS;  break;
    }
    if (a->daily_txn_total + amount > limit) return RC_ERR_LIMIT;
    return RC_OK;
}

void acct_reset_daily_if_needed(Account *a) {
    char today[16];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(today, sizeof(today), "%Y-%m-%d", tm_info);
    if (strcmp(a->daily_txn_date, today) != 0) {
        a->daily_txn_total = 0.0;
        memcpy(a->daily_txn_date, today, sizeof(a->daily_txn_date) - 1);
        a->daily_txn_date[sizeof(a->daily_txn_date) - 1] = '\0';
    }
}

/* ============================================================
 * Validation & checksum
 * ============================================================ */

int acct_validate(const Account *a) {
    if (!a) return 0;
    if (strlen(a->account_id) < 6) return 0;
    if (strlen(a->full_name) < 2) return 0;
    if (a->balance < 0.0) return 0;
    if (a->type < ACCT_SAVINGS || a->type > ACCT_SALARY) return 0;
    return 1;
}

void acct_recompute_checksum(Account *a) {
    unsigned long tmp = a->checksum;
    a->checksum = 0;
    unsigned long cs;
    sec_compute_record_checksum(a, sizeof(Account), &cs);
    a->checksum = cs;
    (void)tmp;
}

int acct_verify_checksum(const Account *a) {
    Account tmp = *a;
    tmp.checksum = 0;
    unsigned long cs;
    sec_compute_record_checksum(&tmp, sizeof(Account), &cs);
    return (cs == a->checksum);
}

/* ============================================================
 * Display
 * ============================================================ */

void acct_print_summary(const Account *a) {
    printf("\n");
    printf(CLR_CYAN "  Account ID   : " CLR_WHITE "%s\n" CLR_RESET, a->account_id);
    printf(CLR_CYAN "  Name         : " CLR_WHITE "%s\n" CLR_RESET, a->full_name);
    printf(CLR_CYAN "  Type         : " CLR_WHITE "%s\n" CLR_RESET, acct_type_str(a->type));
    printf(CLR_CYAN "  Status       : ");
    if (a->status == ACCT_ACTIVE)  printf(CLR_GREEN  "ACTIVE\n"  CLR_RESET);
    else if (a->status == ACCT_FROZEN) printf(CLR_YELLOW "FROZEN\n" CLR_RESET);
    else if (a->status == ACCT_LOCKED) printf(CLR_RED    "LOCKED\n" CLR_RESET);
    else                               printf(CLR_RED     "CLOSED\n" CLR_RESET);
    printf(CLR_CYAN "  Balance      : " CLR_GREEN "INR %.2f\n" CLR_RESET, a->balance);
}

void acct_print_full(const Account *a) {
    char ts[TIMESTAMP_LEN];
    time_t t;

    print_header("ACCOUNT DETAILS");
    printf("\n");
    printf("  %-22s %s\n",   "Account ID:",     a->account_id);
    printf("  %-22s %s\n",   "Full Name:",       a->full_name);
    printf("  %-22s %s\n",   "Phone:",           a->phone);
    printf("  %-22s %s\n",   "Email:",           a->email);
    printf("  %-22s %s\n",   "Address:",         a->address);
    printf("  %-22s %s\n",   "Account Type:",    acct_type_str(a->type));
    printf("  %-22s %s\n",   "Status:",          acct_status_str(a->status));
    printf("  %-22s " CLR_GREEN "INR %.2f\n" CLR_RESET, "Balance:", a->balance);
    printf("  %-22s INR %.2f\n", "Daily Used:",  a->daily_txn_total);
    printf("  %-22s INR %.2f\n", "Total Deposited:", a->total_deposited);
    printf("  %-22s INR %.2f\n", "Total Withdrawn:", a->total_withdrawn);

    t = (time_t)a->created_at;
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&t));
    printf("  %-22s %s\n",   "Created:",         ts);

    if (a->last_login > 0) {
        t = (time_t)a->last_login;
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&t));
        printf("  %-22s %s\n", "Last Login:", ts);
    }
    printf("\n");
}

void acct_print_table_header(void) {
    printf(CLR_BOLD CLR_CYAN);
    printf("  %-12s %-20s %-10s %-12s %-12s %s\n",
           "Account ID", "Name", "Type", "Status", "Balance", "Phone");
    printf("  %-12s %-20s %-10s %-12s %-12s %s\n",
           "──────────", "──────────────────", "────────", "──────────",
           "──────────", "───────────");
    printf(CLR_RESET);
}

void acct_print_table_row(const Account *a) {
    const char *status_clr = CLR_GREEN;
    if (a->status == ACCT_FROZEN) status_clr = CLR_YELLOW;
    else if (a->status == ACCT_LOCKED || a->status == ACCT_CLOSED) status_clr = CLR_RED;

    printf("  %-12s %-20s %-10s %s%-12s" CLR_RESET " INR %-10.2f %s\n",
           a->account_id,
           a->full_name,
           acct_type_str(a->type),
           status_clr,
           acct_status_str(a->status),
           a->balance,
           a->phone);
}

/* ============================================================
 * Sorting (selection sort)
 * ============================================================ */

void acct_sort_by_balance(Account arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int max_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j].balance > arr[max_idx].balance) max_idx = j;
        }
        if (max_idx != i) {
            Account tmp = arr[i];
            arr[i] = arr[max_idx];
            arr[max_idx] = tmp;
        }
    }
}

/* ============================================================
 * Analytics
 * ============================================================ */

double acct_total_deposits_all(void) {
    double total = 0.0;
    for (int i = 0; i < g_account_count; i++) total += g_accounts[i].total_deposited;
    return total;
}

double acct_total_withdrawals_all(void) {
    double total = 0.0;
    for (int i = 0; i < g_account_count; i++) total += g_accounts[i].total_withdrawn;
    return total;
}

int acct_count_active(void) {
    int count = 0;
    for (int i = 0; i < g_account_count; i++) {
        if (g_accounts[i].status == ACCT_ACTIVE) count++;
    }
    return count;
}

/* ============================================================
 * String helpers
 * ============================================================ */

const char *acct_type_str(AccountType t) {
    switch (t) {
        case ACCT_SAVINGS: return "Savings";
        case ACCT_CURRENT: return "Current";
        case ACCT_SALARY:  return "Salary";
        default:           return "Unknown";
    }
}

const char *acct_status_str(AccountStatus s) {
    switch (s) {
        case ACCT_ACTIVE: return "ACTIVE";
        case ACCT_FROZEN: return "FROZEN";
        case ACCT_CLOSED: return "CLOSED";
        case ACCT_LOCKED: return "LOCKED";
        default:          return "UNKNOWN";
    }
}
