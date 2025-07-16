#include "c-chat.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

// Global flag to track libsodium initialization
static bool crypto_initialized = false;

cchat_error_t init_crypto_library(void) {
  if (crypto_initialized) {
    return CCHAT_SUCCESS;
  }

  if (sodium_init() < 0) {
    fprintf(stderr, "Failed to initialize libsodium\n");
    return CCHAT_ERROR_CRYPTO;
  }

  crypto_initialized = true;
  return CCHAT_SUCCESS;
}

void cleanup_crypto_library(void) {
  // libsodium doesn't require explicit cleanup
  crypto_initialized = false;
}

cchat_error_t generate_keypair(unsigned char *public_key,
                               unsigned char *private_key) {
  if (!public_key || !private_key) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  cchat_error_t result = init_crypto_library();
  if (result != CCHAT_SUCCESS) {
    return result;
  }

  if (crypto_box_keypair(public_key, private_key) != 0) {
    fprintf(stderr, "Failed to generate keypair\n");
    return CCHAT_ERROR_KEY_GENERATION;
  }

  printf("Generated cryptographically secure keypair\n");
  return CCHAT_SUCCESS;
}

cchat_error_t encrypt_message(const char *message,
                              const unsigned char *recipient_public_key,
                              unsigned char *encrypted, size_t *encrypted_len) {
  if (!message || !recipient_public_key || !encrypted || !encrypted_len) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  size_t message_len = strlen(message);
  if (message_len == 0 || message_len > MAX_MESSAGE_LEN) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  cchat_error_t result = init_crypto_library();
  if (result != CCHAT_SUCCESS) {
    return result;
  }

  // Use crypto_box_seal for anonymous encryption
  // This creates a sealed box that only the recipient can open with their
  // keypair Provides forward secrecy as sender's identity is not embedded
  if (crypto_box_seal(encrypted, (const unsigned char *)message, message_len,
                      recipient_public_key) != 0) {
    fprintf(stderr, "Failed to encrypt message\n");
    return CCHAT_ERROR_ENCRYPTION;
  }

  *encrypted_len = message_len + crypto_box_SEALBYTES;
  return CCHAT_SUCCESS;
}

cchat_error_t decrypt_message(const unsigned char *encrypted,
                              size_t encrypted_len,
                              const unsigned char *private_key, char *message,
                              size_t *message_len) {
  if (!encrypted || !private_key || !message || !message_len ||
      encrypted_len == 0) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  cchat_error_t result = init_crypto_library();
  if (result != CCHAT_SUCCESS) {
    return result;
  }

  // Generate public key from private key using Curve25519 scalar multiplication
  // Required because crypto_box_seal_open needs both keys to decrypt
  unsigned char public_key[PUBLIC_KEY_SIZE];
  crypto_scalarmult_base(public_key, private_key);

  // Decrypt the message using anonymous encryption (crypto_box_seal)
  // This requires both the recipient's public and private keys
  unsigned char decrypted_buffer[MAX_MESSAGE_LEN + 1];
  if (crypto_box_seal_open(decrypted_buffer, encrypted, encrypted_len,
                           public_key, private_key) != 0) {
    fprintf(stderr, "Failed to decrypt message\n");
    return CCHAT_ERROR_DECRYPTION;
  }

  // Calculate plaintext length by subtracting encryption overhead
  *message_len = encrypted_len - crypto_box_SEALBYTES;
  decrypted_buffer[*message_len] = '\0';

  memcpy(message, decrypted_buffer, *message_len + 1);

  // Clear sensitive data
  secure_zero_memory(decrypted_buffer, sizeof(decrypted_buffer));
  secure_zero_memory(public_key, sizeof(public_key));

  return CCHAT_SUCCESS;
}

