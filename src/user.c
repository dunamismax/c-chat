#include "c-chat.h"

cchat_error_t register_user(const char* username) {
    printf("Registering user: %s\n", username);
    
    // Check if user already exists
    unsigned char existing_public[PUBLIC_KEY_SIZE];
    unsigned char existing_private[PRIVATE_KEY_SIZE];
    char temp_password[256];
    
    get_password_input("Enter a temporary password to check existing keys: ", temp_password, sizeof(temp_password));
    
    if (load_keys_from_file(username, existing_public, existing_private, temp_password) == CCHAT_SUCCESS) {
        printf("User %s already exists. Use --login instead.\n", username);
        secure_zero_memory(temp_password, sizeof(temp_password));
        secure_zero_memory(existing_public, sizeof(existing_public));
        secure_zero_memory(existing_private, sizeof(existing_private));
        return CCHAT_ERROR_AUTH;
    }
    
    // Generate new keypair
    unsigned char public_key[PUBLIC_KEY_SIZE];
    unsigned char private_key[PRIVATE_KEY_SIZE];
    
    cchat_error_t result = generate_keypair(public_key, private_key);
    if (result != CCHAT_SUCCESS) {
        fprintf(stderr, "Failed to generate keypair\n");
        secure_zero_memory(temp_password, sizeof(temp_password));
        return result;
    }
    
    // Get password for key encryption
    char password[256];
    char confirm_password[256];
    
    do {
        get_password_input("Enter password to encrypt your private key: ", password, sizeof(password));
        get_password_input("Confirm password: ", confirm_password, sizeof(confirm_password));
        
        if (strcmp(password, confirm_password) != 0) {
            printf("Passwords do not match. Please try again.\n");
            secure_zero_memory(password, sizeof(password));
            secure_zero_memory(confirm_password, sizeof(confirm_password));
        }
    } while (strcmp(password, confirm_password) != 0);
    
    secure_zero_memory(confirm_password, sizeof(confirm_password));
    secure_zero_memory(temp_password, sizeof(temp_password));
    
    // Store keys locally (encrypted with password)
    result = save_keys_to_file(username, public_key, private_key, password);
    if (result != CCHAT_SUCCESS) {
        fprintf(stderr, "Failed to save keys\n");
        secure_zero_memory(password, sizeof(password));
        secure_zero_memory(public_key, sizeof(public_key));
        secure_zero_memory(private_key, sizeof(private_key));
        return result;
    }
    
    // TODO: Send public key to server for registration
    printf("\nUser registration completed successfully!\n");
    printf("Your cryptographic keys have been generated and stored securely.\n");
    printf("Public key has been prepared for server registration.\n");
    
    // Clear sensitive data
    secure_zero_memory(password, sizeof(password));
    secure_zero_memory(public_key, sizeof(public_key));
    secure_zero_memory(private_key, sizeof(private_key));
    
    return CCHAT_SUCCESS;
}

cchat_error_t login_user(const char* username) {
    printf("Logging in user: %s\n", username);
    
    // Get password to decrypt private key
    char password[256];
    get_password_input("Enter your password: ", password, sizeof(password));
    
    // Load and decrypt keys from local storage
    unsigned char public_key[PUBLIC_KEY_SIZE];
    unsigned char private_key[PRIVATE_KEY_SIZE];
    
    cchat_error_t result = load_keys_from_file(username, public_key, private_key, password);
    if (result != CCHAT_SUCCESS) {
        if (result == CCHAT_ERROR_FILE_IO) {
            fprintf(stderr, "No keys found for user %s. Please register first.\n", username);
        } else if (result == CCHAT_ERROR_DECRYPTION) {
            fprintf(stderr, "Invalid password or corrupted key file.\n");
        } else {
            fprintf(stderr, "Failed to load keys: %d\n", result);
        }
        secure_zero_memory(password, sizeof(password));
        return result;
    }
    
    // TODO: Authenticate with server using loaded keys
    // TODO: Retrieve and verify server's response
    
    printf("\nAuthentication successful!\n");
    printf("Your private key has been loaded and is ready for secure messaging.\n");
    
    // Clear sensitive data (keys will be reloaded when needed)
    secure_zero_memory(password, sizeof(password));
    secure_zero_memory(public_key, sizeof(public_key));
    secure_zero_memory(private_key, sizeof(private_key));
    
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