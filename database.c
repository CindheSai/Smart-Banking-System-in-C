/* ============================================================
 * database.c — Binary file persistence implementation
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "database.h"
#include "account.h"
#include "security.h"
#include "logger.h"
#include "utils.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ============================================================
 * Internal helpers
 * ============================================================ */

static void db_ensure_data_dir(void) {
#ifdef _WIN32
    _mkdir("data");
#else
    mkdir("data", 0700);
#endif
}

static unsigned long db_header_checksum(const DBHeader *h) {
    /* Exclude the checksum field itself */
    unsigned long cs = sec_djb2_hash((const unsigned char *)h,
                                      offsetof(DBHeader, header_checksum));
    return cs;
}

/* ============================================================
 * Lifecycle
 * ============================================================ */

int db_init(void) {
    db_ensure_data_dir();
    return db_load_accounts();
}

void db_close(void) {
    db_save_accounts();
}

/* ============================================================
 * Load all accounts from binary file
 * ============================================================ */

int db_load_accounts(void) {
    FILE *fp = fopen(DB_ACCOUNTS_FILE, "rb");
    if (!fp) {
        /* First run — no file yet */
        log_system(LOG_INFO, "No accounts.dat found; starting fresh.");
        g_account_count = 0;
        return RC_OK;
    }

    DBHeader hdr;
    if (fread(&hdr, sizeof(DBHeader), 1, fp) != 1) {
        fclose(fp);
        log_system(LOG_ERROR, "Failed to read DB header.");
        return RC_ERR_IO;
    }

    /* Verify magic */
    if (hdr.magic != DB_MAGIC) {
        fclose(fp);
        log_system(LOG_CRITICAL, "DB magic mismatch — possible corruption.");
        print_error("Database file is corrupt or not a valid bank DB.");
        return RC_ERR_TAMPER;
    }

    /* Verify version */
    if (hdr.version != DB_VERSION) {
        fclose(fp);
        log_system(LOG_ERROR, "DB version mismatch.");
        return RC_ERR_GENERIC;
    }

    /* Verify header checksum */
    unsigned long expected_cs = db_header_checksum(&hdr);
    if (expected_cs != hdr.header_checksum) {
        fclose(fp);
        log_system(LOG_CRITICAL, "DB header checksum FAILED — possible tampering.");
        print_error("Database integrity check failed. Contact administrator.");
        return RC_ERR_TAMPER;
    }

    int count = hdr.record_count;
    if (count < 0 || count > MAX_ACCOUNTS) {
        fclose(fp);
        log_system(LOG_CRITICAL, "Implausible record count in DB header.");
        return RC_ERR_TAMPER;
    }

    g_account_count = 0;
    for (int i = 0; i < count; i++) {
        Account a;
        if (fread(&a, sizeof(Account), 1, fp) != 1) {
            log_system(LOG_ERROR, "Truncated read at record %d.", i);
            break;
        }
        /* Verify per-record checksum */
        unsigned long stored_cs = a.checksum;
        a.checksum = 0;
        unsigned long computed_cs;
        sec_compute_record_checksum(&a, sizeof(Account), &computed_cs);
        a.checksum = stored_cs;
        if (computed_cs != stored_cs) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Checksum mismatch on account %s — skipped.", a.account_id);
            log_system(LOG_CRITICAL, "%s", msg);
            print_warning(msg);
            continue;
        }
        g_accounts[g_account_count++] = a;
    }

    fclose(fp);
    char logmsg[64];
    snprintf(logmsg, sizeof(logmsg), "Loaded %d accounts from database.", g_account_count);
    log_system(LOG_INFO, "%s", logmsg);
    return RC_OK;
}

/* ============================================================
 * Save all accounts to binary file (atomic write)
 * ============================================================ */

int db_save_accounts(void) {
    db_ensure_data_dir();

    /* Write to temp file first, then rename (atomic) */
    const char *tmp_path = "data/accounts.tmp";

    FILE *fp = fopen(tmp_path, "wb");
    if (!fp) {
        log_system(LOG_ERROR, "Cannot open temp file for DB write.");
        return RC_ERR_IO;
    }

    DBHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic          = DB_MAGIC;
    hdr.version        = DB_VERSION;
    hdr.record_count   = g_account_count;
    hdr.created_at     = get_epoch();
    hdr.last_modified  = get_epoch();
    hdr.header_checksum = db_header_checksum(&hdr);

    if (fwrite(&hdr, sizeof(DBHeader), 1, fp) != 1) {
        fclose(fp);
        log_system(LOG_ERROR, "Failed to write DB header.");
        return RC_ERR_IO;
    }

    for (int i = 0; i < g_account_count; i++) {
        Account a = g_accounts[i];
        /* Recompute checksum before writing */
        a.checksum = 0;
        unsigned long cs;
        sec_compute_record_checksum(&a, sizeof(Account), &cs);
        a.checksum = cs;
        g_accounts[i].checksum = cs;  /* keep in-memory copy current */

        if (fwrite(&a, sizeof(Account), 1, fp) != 1) {
            fclose(fp);
            log_system(LOG_ERROR, "Failed writing account %d to DB.", i);
            return RC_ERR_IO;
        }
    }

    fclose(fp);

    /* Atomic rename */
    if (rename(tmp_path, DB_ACCOUNTS_FILE) != 0) {
        log_system(LOG_ERROR, "Atomic rename of DB file failed.");
        return RC_ERR_IO;
    }

    log_system(LOG_INFO, "Saved %d accounts to database.", g_account_count);
    return RC_OK;
}

/* ============================================================
 * Upsert a single account record
 * ============================================================ */

int db_write_account(const Account *a) {
    if (!a) return RC_ERR_INVALID;

    int idx = acct_find_index_by_id(a->account_id);
    if (idx >= 0) {
        g_accounts[idx] = *a;
    } else {
        if (g_account_count >= MAX_ACCOUNTS) return RC_ERR_LIMIT;
        g_accounts[g_account_count++] = *a;
    }
    return db_save_accounts();
}

int db_read_account(const char *id, Account *out) {
    return acct_find_by_id(id, out);
}

int db_delete_account(const char *id) {
    return acct_delete(id);
}

/* ============================================================
 * Integrity check
 * ============================================================ */

int db_verify_integrity(void) {
    int ok = 1;
    for (int i = 0; i < g_account_count; i++) {
        if (!acct_verify_checksum(&g_accounts[i])) {
            char msg[128];
            snprintf(msg, sizeof(msg),
                     "Integrity FAIL on account[%d] id=%s", i, g_accounts[i].account_id);
            log_system(LOG_CRITICAL, "%s", msg);
            print_warning(msg);
            ok = 0;
        }
    }
    return ok ? RC_OK : RC_ERR_TAMPER;
}

/* ============================================================
 * Stats
 * ============================================================ */

void db_print_stats(void) {
    print_header("DATABASE STATISTICS");
    printf("  %-30s %d\n",  "Total accounts loaded:",  g_account_count);
    printf("  %-30s %d\n",  "Active accounts:",         acct_count_active());
    printf("  %-30s INR %.2f\n", "Total deposits in system:", acct_total_deposits_all());
    printf("  %-30s INR %.2f\n", "Total withdrawals:",        acct_total_withdrawals_all());
}
