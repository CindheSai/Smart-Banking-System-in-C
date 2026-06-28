#ifndef DATABASE_H
#define DATABASE_H

/* ============================================================
 * database.h — Binary file persistence interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "account.h"

#define DB_ACCOUNTS_FILE   "data/accounts.dat"
#define DB_MAGIC           0x42414E4B   /* "BANK" */
#define DB_VERSION         1

/* File header written at the start of accounts.dat */
typedef struct {
    unsigned int  magic;
    unsigned int  version;
    int           record_count;
    long          created_at;
    long          last_modified;
    unsigned long header_checksum;
} DBHeader;

/* ---- Lifecycle ---- */
int  db_init(void);
void db_close(void);

/* ---- Load / Save ---- */
int  db_load_accounts(void);
int  db_save_accounts(void);

/* ---- Single record I/O ---- */
int  db_write_account(const Account *a);    /* upsert by account_id */
int  db_read_account(const char *id, Account *out);
int  db_delete_account(const char *id);

/* ---- Integrity ---- */
int  db_verify_integrity(void);

/* ---- Stats ---- */
void db_print_stats(void);

#endif /* DATABASE_H */
