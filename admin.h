#ifndef ADMIN_H
#define ADMIN_H

/* ============================================================
 * admin.h — Administrative module interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

/* ---- Admin menu entry point ---- */
void admin_menu(void);

/* ---- Sub-menus / operations ---- */
void admin_list_all_accounts(void);
void admin_search_account(void);
void admin_freeze_account(void);
void admin_unfreeze_account(void);
void admin_delete_account(void);
void admin_view_audit_log(void);
void admin_system_analytics(void);
void admin_apply_interest_all(void);
void admin_reset_account_pin(void);
void admin_account_detail(void);

#endif /* ADMIN_H */
