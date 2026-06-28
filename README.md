# SecurBank — Core Banking Management System (C99)

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

SecurBank — A bank-grade, modular Core Banking System written in C (C99).  
Built for engineers who want every low-level detail: atomic persistence, append-only ledgers, strong PIN stretching, and auditable trails.

</div>

---

Badges
- Language: C99
- Build: GCC | Make
- License: MIT
- Status: Production-Ready (design)
- LOC: 3500+

---

Maintained by
- Name: Cindhe Sai Mukesh Rao
- GitHub: @CindheSai
- LinkedIn: https://www.linkedin.com/in/cindhe-sai-mukesh-rao-9a57a53ba/
- Email: saimukeshraocindhe@gmail.com
- Copyright (c) 2026 Cindhe Sai Mukesh Rao

---

TL;DR — Why SecurBank
- Realistic core banking patterns without external DBs.
- Append-only transaction ledger + per-record checksums.
- 4096-round DJB2-style PIN stretching with per-account salt.
- Atomic rename persistence and structured audit logs.
- Minimal dependencies — pure C99, POSIX-friendly.

Table of Contents
1. Features
2. Architecture
3. Security Summary
4. Quick Start
5. Running & Screenshots
6. Data Files
7. Contributing
8. Roadmap
9. License & Contacts

---

1) Features (Insane Level)
- Account lifecycle: registration, KYC fields, types: Savings / Current / Salary
- Secure login: AccountID + PIN (never stored plaintext)
- Session tokens with 15-minute inactivity timeout
- Re-auth for sensitive ops (withdraw, transfer, delete)
- Append-only binary transaction ledger + checksums
- Audit and system logs (structured, timestamped)
- Daily limits, interest engine, admin analytics, freeze/unfreeze accounts
- Recovery-minded persistence: magic header, header checksum, record checksums
- Atomic writes: write to .tmp then rename()
- Trivial PIN blocklist & account lockout (3 failed attempts)

---

2) System Architecture (short)
Layers are unidirectional; presentation → business → infra → foundation.

Files:
- main.c (menus)
- auth.c / auth.h (session + PIN checks)
- account.c / account.h (business rules)
- transaction.c / transaction.h (ledger)
- database.c / database.h (binary persistence)
- security.c / security.h (salt/hash/checksum)
- logger.c / logger.h (audit/system logs)
- utils.c / utils.h (helpers, safe I/O)
- admin.c / admin.h (analytics, admin actions)
- Makefile

---

3) Security Summary (short)
- Per-account unique 16-char salt
- 4096-round DJB2 stretch + XOR folding -> hex digest
- Stored: salt + stretched hash only
- DB header magic + version + checksum
- Per-record checksum validated on load
- Session tokens: 32-char hex, 15-min timeout
- Lockout: 3 failed PIN attempts
- Atomic writes and append-only ledger

---

4) Quick Start (one-liner to build)
Prereqs: gcc, make (POSIX)

Clone & Build:
```bash
git clone https://github.com/CindheSai/securbank.git
cd securbank
make
./securbank
```

Or manual compile:
```bash
gcc -Wall -Wextra -O2 -std=c99 \
  main.c utils.c security.c logger.c database.c \
  account.c transaction.c auth.c admin.c \
  -lm -o securbank
./securbank
```

Makefile targets:
- make        -> build
- make run    -> build + run
- make clean  -> remove objects
- make clean_data -> wipe data/
- make purge  -> remove binary + data

Default admin on first run:
- Account ID: ADMIN001
- PIN: 000000 (change immediately)

---

5) Running & Screenshots (mockups + how to capture real ones)

Important: I can't attach binary images here, so below are faithful terminal screenshot mockups you can paste as files or reproduce. I also give exact commands to capture real terminal screenshots and where to place them in the repo.

