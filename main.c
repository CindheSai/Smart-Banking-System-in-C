/* ============================================================
 * main.c — Application entry point & user menu orchestration
 * Bank-Grade Banking Management System
 *
 * Build: gcc -Wall -Wextra -o securbank main.c utils.c security.c
 *              logger.c database.c account.c transaction.c
 *              auth.c admin.c -lm
 * ============================================================ */

#include "utils.h"
#include "security.h"
#include "logger.h"
#include "database.h"
#include "account.h"
#include "transaction.h"
#include "auth.h"
#include "admin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* ============================================================
 * Forward declarations (user menu functions)
 * ============================================================ */

static void menu_main(void);
static void menu_user(void);
static void menu_register(void);
static void menu_login(void);

static void user_view_profile(void);
static void user_deposit(void);
static void user_withdraw(void);
static void user_transfer(void);
static void user_mini_statement(void);
static void user_change_pin(void);
static void user_view_balance(void);

/* ============================================================
 * Signal handler (graceful shutdown)
 * ============================================================ */

static void handle_signal(int sig) {
    (void)sig;
    printf(CLR_YELLOW "\n\n  [SYSTEM] Signal received — saving data and shutting down...\n" CLR_RESET);
    if (g_session.active) auth_logout();
    db_save_accounts();
    logger_close();
    printf(CLR_GREEN "  [SYSTEM] Shutdown complete.\n" CLR_RESET);
    exit(0);
}

/* ============================================================
 * main()
 * ============================================================ */

int main(void) {
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    /* Initialize subsystems */
    if (logger_init() != RC_OK) {
        fprintf(stderr, "FATAL: Logger init failed.\n");
        return 1;
    }

    if (db_init() != RC_OK) {
        fprintf(stderr, "FATAL: Database init failed.\n");
        logger_close();
        return 1;
    }

    /* Verify database integrity on startup */
    db_verify_integrity();

    /* Bootstrap admin account if not present */
    auth_create_admin_if_absent();

    log_system(LOG_INFO, "SecurBank system started.");

    /* Enter main menu loop */
    menu_main();

    /* Graceful shutdown */
    if (g_session.active) auth_logout();
    db_save_accounts();
    log_system(LOG_INFO, "SecurBank system shutdown cleanly.");
    logger_close();
    return 0;
}

/* ============================================================
 * Main menu (pre-login)
 * ============================================================ */

static void menu_main(void) {
    int choice;
    do {
        clear_screen();
        print_banner();
        printf("  " CLR_BOLD "Welcome to SecurBank Core Banking Platform\n\n" CLR_RESET);
        printf("  " CLR_CYAN "[ 1 ]" CLR_RESET "  Login to Your Account\n");
        printf("  " CLR_CYAN "[ 2 ]" CLR_RESET "  Open New Account\n");
        printf("  " CLR_RED   "[ 0 ]" CLR_RESET "  Exit System\n");
        print_separator();
        printf("  Select option: ");
        choice = read_int(0, 2);

        switch (choice) {
            case 1: menu_login();    break;
            case 2: menu_register(); break;
            case 0:
                if (confirm_action("Exit SecurBank?")) {
                    printf(CLR_GREEN "\n  Thank you for banking with SecurBank. Goodbye.\n\n" CLR_RESET);
                    return;
                }
                break;
            default:
                print_warning("Invalid choice.");
        }
    } while (1);
}

/* ============================================================
 * Login flow
 * ============================================================ */

static void menu_login(void) {
    clear_screen();
    print_banner();
    print_header("SECURE LOGIN");

    printf("  Account ID : ");
    char account_id[ACCOUNT_ID_LEN];
    safe_read_line(account_id, sizeof(account_id));
    str_upper(account_id);
    str_trim(account_id);

    if (!sec_validate_account_id(account_id) && strcmp(account_id, ADMIN_ID) != 0) {
        print_error("Invalid account ID format.");
        pause_prompt();
        return;
    }

    printf("  PIN        : ");
    char pin[PIN_LEN + 2];
    read_pin_hidden(pin, sizeof(pin));

    int rc = auth_login(account_id, pin);
    memset(pin, 0, sizeof(pin));

    switch (rc) {
        case RC_OK:
            print_success("Authentication successful. Welcome!");
            log_system(LOG_INFO, "User %s logged in.", account_id);
            if (auth_is_admin()) {
                admin_menu();
            } else {
                menu_user();
            }
            break;
        case RC_ERR_NOT_FOUND:
            print_error("Account not found. Please check your Account ID.");
            pause_prompt();
            break;
        case RC_ERR_AUTH:
            print_error("Incorrect PIN. Please try again.");
            pause_prompt();
            break;
        case RC_ERR_LOCKED:
            print_error("Account is LOCKED due to multiple failed PIN attempts.");
            print_info("Please contact SecurBank support to unlock your account.");
            pause_prompt();
            break;
        case RC_ERR_FROZEN:
            print_error("Your account has been FROZEN. Contact the bank.");
            pause_prompt();
            break;
        default:
            print_error("Authentication failed. Please try again.");
            pause_prompt();
            break;
    }
}