cchat_error_t save_keys_to_file(const char *username,
                                const unsigned char *public_key,
                                const unsigned char *private_key,
                                const char *password) {
  if (!username || !public_key || !private_key || !password) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  if (create_keys_directory() != CCHAT_SUCCESS) {
    return CCHAT_ERROR_FILE_IO;
  }

  char *home_dir = get_home_directory();
  if (!home_dir) {
    return CCHAT_ERROR_FILE_IO;
  }

  // Construct path safely to prevent buffer overflow
  char keys_path[PATH_MAX];
  int path_len = snprintf(keys_path, sizeof(keys_path), "%s/%s/%s.keys",
                          home_dir, KEYS_DIR, username);
  if (path_len >= (int)sizeof(keys_path) || path_len < 0) {
    fprintf(stderr, "Path too long for key file\n");
    free(home_dir);
    return CCHAT_ERROR_FILE_IO;
  }

  // Generate cryptographically secure random salt for Argon2 key derivation
  // Each user gets a unique salt to prevent rainbow table attacks
  unsigned char salt[KEY_DERIVATION_SALT_SIZE];
  randombytes_buf(salt, sizeof(salt));

  // Derive encryption key from password
  unsigned char derived_key[DERIVED_KEY_SIZE];
  if (derive_key_from_password(password, salt, derived_key) != CCHAT_SUCCESS) {
    free(home_dir);
    secure_zero_memory(derived_key, sizeof(derived_key));
    return CCHAT_ERROR_KEY_DERIVATION;
  }

  // Encrypt private key
  unsigned char encrypted_private[PRIVATE_KEY_SIZE + crypto_secretbox_MACBYTES];
  unsigned char nonce[crypto_secretbox_NONCEBYTES];
  randombytes_buf(nonce, sizeof(nonce));

  if (crypto_secretbox_easy(encrypted_private, private_key, PRIVATE_KEY_SIZE,
                            nonce, derived_key) != 0) {
    free(home_dir);
    secure_zero_memory(derived_key, sizeof(derived_key));
    return CCHAT_ERROR_ENCRYPTION;
  }

  // Save key file in secure format:
  // [salt][nonce][public_key][encrypted_private_key] This layout allows safe
  // key loading without exposing private key structure
  FILE *file = fopen(keys_path, "wb");
  if (!file) {
    perror("Failed to create key file");
    free(home_dir);
    secure_zero_memory(derived_key, sizeof(derived_key));
    return CCHAT_ERROR_FILE_IO;
  }

  // Write components
  if (fwrite(salt, 1, sizeof(salt), file) != sizeof(salt) ||
      fwrite(nonce, 1, sizeof(nonce), file) != sizeof(nonce) ||
      fwrite(public_key, 1, PUBLIC_KEY_SIZE, file) != PUBLIC_KEY_SIZE ||
      fwrite(encrypted_private, 1, sizeof(encrypted_private), file) !=
          sizeof(encrypted_private)) {
    fclose(file);
    unlink(keys_path);
    free(home_dir);
    secure_zero_memory(derived_key, sizeof(derived_key));
    return CCHAT_ERROR_FILE_IO;
  }

  fclose(file);

  // Set secure file permissions (owner read/write only)
  if (secure_file_permissions(keys_path) != CCHAT_SUCCESS) {
    fprintf(stderr, "Warning: Could not set secure file permissions\n");
  }

  free(home_dir);
  secure_zero_memory(derived_key, sizeof(derived_key));
  secure_zero_memory(encrypted_private, sizeof(encrypted_private));

  printf("Keys saved securely to: %s\n", keys_path);
  return CCHAT_SUCCESS;
}

