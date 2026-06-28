<div align="center">

<br/>

```
 ███████╗███████╗ ██████╗██╗   ██╗██████╗ ██████╗  █████╗ ███╗   ██╗██╗  ██╗
 ██╔════╝██╔════╝██╔════╝██║   ██║██╔══██╗██╔══██╗██╔══██╗████╗  ██║██║ ██╔╝
 ███████╗█████╗  ██║     ██║   ██║██████╔╝██████╔╝███████║██╔██╗ ██║█████╔╝ 
 ╚════██║██╔══╝  ██║     ██║   ██║██╔══██╗██╔══██╗██╔══██║██║╚██╗██║██╔═██╗ 
 ███████║███████╗╚██████╗╚██████╔╝██║  ██║██████╔╝██║  ██║██║ ╚████║██║  ██╗
 ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝
```

# SecurBank — Core Banking Management System

**A bank-grade, modular financial transaction engine built entirely in C.**  
Designed with the architecture principles of real-world fintech backends.

<br/>

[![Language](https://img.shields.io/badge/Language-C99-blue?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C99)
[![Build](https://img.shields.io/badge/Build-GCC%20%7C%20Make-brightgreen?style=for-the-badge&logo=gnu&logoColor=white)](https://gcc.gnu.org/)
[![Architecture](https://img.shields.io/badge/Architecture-Modular%20MVC-orange?style=for-the-badge)](https://github.com)
[![Storage](https://img.shields.io/badge/Storage-Binary%20File%20DB-purple?style=for-the-badge)](https://github.com)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](./LICENSE)
[![Status](https://img.shields.io/badge/Status-Production%20Ready-success?style=for-the-badge)](https://github.com)
[![Lines of Code](https://img.shields.io/badge/Lines%20of%20Code-3500%2B-red?style=for-the-badge)](https://github.com)

<br/>

[Features](#-features) · [Architecture](#-system-architecture) · [Security](#-security-design) · [Quick Start](#-quick-start) · [Documentation](#-how-it-works) · [Roadmap](#-roadmap)

<br/>

</div>

---

## 📌 Overview

**SecurBank** is a fully functional, bank-grade **Core Banking Management System (CBS)** implemented in pure C. It simulates the backend engine of a real financial institution — handling account lifecycle management, multi-type transaction processing, a cryptographically-protected authentication system, an append-only transaction ledger, and a structured administrative control plane.

Built without any external libraries or database engines, SecurBank demonstrates how production-grade financial systems architect their persistence, security, and business logic layers — using only the C standard library and disciplined software engineering.

> **This is not a toy project.** Every design decision — from atomic file writes to per-record checksums to PIN-stretch hashing — reflects patterns used in actual banking infrastructure.

### Why This Matters

| Concern | SecurBank's Approach |
|---|---|
| **Data Integrity** | Per-record DJB2 checksums detect file tampering at load time |
| **Persistence** | Atomic rename-based writes prevent DB corruption on crash |
| **Authentication** | 4096-round PIN stretch hashing with per-account random salts |
| **Auditability** | Structured, timestamped, append-only audit log for every action |
| **Concurrency Safety** | Session tokens, activity timeouts, and re-authentication gates |
| **Separation of Concerns** | 9 fully decoupled modules with clean header-defined interfaces |

---

## ✨ Features

### 👤 User Module
- **Account Registration** — Open Savings, Current, or Salary accounts with full KYC fields
- **Secure Login** — Account ID + PIN authentication with session token generation
- **Real-time Balance** — Live balance with daily transaction utilization tracking
- **Cash Deposit** — Single-transaction and daily-limit-enforced deposits
- **Cash Withdrawal** — PIN-gated withdrawals with balance and limit validation
- **Fund Transfer** — Account-to-account transfers with real-time debit/credit ledgering
- **Mini Statement** — Last 10 transactions retrieved from the append-only binary ledger
- **PIN Management** — Secure PIN change with old-PIN verification and format enforcement

### 🧑‍💼 Admin Module
- **Account Directory** — Paginated, balance-sorted view of all accounts in the system
- **Multi-mode Search** — Lookup by Account ID, partial name match, or audit log keyword
- **Account Detail View** — Full profile + inline mini-statement for any account
- **Freeze / Unfreeze** — Reversible account suspension with audit trail
- **Secure Deletion** — Hard-delete with mandatory admin PIN re-authentication and confirmation gate
- **PIN Lock Reset** — Unlock accounts with forced PIN reassignment
- **Batch Interest** — Apply monthly compounded interest across all eligible accounts in one operation
- **System Analytics** — Real-time dashboard: account counts by status/type, total funds, deposit/withdrawal volumes
- **Audit Log Viewer** — Tail or keyword-search the full structured audit trail

### 🔐 Security Module
- **PIN Hashing** — Custom 4096-round DJB2 stretch function with per-account random salt (XOR + fold mixing)
- **Account Lockout** — Automatic lock after 3 consecutive failed PIN attempts
- **Session Management** — In-memory session tokens with 15-minute inactivity timeout
- **Re-authentication Gates** — Sensitive operations (withdraw, transfer, delete) require live PIN re-entry
- **Record Checksums** — Every `Account` struct carries a computed integrity checksum validated on DB load
- **Database Header Integrity** — Magic bytes, version field, and header checksum protect the binary DB file
- **Atomic Writes** — All DB saves write to a `.tmp` file first, then atomically rename — crash-safe
- **Input Sanitization** — All user input is length-clamped, control-character stripped, and type-validated
- **Trivial PIN Rejection** — Hardcoded blocklist rejects `0000`, `1234`, `123456`, and similar sequences

### 📊 Advanced System Features
- **Append-Only Transaction Ledger** — Binary log file; records are never modified, only appended
- **Transaction Checksums** — Every ledger record carries its own integrity signature
- **Interest Calculation Engine** — Type-aware monthly interest: Savings (3.5%), Salary (4.0%), Current (0%)
- **Daily Limit Enforcement** — Per-account-type rolling daily transaction caps reset at midnight
- **Account Balance Consistency** — Dual-field tracking (`total_deposited` vs `total_withdrawn`) for reconciliation
- **Structured Audit Logging** — Actor, action, target, timestamp, and severity level on every log entry
- **Balance-sorted Reporting** — Admin account list uses in-place selection sort by balance descending
- **System Analytics Engine** — Aggregated financial metrics computed live from in-memory account state

---

## 🏗 System Architecture

SecurBank is designed around a **strict layered architecture** with unidirectional dependencies. No lower layer knows about any higher layer.

```
┌─────────────────────────────────────────────────────────┐
│                   PRESENTATION LAYER                    │
│              main.c  ·  admin.c  (UI/Menus)            │
├─────────────────────────────────────────────────────────┤
│                   BUSINESS LOGIC LAYER                  │
│     account.c  ·  transaction.c  ·  auth.c             │
├─────────────────────────────────────────────────────────┤
│                   INFRASTRUCTURE LAYER                  │
│          database.c  ·  logger.c  ·  security.c        │
├─────────────────────────────────────────────────────────┤
│                   FOUNDATION LAYER                      │
│                       utils.c                          │
└─────────────────────────────────────────────────────────┘
```

### Module Responsibilities

| Module | File | Responsibility |
|---|---|---|
| **Utils** | `utils.c / .h` | Terminal UI, safe I/O, string sanitization, timestamps |
| **Security** | `security.c / .h` | PIN hashing, salts, checksums, input validators, XOR obfuscation |
| **Logger** | `logger.c / .h` | Structured audit log, system log, log reader/search |
| **Database** | `database.c / .h` | Binary file persistence, atomic writes, header integrity |
| **Account** | `account.c / .h` | Account CRUD, deposit/withdraw/transfer, limits, interest |
| **Transaction** | `transaction.c / .h` | Append-only binary ledger, mini-statement, analytics |
| **Auth** | `auth.c / .h` | Session lifecycle, PIN verification, re-auth, lockout |
| **Admin** | `admin.c / .h` | Administrative control plane, analytics, audit viewer |
| **Main** | `main.c` | Application bootstrap, menu orchestration, user flows |

### Data Flow: A Withdrawal Transaction

```
User Input (PIN + Amount)
        │
        ▼
   auth_reauth()          ← re-verifies PIN against stored hash
        │
        ▼
  acct_withdraw()         ← validates amount, daily limit, balance
        │
        ▼
   txn_record()           ← appends record to binary ledger
        │
        ▼
  db_save_accounts()      ← atomic write to accounts.dat
        │
        ▼
   log_audit()            ← structured entry written to audit.log
```

---

## 📁 Project Structure

```
BankingSystem/
│
├── main.c                  # Entry point, menu orchestration, user flows
│
├── auth.c / auth.h         # Authentication, session tokens, PIN verification
├── account.c / account.h   # Account model, CRUD, business logic, limits
├── transaction.c / .h      # Append-only binary ledger, mini-statement
├── admin.c / admin.h       # Admin control panel, analytics
├── security.c / security.h # Hashing, salting, checksums, input validation
├── database.c / database.h # Binary file DB, atomic writes, integrity checks
├── logger.c / logger.h     # Audit + system logging
├── utils.c / utils.h       # I/O, UI helpers, string utils, time
│
├── Makefile                # Build system
│
└── data/                   # Runtime-generated (auto-created on first run)
    ├── accounts.dat        # Binary account database (magic-header protected)
    ├── transactions.log    # Append-only binary transaction ledger
    ├── audit.log           # Human-readable structured audit trail
    └── system.log          # System lifecycle and error events
```

---

## 🛠 Technologies Used

| Technology | Usage |
|---|---|
| **C99** | Core language; chosen for system-level control and performance |
| **GCC / GNU Make** | Compilation toolchain with strict warning flags (`-Wall -Wextra -Wpedantic`) |
| **Binary File I/O** | `fread` / `fwrite` for structured binary account and transaction records |
| **Atomic File Rename** | `rename()` syscall for crash-safe database persistence |
| **POSIX Terminal API** | `termios` for hidden PIN entry (echo suppression) |
| **Standard C Library** | `time.h`, `string.h`, `stdlib.h` — zero external dependencies |
| **Custom Hash Function** | DJB2-variant with multi-round stretching — no OpenSSL required |
| **Append-only Logging** | Binary and text log files opened in `"ab"` / `"a"` mode exclusively |

**Build Requirements:**
- GCC 7.0+ (tested on GCC 13.3)
- GNU Make 4.0+
- POSIX-compatible OS (Linux, macOS, WSL2)

---

## 🔐 Security Design

### PIN Protection Pipeline

```
User PIN ("7291")
      │
      ▼
sec_generate_salt()     → 16-char random salt per account (stored)
      │
      ▼
sec_hash_pin()          → salt + PIN + salt → 4096 rounds of DJB2 stretch
      │                   XOR mixing + fold → 32-char hex digest
      ▼
stored in Account.pin_hash (never stored in plaintext, ever)
```

### Integrity Verification Pipeline

```
accounts.dat loaded
      │
      ▼
DBHeader verified       → magic (0x42414E4B) + version + header checksum
      │
      ▼
Per-record checksum     → each Account struct checksum verified on read
      │
      ▼
Tampered records        → skipped + logged as CRITICAL in system.log
```

### Session Security Model

```
Successful Login
      │
      ├─ Session token generated (32-char random hex)
      ├─ Login timestamp recorded
      ├─ 15-minute inactivity timer started
      │
      ▼
Each sensitive operation
      ├─ auth_is_authenticated() — checks session active + not expired
      └─ auth_reauth(pin) — for withdraw / transfer / delete operations
```

### Security Checklist

- [x] Passwords (PINs) never stored in plaintext
- [x] Per-account unique salt prevents rainbow table attacks
- [x] Key-stretching (4096 rounds) slows brute-force attempts
- [x] Account lockout after 3 failed attempts
- [x] Session timeout after 15 minutes of inactivity
- [x] Re-authentication required for all financial transactions
- [x] All user input sanitized before processing
- [x] Trivial PIN blocklist enforced at registration and change
- [x] Database file integrity verified on every startup
- [x] All security events logged with actor, timestamp, and severity

---

## ⚙️ How It Works

### Account Lifecycle

```
Registration → PIN Setup → Account Created → Active
                                               │
                              ┌────────────────┼────────────────┐
                              ▼                ▼                ▼
                           Deposit         Withdraw          Transfer
                              │                │                │
                              └────────────────┴────────────────┘
                                               │
                                    Ledger Entry Written
                                    Balance Updated
                                    Daily Limit Tracked
                                    Audit Log Entry Written
                                    DB Atomically Saved
```

### Transaction Limits

| Account Type | Daily Limit | Interest Rate | Min Balance |
|---|---|---|---|
| Savings | ₹50,000 | 3.5% p.a. | ₹500 |
| Current | ₹2,00,000 | 0% | ₹500 |
| Salary | ₹1,00,000 | 4.0% p.a. | ₹500 |

### Interest Calculation

```
Monthly Interest = (Account Balance × Annual Rate%) / 12
Applied via: Admin Panel → Apply Monthly Interest (All Accounts)
Recorded as: TXN_INTEREST entry in the transaction ledger
```

---

## 🚀 Quick Start

### Prerequisites

```bash
# Ubuntu / Debian / WSL2
sudo apt update && sudo apt install gcc make -y

# macOS
xcode-select --install

# Verify
gcc --version && make --version
```

### One-Command Install (Self-Extracting Script)

```bash
# Download build_securbank.sh, then:
chmod +x build_securbank.sh
bash build_securbank.sh

# The script will:
# 1. Check dependencies
# 2. Create BankingSystem/ directory
# 3. Extract all 18 source files
# 4. Compile the full system
# 5. Print run instructions
```

### Manual Build

```bash
# Clone / download and enter project directory
cd BankingSystem

# Build using Makefile
make

# Or compile manually with GCC
gcc -Wall -Wextra -O2 -std=c99 \
    main.c utils.c security.c logger.c database.c \
    account.c transaction.c auth.c admin.c \
    -lm -o securbank

# Run
./securbank
```

### Makefile Targets

```bash
make              # Build the binary
make run          # Build and run immediately
make clean        # Remove object files and binary
make clean_data   # Wipe all data files (fresh database)
make purge        # Remove everything including data
```

---

## 🖥 Usage

### First Run

```
./securbank

  ╔══════════════════════════════════════════════════════╗
  ║         SECURBANK — CORE BANKING SYSTEM v1.0        ║
  ╚══════════════════════════════════════════════════════╝

  [SYSTEM] Admin account created.
  [SYSTEM] Default PIN: 000000 — Change immediately after login!

  [ 1 ]  Login to Your Account
  [ 2 ]  Open New Account
  [ 0 ]  Exit System
```

### Default Admin Credentials

| Field | Value |
|---|---|
| Account ID | `ADMIN001` |
| PIN | `000000` |

> ⚠️ Change the admin PIN immediately on first login.

### Creating a User Account

```
Select option: 2

  Full Name     : Jane Doe
  Phone         : 9876543210
  Email         : jane@example.com
  Address       : 42 Finance Street, Mumbai
  Account Type  : 1  (Savings)
  Initial Deposit: 10000
  PIN           : 8472   ← 4-6 digits, not trivial
  Confirm PIN   : 8472
  Confirm       : y

  ✔  Account created!
  Your Account ID: SBK2165642
```

---

## 📸 Screenshots

> Screenshots from a live session — add your own by running the system and capturing terminal output.

| Screen | Preview |
|---|---|
| **Main Menu** | `screenshots/main_menu.png` |
| **Account Registration** | `screenshots/registration.png` |
| **User Dashboard** | `screenshots/user_dashboard.png` |
| **Mini Statement** | `screenshots/mini_statement.png` |
| **Admin Control Panel** | `screenshots/admin_panel.png` |
| **System Analytics** | `screenshots/analytics.png` |
| **Audit Log Viewer** | `screenshots/audit_log.png` |

---

## 📂 Data Files Reference

All data files are auto-created inside `data/` on first run.

| File | Format | Purpose |
|---|---|---|
| `accounts.dat` | Binary (`DBHeader` + `Account[]`) | Primary account database with integrity header |
| `transactions.log` | Binary (`Transaction[]`) | Append-only immutable transaction ledger |
| `audit.log` | Plaintext (structured) | Every user and admin action with timestamp and actor |
| `system.log` | Plaintext | System lifecycle events, errors, and startup/shutdown markers |

### Inspect the Audit Log

```bash
# View all audit entries
cat data/audit.log

# Watch live (during a session)
tail -f data/audit.log

# Search for a specific account's history
grep "SBK2165642" data/audit.log
```

---

## 🗺 Roadmap

### v1.1 — Planned
- [ ] **OTP Simulation** — Time-based one-time PIN for high-value transactions
- [ ] **Multi-currency Support** — USD, EUR, INR account balances with conversion
- [ ] **Statement Export** — Generate PDF/CSV mini-statements from the ledger
- [ ] **Recurring Transactions** — Scheduled deposits/transfers with date-based triggers

### v2.0 — In Research
- [ ] **SQLite Integration** — Replace binary file DB with embedded relational database
- [ ] **AI Fraud Detection** — Statistical anomaly detection on transaction patterns
- [ ] **REST API Layer** — HTTP interface using a lightweight C HTTP server (libmicrohttpd)
- [ ] **ncurses GUI** — Full terminal UI with panels, colors, and keyboard navigation
- [ ] **Multi-user Concurrency** — File locking for concurrent session support

### v3.0 — Conceptual
- [ ] **Microservice Decomposition** — Split modules into independent networked services
- [ ] **HSM Simulation** — Hardware Security Module emulation for PIN key management
- [ ] **Blockchain Audit Trail** — Tamper-evident chained log using hash linking
- [ ] **Mobile Frontend** — React Native app communicating over the REST API

---

## 🤝 Contributing

Contributions are welcome from systems programmers, security researchers, and fintech engineers.

```bash
# Fork the repo, then:
git clone https://github.com/yourusername/securbank.git
cd securbank

# Create a feature branch
git checkout -b feature/otp-simulation

# Make your changes, ensure no new warnings:
make clean && make

# Submit a pull request with a clear description
```

**Code Standards:**
- C99 compliant
- Zero GCC warnings with `-Wall -Wextra -Wpedantic`
- Every public function must have a declaration in its module's `.h` file
- All security-sensitive operations must write an audit log entry
- New features must not break existing `make` build

---

## 📄 License

```
MIT License

Copyright (c) 2025 [Your Name]

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

**[Your Name]**

*Systems Engineer · Fintech Backend Developer*

[![GitHub](https://img.shields.io/badge/GitHub-@yourusername-black?style=flat-square&logo=github)](https://github.com/yourusername)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Connect-blue?style=flat-square&logo=linkedin)](https://linkedin.com/in/yourprofile)
[![Email](https://img.shields.io/badge/Email-Contact-red?style=flat-square&logo=gmail)](mailto:you@example.com)

*Built with precision. Engineered for reliability.*

</div>

---

<div align="center">

**SecurBank** — Because financial systems deserve better than shortcuts.

⭐ Star this repository if it helped you understand real-world C systems design.

</div>
