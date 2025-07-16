#ifndef C_CHAT_H
#define C_CHAT_H

#include <errno.h>
#include <getopt.h>
#include <sodium.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Constants
#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 1024
#define MAX_COMMAND_LEN 64
#define SERVER_HOST "localhost"
#define SERVER_PORT 8080

// Cryptographic constants (libsodium)
#define PUBLIC_KEY_SIZE crypto_box_PUBLICKEYBYTES
#define PRIVATE_KEY_SIZE crypto_box_SECRETKEYBYTES
#define ENCRYPTED_MSG_SIZE (MAX_MESSAGE_LEN + crypto_box_SEALBYTES)
#define NONCE_SIZE crypto_box_NONCEBYTES
#define KEY_DERIVATION_SALT_SIZE crypto_pwhash_SALTBYTES
#define DERIVED_KEY_SIZE crypto_secretbox_KEYBYTES

// File paths
#define KEYS_DIR ".c-chat"
#define PRIVATE_KEY_FILE "private_key"
#define PUBLIC_KEY_FILE "public_key"

// Error codes
typedef enum {
  CCHAT_SUCCESS = 0,
  CCHAT_ERROR_INVALID_ARGS,
  CCHAT_ERROR_NETWORK,
  CCHAT_ERROR_CRYPTO,
  CCHAT_ERROR_AUTH,
  CCHAT_ERROR_USER_NOT_FOUND,
  CCHAT_ERROR_MEMORY,
  CCHAT_ERROR_FILE_IO,
  CCHAT_ERROR_KEY_GENERATION,
  CCHAT_ERROR_ENCRYPTION,
  CCHAT_ERROR_DECRYPTION,
  CCHAT_ERROR_KEY_DERIVATION,
  CCHAT_ERROR_PERMISSION_DENIED
} cchat_error_t;

// User structure
typedef struct {
  char username[MAX_USERNAME_LEN];
  unsigned char public_key[PUBLIC_KEY_SIZE];
  unsigned char private_key[PRIVATE_KEY_SIZE];
  bool is_authenticated;
  bool keys_loaded;
} user_t;

// Chat session structure
typedef struct {
  user_t current_user;
  user_t chat_partner;
  bool is_in_chat;
} chat_session_t;

// Function declarations

// Command line interface
void print_usage(const char *program_name);
void print_version(void);

// User management
cchat_error_t register_user(const char *username);
cchat_error_t login_user(const char *username);
cchat_error_t list_users(void);

// Chat functionality
cchat_error_t start_chat(const char *username);
cchat_error_t send_message(const char *message);
cchat_error_t receive_messages(void);

// Cryptography
cchat_error_t generate_keypair(unsigned char *public_key,
                               unsigned char *private_key);
cchat_error_t encrypt_message(const char *message,
                              const unsigned char *recipient_public_key,
                              unsigned char *encrypted, size_t *encrypted_len);
cchat_error_t decrypt_message(const unsigned char *encrypted,
                              size_t encrypted_len,
                              const unsigned char *private_key, char *message,
                              size_t *message_len);

// Key management
cchat_error_t save_keys_to_file(const char *username,
                                const unsigned char *public_key,
                                const unsigned char *private_key,
                                const char *password);
cchat_error_t load_keys_from_file(const char *username,
                                  unsigned char *public_key,
                                  unsigned char *private_key,
                                  const char *password);
cchat_error_t create_keys_directory(void);

// Password utilities
cchat_error_t derive_key_from_password(const char *password,
                                       const unsigned char *salt,
                                       unsigned char *derived_key);
void secure_zero_memory(void *ptr, size_t size);

// Network
cchat_error_t connect_to_server(void);
cchat_error_t disconnect_from_server(void);

// Main chat loop
void run_chat_interface(void);

// Utility functions
void clear_screen(void);
void safe_strncpy(char *dest, const char *src, size_t size);
cchat_error_t validate_username(const char *username);
void get_password_input(const char *prompt, char *password, size_t max_len);
char *get_home_directory(void);
cchat_error_t secure_file_permissions(const char *filepath);

#endif // C_CHAT_H