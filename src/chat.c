#include "c-chat.h"

chat_session_t current_session = {0};

cchat_error_t start_chat(const char *username) {
  printf("Initiating secure chat with %s...\n", username);

  // Use stack-allocated keys for better security
  secure_session_keys_t session_keys = {0};

  if (validate_username(username) != CCHAT_SUCCESS) {
    printf("Error: Invalid username\n");
    return CCHAT_ERROR_INVALID_ARGS;
  }

  // Load current user's private key for decryption
  if (!session_keys.keys_loaded) {
    char password[MAX_PASSWORD_LEN];
    get_password_input("Enter your password to start secure chat: ", password,
                       sizeof(password));

    unsigned char public_key_temp[PUBLIC_KEY_SIZE];
    cchat_error_t result = load_keys_from_file(
        current_session.current_user.username, public_key_temp,
        session_keys.private_key, password);
    if (result != CCHAT_SUCCESS) {
      fprintf(stderr, "Failed to load your keys for secure messaging\n");
      secure_zero_memory(password, sizeof(password));
      secure_zero_memory(&session_keys, sizeof(session_keys));
      return result;
    }

    secure_zero_memory(password, sizeof(password));
    secure_zero_memory(public_key_temp, sizeof(public_key_temp));
    session_keys.keys_loaded = true;
  }

  // Connect to server and retrieve target user's public key
  cchat_error_t server_result = connect_to_server();
  if (server_result != CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to connect to server\n");
    secure_zero_memory(&session_keys, sizeof(session_keys));
    return CCHAT_ERROR_NETWORK;
  }

  printf("Retrieving %s's public key from server...\n", username);
  server_result =
      get_user_public_key(username, session_keys.partner_public_key);
  if (server_result != CCHAT_SUCCESS) {
    if (server_result == CCHAT_ERROR_USER_NOT_FOUND) {
      fprintf(stderr, "User %s not found on server\n", username);
    } else {
      fprintf(stderr, "Failed to retrieve public key for %s\n", username);
    }
    disconnect_from_server();
    secure_zero_memory(&session_keys, sizeof(session_keys));
    return server_result;
  }

  safe_strncpy(current_session.chat_partner.username, username,
               sizeof(current_session.chat_partner.username));
  memcpy(current_session.chat_partner.public_key,
         session_keys.partner_public_key, PUBLIC_KEY_SIZE);
  current_session.is_in_chat = true;

  printf("\nSecure chat established with %s\n", username);
  printf("End-to-end encryption active (ChaCha20-Poly1305)\n");
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
      // Disconnect from server and securely clear all session keys before exit
      disconnect_from_server();
      secure_zero_memory(&session_keys, sizeof(session_keys));
      break;
    }

    if (strlen(message) > 0) {
      cchat_error_t result = send_message_secure(message, &session_keys);
      if (result != CCHAT_SUCCESS) {
        printf("Failed to send message\n");
      }
    }
  }

  // Ensure keys are cleared and server disconnected even on normal exit
  disconnect_from_server();
  secure_zero_memory(&session_keys, sizeof(session_keys));
  return CCHAT_SUCCESS;
}

cchat_error_t send_message_secure(const char *message,
                                  secure_session_keys_t *keys) {
  if (!message || strlen(message) == 0 || !keys) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  if (!keys->keys_loaded) {
    fprintf(stderr, "Session keys not loaded\n");
    return CCHAT_ERROR_CRYPTO;
  }

  // Encrypt message with recipient's public key
  unsigned char encrypted[ENCRYPTED_MSG_SIZE];
  size_t encrypted_len;

  cchat_error_t result = encrypt_message(message, keys->partner_public_key,
                                         encrypted, &encrypted_len);
  if (result != CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to encrypt message\n");
    return result;
  }

  printf("You: %s\n", message);
  printf("[Message encrypted (%zu bytes) and ready for transmission]\n",
         encrypted_len);

  // Send encrypted message to server for relay to recipient
  cchat_error_t send_result = send_message_to_server(
      current_session.chat_partner.username, encrypted, encrypted_len);
  if (send_result == CCHAT_SUCCESS) {
    printf("[Delivered to %s]\n", current_session.chat_partner.username);
  } else {
    printf("[Failed to deliver message to %s]\n",
           current_session.chat_partner.username);
  }

  // Clear sensitive data
  secure_zero_memory(encrypted, sizeof(encrypted));

  return CCHAT_SUCCESS;
}

cchat_error_t receive_messages_secure(secure_session_keys_t *keys) {
  if (!keys || !keys->keys_loaded) {
    return CCHAT_ERROR_CRYPTO;
  }

  // TODO: Poll server for incoming encrypted messages
  // For now, simulate receiving an encrypted message

  // Simulate encrypted message from partner
  const char *demo_message = "Hello from the other side!";
  unsigned char encrypted[ENCRYPTED_MSG_SIZE];
  size_t encrypted_len;

  // Encrypt with our own public key for demo (in reality, this would come from
  // server)
  unsigned char our_public[PUBLIC_KEY_SIZE];
  crypto_scalarmult_base(our_public, keys->private_key);

  if (encrypt_message(demo_message, our_public, encrypted, &encrypted_len) !=
      CCHAT_SUCCESS) {
    secure_zero_memory(our_public, sizeof(our_public));
    return CCHAT_ERROR_ENCRYPTION;
  }

  // Decrypt the received message
  char decrypted[MAX_MESSAGE_LEN];
  size_t decrypted_len;

  cchat_error_t result = decrypt_message(
      encrypted, encrypted_len, keys->private_key, decrypted, &decrypted_len);
  if (result != CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to decrypt received message\n");
    secure_zero_memory(our_public, sizeof(our_public));
    secure_zero_memory(encrypted, sizeof(encrypted));
    return result;
  }

  printf("%s: %s\n", current_session.chat_partner.username, decrypted);
  printf("[Message decrypted and verified]\n");

  // Clear sensitive data
  secure_zero_memory(our_public, sizeof(our_public));
  secure_zero_memory(encrypted, sizeof(encrypted));
  secure_zero_memory(decrypted, sizeof(decrypted));

  return CCHAT_SUCCESS;
}

// Legacy wrapper function for compatibility
cchat_error_t send_message(const char *message) {
  (void)message; // Suppress unused parameter warning
  fprintf(stderr,
          "Warning: send_message() called without secure session context\n");
  return CCHAT_ERROR_CRYPTO;
}

// Legacy wrapper function for compatibility
cchat_error_t receive_messages(void) {
  fprintf(
      stderr,
      "Warning: receive_messages() called without secure session context\n");
  return CCHAT_ERROR_CRYPTO;
}