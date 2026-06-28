/* ============================================================
 * auth.c — Authentication & session management implementation
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "auth.h"
#include "account.h"
#include "security.h"
#include "database.h"
#include "logger.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================
 * Global session
 * ============================================================ */

Session g_session;

static void session_clear(void) {
    memset(&g_session, 0, sizeof(Session));
    g_session.active = 0;
    g_session.role   = ROLE_NONE;
}

/* ============================================================
 * Login
 * ============================================================ */

int auth_login(const char *account_id, const char *pin) {
    if (!account_id || !pin) return RC_ERR_INVALID;

    /* Basic format validation */
    if (!sec_validate_account_id(account_id)) {
        log_audit(account_id, "LOGIN_FAILED", account_id,
                  "Invalid account_id format", LOG_WARNING);
        return RC_ERR_INVALID;
    }

    Account a;
    int rc = acct_find_by_id(account_id, &a);
    if (rc != RC_OK) {
        log_audit(account_id, "LOGIN_FAILED", account_id,
                  "Account not found", LOG_WARNING);
        return RC_ERR_NOT_FOUND;
    }

    if (a.status == ACCT_LOCKED) {
        log_audit(account_id, "LOGIN_BLOCKED", account_id,
                  "Account locked", LOG_WARNING);
        return RC_ERR_LOCKED;
    }

    if (a.status == ACCT_FROZEN) {
        log_audit(account_id, "LOGIN_BLOCKED", account_id,
                  "Account frozen", LOG_WARNING);
        return RC_ERR_FROZEN;
    }

    if (a.status == ACCT_CLOSED) {
        return RC_ERR_INVALID;
    }

    /* Verify PIN */
    if (!sec_verify_pin(pin, a.pin_salt, a.pin_hash)) {
        acct_increment_failed_pin(account_id);
        db_save_accounts();

        /* Re-read to get updated state */
        acct_find_by_id(account_id, &a);
        char detail[64];
        snprintf(detail, sizeof(detail), "Failed attempt %d/3", a.failed_pin_attempts);
        log_audit(account_id, "LOGIN_FAILED_PIN", account_id, detail, LOG_WARNING);

        if (a.status == ACCT_LOCKED) return RC_ERR_LOCKED;
        return RC_ERR_AUTH;
    }

    /* PIN correct — reset failed counter */
    acct_reset_failed_pin(account_id);

    /* Update last login */
    int idx = acct_find_index_by_id(account_id);
    if (idx >= 0) {
        g_accounts[idx].last_login = get_epoch();
        acct_recompute_checksum(&g_accounts[idx]);
    }
    db_save_accounts();

    /* Establish session */
    session_clear();
    g_session.active        = 1;
    g_session.login_time    = get_epoch();
    g_session.last_activity = get_epoch();
    g_session.role          = a.is_admin ? ROLE_ADMIN : ROLE_USER;
    strncpy(g_session.account_id, account_id, sizeof(g_session.account_id) - 1);
    sec_generate_token(g_session.token, SESSION_TOKEN_LEN);
    acct_find_by_id(account_id, &g_session.account_cache);

    log_audit(account_id, "LOGIN_SUCCESS", account_id,
              a.is_admin ? "Admin login" : "User login", LOG_AUDIT);
    return RC_OK;
}

/* ============================================================
 * Logout
 * ============================================================ */

void auth_logout(void) {
    if (g_session.active) {
        log_audit(g_session.account_id, "LOGOUT", g_session.account_id, "", LOG_AUDIT);
    }
    session_clear();
}

/* ============================================================
 * Session checks
 * ============================================================ */

int auth_is_authenticated(void) {
    if (!g_session.active) return 0;
    if (auth_session_expired()) {
        log_audit(g_session.account_id, "SESSION_EXPIRED", g_session.account_id, "", LOG_WARNING);
        session_clear();
        return 0;
    }
    return 1;
}

int auth_is_admin(void) {
    return auth_is_authenticated() && (g_session.role == ROLE_ADMIN);
}

int auth_session_expired(void) {
    if (!g_session.active) return 1;
    long now = get_epoch();
    return (now - g_session.last_activity) > SESSION_TIMEOUT;
}

