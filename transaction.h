#ifndef TRANSACTION_H
#define TRANSACTION_H

/* ============================================================
 * transaction.h — Transaction ledger interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "utils.h"

#define TXN_LOG_FILE      "data/transactions.log"
#define MINI_STMT_LIMIT   10

typedef enum {
    TXN_DEPOSIT    = 1,
    TXN_WITHDRAW   = 2,
    TXN_TRANSFER   = 3,
    TXN_INTEREST   = 4,
    TXN_FEE        = 5,
    TXN_REVERSAL   = 6
} TxnType;

typedef enum {
    TXN_STATUS_SUCCESS = 0,
    TXN_STATUS_FAILED  = 1,
    TXN_STATUS_PENDING = 2
} TxnStatus;

/* ---- Immutable transaction record ---- */
typedef struct {
    char        txn_id[20];
    char        account_id[ACCOUNT_ID_LEN];
    char        counterpart_id[ACCOUNT_ID_LEN]; /* for transfers */
    TxnType     type;
    TxnStatus   status;
    double      amount;
    double      balance_after;
    char        note[128];
    char        timestamp[TIMESTAMP_LEN];
    long        epoch;
    unsigned long checksum;
} Transaction;

/* ---- Ledger API ---- */
int  txn_record(const char *account_id, const char *counterpart_id,
                TxnType type, TxnStatus status,
                double amount, double balance_after, const char *note);

int  txn_get_mini_statement(const char *account_id,
                             Transaction out[], int max, int *found);

int  txn_get_all_for_account(const char *account_id,
                              Transaction out[], int max, int *found);

/* ---- Display ---- */
void txn_print_header(void);
void txn_print_row(const Transaction *t);
void txn_print_mini_statement(const char *account_id);

/* ---- Analytics ---- */
double txn_sum_by_type(const char *account_id, TxnType type);

/* ---- TxnType helper ---- */
const char *txn_type_str(TxnType t);
const char *txn_status_str(TxnStatus s);
void        txn_generate_id(char *out);

#endif /* TRANSACTION_H */
