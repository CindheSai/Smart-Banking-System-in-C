#ifndef AUTH_H
#define AUTH_H

/* ============================================================
 * auth.h — Authentication & session management interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "account.h"

#define SESSION_TOKEN_LEN  33
#define SESSION_TIMEOUT    900   /* 15 minutes (seconds) */

typedef enum {
    ROLE_NONE   = 0,
    ROLE_USER   = 1,
    ROLE_ADMIN  = 2
} SessionRole;

/* ---- Active session (in-memory only) ---- */
typedef struct {
    int         active;
    char        account_id[ACCOUNT_ID_LEN];
    char        token[SESSION_TOKEN_LEN];
    SessionRole role;
    long        login_time;
    long        last_activity;
    Account     account_cache;   /* cached copy; refresh on sensitive ops */
} Session;

/* Global current session */
extern Session g_session;

/* ---- Auth API ---- */
int  auth_login(const char *account_id, const char *pin);
void auth_logout(void);
int  auth_is_authenticated(void);
int  auth_is_admin(void);
int  auth_session_expired(void);
void auth_refresh_activity(void);

/* ---- Re-auth for sensitive operations ---- */
int  auth_reauth(const char *pin);

/* ---- Admin bootstrap ---- */
int  auth_create_admin_if_absent(void);

/* ---- PIN change ---- */
int  auth_change_pin(const char *old_pin, const char *new_pin);

#endif /* AUTH_H */