cchat_error_t load_keys_from_file(const char *username,
                                  unsigned char *public_key,
                                  unsigned char *private_key,
                                  const char *password) {
  if (!username || !public_key || !private_key || !password) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  char *home_dir = get_home_directory();
  if (!home_dir) {
    return CCHAT_ERROR_FILE_IO;
  }

  char keys_path[PATH_MAX];
  int path_len = snprintf(keys_path, sizeof(keys_path), "%s/%s/%s.keys",
                          home_dir, KEYS_DIR, username);
  if (path_len >= (int)sizeof(keys_path) || path_len < 0) {
    fprintf(stderr, "Path too long for key file\n");
    free(home_dir);
    return CCHAT_ERROR_FILE_IO;
  }

  FILE *file = fopen(keys_path, "rb");
  if (!file) {
    free(home_dir);
    return CCHAT_ERROR_FILE_IO;
  }

  // Read components
  unsigned char salt[KEY_DERIVATION_SALT_SIZE];
  unsigned char nonce[crypto_secretbox_NONCEBYTES];
  unsigned char encrypted_private[PRIVATE_KEY_SIZE + crypto_secretbox_MACBYTES];

  if (fread(salt, 1, sizeof(salt), file) != sizeof(salt) ||
      fread(nonce, 1, sizeof(nonce), file) != sizeof(nonce) ||
      fread(public_key, 1, PUBLIC_KEY_SIZE, file) != PUBLIC_KEY_SIZE ||
      fread(encrypted_private, 1, sizeof(encrypted_private), file) !=
          sizeof(encrypted_private)) {
    fclose(file);
    free(home_dir);
    return CCHAT_ERROR_FILE_IO;
  }

  fclose(file);
  free(home_dir);

  // Derive decryption key from password
  unsigned char derived_key[DERIVED_KEY_SIZE];
  if (derive_key_from_password(password, salt, derived_key) != CCHAT_SUCCESS) {
    secure_zero_memory(derived_key, sizeof(derived_key));
    return CCHAT_ERROR_KEY_DERIVATION;
  }

  // Decrypt private key
  if (crypto_secretbox_open_easy(private_key, encrypted_private,
                                 sizeof(encrypted_private), nonce,
                                 derived_key) != 0) {
    secure_zero_memory(derived_key, sizeof(derived_key));
    return CCHAT_ERROR_DECRYPTION;
  }

  secure_zero_memory(derived_key, sizeof(derived_key));
  secure_zero_memory(encrypted_private, sizeof(encrypted_private));

  return CCHAT_SUCCESS;
}

cchat_error_t create_keys_directory(void) {
  char *home_dir = get_home_directory();
  if (!home_dir) {
    return CCHAT_ERROR_FILE_IO;
  }

  char keys_path[PATH_MAX];
  int path_len =
      snprintf(keys_path, sizeof(keys_path), "%s/%s", home_dir, KEYS_DIR);
  if (path_len >= (int)sizeof(keys_path) || path_len < 0) {
    fprintf(stderr, "Path too long for keys directory\n");
    free(home_dir);
    return CCHAT_ERROR_FILE_IO;
  }

  struct stat st = {0};
  if (stat(keys_path, &st) == -1) {
    if (mkdir(keys_path, 0700) != 0) {
      perror("Failed to create keys directory");
      free(home_dir);
      return CCHAT_ERROR_FILE_IO;
    }
    printf("Created keys directory: %s\n", keys_path);
  }

  free(home_dir);
  return CCHAT_SUCCESS;
}

cchat_error_t derive_key_from_password(const char *password,
                                       const unsigned char *salt,
                                       unsigned char *derived_key) {
  if (!password || !salt || !derived_key) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  // Use Argon2id key derivation function (memory-hard, side-channel resistant)
  // INTERACTIVE settings balance security with user experience (~100ms)
  if (crypto_pwhash(derived_key, DERIVED_KEY_SIZE, password, strlen(password),
                    salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE,
                    crypto_pwhash_ALG_DEFAULT) != 0) {
    fprintf(stderr, "Key derivation failed - out of memory\n");
    return CCHAT_ERROR_KEY_DERIVATION;
  }

  return CCHAT_SUCCESS;
}

void secure_zero_memory(void *ptr, size_t size) {
  if (ptr && size > 0) {
    sodium_memzero(ptr, size);
  }
}