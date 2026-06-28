#ifndef LOGGER_H
#define LOGGER_H

/* ============================================================
 * logger.h — Audit & transaction logging interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "utils.h"

#define LOG_AUDIT_FILE        "data/audit.log"
#define LOG_TRANSACTION_FILE  "data/transactions.log"
#define LOG_SYSTEM_FILE       "data/system.log"

typedef enum {
    LOG_INFO    = 0,
    LOG_WARNING = 1,
    LOG_ERROR   = 2,
    LOG_CRITICAL= 3,
    LOG_AUDIT   = 4
} LogLevel;

/* Structured audit log entry */
typedef struct {
    char        timestamp[TIMESTAMP_LEN];
    char        actor[NAME_LEN];          /* who performed the action */
    char        action[128];              /* what was done */
    char        target[ACCOUNT_ID_LEN];   /* affected account */
    char        detail[256];              /* extra detail */
    LogLevel    level;
} AuditEntry;

/* ---- Logging API ---- */
int  logger_init(void);
void logger_close(void);

void log_audit(const char *actor, const char *action,
               const char *target, const char *detail, LogLevel level);

void log_system(LogLevel level, const char *fmt, ...);

/* ---- Audit log reader (for admin) ---- */
int  log_print_audit_tail(int n_lines);
int  log_search_audit(const char *keyword);

#endif /* LOGGER_H */