void auth_refresh_activity(void) {
    if (g_session.active) {
        g_session.last_activity = get_epoch();
        /* Refresh account cache */
        acct_find_by_id(g_session.account_id, &g_session.account_cache);
    }
}

/* ============================================================
 * Re-authentication for sensitive ops
 * ============================================================ */

int auth_reauth(const char *pin) {
    if (!auth_is_authenticated()) return RC_ERR_AUTH;
    Account a;
    if (acct_find_by_id(g_session.account_id, &a) != RC_OK) return RC_ERR_NOT_FOUND;
    if (!sec_verify_pin(pin, a.pin_salt, a.pin_hash)) {
        log_audit(g_session.account_id, "REAUTH_FAILED", g_session.account_id, "", LOG_WARNING);
        return RC_ERR_AUTH;
    }
    auth_refresh_activity();
    return RC_OK;
}

/* ============================================================
 * Admin bootstrap
 * ============================================================ */

int auth_create_admin_if_absent(void) {
    if (acct_find_index_by_id(ADMIN_ID) >= 0) return RC_OK;

    Account admin;
    memset(&admin, 0, sizeof(Account));
    strncpy(admin.account_id, ADMIN_ID, sizeof(admin.account_id) - 1);
    strncpy(admin.full_name,  "System Administrator", sizeof(admin.full_name) - 1);
    strncpy(admin.phone,      "0000000000",           sizeof(admin.phone) - 1);
    strncpy(admin.email,      "admin@securbank.internal", sizeof(admin.email) - 1);
    strncpy(admin.address,    "SecurBank HQ",          sizeof(admin.address) - 1);
    admin.type      = ACCT_CURRENT;
    admin.status    = ACCT_ACTIVE;
    admin.balance   = 0.0;
    admin.is_admin  = 1;
    admin.created_at = get_epoch();

    /* Default admin PIN: 000000 — MUST be changed on first login */
    sec_generate_salt(admin.pin_salt, sizeof(admin.pin_salt));
    sec_hash_pin("000000", admin.pin_salt, admin.pin_hash, sizeof(admin.pin_hash));

    acct_recompute_checksum(&admin);
    int rc = acct_create(&admin);
    if (rc == RC_OK) {
        db_save_accounts();
        log_system(LOG_INFO, "Admin account created with default PIN.");
        printf(CLR_YELLOW
               "  [SYSTEM] Admin account created.\n"
               "  [SYSTEM] Default PIN: 000000 — Change immediately after login!\n"
               CLR_RESET);
    }
    return rc;
}

/* ============================================================
 * PIN change
 * ============================================================ */

int auth_change_pin(const char *old_pin, const char *new_pin) {
    if (!auth_is_authenticated()) return RC_ERR_AUTH;
    if (!old_pin || !new_pin) return RC_ERR_INVALID;

    if (!sec_validate_pin_format(new_pin)) {
        print_error("New PIN must be 4-6 digits and not a trivial sequence.");
        return RC_ERR_INVALID;
    }

    Account a;
    if (acct_find_by_id(g_session.account_id, &a) != RC_OK) return RC_ERR_NOT_FOUND;

    if (!sec_verify_pin(old_pin, a.pin_salt, a.pin_hash)) {
        log_audit(g_session.account_id, "PIN_CHANGE_FAILED", g_session.account_id,
                  "Wrong old PIN", LOG_WARNING);
        return RC_ERR_AUTH;
    }

    /* Generate new salt and hash */
    sec_generate_salt(a.pin_salt, sizeof(a.pin_salt));
    sec_hash_pin(new_pin, a.pin_salt, a.pin_hash, sizeof(a.pin_hash));
    a.last_modified = get_epoch();
    acct_recompute_checksum(&a);

    int rc = acct_update(&a);
    if (rc == RC_OK) {
        db_save_accounts();
        /* Refresh session cache */
        g_session.account_cache = a;
        log_audit(g_session.account_id, "PIN_CHANGED", g_session.account_id, "", LOG_AUDIT);
    }
    return rc;
}
