#ifndef SECURITY_H
#define SECURITY_H

/* ============================================================
 * security.h — Cryptographic & security interfaces
 * Bank-Grade Banking Management System
 * ============================================================ */

#include "utils.h"

/* ---- PIN security ---- */
void   sec_generate_salt(char *salt, size_t len);
void   sec_hash_pin(const char *pin, const char *salt, char *out_hash, size_t out_len);
int    sec_verify_pin(const char *pin, const char *salt, const char *stored_hash);

/* ---- Lightweight integrity hash (anti-tamper) ---- */
unsigned long  sec_djb2_hash(const unsigned char *data, size_t len);
void           sec_compute_record_checksum(const void *data, size_t data_len, unsigned long *out_cs);
int            sec_verify_record_checksum(const void *data, size_t data_len, unsigned long stored_cs);

/* ---- Input sanitization ---- */
void   sec_sanitize_name(char *s);
void   sec_sanitize_numeric(char *s);
int    sec_validate_pin_format(const char *pin);
int    sec_validate_amount(double amount);
int    sec_validate_account_id(const char *id);
int    sec_validate_email(const char *email);
int    sec_validate_phone(const char *phone);

/* ---- XOR obfuscation for in-memory sensitive data ---- */
void   sec_xor_obfuscate(unsigned char *data, size_t len, const unsigned char *key, size_t key_len);

/* ---- Session token ---- */
void   sec_generate_token(char *token, size_t len);

#endif /* SECURITY_H */
