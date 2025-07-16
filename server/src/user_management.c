#include "../include/c-chat-server.h"

user_record_t *find_user(const char *username) {
  if (!username) {
    return NULL;
  }

  pthread_mutex_lock(&server.users_mutex);

  for (int i = 0; i < server.user_count; i++) {
    if (server.users[i].is_registered &&
        strcmp(server.users[i].username, username) == 0) {
      pthread_mutex_unlock(&server.users_mutex);
      return &server.users[i];
    }
  }

  pthread_mutex_unlock(&server.users_mutex);
  return NULL;
}

client_connection_t *find_client_by_username(const char *username) {
  if (!username) {
    return NULL;
  }

  pthread_mutex_lock(&server.clients_mutex);

  for (int i = 0; i < server.client_count; i++) {
    if (server.clients[i].connected && server.clients[i].authenticated &&
        strcmp(server.clients[i].username, username) == 0) {
      pthread_mutex_unlock(&server.clients_mutex);
      return &server.clients[i];
    }
  }

  pthread_mutex_unlock(&server.clients_mutex);
  return NULL;
}

int add_user(const char *username, const unsigned char *public_key) {
  if (!username || !public_key) {
    return -1;
  }

  if (validate_username_server(username) < 0) {
    return -1;
  }

  pthread_mutex_lock(&server.users_mutex);

  if (server.user_count >= MAX_CLIENTS) {
    pthread_mutex_unlock(&server.users_mutex);
    return -1;
  }

  for (int i = 0; i < server.user_count; i++) {
    if (server.users[i].is_registered &&
        strcmp(server.users[i].username, username) == 0) {
      pthread_mutex_unlock(&server.users_mutex);
      return -2;
    }
  }

  int user_index = server.user_count;
  strncpy(server.users[user_index].username, username, MAX_USERNAME_LEN - 1);
  server.users[user_index].username[MAX_USERNAME_LEN - 1] = '\0';

  memcpy(server.users[user_index].public_key, public_key, PUBLIC_KEY_SIZE);

  server.users[user_index].status = STATUS_OFFLINE;
  server.users[user_index].last_seen = time(NULL);
  server.users[user_index].is_registered = true;

  server.user_count++;

  pthread_mutex_unlock(&server.users_mutex);

  log_info("User %s registered successfully", username);
  return 0;
}

int authenticate_user(client_connection_t *client, const char *username,
                      const unsigned char *signature) {
  if (!client || !username || !signature) {
    return -1;
  }

  user_record_t *user = find_user(username);
  if (!user) {
    return -1;
  }

  if (crypto_sign_verify_detached(signature, client->challenge, CHALLENGE_SIZE,
                                  user->public_key) != 0) {
    log_error("Authentication failed for user %s: invalid signature", username);
    return -1;
  }

  pthread_mutex_lock(&client->mutex);
  client->authenticated = true;
  strncpy(client->username, username, MAX_USERNAME_LEN - 1);
  client->username[MAX_USERNAME_LEN - 1] = '\0';
  pthread_mutex_unlock(&client->mutex);

  pthread_mutex_lock(&user->mutex);
  user->status = STATUS_ONLINE;
  user->last_seen = time(NULL);
  pthread_mutex_unlock(&user->mutex);

  broadcast_status_update(username, STATUS_ONLINE);

  log_info("User %s authenticated successfully", username);
  return 0;
}

void broadcast_status_update(const char *username, user_status_t status) {
  if (!username) {
    return;
  }

  size_t username_len = strlen(username);
  uint8_t *payload = malloc(1 + username_len + 1);
  if (!payload) {
    return;
  }

  payload[0] = (uint8_t)username_len;
  memcpy(&payload[1], username, username_len);
  payload[1 + username_len] = (uint8_t)status;

  pthread_mutex_lock(&server.clients_mutex);

  for (int i = 0; i < server.client_count; i++) {
    if (server.clients[i].connected && server.clients[i].authenticated &&
        strcmp(server.clients[i].username, username) != 0) {

      send_network_message(server.clients[i].socket_fd, MSG_STATUS_UPDATE,
                           payload, 1 + username_len + 1);
    }
  }

  pthread_mutex_unlock(&server.clients_mutex);

  sodium_memzero(payload, 1 + username_len + 1);
  free(payload);

  log_debug("Broadcasted status update for %s: %d", username, status);
}