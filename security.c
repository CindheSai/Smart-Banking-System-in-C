/* ============================================================
 * security.c — Cryptographic & security implementations
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "security.h"
#include "account.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ============================================================
 * Salt generation
 * ============================================================ */

static const char SALT_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$";

void sec_generate_salt(char *salt, size_t len) {
    if (!salt || len < 2) return;
    /* Seed with high-resolution time bits XOR'd together */
    unsigned int seed = (unsigned int)(time(NULL) ^ (unsigned long)salt ^ 0xDEADBEEFUL);
    srand(seed);
    for (size_t i = 0; i < len - 1; i++) {
        salt[i] = SALT_CHARS[rand() % (sizeof(SALT_CHARS) - 1)];
    }
    salt[len - 1] = '\0';
}

/* ============================================================
 * Custom lightweight hash (DJB2 variant + XOR mixing)
 * Production note: replace with bcrypt/argon2 in real deployment.
 * This simulates structured PIN hashing for C-only constraint.
 * ============================================================ */

unsigned long sec_djb2_hash(const unsigned char *data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) ^ data[i];
    }
    return hash;
}

/* Multi-round stretched hash of PIN+SALT -> hex string */
void sec_hash_pin(const char *pin, const char *salt, char *out_hash, size_t out_len) {
    if (!pin || !salt || !out_hash || out_len < 17) return;

    /* Stage 1: concatenate pin + salt into working buffer */
    char work[512];
    snprintf(work, sizeof(work), "%s%s%s", salt, pin, salt);

    /* Stage 2: 4096 rounds of stretching */
    unsigned long h = 5381;
    for (int round = 0; round < 4096; round++) {
        for (size_t i = 0; work[i]; i++) {
            h = ((h << 5) + h) ^ (unsigned char)work[i];
            h ^= (unsigned long)(round * 31 + i * 7);
        }
        /* Mix in round-derived constant */
        h ^= ((unsigned long)(round + 1) * 0x9e3779b9UL);
    }

    /* Stage 3: XOR-fold down to 8 bytes for hex representation */
    unsigned char bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (unsigned char)((h >> (i * 8)) & 0xFF);
        h = ((h << 5) + h) ^ bytes[i];
    }

    /* Stage 4: second pass hash over bytes */
    unsigned long h2 = sec_djb2_hash(bytes, 8);
    unsigned char bytes2[8];
    for (int i = 0; i < 8; i++) {
        bytes2[i] = (unsigned char)((h2 >> (i * 8)) & 0xFF);
    }

    /* Output: combine both hashes as 32-char hex */
    size_t written = 0;
    for (int i = 0; i < 8 && written + 2 < out_len; i++) {
        written += snprintf(out_hash + written, out_len - written, "%02x", bytes[i]);
    }
    for (int i = 0; i < 8 && written + 2 < out_len; i++) {
        written += snprintf(out_hash + written, out_len - written, "%02x", bytes2[i]);
    }
    out_hash[written] = '\0';
}

int sec_verify_pin(const char *pin, const char *salt, const char *stored_hash) {
    if (!pin || !salt || !stored_hash) return 0;
    char computed[HASH_LEN];
    sec_hash_pin(pin, salt, computed, sizeof(computed));
    return (strcmp(computed, stored_hash) == 0);
}

/* ============================================================
 * Record checksum (anti-tamper)
 * ============================================================ */

void sec_compute_record_checksum(const void *data, size_t data_len, unsigned long *out_cs) {
    if (!data || !out_cs) return;
    *out_cs = sec_djb2_hash((const unsigned char *)data, data_len);
}

int sec_verify_record_checksum(const void *data, size_t data_len, unsigned long stored_cs) {
    unsigned long computed = 0;
    sec_compute_record_checksum(data, data_len, &computed);
    return (computed == stored_cs);
}

/* ============================================================
 * Input sanitization
 * ============================================================ */

void sec_sanitize_name(char *s) {
    if (!s) return;
    str_trim(s);
    /* Allow letters, spaces, dots, hyphens */
    for (size_t i = 0; s[i]; i++) {
        if (!isalpha((unsigned char)s[i]) &&
            s[i] != ' ' && s[i] != '.' && s[i] != '-' && s[i] != '\'') {
            s[i] = '?';
        }
    }
}

void sec_sanitize_numeric(char *s) {
    if (!s) return;
    str_trim(s);
    for (size_t i = 0; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) s[i] = '0';
    }
}

int sec_validate_pin_format(const char *pin) {
    if (!pin) return 0;
    size_t len = strlen(pin);
    if (len < 4 || len > 6) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)pin[i])) return 0;
    }
    /* Reject trivial PINs */
    const char *trivial[] = {"0000", "1111", "2222", "3333", "4444",
                              "5555", "6666", "7777", "8888", "9999",
                              "1234", "4321", "0000", "123456", "000000", NULL};
    for (int i = 0; trivial[i]; i++) {
        if (strcmp(pin, trivial[i]) == 0) return 0;
    }
    return 1;
}

int sec_validate_amount(double amount) {
    return (amount >= MIN_DEPOSIT && amount <= MAX_SINGLE_TXN);
}

int sec_validate_account_id(const char *id) {
    if (!id) return 0;
    size_t len = strlen(id);
    if (len < 6 || len >= ACCOUNT_ID_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)id[i])) return 0;
    }
    return 1;
}

int sec_validate_email(const char *email) {
    if (!email) return 0;
    const char *at = strchr(email, '@');
    if (!at) return 0;
    if (at == email) return 0;
    const char *dot = strchr(at, '.');
    if (!dot || dot == at + 1) return 0;
    if (strlen(email) > EMAIL_LEN - 1) return 0;
    return 1;
}

int sec_validate_phone(const char *phone) {
    if (!phone) return 0;
    size_t len = strlen(phone);
    if (len < 7 || len > PHONE_LEN - 1) return 0;
    size_t start = (phone[0] == '+') ? 1 : 0;
    for (size_t i = start; i < len; i++) {
        if (!isdigit((unsigned char)phone[i])) return 0;
    }
    return 1;
}

/* ============================================================
 * XOR obfuscation (for in-memory PIN buffer zeroing helper)
 * ============================================================ */

void sec_xor_obfuscate(unsigned char *data, size_t len,
                        const unsigned char *key, size_t key_len) {
    if (!data || !key || key_len == 0) return;
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key[i % key_len];
    }
}

/* ============================================================
 * Session token generator
 * ============================================================ */

void sec_generate_token(char *token, size_t len) {
    if (!token || len < 2) return;
    static const char chars[] = "0123456789ABCDEF";
    unsigned int seed = (unsigned int)(time(NULL) ^ (unsigned long)token);
    srand(seed);
    for (size_t i = 0; i < len - 1; i++) {
        token[i] = chars[rand() % 16];
    }
    token[len - 1] = '\0';
}