/* ============================================================
 * User menu (post-login)
 * ============================================================ */

static void menu_user(void) {
    int choice;
    do {
        if (!auth_is_authenticated()) {
            print_warning("Session expired. Please login again.");
            return;
        }
        auth_refresh_activity();

        clear_screen();
        print_banner();

        /* Session info bar */
        printf(CLR_BOLD "  Logged in as: " CLR_GREEN "%s" CLR_RESET
               "  |  " CLR_BOLD "Account: " CLR_CYAN "%s\n\n" CLR_RESET,
               g_session.account_cache.full_name,
               g_session.account_id);

        /* Balance quick view */
        printf("  " CLR_BOLD "Available Balance: " CLR_GREEN "INR %.2f\n\n" CLR_RESET,
               g_session.account_cache.balance);

        print_header("USER BANKING MENU");
        printf("  " CLR_CYAN "[ 1 ]" CLR_RESET "  View Profile & Account Details\n");
        printf("  " CLR_CYAN "[ 2 ]" CLR_RESET "  Check Balance\n");
        printf("  " CLR_CYAN "[ 3 ]" CLR_RESET "  Deposit Funds\n");
        printf("  " CLR_CYAN "[ 4 ]" CLR_RESET "  Withdraw Funds\n");
        printf("  " CLR_CYAN "[ 5 ]" CLR_RESET "  Transfer Money\n");
        printf("  " CLR_CYAN "[ 6 ]" CLR_RESET "  Mini Statement (Last 10 Txns)\n");
        printf("  " CLR_CYAN "[ 7 ]" CLR_RESET "  Change PIN\n");
        printf("  " CLR_RED   "[ 0 ]" CLR_RESET "  Logout\n");
        print_separator();
        printf("  Select option: ");
        choice = read_int(0, 7);

        switch (choice) {
            case 1: user_view_profile();    pause_prompt(); break;
            case 2: user_view_balance();    pause_prompt(); break;
            case 3: user_deposit();         pause_prompt(); break;
            case 4: user_withdraw();        pause_prompt(); break;
            case 5: user_transfer();        pause_prompt(); break;
            case 6: user_mini_statement();  pause_prompt(); break;
            case 7: user_change_pin();      pause_prompt(); break;
            case 0:
                auth_logout();
                print_success("Logged out successfully. Goodbye!");
                pause_prompt();
                break;
            default:
                print_warning("Invalid option. Please try again.");
                break;
        }
    } while (choice != 0 && auth_is_authenticated());
}

/* ============================================================
 * Account registration
 * ============================================================ */

