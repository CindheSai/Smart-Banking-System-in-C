/* ============================================================
 * admin.c — Administrative module implementation
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "admin.h"
#include "account.h"
#include "auth.h"
#include "database.h"
#include "logger.h"
#include "transaction.h"
#include "security.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================
 * Guard macro
 * ============================================================ */

#define ADMIN_GUARD() \
    do { \
        if (!auth_is_admin()) { \
            print_error("Access denied: administrator privileges required."); \
            return; \
        } \
        auth_refresh_activity(); \
    } while(0)

/* ============================================================
 * Admin menu
 * ============================================================ */

void admin_menu(void) {
    if (!auth_is_admin()) {
        print_error("Unauthorized access attempt.");
        log_audit(g_session.account_id, "ADMIN_UNAUTH", "", "Unauthorized admin access", LOG_CRITICAL);
        return;
    }

    int choice;
    do {
        clear_screen();
        print_banner();
        print_header("ADMINISTRATOR CONTROL PANEL");
        printf("\n");
        printf("  " CLR_CYAN "[ 1 ]" CLR_RESET "  View All Accounts\n");
        printf("  " CLR_CYAN "[ 2 ]" CLR_RESET "  Search Account (ID / Name)\n");
        printf("  " CLR_CYAN "[ 3 ]" CLR_RESET "  Account Full Detail\n");
        printf("  " CLR_CYAN "[ 4 ]" CLR_RESET "  Freeze Account\n");
        printf("  " CLR_CYAN "[ 5 ]" CLR_RESET "  Unfreeze Account\n");
        printf("  " CLR_CYAN "[ 6 ]" CLR_RESET "  Delete Account (Secure)\n");
        printf("  " CLR_CYAN "[ 7 ]" CLR_RESET "  Reset Account PIN Lock\n");
        printf("  " CLR_CYAN "[ 8 ]" CLR_RESET "  Apply Monthly Interest (All)\n");
        printf("  " CLR_CYAN "[ 9 ]" CLR_RESET "  System Analytics\n");
        printf("  " CLR_CYAN "[10 ]" CLR_RESET "  View Audit Log\n");
        printf("  " CLR_CYAN "[11 ]" CLR_RESET "  Search Audit Log\n");
        printf("  " CLR_RED   "[ 0 ]" CLR_RESET "  Logout\n");
        print_separator();
        printf("  Enter choice: ");
        choice = read_int(0, 11);

        switch (choice) {
            case 1:  admin_list_all_accounts();    pause_prompt(); break;
            case 2:  admin_search_account();       pause_prompt(); break;
            case 3:  admin_account_detail();       pause_prompt(); break;
            case 4:  admin_freeze_account();       pause_prompt(); break;
            case 5:  admin_unfreeze_account();     pause_prompt(); break;
            case 6:  admin_delete_account();       pause_prompt(); break;
            case 7:  admin_reset_account_pin();    pause_prompt(); break;
            case 8:  admin_apply_interest_all();   pause_prompt(); break;
            case 9:  admin_system_analytics();     pause_prompt(); break;
            case 10: admin_view_audit_log();       pause_prompt(); break;
            case 11: admin_search_account();       pause_prompt(); break;
            case 0:  auth_logout(); break;
            default: print_warning("Invalid option."); break;
        }
    } while (choice != 0 && auth_is_admin());
}

/* ============================================================
 * List all accounts
 * ============================================================ */

void admin_list_all_accounts(void) {
    ADMIN_GUARD();

    if (g_account_count == 0) {
        print_info("No accounts in the system.");
        return;
    }

    /* Sort copy by balance */
    Account sorted[MAX_ACCOUNTS];
    int count = g_account_count;
    memcpy(sorted, g_accounts, (size_t)count * sizeof(Account));
    acct_sort_by_balance(sorted, count);

    char title[64];
    snprintf(title, sizeof(title), "ALL ACCOUNTS (%d total)", count);
    print_header(title);
    acct_print_table_header();

    for (int i = 0; i < count; i++) {
        acct_print_table_row(&sorted[i]);
    }
    printf("\n");
    log_audit(g_session.account_id, "ADMIN_LIST_ACCOUNTS", "", "", LOG_AUDIT);
}

/* ============================================================
 * Search account
 * ============================================================ */

void admin_search_account(void) {
    ADMIN_GUARD();
    print_header("SEARCH ACCOUNT");
    printf("  Search by: [1] Account ID  [2] Name  [3] Audit keyword: ");
    int opt = read_int(1, 3);

    if (opt == 1) {
        printf("  Enter Account ID: ");
        char id[ACCOUNT_ID_LEN];
        safe_read_line(id, sizeof(id));
        str_upper(id);
        str_trim(id);

        Account a;
        if (acct_find_by_id(id, &a) == RC_OK) {
            acct_print_table_header();
            acct_print_table_row(&a);
        } else {
            print_error("Account not found.");
        }
        log_audit(g_session.account_id, "ADMIN_SEARCH_ID", id, "", LOG_AUDIT);

    } else if (opt == 2) {
        printf("  Enter name (partial): ");
        char name[NAME_LEN];
        safe_read_line(name, sizeof(name));
        sec_sanitize_name(name);

        Account results[20];
        int found = 0;
        acct_find_by_name(name, results, 20, &found);

        if (found == 0) {
            print_error("No accounts found matching that name.");
        } else {
            char msg[64];
            snprintf(msg, sizeof(msg), "Found %d account(s):", found);
            print_info(msg);
            acct_print_table_header();
            for (int i = 0; i < found; i++) acct_print_table_row(&results[i]);
        }
        log_audit(g_session.account_id, "ADMIN_SEARCH_NAME", name, "", LOG_AUDIT);

    } else {
        printf("  Enter keyword to search in audit log: ");
        char kw[128];
        safe_read_line(kw, sizeof(kw));
        str_sanitize(kw, sizeof(kw));
        log_search_audit(kw);
    }
}

