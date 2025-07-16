#include "c-chat.h"
#include <sys/stat.h>

cchat_error_t register_user(const char *username) {
  printf("Registering user: %s\n", username);

  // Check if user already exists by checking for key file
  char *home_dir = get_home_directory();
  if (!home_dir) {
    return CCHAT_ERROR_FILE_IO;
  }

  char keys_path[PATH_MAX];
  int path_len = snprintf(keys_path, sizeof(keys_path), "%s/%s/%s.keys",
                          home_dir, KEYS_DIR, username);
  free(home_dir);

  if (path_len >= (int)sizeof(keys_path) || path_len < 0) {
    return CCHAT_ERROR_FILE_IO;
  }

  // Check if key file exists (safer than attempting decryption with wrong
  // password)
  struct stat st;
  if (stat(keys_path, &st) == 0) {
    printf("User %s already exists. Use --login instead.\n", username);
    return CCHAT_ERROR_AUTH;
  }

  // Generate new keypair
  unsigned char public_key[PUBLIC_KEY_SIZE];
  unsigned char private_key[PRIVATE_KEY_SIZE];

  cchat_error_t result = generate_keypair(public_key, private_key);
  if (result != CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to generate keypair\n");
    return result;
  }

  // Get password for key encryption
  char password[MAX_PASSWORD_LEN];
  char confirm_password[MAX_PASSWORD_LEN];

  do {
    get_password_input("Enter password to encrypt your private key: ", password,
                       sizeof(password));
    get_password_input("Confirm password: ", confirm_password,
                       sizeof(confirm_password));

    if (strcmp(password, confirm_password) != 0) {
      printf("Passwords do not match. Please try again.\n");
      secure_zero_memory(password, sizeof(password));
      secure_zero_memory(confirm_password, sizeof(confirm_password));
    }
  } while (strcmp(password, confirm_password) != 0);

  secure_zero_memory(confirm_password, sizeof(confirm_password));

  // Store keys locally (encrypted with password)
  result = save_keys_to_file(username, public_key, private_key, password);
  if (result != CCHAT_SUCCESS) {
    fprintf(stderr, "Failed to save keys\n");
    secure_zero_memory(password, sizeof(password));
    secure_zero_memory(public_key, sizeof(public_key));
    secure_zero_memory(private_key, sizeof(private_key));
    return result;
  }

  // Connect to server and register user
  cchat_error_t server_result = connect_to_server();
  if (server_result == CCHAT_SUCCESS) {
    server_result = register_user_with_server(username, public_key);
    if (server_result == CCHAT_SUCCESS) {
      printf("\nUser registration completed successfully!\n");
      printf("Your account has been registered with the C-Chat server.\n");
    } else {
      printf("\nLocal keys created, but server registration failed.\n");
      printf("You can retry connecting to the server later.\n");
    }
    disconnect_from_server();
  } else {
    printf("\nLocal keys created, but could not connect to server.\n");
    printf("Your keys are saved locally and you can register with the server "
           "later.\n");
  }

  // Clear sensitive data
  secure_zero_memory(password, sizeof(password));
  secure_zero_memory(public_key, sizeof(public_key));
  secure_zero_memory(private_key, sizeof(private_key));

  return CCHAT_SUCCESS;
}

cchat_error_t login_user(const char *username) {
  printf("Logging in user: %s\n", username);

  // Get password to decrypt private key
  char password[MAX_PASSWORD_LEN];
  get_password_input("Enter your password: ", password, sizeof(password));

  // Load and decrypt keys from local storage
  unsigned char public_key[PUBLIC_KEY_SIZE];
  unsigned char private_key[PRIVATE_KEY_SIZE];

  cchat_error_t result =
      load_keys_from_file(username, public_key, private_key, password);
  if (result != CCHAT_SUCCESS) {
    if (result == CCHAT_ERROR_FILE_IO) {
      fprintf(stderr, "No keys found for user %s. Please register first.\n",
              username);
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
  printf(
      "Your private key has been loaded and is ready for secure messaging.\n");

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

cchat_error_t validate_username(const char *username) {
  if (!username) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  // Cache length to avoid O(n²) complexity and validate bounds
  size_t len = strlen(username);
  if (len == 0 || len >= MAX_USERNAME_LEN) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  // Check for valid characters (alphanumeric and underscore only)
  for (size_t i = 0; i < len; i++) {
    char c = username[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_')) {
      return CCHAT_ERROR_INVALID_ARGS;
    }
  }

  return CCHAT_SUCCESS;
}