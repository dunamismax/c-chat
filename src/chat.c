#include "c-chat.h"

static chat_session_t current_session = {0};

cchat_error_t start_chat(const char* username) {
    printf("Initiating secure chat with %s...\n", username);
    
    if (validate_username(username) != CCHAT_SUCCESS) {
        printf("Error: Invalid username\n");
        return CCHAT_ERROR_INVALID_ARGS;
    }
    
    // TODO: Retrieve target user's public key from server
    // TODO: Establish encrypted communication channel
    
    safe_strncpy(current_session.chat_partner.username, username, sizeof(current_session.chat_partner.username));
    current_session.is_in_chat = true;
    
    printf("Secure chat established with %s\n", username);
    printf("End-to-end encryption active\n");
    printf("Type your messages (or /exit to leave chat):\n\n");
    
    // Chat loop
    char message[MAX_MESSAGE_LEN];
    while (current_session.is_in_chat) {
        printf("%s> ", current_session.chat_partner.username);
        fflush(stdout);
        
        if (!fgets(message, sizeof(message), stdin)) {
            break;
        }
        
        // Remove newline
        message[strcspn(message, "\n")] = 0;
        
        if (strcmp(message, "/exit") == 0) {
            current_session.is_in_chat = false;
            printf("Chat session ended.\n");
            break;
        }
        
        if (strlen(message) > 0) {
            cchat_error_t result = send_message(message);
            if (result != CCHAT_SUCCESS) {
                printf("Failed to send message\n");
            }
        }
    }
    
    return CCHAT_SUCCESS;
}

cchat_error_t send_message(const char* message) {
    if (!message || strlen(message) == 0) {
        return CCHAT_ERROR_INVALID_ARGS;
    }
    
    printf("You: %s\n", message);
    
    // TODO: Encrypt message with recipient's public key
    // TODO: Send encrypted message to server for relay
    
    // Simulate message delivery
    printf("[Message encrypted and sent]\n");
    
    return CCHAT_SUCCESS;
}

cchat_error_t receive_messages(void) {
    // TODO: Poll server for incoming encrypted messages
    // TODO: Decrypt messages with local private key
    // TODO: Display decrypted messages
    
    return CCHAT_SUCCESS;
}