/* ============================================================
 * Account full detail
 * ============================================================ */

void admin_account_detail(void) {
    ADMIN_GUARD();
    printf("  Enter Account ID: ");
    char id[ACCOUNT_ID_LEN];
    safe_read_line(id, sizeof(id));
    str_upper(id); str_trim(id);

    Account a;
    if (acct_find_by_id(id, &a) != RC_OK) {
        print_error("Account not found.");
        return;
    }
    acct_print_full(&a);
    txn_print_mini_statement(id);
    log_audit(g_session.account_id, "ADMIN_VIEW_DETAIL", id, "", LOG_AUDIT);
}

/* ============================================================
 * Freeze / Unfreeze
 * ============================================================ */

void admin_freeze_account(void) {
    ADMIN_GUARD();
    printf("  Enter Account ID to freeze: ");
    char id[ACCOUNT_ID_LEN];
    safe_read_line(id, sizeof(id));
    str_upper(id); str_trim(id);

    Account a;
    if (acct_find_by_id(id, &a) != RC_OK) {
        print_error("Account not found.");
        return;
    }
    if (a.is_admin) {
        print_error("Cannot freeze admin account.");
        return;
    }
    if (a.status == ACCT_FROZEN) {
        print_warning("Account is already frozen.");
        return;
    }

    acct_print_summary(&a);
    if (!confirm_action("Confirm freeze this account?")) {
        print_info("Operation cancelled.");
        return;
    }

    /* Require admin re-auth */
    printf("  Confirm admin PIN: ");
    char pin[PIN_LEN + 1];
    read_pin_hidden(pin, sizeof(pin));
    if (auth_reauth(pin) != RC_OK) {
        print_error("Re-authentication failed.");
        memset(pin, 0, sizeof(pin));
        return;
    }
    memset(pin, 0, sizeof(pin));

    if (acct_freeze(id) == RC_OK) {
        db_save_accounts();
        print_success("Account frozen successfully.");
    } else {
        print_error("Failed to freeze account.");
    }
}

void admin_unfreeze_account(void) {
    ADMIN_GUARD();
    printf("  Enter Account ID to unfreeze: ");
    char id[ACCOUNT_ID_LEN];
    safe_read_line(id, sizeof(id));
    str_upper(id); str_trim(id);

    Account a;
    if (acct_find_by_id(id, &a) != RC_OK) {
        print_error("Account not found.");
        return;
    }

    if (a.status != ACCT_FROZEN) {
        print_warning("Account is not frozen.");
        return;
    }
    acct_print_summary(&a);

    if (acct_unfreeze(id) == RC_OK) {
        db_save_accounts();
        print_success("Account unfrozen successfully.");
    } else {
        print_error("Failed to unfreeze account.");
    }
}

/* ============================================================
 * Delete account (secure)
 * ============================================================ */

void admin_delete_account(void) {
    ADMIN_GUARD();
    printf("  Enter Account ID to delete: ");
    char id[ACCOUNT_ID_LEN];
    safe_read_line(id, sizeof(id));
    str_upper(id); str_trim(id);

    Account a;
    if (acct_find_by_id(id, &a) != RC_OK) {
        print_error("Account not found.");
        return;
    }
    if (a.is_admin) {
        print_error("Cannot delete admin account.");
        return;
    }

    acct_print_full(&a);
    print_warning("THIS ACTION IS PERMANENT AND CANNOT BE UNDONE.");

    if (!confirm_action("Type 'y' to confirm permanent deletion")) {
        print_info("Deletion cancelled.");
        return;
    }

    /* Require admin PIN re-auth */
    printf("  Enter admin PIN to authorize deletion: ");
    char pin[PIN_LEN + 1];
    read_pin_hidden(pin, sizeof(pin));
    if (auth_reauth(pin) != RC_OK) {
        print_error("Re-authentication failed. Deletion aborted.");
        memset(pin, 0, sizeof(pin));
        log_audit(g_session.account_id, "ADMIN_DELETE_REAUTH_FAIL", id, "", LOG_WARNING);
        return;
    }
    memset(pin, 0, sizeof(pin));

    char detail[128];
    snprintf(detail, sizeof(detail), "Deleted account: %s (%s) balance=%.2f",
             a.account_id, a.full_name, a.balance);

    if (acct_delete(id) == RC_OK) {
        db_save_accounts();
        print_success("Account deleted successfully.");
        log_audit(g_session.account_id, "ACCOUNT_DELETED", id, detail, LOG_CRITICAL);
    } else {
        print_error("Deletion failed.");
    }
}

