/* ============================================================
 * transaction.c — Append-only transaction ledger implementation
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "transaction.h"
#include "security.h"
#include "logger.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * TXN ID generator
 * ============================================================ */

void txn_generate_id(char *out) {
    static unsigned long seq = 10000;
    char ts[TIMESTAMP_LEN];
    get_timestamp(ts, sizeof(ts));
    /* TXN-YYYYMMDD-seq */
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    snprintf(out, 20, "T%04d%02d%02d%05lu",
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             seq++);
}

/* ============================================================
 * Record a transaction (append-only to binary log)
 * ============================================================ */

int txn_record(const char *account_id, const char *counterpart_id,
               TxnType type, TxnStatus status,
               double amount, double balance_after, const char *note) {

    Transaction txn;
    memset(&txn, 0, sizeof(txn));

    txn_generate_id(txn.txn_id);
    strncpy(txn.account_id, account_id, sizeof(txn.account_id) - 1);
    if (counterpart_id && *counterpart_id)
        strncpy(txn.counterpart_id, counterpart_id, sizeof(txn.counterpart_id) - 1);

    txn.type          = type;
    txn.status        = status;
    txn.amount        = amount;
    txn.balance_after = balance_after;
    txn.epoch         = get_epoch();
    get_timestamp(txn.timestamp, sizeof(txn.timestamp));
    if (note) strncpy(txn.note, note, sizeof(txn.note) - 1);

    /* Compute record checksum */
    txn.checksum = 0;
    sec_compute_record_checksum(&txn, sizeof(Transaction), &txn.checksum);

    /* Append to binary log */
    FILE *fp = fopen(TXN_LOG_FILE, "ab");
    if (!fp) {
        log_system(LOG_ERROR, "Cannot open transaction log for append.");
        return RC_ERR_IO;
    }
    fwrite(&txn, sizeof(Transaction), 1, fp);
    fclose(fp);

    return RC_OK;
}

/* ============================================================
 * Read transactions for an account
 * ============================================================ */

int txn_get_all_for_account(const char *account_id,
                             Transaction out[], int max, int *found) {
    if (!account_id || !out || !found) return RC_ERR_INVALID;
    *found = 0;

    FILE *fp = fopen(TXN_LOG_FILE, "rb");
    if (!fp) return RC_OK;  /* no log yet */

    Transaction t;
    while (fread(&t, sizeof(Transaction), 1, fp) == 1 && *found < max) {
        if (strcmp(t.account_id, account_id) == 0) {
            out[(*found)++] = t;
        }
    }
    fclose(fp);
    return RC_OK;
}

int txn_get_mini_statement(const char *account_id,
                            Transaction out[], int max, int *found) {
    if (!account_id || !out || !found) return RC_ERR_INVALID;
    *found = 0;

    /* Load all, then return last `max` */
    int buf_size = 2048;
    Transaction *buf = calloc((size_t)buf_size, sizeof(Transaction));
    if (!buf) return RC_ERR_GENERIC;

    int total = 0;
    FILE *fp = fopen(TXN_LOG_FILE, "rb");
    if (fp) {
        Transaction t;
        while (fread(&t, sizeof(Transaction), 1, fp) == 1) {
            if (strcmp(t.account_id, account_id) == 0) {
                if (total < buf_size) buf[total++] = t;
                else {
                    /* Shift window */
                    memmove(buf, buf + 1, (size_t)(buf_size - 1) * sizeof(Transaction));
                    buf[buf_size - 1] = t;
                    total = buf_size;
                }
            }
        }
        fclose(fp);
    }

    int start = (total > max) ? total - max : 0;
    for (int i = start; i < total && *found < max; i++) {
        out[(*found)++] = buf[i];
    }
    free(buf);
    return RC_OK;
}

/* ============================================================
 * Display
 * ============================================================ */

void txn_print_header(void) {
    printf(CLR_BOLD CLR_CYAN);
    printf("  %-18s %-12s %-12s %-12s %-16s %s\n",
           "TXN ID", "Date", "Type", "Status", "Amount", "Note");
    printf("  %-18s %-12s %-12s %-12s %-16s %s\n",
           "──────────────────", "──────────", "──────────",
           "──────────", "──────────────", "──────────");
    printf(CLR_RESET);
}

void txn_print_row(const Transaction *t) {
    char date[16];
    strncpy(date, t->timestamp, 10);
    date[10] = '\0';

    const char *clr = CLR_GREEN;
    if (t->type == TXN_WITHDRAW || t->type == TXN_FEE) clr = CLR_RED;
    if (t->type == TXN_TRANSFER) {
        /* Debit or credit depends on context — display neutral */
        clr = CLR_YELLOW;
    }

    printf("  %-18s %-12s %-12s %-12s %s%-16.2f" CLR_RESET " %s\n",
           t->txn_id,
           date,
           txn_type_str(t->type),
           txn_status_str(t->status),
           clr,
           t->amount,
           t->note);
}

void txn_print_mini_statement(const char *account_id) {
    Transaction stmt[MINI_STMT_LIMIT];
    int found = 0;
    txn_get_mini_statement(account_id, stmt, MINI_STMT_LIMIT, &found);

    print_header("MINI STATEMENT — LAST 10 TRANSACTIONS");

    if (found == 0) {
        print_info("No transactions found for this account.");
        return;
    }
    txn_print_header();
    for (int i = 0; i < found; i++) {
        txn_print_row(&stmt[i]);
    }
    printf("\n");
}

/* ============================================================
 * Analytics
 * ============================================================ */

double txn_sum_by_type(const char *account_id, TxnType type) {
    double sum = 0.0;
    FILE *fp = fopen(TXN_LOG_FILE, "rb");
    if (!fp) return 0.0;
    Transaction t;
    while (fread(&t, sizeof(Transaction), 1, fp) == 1) {
        if (strcmp(t.account_id, account_id) == 0 &&
            t.type == type &&
            t.status == TXN_STATUS_SUCCESS) {
            sum += t.amount;
        }
    }
    fclose(fp);
    return sum;
}

/* ============================================================
 * String helpers
 * ============================================================ */

const char *txn_type_str(TxnType t) {
    switch (t) {
        case TXN_DEPOSIT:  return "Deposit";
        case TXN_WITHDRAW: return "Withdraw";
        case TXN_TRANSFER: return "Transfer";
        case TXN_INTEREST: return "Interest";
        case TXN_FEE:      return "Fee";
        case TXN_REVERSAL: return "Reversal";
        default:           return "Unknown";
    }
}

const char *txn_status_str(TxnStatus s) {
    switch (s) {
        case TXN_STATUS_SUCCESS: return "SUCCESS";
        case TXN_STATUS_FAILED:  return "FAILED";
        case TXN_STATUS_PENDING: return "PENDING";
        default:                 return "UNKNOWN";
    }
}
