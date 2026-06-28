/* ============================================================
 * logger.c — Audit & system logging implementation
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "logger.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

static FILE *g_audit_fp  = NULL;
static FILE *g_system_fp = NULL;

static const char *level_str[] = {
    "INFO", "WARNING", "ERROR", "CRITICAL", "AUDIT"
};

/* ============================================================
 * Init / Close
 * ============================================================ */

int logger_init(void) {
    g_audit_fp = fopen(LOG_AUDIT_FILE, "a");
    if (!g_audit_fp) {
        fprintf(stderr, "[LOGGER] Cannot open audit log: %s\n", LOG_AUDIT_FILE);
        return RC_ERR_IO;
    }
    g_system_fp = fopen(LOG_SYSTEM_FILE, "a");
    if (!g_system_fp) {
        fprintf(stderr, "[LOGGER] Cannot open system log: %s\n", LOG_SYSTEM_FILE);
        fclose(g_audit_fp);
        g_audit_fp = NULL;
        return RC_ERR_IO;
    }

    /* Write session boundary */
    char ts[TIMESTAMP_LEN];
    get_timestamp(ts, sizeof(ts));
    fprintf(g_audit_fp,  "\n=== SESSION START: %s ===\n", ts);
    fprintf(g_system_fp, "\n=== SESSION START: %s ===\n", ts);
    fflush(g_audit_fp);
    fflush(g_system_fp);
    return RC_OK;
}

void logger_close(void) {
    char ts[TIMESTAMP_LEN];
    get_timestamp(ts, sizeof(ts));
    if (g_audit_fp) {
        fprintf(g_audit_fp, "=== SESSION END: %s ===\n", ts);
        fclose(g_audit_fp);
        g_audit_fp = NULL;
    }
    if (g_system_fp) {
        fprintf(g_system_fp, "=== SESSION END: %s ===\n", ts);
        fclose(g_system_fp);
        g_system_fp = NULL;
    }
}

/* ============================================================
 * Audit log
 * ============================================================ */

void log_audit(const char *actor, const char *action,
               const char *target, const char *detail, LogLevel level) {
    char ts[TIMESTAMP_LEN];
    get_timestamp(ts, sizeof(ts));

    /* Always write to file */
    FILE *fp = g_audit_fp;
    if (fp) {
        fprintf(fp, "[%s] [%-8s] ACTOR=%-16s ACTION=%-30s TARGET=%-12s DETAIL=%s\n",
                ts,
                level >= 0 && level <= LOG_AUDIT ? level_str[level] : "UNKNOWN",
                actor  ? actor  : "-",
                action ? action : "-",
                target ? target : "-",
                detail ? detail : "");
        fflush(fp);
    }
}

/* ============================================================
 * System log (vararg)
 * ============================================================ */

void log_system(LogLevel level, const char *fmt, ...) {
    char ts[TIMESTAMP_LEN];
    get_timestamp(ts, sizeof(ts));

    char msg[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    FILE *fp = g_system_fp;
    if (fp) {
        fprintf(fp, "[%s] [%s] %s\n",
                ts,
                level >= 0 && level <= LOG_AUDIT ? level_str[level] : "UNKNOWN",
                msg);
        fflush(fp);
    }
}

/* ============================================================
 * Audit log reader (tail)
 * ============================================================ */

int log_print_audit_tail(int n_lines) {
    FILE *fp = fopen(LOG_AUDIT_FILE, "r");
    if (!fp) {
        print_error("Cannot open audit log.");
        return RC_ERR_IO;
    }

    /* Read all lines into a circular buffer */
    char **lines = calloc((size_t)n_lines, sizeof(char *));
    if (!lines) { fclose(fp); return RC_ERR_GENERIC; }
    for (int i = 0; i < n_lines; i++) {
        lines[i] = calloc(512, 1);
        if (!lines[i]) {
            for (int j = 0; j < i; j++) free(lines[j]);
            free(lines);
            fclose(fp);
            return RC_ERR_GENERIC;
        }
    }

    int head = 0, count = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) {
        memcpy(lines[head], buf, 511); lines[head][511] = '\0';
        head = (head + 1) % n_lines;
        if (count < n_lines) count++;
    }
    fclose(fp);

    print_header("AUDIT LOG (Last Entries)");
    int start = (count < n_lines) ? 0 : head;
    for (int i = 0; i < count; i++) {
        int idx = (start + i) % n_lines;
        printf("  %s", lines[idx]);
    }

    for (int i = 0; i < n_lines; i++) free(lines[i]);
    free(lines);
    return RC_OK;
}

int log_search_audit(const char *keyword) {
    if (!keyword || !*keyword) return RC_ERR_INVALID;

    FILE *fp = fopen(LOG_AUDIT_FILE, "r");
    if (!fp) {
        print_error("Cannot open audit log.");
        return RC_ERR_IO;
    }

    char buf[512];
    int found = 0;
    printf(CLR_CYAN "\n  Audit entries matching: \"%s\"\n" CLR_RESET, keyword);
    print_separator();
    while (fgets(buf, sizeof(buf), fp)) {
        if (strstr(buf, keyword)) {
            printf("  %s", buf);
            found++;
        }
    }
    fclose(fp);

    if (found == 0) {
        print_info("No matching audit entries found.");
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "Found %d matching entries.", found);
        print_info(msg);
    }
    return RC_OK;
}