/* ============================================================
 * Reset PIN lock
 * ============================================================ */

void admin_reset_account_pin(void) {
    ADMIN_GUARD();
    printf("  Enter Account ID to unlock: ");
    char id[ACCOUNT_ID_LEN];
    safe_read_line(id, sizeof(id));
    str_upper(id); str_trim(id);

    Account a;
    if (acct_find_by_id(id, &a) != RC_OK) {
        print_error("Account not found.");
        return;
    }

    if (a.status != ACCT_LOCKED) {
        print_warning("Account is not PIN-locked.");
        return;
    }

    /* Set a temporary PIN */
    printf("  Enter new temporary PIN for account: ");
    char new_pin[PIN_LEN + 1];
    read_pin_hidden(new_pin, sizeof(new_pin));

    if (!sec_validate_pin_format(new_pin)) {
        print_error("Invalid PIN format (4-6 digits, not trivial).");
        return;
    }

    /* Update PIN */
    sec_generate_salt(a.pin_salt, sizeof(a.pin_salt));
    sec_hash_pin(new_pin, a.pin_salt, a.pin_hash, sizeof(a.pin_hash));
    memset(new_pin, 0, sizeof(new_pin));

    acct_unlock(id);
    acct_update(&a);
    db_save_accounts();

    print_success("Account unlocked and PIN reset. User must change PIN on next login.");
    log_audit(g_session.account_id, "ADMIN_PIN_RESET", id, "", LOG_AUDIT);
}

/* ============================================================
 * Apply monthly interest to all eligible accounts
 * ============================================================ */

void admin_apply_interest_all(void) {
    ADMIN_GUARD();
    if (!confirm_action("Apply monthly interest to all eligible accounts?")) {
        print_info("Cancelled.");
        return;
    }

    int applied = acct_apply_interest_all();
    db_save_accounts();

    char msg[64];
    snprintf(msg, sizeof(msg), "Monthly interest applied to %d accounts.", applied);
    print_success(msg);
    log_audit(g_session.account_id, "INTEREST_BATCH", "", msg, LOG_AUDIT);
}

/* ============================================================
 * System analytics
 * ============================================================ */

void admin_system_analytics(void) {
    ADMIN_GUARD();
    print_header("SYSTEM ANALYTICS");

    int total    = g_account_count;
    int active   = 0, frozen = 0, locked = 0, closed = 0;
    int savings  = 0, current = 0, salary = 0;
    double total_balance = 0.0;

    for (int i = 0; i < total; i++) {
        Account *a = &g_accounts[i];
        if (a->is_admin) continue;
        switch (a->status) {
            case ACCT_ACTIVE: active++;  break;
            case ACCT_FROZEN: frozen++;  break;
            case ACCT_LOCKED: locked++;  break;
            case ACCT_CLOSED: closed++;  break;
        }
        switch (a->type) {
            case ACCT_SAVINGS: savings++;  break;
            case ACCT_CURRENT: current++;  break;
            case ACCT_SALARY:  salary++;   break;
        }
        total_balance += a->balance;
    }

    printf("\n");
    printf("  " CLR_BOLD "Account Status Distribution\n" CLR_RESET);
    printf("  %-25s %d\n", "Total Accounts:", total - 1);  /* exclude admin */
    printf("  %-25s " CLR_GREEN "%d\n" CLR_RESET, "Active:",  active);
    printf("  %-25s " CLR_YELLOW "%d\n" CLR_RESET, "Frozen:", frozen);
    printf("  %-25s " CLR_RED "%d\n" CLR_RESET, "Locked:",    locked);
    printf("  %-25s " CLR_RED "%d\n" CLR_RESET, "Closed:",    closed);

    printf("\n  " CLR_BOLD "Account Type Distribution\n" CLR_RESET);
    printf("  %-25s %d\n", "Savings:",  savings);
    printf("  %-25s %d\n", "Current:",  current);
    printf("  %-25s %d\n", "Salary:",   salary);

    printf("\n  " CLR_BOLD "Financial Summary\n" CLR_RESET);
    printf("  %-25s " CLR_GREEN "INR %.2f\n" CLR_RESET, "Total Funds in System:", total_balance);
    printf("  %-25s INR %.2f\n", "Total Deposited:", acct_total_deposits_all());
    printf("  %-25s INR %.2f\n", "Total Withdrawn:", acct_total_withdrawals_all());

    printf("\n");
    log_audit(g_session.account_id, "ADMIN_ANALYTICS", "", "", LOG_AUDIT);
}

/* ============================================================
 * View audit log
 * ============================================================ */

void admin_view_audit_log(void) {
    ADMIN_GUARD();
    printf("  How many recent lines to show? [10-200]: ");
    int n = read_int(10, 200);
    if (n <= 0) n = 50;
    log_print_audit_tail(n);
    log_audit(g_session.account_id, "ADMIN_VIEW_AUDIT", "", "", LOG_AUDIT);
}