static void menu_register(void) {
    clear_screen();
    print_banner();
    print_header("OPEN NEW ACCOUNT");

    Account a;
    memset(&a, 0, sizeof(Account));

    /* Full name */
    printf("  Full Name          : ");
    safe_read_line(a.full_name, sizeof(a.full_name));
    sec_sanitize_name(a.full_name);
    if (strlen(a.full_name) < 2) {
        print_error("Name too short.");
        pause_prompt(); return;
    }

    /* Phone */
    printf("  Phone Number       : ");
    safe_read_line(a.phone, sizeof(a.phone));
    str_trim(a.phone);
    if (!sec_validate_phone(a.phone)) {
        print_error("Invalid phone number.");
        pause_prompt(); return;
    }

    /* Email */
    printf("  Email Address      : ");
    safe_read_line(a.email, sizeof(a.email));
    str_trim(a.email);
    if (!sec_validate_email(a.email)) {
        print_error("Invalid email address.");
        pause_prompt(); return;
    }

    /* Address */
    printf("  Address            : ");
    safe_read_line(a.address, sizeof(a.address));
    str_sanitize(a.address, sizeof(a.address));

    /* Account type */
    printf("\n  Account Type:\n");
    printf("    [1] Savings  (%.1f%% p.a. interest, daily limit INR %.0f)\n",
           INTEREST_SAVINGS, DAILY_LIMIT_SAVINGS);
    printf("    [2] Current  (No interest, daily limit INR %.0f)\n",
           DAILY_LIMIT_CURRENT);
    printf("    [3] Salary   (%.1f%% p.a. interest, daily limit INR %.0f)\n",
           INTEREST_SALARY, DAILY_LIMIT_SALARY);
    printf("  Select type [1-3]: ");
    int type_choice = read_int(1, 3);
    a.type = (AccountType)type_choice;

    /* Initial deposit */
    double min_deposit = 500.0;
    printf("\n  Initial Deposit (minimum INR %.2f): INR ", min_deposit);
    double initial = read_amount();
    if (initial < min_deposit) {
        print_error("Initial deposit below minimum.");
        pause_prompt(); return;
    }
    if (initial > MAX_SINGLE_TXN) {
        print_error("Initial deposit exceeds maximum single transaction limit.");
        pause_prompt(); return;
    }

    /* PIN setup */
    printf("\n  Set PIN (4-6 digits): ");
    char pin1[PIN_LEN + 2];
    read_pin_hidden(pin1, sizeof(pin1));

    if (!sec_validate_pin_format(pin1)) {
        print_error("PIN must be 4-6 digits and not a trivial sequence (e.g., 1234).");
        memset(pin1, 0, sizeof(pin1));
        pause_prompt(); return;
    }

    printf("  Confirm PIN       : ");
    char pin2[PIN_LEN + 2];
    read_pin_hidden(pin2, sizeof(pin2));

    if (strcmp(pin1, pin2) != 0) {
        print_error("PINs do not match.");
        memset(pin1, 0, sizeof(pin1));
        memset(pin2, 0, sizeof(pin2));
        pause_prompt(); return;
    }
    memset(pin2, 0, sizeof(pin2));

    /* Confirmation */
    print_separator();
    printf("\n  " CLR_BOLD "Confirm New Account:\n" CLR_RESET);
    printf("  Name     : %s\n", a.full_name);
    printf("  Phone    : %s\n", a.phone);
    printf("  Email    : %s\n", a.email);
    printf("  Type     : %s\n", acct_type_str(a.type));
    printf("  Deposit  : INR %.2f\n", initial);

    if (!confirm_action("Create this account?")) {
        memset(pin1, 0, sizeof(pin1));
        print_info("Account creation cancelled.");
        pause_prompt();
        return;
    }

    /* Generate account ID */
    acct_generate_id(a.account_id);
    a.status    = ACCT_ACTIVE;
    a.balance   = 0.0;
    a.created_at = get_epoch();
    a.is_admin  = 0;

    /* Hash PIN */
    sec_generate_salt(a.pin_salt, sizeof(a.pin_salt));
    sec_hash_pin(pin1, a.pin_salt, a.pin_hash, sizeof(a.pin_hash));
    memset(pin1, 0, sizeof(pin1));

    acct_recompute_checksum(&a);

    int rc = acct_create(&a);
    if (rc != RC_OK) {
        print_error("Failed to create account. Please try again.");
        pause_prompt();
        return;
    }

    /* Record initial deposit */
    g_accounts[acct_find_index_by_id(a.account_id)].balance = 0.0;
    acct_deposit(a.account_id, initial, "Initial account deposit");

    db_save_accounts();

    print_separator();
    print_success("Account created successfully!");
    printf("\n  " CLR_BOLD CLR_CYAN "Your Account ID: " CLR_WHITE "%s\n" CLR_RESET, a.account_id);
    printf("  " CLR_BOLD "Please save your Account ID. You need it to login.\n" CLR_RESET);
    print_separator();

    log_system(LOG_INFO, "New account created: %s (%s)", a.account_id, a.full_name);
    pause_prompt();
}

/* ============================================================
 * User operations
 * ============================================================ */

static void user_view_profile(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    auth_refresh_activity();
    Account a;
    acct_find_by_id(g_session.account_id, &a);
    acct_print_full(&a);
}

