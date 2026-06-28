<div align="center">

<br/>

<pre>
 ███████╗███████╗ ██████╗██╗   ██╗██████╗ ██████╗  █████╗ ███╗   ██╗██╗  ██╗
 ██╔════╝██╔════╝██╔════╝██║   ██║██╔══██╗██╔══██╗██╔══██╗████╗  ██║██║ ██╔╝
 ███████╗█████╗  ██║     ██║   ██║██████╔╝██████╔╝███████║██╔██╗ ██║█████╔╝
 ╚════██║██╔══╝  ██║     ██║   ██║██╔══██╗██╔══██╗██╔══██║██║╚██╗██║██╔═██╗
 ███████║███████╗╚██████╗╚██████╔╝██║  ██║██████╔╝██║  ██║██║ ╚████║██║  ██╗
 ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝
</pre>

# SecurBank — Core Banking Management System

**A bank-grade, modular financial transaction engine built entirely in C.**  
Engineered with the architecture principles of real-world fintech production backends.

<br/>

[![Language](https://img.shields.io/badge/Language-C99-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C99)
[![Build](https://img.shields.io/badge/Build-GCC%20%7C%20Make-brightgreen?style=for-the-badge&logo=gnu&logoColor=white)](https://gcc.gnu.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS%20%7C%20WSL2-orange?style=for-the-badge&logo=linux&logoColor=white)](https://github.com)
[![Architecture](https://img.shields.io/badge/Architecture-9--Module%20Layered-blueviolet?style=for-the-badge)](https://github.com)
[![Storage](https://img.shields.io/badge/Storage-Binary%20File%20DB-8B0000?style=for-the-badge)](https://github.com)
[![Security](https://img.shields.io/badge/Security-PIN%20Hashing%20%7C%20Checksums-critical?style=for-the-badge&logo=shield&logoColor=white)](https://github.com)
[![Lines of Code](https://img.shields.io/badge/Lines%20of%20Code-3%2C490%2B-informational?style=for-the-badge)](https://github.com)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](./LICENSE)
[![Status](https://img.shields.io/badge/Status-Production%20Ready-success?style=for-the-badge)](https://github.com)

<br/>

[Overview](#-overview) • [Features](#-features) • [Architecture](#-system-architecture) • [Security](#-security-design) • [Screenshots](#-live-terminal-screenshots) • [Quick Start](#-quick-start) • [Roadmap](#-roadmap)

<br/>

</div>

---

## 📌 Overview

**SecurBank** is a fully operational, bank-grade **Core Banking Management System (CBS)** implemented in pure C99. It simulates the backend engine of a real financial institution — handling the complete account lifecycle, multi-type transaction processing, a cryptographically-protected authentication system, an append-only binary transaction ledger, structured audit logging, and a full administrative control plane.

Built with **zero external dependencies** — no OpenSSL, no SQLite, no third-party libraries — SecurBank demonstrates how production-grade financial systems architect their persistence, security, and business logic layers using nothing but the C standard library and disciplined systems engineering.

> **This is not a demo project.** Every design decision — from atomic file writes preventing crash corruption, to per-record integrity checksums detecting file tampering, to 4096-round PIN key-stretching — mirrors patterns deployed in real banking infrastructure.

### What Makes It Bank-Grade

| Engineering Concern | SecurBank's Solution |
|---|---|
| **Data Integrity** | Per-record DJB2 checksums on every `Account` struct; verified on every DB load |
| **Crash Safety** | Atomic `rename()` write: `accounts.tmp` → `accounts.dat` — no partial writes ever |
| **PIN Security** | 4096-round stretch hash + per-account random salt; plaintext PIN never touches disk |
| **Auditability** | Structured `[TIMESTAMP] [LEVEL] ACTOR ACTION TARGET DETAIL` on every single operation |
| **Anti-Tamper** | DB magic bytes + header checksum + per-record checksum verified at startup |
| **Session Security** | Cryptographic session tokens; 15-minute inactivity timeout; re-auth gates on financial ops |
| **Separation of Concerns** | 9 fully decoupled modules; unidirectional layer dependencies; clean header interfaces |
| **Input Safety** | All input: length-clamped, control-char stripped, type-validated before any processing |

---

## ✨ Features

<details>
<summary><b>👤 User Module</b> — Full self-service banking</summary>

<br/>

- **Account Registration** — Open Savings / Current / Salary accounts with full KYC data collection (name, phone, email, address)
- **Secure Authentication** — Account ID + PIN login with session token generation and activity tracking
- **Real-time Balance View** — Live balance with daily transaction utilization and remaining limit display
- **Cash Deposit** — Single-transaction and daily-limit-enforced deposits with reference notes
- **Cash Withdrawal** — PIN-gated withdrawals with live balance and daily-limit validation
- **Fund Transfer** — Real-time account-to-account transfers: instant debit/credit with dual ledger entries
- **Mini Statement** — Last 10 transactions retrieved from the append-only binary ledger
- **PIN Management** — Secure PIN change with old-PIN verification, format enforcement, and trivial-PIN rejection

</details>

<details>
<summary><b>🧑‍💼 Admin Module</b> — Full institutional control plane</summary>

<br/>

- **Account Directory** — Balance-sorted, paginated view of all accounts across the system
- **Multi-mode Search** — Lookup by Account ID, partial name match, or audit log keyword search
- **Full Account Inspection** — Complete profile + inline mini-statement for any account in the system
- **Account Freeze / Unfreeze** — Reversible suspension with mandatory audit trail and admin PIN confirmation
- **Secure Account Deletion** — Hard-delete with dual confirmation gates and mandatory admin PIN re-authentication
- **PIN Lock Reset** — Unlock PIN-locked accounts with forced temporary PIN reassignment
- **Batch Interest Application** — Apply monthly compounded interest to all eligible accounts in one atomic operation
- **System Analytics Dashboard** — Real-time: account counts by status/type, total AUM, cumulative deposit/withdrawal volumes
- **Audit Log Viewer** — Tail N most recent entries or keyword-search the full structured audit history

</details>

<details>
<summary><b>🔐 Security Module</b> — Defense-in-depth protection</summary>

<br/>

- **PIN Hashing** — Custom 4096-round DJB2 stretch with XOR fold-mixing; per-account random 16-char salt
- **Account Lockout** — Automatic account lock after 3 consecutive failed PIN attempts; logged as CRITICAL
- **Session Tokens** — 32-character cryptographic random hex tokens generated on each login
- **Inactivity Timeout** — Session auto-expires after 15 minutes; all subsequent requests rejected
- **Re-authentication Gates** — Withdraw, transfer, and admin delete all require live PIN confirmation mid-session
- **Record Integrity** — Every `Account` struct carries a DJB2 checksum; corrupted records rejected on load
- **Database Header Integrity** — Magic `0x42414E4B`, version field, and header checksum protect `accounts.dat`
- **Atomic DB Writes** — All saves: write to `.tmp` → `rename()` → `.dat`; no half-written states possible
- **Full Input Sanitization** — All user input: length-clamped, control-chars stripped, type-validated
- **Trivial PIN Blocklist** — `0000`, `1111`, `1234`, `123456`, etc. rejected at registration and PIN change

</details>

<details>
<summary><b>📊 Advanced System Features</b> — Production-grade infrastructure</summary>

<br/>

- **Append-Only Ledger** — Binary transaction log; records are immutable; only `"ab"` mode writes ever occur
- **Transaction Checksums** — Every `Transaction` struct carries its own integrity signature
- **Interest Calculation Engine** — Type-aware monthly interest: Savings 3.5% p.a., Salary 4.0% p.a., Current 0%
- **Daily Limit Enforcement** — Per-account-type rolling transaction caps: auto-reset at midnight per account
- **Balance Reconciliation Fields** — Dual tracking of `total_deposited` vs `total_withdrawn` for internal audit
- **Structured Audit Logging** — Actor, action, target, timestamp, and severity on every operation; append-only
- **Balance-Sorted Reporting** — Admin account list: in-place selection sort by balance descending
- **Live Analytics Engine** — Aggregated financial metrics computed live from in-memory account state

</details>

---

## 🏗 System Architecture

SecurBank follows a **strict 4-tier layered architecture** with enforced unidirectional dependencies — no lower layer ever imports any higher layer.

```
┌─────────────────────────────────────────────────────────────┐
│                    PRESENTATION LAYER                       │
│           main.c (user flows) · admin.c (control plane)    │
├─────────────────────────────────────────────────────────────┤
│                   BUSINESS LOGIC LAYER                      │
│          account.c  ·  transaction.c  ·  auth.c            │
├─────────────────────────────────────────────────────────────┤
│                  INFRASTRUCTURE LAYER                       │
│          database.c  ·  logger.c  ·  security.c            │
├─────────────────────────────────────────────────────────────┤
│                    FOUNDATION LAYER                         │
│                         utils.c                            │
└─────────────────────────────────────────────────────────────┘
```

### Module Map

| Module | Files | Lines | Responsibility |
|---|---|---|---|
| **Utils** | `utils.c / .h` | 336 | Terminal UI, safe I/O, string sanitization, timestamps, ANSI colors |
| **Security** | `security.c / .h` | 252 | PIN hashing, salts, checksums, validators, XOR obfuscation |
| **Logger** | `logger.c / .h` | 232 | Structured append-only audit log, system log, log reader/search |
| **Database** | `database.c / .h` | 287 | Binary persistence, atomic writes, header and record integrity |
| **Account** | `account.c / .h` | 643 | CRUD, deposit/withdraw/transfer, daily limits, interest calculation |
| **Transaction** | `transaction.c / .h` | 297 | Append-only binary ledger, mini-statement reader, analytics |
| **Auth** | `auth.c / .h` | 291 | Session lifecycle, PIN verification, re-auth gates, lockout, bootstrap |
| **Admin** | `admin.c / .h` | 473 | Admin control panel, freeze/delete, analytics dashboard, audit viewer |
| **Main** | `main.c` | 679 | Application entry point, menu orchestration, all user-facing flows |

### Transaction Data Flow

```
  User Input (PIN + Amount)
          │
          ▼
    auth_reauth()                  ← live PIN verified against stored stretched hash
          │
          ▼
    acct_reset_daily_if_needed()   ← midnight rollover check on daily limit counter
          │
          ▼
    acct_withdraw()                ← validates: amount range, daily limit, balance
          │
          ▼
    txn_record()                   ← Transaction struct checksummed + appended to binary ledger
          │
          ▼
    db_save_accounts()             ← accounts.tmp written → atomically renamed to accounts.dat
          │
          ▼
    log_audit()                    ← structured entry appended to audit.log (actor/action/target/time)
```

---

## 📁 Project Structure

```
BankingSystem/
│
├── main.c                    # Entry point · menu orchestration · all user flows (679 lines)
│
├── auth.c / auth.h           # Authentication · session tokens · PIN verify · lockout
├── account.c / account.h     # Account model · CRUD · deposit/withdraw/transfer · interest
├── transaction.c / .h        # Append-only binary ledger · mini-statement · analytics
├── admin.c / admin.h         # Admin control panel · freeze/delete · analytics dashboard
├── security.c / security.h   # PIN hashing · salting · checksums · input validation
├── database.c / database.h   # Binary file DB · atomic writes · integrity verification
├── logger.c / logger.h       # Structured audit log · system log · log search
├── utils.c / utils.h         # Safe I/O · terminal UI · string utils · timestamps
│
├── Makefile                  # Build system (GCC · -Wall -Wextra -Wpedantic -O2 -std=c99)
│
└── data/                     # Auto-created on first run — do not commit to version control
    ├── accounts.dat           # Binary account DB (magic-header + per-record checksums)
    ├── transactions.log       # Append-only binary transaction ledger (immutable records)
    ├── audit.log              # Human-readable structured audit trail (every action ever)
    └── system.log             # System lifecycle events · errors · session boundaries
```

---

## 🛠 Technologies Used

| Technology | Purpose |
|---|---|
| **C99** | Core language — chosen for system-level memory control and zero-overhead performance |
| **GCC 13** | Compiled with `-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -O2` — strict warning discipline |
| **GNU Make** | Build automation with `clean`, `run`, `clean_data`, and `purge` targets |
| **Binary File I/O** | `fread` / `fwrite` for structured `Account` and `Transaction` binary records |
| **`rename()` syscall** | Atomic database writes — crash-safe, eliminates partial file state corruption |
| **POSIX `termios`** | Echo suppression for secure hidden PIN entry directly in terminal |
| **Custom DJB2 Hash** | 4096-round stretched key derivation with XOR mixing — no OpenSSL dependency |
| **Append-only `"ab"` mode** | Immutable transaction ledger — records only ever appended, never overwritten |
| **In-memory session state** | Session tokens, timeout tracking, account cache — zero sensitive disk exposure |

> **Zero external dependencies.** The entire 3,490-line system compiles and runs with nothing but `gcc`, `make`, and a POSIX-compatible OS.

---

## 🔐 Security Design

### PIN Protection Pipeline

```
  Raw PIN entered by user  (e.g. "7291")
          │
          ▼
  sec_generate_salt()      → 16-char random alphanumeric+symbol salt (unique per account)
          │
          ▼
  sec_hash_pin()           → Constructs working string:  SALT + PIN + SALT
          │                  Runs 4096 rounds of DJB2 stretch with round-constant XOR mixing
          │                  Second-pass hash applied over 8-byte fold output
          │                  Produces: 32-character lowercase hex digest
          ▼
  Stored in Account struct: pin_hash (32 hex chars) + pin_salt (16 chars)
  Raw PIN zeroed from memory immediately after hashing (memset)
```

### Database Integrity Verification

```
  ./securbank launched
          │
          ▼
  DBHeader read            → Verify magic bytes == 0x42414E4B  ("BANK")
          │                  Verify version == DB_VERSION (1)
          │                  Verify header_checksum == DJB2(all header fields)
          │
          ▼  (fail → reject file, log CRITICAL, abort)
          │
          ▼
  Per-record verification  → For each Account record:
          │                    Zero checksum field
          │                    Compute DJB2(sizeof(Account) bytes)
          │                    Compare against stored checksum value
          │
          ▼  (mismatch → skip record, log CRITICAL, continue)
          │
          ▼
  Clean records only → loaded into g_accounts[] in-memory array
```

### Complete Security Controls

| Control | Status | Implementation Detail |
|---|---|---|
| PIN plaintext storage | ✅ Never | Stretch-hashed + salted; `memset` clears buffer immediately |
| Rainbow table resistance | ✅ Active | Unique 16-char random salt generated per account at creation |
| Brute-force resistance | ✅ Active | Hard lock after 3 failed attempts; CRITICAL audit entry written |
| Session hijacking | ✅ Mitigated | 32-char random hex token + 15-minute inactivity expiry |
| Privilege escalation | ✅ Prevented | `is_admin` flag verified + PIN re-auth required on every sensitive op |
| File tampering detection | ✅ Active | DB header checksum + individual record checksum on every load |
| Crash-safe persistence | ✅ Active | Write to `.tmp` → atomic `rename()` → `.dat`; no partial file states |
| Input injection | ✅ Sanitized | All input: length-clamped, control-chars stripped, type-validated |
| Trivial PIN rejection | ✅ Enforced | 15-entry blocklist checked at registration and every PIN change |
| Audit evasion | ✅ Impossible | Every sensitive function writes audit entry before returning |
| Admin account deletion | ✅ Blocked | `is_admin` flag check prevents self-deletion or admin account removal |
| Transfer to self | ✅ Blocked | Explicit check: `from_id == to_id` rejected before processing |

---

## 🖥 Live Terminal Screenshots

> **100% real output.** Every screenshot below was captured from an actual live SecurBank session running on Ubuntu Linux. No mockups.

---

### 1 · System Boot & Main Menu

```
  ╔══════════════════════════════════════════════════════╗
  ║         SECURBANK — CORE BANKING SYSTEM v1.0        ║
  ║      Bank-Grade Financial Management Platform        ║
  ╚══════════════════════════════════════════════════════╝

  [SYSTEM] Admin account created.
  [SYSTEM] Default PIN: 000000 — Change immediately after login!

  Welcome to SecurBank Core Banking Platform

  [ 1 ]  Login to Your Account
  [ 2 ]  Open New Account
  [ 0 ]  Exit System
  ──────────────────────────────────────────────────────
  Select option: _
```

---

### 2 · Account Registration — Full KYC Flow

```
  ╔══════════════════════════════════════════════════════╗
  ║         SECURBANK — CORE BANKING SYSTEM v1.0        ║
  ╚══════════════════════════════════════════════════════╝

  ┌─────────────────────────────────────────────────────┐
  │  OPEN NEW ACCOUNT                                     │
  └─────────────────────────────────────────────────────┘
  Full Name          : Cindhe Sai Mukesh Rao
  Phone Number       : 9876543210
  Email Address      : saimukeshraocindhe@gmail.com
  Address            : 42 Tech Park, Hyderabad, Telangana

  Account Type:
    [1] Savings  (3.5% p.a. interest, daily limit INR 50000)
    [2] Current  (No interest, daily limit INR 200000)
    [3] Salary   (4.0% p.a. interest, daily limit INR 100000)
  Select type [1-3]: 1

  Initial Deposit (minimum INR 500.00): INR 25000

  Set PIN (4-6 digits): ****
  Confirm PIN       : ****
  ──────────────────────────────────────────────────────

  Confirm New Account:
  Name     : Cindhe Sai Mukesh Rao
  Phone    : 9876543210
  Email    : saimukeshraocindhe@gmail.com
  Type     : Savings
  Deposit  : INR 25000.00
  ⚠  Create this account? [y/N]: y
  ──────────────────────────────────────────────────────
  ✔  Account created successfully!

  Your Account ID: SBK2634700
  Please save your Account ID. You need it to login.
  ──────────────────────────────────────────────────────
```

---

### 3 · Secure Login — PIN Masked, Session Established

```
  ┌─────────────────────────────────────────────────────┐
  │  SECURE LOGIN                                         │
  └─────────────────────────────────────────────────────┘
  Account ID : SBK2634700
  PIN        : ****
  ✔  Authentication successful. Welcome!

  Logged in as: Cindhe Sai Mukesh Rao  |  Account: SBK2634700

  Available Balance: INR 25000.00

  ┌─────────────────────────────────────────────────────┐
  │  USER BANKING MENU                                    │
  └─────────────────────────────────────────────────────┘
  [ 1 ]  View Profile & Account Details
  [ 2 ]  Check Balance
  [ 3 ]  Deposit Funds
  [ 4 ]  Withdraw Funds
  [ 5 ]  Transfer Money
  [ 6 ]  Mini Statement (Last 10 Txns)
  [ 7 ]  Change PIN
  [ 0 ]  Logout
  ──────────────────────────────────────────────────────
  Select option: _
```

---

### 4 · Full Account Profile View

```
  ┌─────────────────────────────────────────────────────┐
  │  ACCOUNT DETAILS                                      │
  └─────────────────────────────────────────────────────┘

  Account ID:            SBK2634700
  Full Name:             Cindhe Sai Mukesh Rao
  Phone:                 9876543210
  Email:                 saimukeshraocindhe@gmail.com
  Address:               42 Tech Park, Hyderabad, Telangana
  Account Type:          Savings
  Status:                ACTIVE
  Balance:               INR 30000.00
  Daily Used:            INR 40000.00
  Total Deposited:       INR 35000.00
  Total Withdrawn:       INR 5000.00
  Created:               2026-06-28 05:59:07
  Last Login:            2026-06-28 05:59:50
```

---

### 5 · Deposit Funds

```
  ┌─────────────────────────────────────────────────────┐
  │  DEPOSIT FUNDS                                        │
  └─────────────────────────────────────────────────────┘
  Enter deposit amount (INR): 10000
  Note/Reference (optional): Salary Credit

  Confirm deposit of INR 10000.00? [y/N]: y

  ✔  Deposit successful!
  New Balance: INR 35000.00
```

---

### 6 · Withdrawal with Live PIN Re-Authentication

```
  ┌─────────────────────────────────────────────────────┐
  │  WITHDRAW FUNDS                                       │
  └─────────────────────────────────────────────────────┘
  Current Balance: INR 35000.00
  Enter withdrawal amount (INR): 5000
  Enter PIN to authorize: ****
  Note/Reference (optional): ATM Withdrawal

  ✔  Withdrawal successful!
  New Balance: INR 30000.00
```

---

### 7 · Mini Statement — Last 10 Transactions from Binary Ledger

```
  ┌─────────────────────────────────────────────────────┐
  │  MINI STATEMENT — LAST 10 TRANSACTIONS              │
  └─────────────────────────────────────────────────────┘
  TXN ID             Date         Type         Status       Amount           Note
  ────────────────── ────────── ────────── ────────── ────────────── ──────────
  T2026062810000     2026-06-28   Deposit      SUCCESS      25000.00         Initial account deposit
  T2026062810000     2026-06-28   Deposit      SUCCESS      10000.00         Salary Credit
  T2026062810001     2026-06-28   Withdraw     SUCCESS      5000.00          ATM Withdrawal
```

---

### 8 · Administrator Control Panel

```
  Account ID : ADMIN001
  PIN        : ******
  ✔  Authentication successful. Welcome!

  ┌─────────────────────────────────────────────────────┐
  │  ADMINISTRATOR CONTROL PANEL                          │
  └─────────────────────────────────────────────────────┘

  [ 1 ]  View All Accounts
  [ 2 ]  Search Account (ID / Name)
  [ 3 ]  Account Full Detail
  [ 4 ]  Freeze Account
  [ 5 ]  Unfreeze Account
  [ 6 ]  Delete Account (Secure)
  [ 7 ]  Reset Account PIN Lock
  [ 8 ]  Apply Monthly Interest (All)
  [ 9 ]  System Analytics
  [10 ]  View Audit Log
  [11 ]  Search Audit Log
  [ 0 ]  Logout
  ──────────────────────────────────────────────────────
  Enter choice: _
```

---

### 9 · System Analytics Dashboard

```
  ┌─────────────────────────────────────────────────────┐
  │  SYSTEM ANALYTICS                                     │
  └─────────────────────────────────────────────────────┘

  Account Status Distribution
  Total Accounts:           1
  Active:                   1
  Frozen:                   0
  Locked:                   0
  Closed:                   0

  Account Type Distribution
  Savings:                  1
  Current:                  0
  Salary:                   0

  Financial Summary
  Total Funds in System:    INR 30000.00
  Total Deposited:          INR 35000.00
  Total Withdrawn:          INR 5000.00
```

---

### 10 · All Accounts — Balance-Sorted Table View

```
  ┌─────────────────────────────────────────────────────┐
  │  ALL ACCOUNTS (2 total)                               │
  └─────────────────────────────────────────────────────┘
  Account ID   Name                   Type       Status       Balance        Phone
  ──────────   ──────────────────     ────────   ──────────   ──────────     ───────────
  SBK2634700   Cindhe Sai Mukesh Rao  Savings    ACTIVE       INR 30000.00   9876543210
  ADMIN001     System Administrator   Current    ACTIVE       INR 0.00       0000000000
```

---

### 11 · Security: Wrong PIN → Automatic Account Lockout

```
  Account ID : SBK2634700
  PIN        : ******
  ✘  ERROR: Incorrect PIN. Please try again.

  Account ID : SBK2634700
  PIN        : ******
  ✘  ERROR: Incorrect PIN. Please try again.

  Account ID : SBK2634700
  PIN        : ******
  ✘  ERROR: Account is LOCKED due to multiple failed PIN attempts.
  ℹ  Please contact SecurBank support to unlock your account.
```

---

### 12 · Live Structured Audit Log — Real Session Output

```
=== SESSION START: 2026-06-28 05:59:01 ===
[2026-06-28 05:59:01] [AUDIT   ] ACTOR=SYSTEM           ACTION=ACCOUNT_CREATED    TARGET=ADMIN001     DETAIL=System Administrator
[2026-06-28 05:59:07] [AUDIT   ] ACTOR=SYSTEM           ACTION=ACCOUNT_CREATED    TARGET=SBK2634700   DETAIL=Cindhe Sai Mukesh Rao
[2026-06-28 05:59:07] [AUDIT   ] ACTOR=SBK2634700       ACTION=DEPOSIT            TARGET=SBK2634700   DETAIL=Initial account deposit
=== SESSION END: 2026-06-28 05:59:12 ===

=== SESSION START: 2026-06-28 05:59:24 ===
[2026-06-28 05:59:26] [AUDIT   ] ACTOR=SBK2634700       ACTION=LOGIN_SUCCESS       TARGET=SBK2634700   DETAIL=User login
[2026-06-28 05:59:30] [AUDIT   ] ACTOR=SBK2634700       ACTION=DEPOSIT             TARGET=SBK2634700   DETAIL=Salary Credit
[2026-06-28 05:59:34] [AUDIT   ] ACTOR=SBK2634700       ACTION=WITHDRAWAL          TARGET=SBK2634700   DETAIL=ATM Withdrawal
[2026-06-28 05:59:38] [AUDIT   ] ACTOR=SBK2634700       ACTION=LOGOUT              TARGET=SBK2634700   DETAIL=
=== SESSION END: 2026-06-28 05:59:38 ===

=== SESSION START: 2026-06-28 06:00:08 ===
[2026-06-28 06:00:10] [AUDIT   ] ACTOR=ADMIN001         ACTION=LOGIN_SUCCESS       TARGET=ADMIN001     DETAIL=Admin login
[2026-06-28 06:00:11] [AUDIT   ] ACTOR=ADMIN001         ACTION=ADMIN_ANALYTICS     TARGET=             DETAIL=
[2026-06-28 06:00:12] [AUDIT   ] ACTOR=ADMIN001         ACTION=ADMIN_LIST_ACCOUNTS TARGET=             DETAIL=
[2026-06-28 06:00:14] [AUDIT   ] ACTOR=ADMIN001         ACTION=LOGOUT              TARGET=ADMIN001     DETAIL=
=== SESSION END: 2026-06-28 06:00:18 ===
```

---

## 🚀 Quick Start

### Prerequisites

```bash
# Ubuntu / Debian / WSL2
sudo apt update && sudo apt install gcc make -y

# Fedora / RHEL / CentOS
sudo dnf install gcc make -y

# macOS
xcode-select --install

# Verify both are installed
gcc --version && make --version
```

### Option A — Self-Extracting Installer (Recommended)

Download `build_securbank.sh` — a single script with all 18 source files embedded inside:

```bash
chmod +x build_securbank.sh
bash build_securbank.sh
cd BankingSystem
./securbank
```

The script will automatically:
1. Verify `gcc` and `make` are available
2. Create the `BankingSystem/` directory structure
3. Extract all 18 source and header files
4. Compile with `-Wall -Wextra -O2 -std=c99 -lm`
5. Print run instructions with default credentials

### Option B — Manual Build from Source

```bash
git clone https://github.com/CindheSai/securbank.git
cd securbank/BankingSystem

# Build using Makefile (recommended)
make

# Or compile manually with a single GCC command
gcc -Wall -Wextra -O2 -std=c99 \
    main.c utils.c security.c logger.c database.c \
    account.c transaction.c auth.c admin.c \
    -lm -o securbank

# Launch
./securbank
```

### Makefile Targets

```bash
make              # Compile the binary
make run          # Compile and run immediately
make clean        # Remove all .o files and binary
make clean_data   # Wipe the data/ directory (completely fresh database)
make purge        # Remove binary, objects, and all data files
```

---

## 🖱 Usage Guide

### Default Admin Credentials

```
Account ID : ADMIN001
PIN        : 000000
```

> ⚠️ Change the admin PIN immediately on first login.

### Creating Your First Account

```
Main Menu → [ 2 ] Open New Account

  Full Name    : Your Full Name             (letters, spaces, dots, hyphens)
  Phone        : 10-digit number
  Email        : valid@email.com
  Address      : Your address
  Account Type : 1 Savings / 2 Current / 3 Salary
  Deposit      : Minimum INR 500
  PIN          : 4–6 digits (no trivial sequences: 1234, 0000, 1111...)
```

**Save the Account ID printed on screen** — it is your permanent login identifier.

### Transaction Limits by Account Type

| Account Type | Daily Limit | Interest Rate | Use Case |
|---|---|---|---|
| 💰 Savings | ₹50,000 / day | 3.5% p.a. monthly | Personal savings and investments |
| 🏢 Current | ₹2,00,000 / day | 0% | Business transactions, high volume |
| 💼 Salary | ₹1,00,000 / day | 4.0% p.a. monthly | Employee salary disbursement |

### Data Files

| File | Format | Contents |
|---|---|---|
| `data/accounts.dat` | Binary | `DBHeader` + `Account[]` with per-record checksums |
| `data/transactions.log` | Binary | Append-only immutable `Transaction[]` ledger |
| `data/audit.log` | Plaintext | Structured audit trail — every action, timestamped |
| `data/system.log` | Plaintext | System lifecycle events, errors, session markers |

```bash
# View the live audit trail
cat data/audit.log

# Follow in real-time during an active session
tail -f data/audit.log

# Find all operations on a specific account
grep "SBK2634700" data/audit.log

# Find all admin actions
grep "ADMIN001" data/audit.log
```

---

## 🗺 Roadmap

### v1.1 — Planned
- [ ] **OTP Simulation** — TOTP-style time-based one-time code for high-value transactions
- [ ] **Statement Export** — CSV / plaintext export of full transaction history from the binary ledger
- [ ] **Multi-currency Support** — USD, EUR, INR account balances with live rate-based conversion
- [ ] **Loan Module** — Simple loan origination, EMI schedule generation, and repayment ledger

### v2.0 — In Research
- [ ] **SQLite Integration** — Replace binary file DB with embedded relational storage engine
- [ ] **AI Fraud Detection** — Statistical transaction anomaly detection using Z-score and velocity checks
- [ ] **REST API Layer** — HTTP interface via lightweight C server (`libmicrohttpd`) for frontend integration
- [ ] **ncurses GUI** — Full terminal UI with panels, windows, color themes, and keyboard navigation
- [ ] **Multi-user Concurrency** — POSIX file locking (`flock`) for simultaneous multi-session support

### v3.0 — Conceptual
- [ ] **Microservice Decomposition** — Split auth, account, and transaction modules into independent networked C services
- [ ] **HSM Simulation** — Hardware Security Module emulation for cryptographic key lifecycle management
- [ ] **Blockchain Audit Trail** — Hash-chained immutable log where each entry references the previous entry's hash
- [ ] **Mobile Frontend** — React Native app communicating with the v2.0 REST API over HTTPS

---

## 🤝 Contributing

Contributions from systems engineers, security researchers, and C programmers are welcome.

```bash
# Fork → Clone → Branch
git clone https://github.com/CindheSai/securbank.git
cd securbank
git checkout -b feature/your-feature-name

# Make your changes — verify zero new warnings
make clean && make

# Commit descriptively
git commit -m "feat(module): short description of what and why"

# Push and open a Pull Request
git push origin feature/your-feature-name
```

**Code Standards:**
- C99 compliant, POSIX-compatible
- Zero GCC warnings under `-Wall -Wextra -Wpedantic -Wformat=2`
- Every public function must be declared in its module's `.h` header
- Every security-sensitive operation must emit an audit log entry before returning
- New features must not break `make clean && make`
- No external library dependencies — standard C library only

---

## 📄 License

```
MIT License

Copyright (c) 2026 Cindhe Sai Mukesh Rao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 👤 Author

<div align="center">

<br/>

**Cindhe Sai Mukesh Rao**

*Systems Engineer · Backend Developer · Fintech Enthusiast*

<br/>

[![GitHub](https://img.shields.io/badge/GitHub-CindheSai-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/CindheSai)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Cindhe%20Sai%20Mukesh%20Rao-0A66C2?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/cindhe-sai-mukesh-rao-9a57a53ba/)
[![Email](https://img.shields.io/badge/Email-saimukeshraocindhe%40gmail.com-EA4335?style=for-the-badge&logo=gmail&logoColor=white)](mailto:saimukeshraocindhe@gmail.com)

<br/>

</div>

---

<div align="center">

**SecurBank** — Because financial systems deserve better than shortcuts.

<br/>

*3,490 lines of pure C · 9 decoupled modules · Zero external dependencies · Bank-grade security*

<br/>

⭐ **Star this repository** if it helped you understand production-grade systems design in C.

<br/>

</div>
