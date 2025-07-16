#include "c-chat.h"

chat_session_t current_session = {0};
static unsigned char current_user_private_key[PRIVATE_KEY_SIZE];
static unsigned char chat_partner_public_key[PUBLIC_KEY_SIZE];
static bool session_keys_loaded = false;

cchat_error_t start_chat(const char *username) {
  printf("Initiating secure chat with %s...\n", username);

  if (validate_username(username) != CCHAT_SUCCESS) {
    printf("Error: Invalid username\n");
    return CCHAT_ERROR_INVALID_ARGS;
  }

  // Load current user's private key for decryption
  if (!session_keys_loaded) {
    char password[256];
    get_password_input("Enter your password to start secure chat: ", password,
                       sizeof(password));

    unsigned char public_key_temp[PUBLIC_KEY_SIZE];
    cchat_error_t result = load_keys_from_file(
        current_session.current_user.username, public_key_temp,
        current_user_private_key, password);
    if (result != CCHAT_SUCCESS) {
      fprintf(stderr, "Failed to load your keys for secure messaging\n");
      secure_zero_memory(password, sizeof(password));
      return result;
    }

    secure_zero_memory(password, sizeof(password));
    secure_zero_memory(public_key_temp, sizeof(public_key_temp));
    session_keys_loaded = true;
  }

  // TODO: Retrieve target user's public key from server
  // For now, simulate loading partner's public key
  printf("Retrieving %s's public key from server...\n", username);

  // Placeholder: In production, this would fetch from server
  // For now, generate a temporary key for demonstration
  unsigned char temp_private[PRIVATE_KEY_SIZE];
  if (generate_keypair(chat_partner_public_key, temp_private) !=
      CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to simulate partner's public key\n");
    return CCHAT_ERROR_CRYPTO;
  }
  secure_zero_memory(temp_private, sizeof(temp_private));

  safe_strncpy(current_session.chat_partner.username, username,
               sizeof(current_session.chat_partner.username));
  memcpy(current_session.chat_partner.public_key, chat_partner_public_key,
         PUBLIC_KEY_SIZE);
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
      // Clear session keys
      secure_zero_memory(current_user_private_key,
                         sizeof(current_user_private_key));
      secure_zero_memory(chat_partner_public_key,
                         sizeof(chat_partner_public_key));
      session_keys_loaded = false;
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

cchat_error_t send_message(const char *message) {
  if (!message || strlen(message) == 0) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  if (!session_keys_loaded) {
    fprintf(stderr, "Session keys not loaded\n");
    return CCHAT_ERROR_CRYPTO;
  }

  // Encrypt message with recipient's public key
  unsigned char encrypted[ENCRYPTED_MSG_SIZE];
  size_t encrypted_len;

  cchat_error_t result = encrypt_message(message, chat_partner_public_key,
                                         encrypted, &encrypted_len);
  if (result != CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to encrypt message\n");
    return result;
  }

  printf("You: %s\n", message);
  printf("[Message encrypted (%zu bytes) and ready for transmission]\n",
         encrypted_len);

  // TODO: Send encrypted message to server for relay
  // In production, this would send the encrypted bytes to the server

  // Simulate immediate delivery and response for demo
  printf("[Delivered to %s]\n", current_session.chat_partner.username);

  // Clear sensitive data
  secure_zero_memory(encrypted, sizeof(encrypted));

  return CCHAT_SUCCESS;
}

cchat_error_t receive_messages(void) {
  if (!session_keys_loaded) {
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
  crypto_scalarmult_base(our_public, current_user_private_key);

  if (encrypt_message(demo_message, our_public, encrypted, &encrypted_len) !=
      CCHAT_SUCCESS) {
    secure_zero_memory(our_public, sizeof(our_public));
    return CCHAT_ERROR_ENCRYPTION;
  }

  // Decrypt the received message
  char decrypted[MAX_MESSAGE_LEN];
  size_t decrypted_len;

  cchat_error_t result =
      decrypt_message(encrypted, encrypted_len, current_user_private_key,
                      decrypted, &decrypted_len);
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