static void user_view_balance(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    auth_refresh_activity();
    Account a;
    acct_find_by_id(g_session.account_id, &a);

    print_header("ACCOUNT BALANCE");
    printf("\n");
    printf("  " CLR_BOLD "Account ID : " CLR_CYAN "%s\n" CLR_RESET, a.account_id);
    printf("  " CLR_BOLD "Name       : " CLR_WHITE "%s\n" CLR_RESET, a.full_name);
    printf("  " CLR_BOLD "Type       : " CLR_WHITE "%s\n" CLR_RESET, acct_type_str(a.type));

    printf("\n  " CLR_BOLD "Available Balance:\n" CLR_RESET);
    printf("  " CLR_GREEN CLR_BOLD "  INR %.2f\n\n" CLR_RESET, a.balance);

    double daily_limit = (a.type == ACCT_SAVINGS)  ? DAILY_LIMIT_SAVINGS :
                         (a.type == ACCT_CURRENT)  ? DAILY_LIMIT_CURRENT :
                                                      DAILY_LIMIT_SALARY;
    printf("  %-25s INR %.2f\n", "Daily Txn Used:",    a.daily_txn_total);
    printf("  %-25s INR %.2f\n", "Daily Txn Remaining:", daily_limit - a.daily_txn_total);
    printf("  %-25s INR %.2f\n", "Total Deposited:",   a.total_deposited);
    printf("  %-25s INR %.2f\n", "Total Withdrawn:",   a.total_withdrawn);
    printf("\n");
}

static void user_deposit(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    auth_refresh_activity();

    print_header("DEPOSIT FUNDS");
    printf("  Enter deposit amount (INR): ");
    double amount = read_amount();

    if (amount <= 0 || !sec_validate_amount(amount)) {
        print_error("Invalid amount. Must be between INR 1 and INR 500,000.");
        return;
    }

    printf("  Note/Reference (optional): ");
    char note[128];
    safe_read_line(note, sizeof(note));
    str_sanitize(note, sizeof(note));
    if (strlen(note) == 0) strcpy(note, "Cash Deposit");

    printf("\n  Confirm deposit of " CLR_GREEN "INR %.2f" CLR_RESET "? [y/N]: ", amount);
    char confirm[8];
    safe_read_line(confirm, sizeof(confirm));
    str_lower(confirm);
    if (strcmp(confirm, "y") != 0) {
        print_info("Transaction cancelled.");
        return;
    }

    int rc = acct_deposit(g_session.account_id, amount, note);
    switch (rc) {
        case RC_OK:
            db_save_accounts();
            auth_refresh_activity();
            printf("\n");
            print_success("Deposit successful!");
            printf("  " CLR_BOLD "New Balance: " CLR_GREEN "INR %.2f\n\n" CLR_RESET,
                   g_session.account_cache.balance);
            break;
        case RC_ERR_LIMIT:
            print_error("Daily transaction limit exceeded for today.");
            break;
        case RC_ERR_FROZEN:
            print_error("Your account is frozen. Contact the bank.");
            break;
        case RC_ERR_INVALID:
            print_error("Invalid amount.");
            break;
        default:
            print_error("Deposit failed. Please try again.");
            break;
    }
}

static void user_withdraw(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    auth_refresh_activity();

    print_header("WITHDRAW FUNDS");

    Account a;
    acct_find_by_id(g_session.account_id, &a);
    printf("  Current Balance: " CLR_GREEN "INR %.2f\n" CLR_RESET, a.balance);
    printf("  Enter withdrawal amount (INR): ");
    double amount = read_amount();

    if (amount <= 0 || !sec_validate_amount(amount)) {
        print_error("Invalid amount. Must be between INR 1 and INR 500,000.");
        return;
    }

    /* Re-authenticate for withdrawals */
    printf("  Enter PIN to authorize: ");
    char pin[PIN_LEN + 2];
    read_pin_hidden(pin, sizeof(pin));
    if (auth_reauth(pin) != RC_OK) {
        memset(pin, 0, sizeof(pin));
        print_error("PIN verification failed.");
        return;
    }
    memset(pin, 0, sizeof(pin));

    printf("  Note/Reference (optional): ");
    char note[128];
    safe_read_line(note, sizeof(note));
    str_sanitize(note, sizeof(note));
    if (strlen(note) == 0) strcpy(note, "Cash Withdrawal");

    int rc = acct_withdraw(g_session.account_id, amount, note);
    switch (rc) {
        case RC_OK:
            db_save_accounts();
            auth_refresh_activity();
            printf("\n");
            print_success("Withdrawal successful!");
            printf("  " CLR_BOLD "New Balance: " CLR_GREEN "INR %.2f\n\n" CLR_RESET,
                   g_session.account_cache.balance);
            break;
        case RC_ERR_BALANCE:
            print_error("Insufficient funds.");
            break;
        case RC_ERR_LIMIT:
            print_error("Daily transaction limit exceeded.");
            break;
        case RC_ERR_FROZEN:
            print_error("Account is frozen. Contact the bank.");
            break;
        default:
            print_error("Withdrawal failed. Please try again.");
            break;
    }
}

