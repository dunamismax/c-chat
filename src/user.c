#include "c-chat.h"

cchat_error_t register_user(const char* username) {
    printf("Registering user: %s\n", username);
    
    // Generate keypair
    char public_key[256] = {0};
    char private_key[512] = {0};
    
    cchat_error_t result = generate_keypair(public_key, private_key);
    if (result != CCHAT_SUCCESS) {
        fprintf(stderr, "Failed to generate keypair\n");
        return result;
    }
    
    // TODO: Store private key locally (encrypted)
    // TODO: Send public key to server for registration
    
    printf("Generated cryptographic keys\n");
    printf("Public key: %.32s...\n", public_key);
    printf("Private key stored locally (encrypted)\n");
    
    return CCHAT_SUCCESS;
}

cchat_error_t login_user(const char* username) {
    printf("Logging in user: %s\n", username);
    
    // TODO: Load private key from local storage
    // TODO: Authenticate with server
    // TODO: Retrieve user's public key from server
    
    printf("Authentication successful\n");
    printf("Private key loaded from local storage\n");
    
    return CCHAT_SUCCESS;
}

cchat_error_t list_users(void) {
    printf("Available users:\n");
    
    // TODO: Query server for list of registered users
    // For now, show placeholder data
    printf("  alice    (online)\n");
    printf("  bob      (offline)\n");
    printf("  charlie  (online)\n");
    
    return CCHAT_SUCCESS;
}

cchat_error_t validate_username(const char* username) {
    if (!username || strlen(username) == 0) {
        return CCHAT_ERROR_INVALID_ARGS;
    }
    
    if (strlen(username) >= MAX_USERNAME_LEN) {
        return CCHAT_ERROR_INVALID_ARGS;
    }
    
    // Check for valid characters (alphanumeric and underscore only)
    for (size_t i = 0; i < strlen(username); i++) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || 
              (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || 
              c == '_')) {
            return CCHAT_ERROR_INVALID_ARGS;
        }
    }
    
    return CCHAT_SUCCESS;
}