Mockup — Main Menu (terminal)
```text
[SCREENSHOT: screenshots/main_menu.txt]
  ╔══════════════════════════════════════════════════════╗
  ║         SECURBANK — CORE BANKING SYSTEM v1.0         ║
  ╚══════════════════════════════════════════════════════╝

  [SYSTEM] Admin account created.
  [SYSTEM] Default PIN: 000000 — Change immediately after login!

  [ 1 ]  Login to Your Account
  [ 2 ]  Open New Account
  [ 9 ]  Admin Panel
  [ 0 ]  Exit System
```

Mockup — Register (terminal)
```text
[SCREENSHOT: screenshots/registration.txt]
Full Name     : Jane Doe
Phone         : 9876543210
Email         : jane@example.com
Address       : 42 Finance Street, Mumbai
Account Type  : 1  (Savings)
Initial Deposit: 10000
PIN           : [hidden]
Confirm PIN   : [hidden]

✔ Account created!
Your Account ID: SBK2165642
```

Mockup — Mini Statement (terminal)
```text
[SCREENSHOT: screenshots/mini_statement.txt]
Mini Statement for SBK2165642
----------------------------------------------------
#  Timestamp              Type       Amount     Bal
1  2026-06-28 11:22:05    DEPOSIT    +10000     10000
2  2026-07-01 09:14:12    WITHDRAW   -200       9800
...
```

How to capture real terminal screenshots (examples)
- Capture as PNG with GNOME:
  gnome-screenshot -w -f screenshots/main_menu.png
- Capture terminal output as an image (using terminal's PrintScreen or a tool like wkhtmltoimage on an HTML-rendered terminal capture)
- Save terminal text output (easy, reproducible):
  ./securbank | tee screenshots/run_output.txt
- To create PNG from text (quick), you can:
  convert -size 800x600 -background black -fill white -font DejaVu-Sans-Mono -pointsize 12 label:@screenshots/run_output.txt screenshots/main_menu.png

Place screenshots (recommended files):
- screenshots/main_menu.png
- screenshots/registration.png
- screenshots/user_dashboard.png
- screenshots/mini_statement.png
- screenshots/admin_panel.png

Add them to the repo:
```bash
mkdir -p screenshots
# copy/produce the PNGs into screenshots/
git add screenshots/*
git commit -m "Add screenshots for README"
```

---

6) Data files (auto-created inside data/)
- data/accounts.dat        — Binary DB (header + Account records)
- data/transactions.log    — Binary append-only ledger
- data/audit.log           — Plaintext structured audit log
- data/system.log          — System lifecycle & error messages

Inspect logs:
```bash
tail -n 200 data/audit.log
grep "SBK2165642" data/audit.log
```

---

7) Testing & Safety
- All inputs clamped and sanitized
- Run unit-style quick checks using: make test (if you add tests)
- For safe experimentation: make clean_data then run to create a fresh DB

---

8) Contributing
- Fork, feature branch, and open PRs.
- Maintain C99 and keep zero warnings: -Wall -Wextra -Wpedantic
- Every public function must be declared in .h and documented
- Security-sensitive ops must write an audit entry

Suggested PR workflow:
```bash
git checkout -b feature/your-feature
# edit code
make && make test
git push origin feature/your-feature
# create PR
```

---

9) Roadmap (selected)
- v1.1: OTP simulation, recurring transactions, statement export
- v2.0: SQLite option, REST API, ncurses UI
- v3.0: Microservices, HSM emulation, blockchain audit chain

---

License
MIT License — see LICENSE (Copyright (c) 2026 Cindhe Sai Mukesh Rao)

---

Contacts & Credits
- Author: Cindhe Sai Mukesh Rao — saimukeshraocindhe@gmail.com
- GitHub: https://github.com/CindheSai
- LinkedIn: https://www.linkedin.com/in/cindhe-sai-mukesh-rao-9a57a53ba/

---

Appendix — Terminal session example (copyable)
```bash
# Build and run (example)
make
./securbank

# Create a new user (interactive)
# Use the on-screen menu; to export the mini-statement:
./securbank --export-mini SBK2165642 > screenshots/mini_statement.txt
```
