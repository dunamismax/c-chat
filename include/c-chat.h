#ifndef C_CHAT_H
#define C_CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

// Constants
#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 1024
#define MAX_COMMAND_LEN 64
#define SERVER_HOST "localhost"
#define SERVER_PORT 8080

// Error codes
typedef enum {
    CCHAT_SUCCESS = 0,
    CCHAT_ERROR_INVALID_ARGS,
    CCHAT_ERROR_NETWORK,
    CCHAT_ERROR_CRYPTO,
    CCHAT_ERROR_AUTH,
    CCHAT_ERROR_USER_NOT_FOUND,
    CCHAT_ERROR_MEMORY
} cchat_error_t;

// User structure
typedef struct {
    char username[MAX_USERNAME_LEN];
    char public_key[256];  // Base64 encoded public key
    bool is_authenticated;
} user_t;

// Chat session structure
typedef struct {
    user_t current_user;
    user_t chat_partner;
    bool is_in_chat;
} chat_session_t;

// Function declarations

// Command line interface
void print_usage(const char* program_name);
void print_version(void);

// User management
cchat_error_t register_user(const char* username);
cchat_error_t login_user(const char* username);
cchat_error_t list_users(void);

// Chat functionality
cchat_error_t start_chat(const char* username);
cchat_error_t send_message(const char* message);
cchat_error_t receive_messages(void);

// Cryptography
cchat_error_t generate_keypair(char* public_key, char* private_key);
cchat_error_t encrypt_message(const char* message, const char* public_key, char* encrypted);
cchat_error_t decrypt_message(const char* encrypted, const char* private_key, char* message);

// Network
cchat_error_t connect_to_server(void);
cchat_error_t disconnect_from_server(void);

// Main chat loop
void run_chat_interface(void);

// Utility functions
void clear_screen(void);
void safe_strncpy(char* dest, const char* src, size_t size);
cchat_error_t validate_username(const char* username);

#endif // C_CHAT_H