static void user_transfer(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    auth_refresh_activity();

    print_header("MONEY TRANSFER");

    Account a;
    acct_find_by_id(g_session.account_id, &a);
    printf("  Current Balance: " CLR_GREEN "INR %.2f\n\n" CLR_RESET, a.balance);

    printf("  Recipient Account ID: ");
    char to_id[ACCOUNT_ID_LEN];
    safe_read_line(to_id, sizeof(to_id));
    str_upper(to_id); str_trim(to_id);

    if (strcmp(to_id, g_session.account_id) == 0) {
        print_error("Cannot transfer to your own account.");
        return;
    }

    Account recipient;
    if (acct_find_by_id(to_id, &recipient) != RC_OK) {
        print_error("Recipient account not found.");
        return;
    }
    if (recipient.status == ACCT_FROZEN || recipient.status == ACCT_CLOSED) {
        print_error("Recipient account is not active.");
        return;
    }

    printf("  Recipient: " CLR_CYAN "%s\n" CLR_RESET, recipient.full_name);
    printf("  Transfer Amount (INR): ");
    double amount = read_amount();

    if (amount <= 0 || !sec_validate_amount(amount)) {
        print_error("Invalid amount.");
        return;
    }

    /* PIN re-auth required for transfers */
    printf("  Enter PIN to authorize transfer: ");
    char pin[PIN_LEN + 2];
    read_pin_hidden(pin, sizeof(pin));
    if (auth_reauth(pin) != RC_OK) {
        memset(pin, 0, sizeof(pin));
        print_error("PIN verification failed. Transfer aborted.");
        return;
    }
    memset(pin, 0, sizeof(pin));

    /* Final confirmation */
    printf("\n  Transfer Summary:\n");
    printf("  From  : %s (%s)\n", g_session.account_id, a.full_name);
    printf("  To    : %s (%s)\n", to_id, recipient.full_name);
    printf("  Amount: " CLR_GREEN "INR %.2f\n" CLR_RESET, amount);

    if (!confirm_action("Confirm this transfer?")) {
        print_info("Transfer cancelled.");
        return;
    }

    int rc = acct_transfer(g_session.account_id, to_id, amount);
    switch (rc) {
        case RC_OK:
            db_save_accounts();
            auth_refresh_activity();
            printf("\n");
            print_success("Transfer successful!");
            printf("  " CLR_BOLD "New Balance: " CLR_GREEN "INR %.2f\n\n" CLR_RESET,
                   g_session.account_cache.balance);
            break;
        case RC_ERR_BALANCE:
            print_error("Insufficient funds for this transfer.");
            break;
        case RC_ERR_LIMIT:
            print_error("Daily transaction limit exceeded.");
            break;
        case RC_ERR_FROZEN:
            print_error("Account is frozen.");
            break;
        case RC_ERR_NOT_FOUND:
            print_error("Recipient account not found.");
            break;
        default:
            print_error("Transfer failed.");
            break;
    }
}

static void user_mini_statement(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    auth_refresh_activity();
    txn_print_mini_statement(g_session.account_id);
}

static void user_change_pin(void) {
    if (!auth_is_authenticated()) { print_error("Session expired."); return; }
    print_header("CHANGE PIN");

    printf("  Current PIN : ");
    char old_pin[PIN_LEN + 2];
    read_pin_hidden(old_pin, sizeof(old_pin));

    printf("  New PIN     : ");
    char new_pin[PIN_LEN + 2];
    read_pin_hidden(new_pin, sizeof(new_pin));

    printf("  Confirm PIN : ");
    char confirm_pin[PIN_LEN + 2];
    read_pin_hidden(confirm_pin, sizeof(confirm_pin));

    if (strcmp(new_pin, confirm_pin) != 0) {
        print_error("New PINs do not match.");
        memset(old_pin, 0, sizeof(old_pin));
        memset(new_pin, 0, sizeof(new_pin));
        memset(confirm_pin, 0, sizeof(confirm_pin));
        return;
    }
    memset(confirm_pin, 0, sizeof(confirm_pin));

    int rc = auth_change_pin(old_pin, new_pin);
    memset(old_pin, 0, sizeof(old_pin));
    memset(new_pin, 0, sizeof(new_pin));

    switch (rc) {
        case RC_OK:
            print_success("PIN changed successfully. Please remember your new PIN.");
            break;
        case RC_ERR_AUTH:
            print_error("Current PIN is incorrect.");
            break;
        case RC_ERR_INVALID:
            print_error("New PIN is invalid. Use 4-6 digits, avoid trivial sequences.");
            break;
        default:
            print_error("PIN change failed. Please try again.");
            break;
    }
}
