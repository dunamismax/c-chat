#include "../include/c-chat-server.h"

int handle_register_user(client_connection_t *client, const uint8_t *payload,
                         uint32_t payload_len) {
  if (!payload || payload_len < 1 + PUBLIC_KEY_SIZE) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT,
               "Invalid registration data");
    return -1;
  }

  uint8_t username_len = payload[0];
  if (username_len == 0 || username_len >= MAX_USERNAME_LEN ||
      payload_len < 1 + username_len + PUBLIC_KEY_SIZE) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT,
               "Invalid username length");
    return -1;
  }

  char username[MAX_USERNAME_LEN];
  memcpy(username, &payload[1], username_len);
  username[username_len] = '\0';

  if (validate_username_server(username) < 0) {
    send_error(client->socket_fd, ERR_INVALID_USERNAME,
               "Invalid username format");
    return -1;
  }

  const unsigned char *public_key = &payload[1 + username_len];

  int result = add_user(username, public_key);

  uint8_t response[2];
  if (result == 0) {
    response[0] = 1;
    response[1] = 0;
    log_info("User %s registered successfully", username);
  } else if (result == -2) {
    response[0] = 0;
    response[1] = ERR_USER_EXISTS;
    log_info("Registration failed for %s: user already exists", username);
  } else {
    response[0] = 0;
    response[1] = ERR_SERVER_ERROR;
    log_error("Registration failed for %s: server error", username);
  }

  return send_network_message(client->socket_fd, MSG_REGISTER_RESPONSE,
                              response, sizeof(response));
}

int handle_login_user(client_connection_t *client, const uint8_t *payload,
                      uint32_t payload_len) {
  if (!payload || payload_len < 1 + SIGNATURE_SIZE) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT, "Invalid login data");
    return -1;
  }

  uint8_t username_len = payload[0];
  if (username_len == 0 || username_len >= MAX_USERNAME_LEN ||
      payload_len < 1 + username_len + SIGNATURE_SIZE) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT,
               "Invalid username length");
    return -1;
  }

  char username[MAX_USERNAME_LEN];
  memcpy(username, &payload[1], username_len);
  username[username_len] = '\0';

  const unsigned char *signature = &payload[1 + username_len];

  int result = authenticate_user(client, username, signature);

  uint8_t response[1 + CHALLENGE_SIZE];
  if (result == 0) {
    response[0] = 1;
    memcpy(&response[1], client->challenge, CHALLENGE_SIZE);

    deliver_queued_messages(client);

    log_info("User %s logged in successfully", username);
    return send_network_message(client->socket_fd, MSG_LOGIN_RESPONSE, response,
                                sizeof(response));
  } else {
    response[0] = 0;
    log_info("Login failed for %s", username);
    return send_network_message(client->socket_fd, MSG_LOGIN_RESPONSE, response,
                                1);
  }
}

int handle_get_public_key(client_connection_t *client, const uint8_t *payload,
                          uint32_t payload_len) {
  if (!payload || payload_len < 1) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT,
               "Invalid public key request");
    return -1;
  }

  uint8_t username_len = payload[0];
  if (username_len == 0 || username_len >= MAX_USERNAME_LEN ||
      payload_len < 1 + username_len) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT,
               "Invalid username length");
    return -1;
  }

  char username[MAX_USERNAME_LEN];
  memcpy(username, &payload[1], username_len);
  username[username_len] = '\0';

  user_record_t *user = find_user(username);

  uint8_t response[1 + PUBLIC_KEY_SIZE];
  if (user) {
    response[0] = 1;
    pthread_mutex_lock(&user->mutex);
    memcpy(&response[1], user->public_key, PUBLIC_KEY_SIZE);
    pthread_mutex_unlock(&user->mutex);

    log_debug("Sent public key for user %s to %s", username, client->username);
    return send_network_message(client->socket_fd, MSG_PUBLIC_KEY_RESPONSE,
                                response, sizeof(response));
  } else {
    response[0] = 0;
    log_debug("Public key not found for user %s (requested by %s)", username,
              client->username);
    return send_network_message(client->socket_fd, MSG_PUBLIC_KEY_RESPONSE,
                                response, 1);
  }
}

int handle_send_message(client_connection_t *client, const uint8_t *payload,
                        uint32_t payload_len) {
  if (!payload || payload_len < 3) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT, "Invalid message data");
    return -1;
  }

  uint8_t recipient_len = payload[0];
  if (recipient_len == 0 || recipient_len >= MAX_USERNAME_LEN ||
      payload_len < 1 + recipient_len + 2) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT,
               "Invalid recipient length");
    return -1;
  }

  char recipient[MAX_USERNAME_LEN];
  memcpy(recipient, &payload[1], recipient_len);
  recipient[recipient_len] = '\0';

  uint16_t message_len = ((uint16_t)payload[1 + recipient_len] << 8) |
                         payload[1 + recipient_len + 1];

  if (message_len == 0 || payload_len < 1 + recipient_len + 2 + message_len) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT, "Invalid message length");
    return -1;
  }

  const unsigned char *encrypted_message = &payload[1 + recipient_len + 2];

  user_record_t *recipient_user = find_user(recipient);
  if (!recipient_user) {
    send_error(client->socket_fd, ERR_USER_NOT_FOUND, "Recipient not found");
    return -1;
  }

  uint32_t message_id;
  pthread_mutex_lock(&server.message_id_mutex);
  message_id = server.next_message_id++;
  pthread_mutex_unlock(&server.message_id_mutex);

  client_connection_t *recipient_client = find_client_by_username(recipient);

  uint8_t ack_response[5];
  ack_response[0] = (message_id >> 24) & 0xFF;
  ack_response[1] = (message_id >> 16) & 0xFF;
  ack_response[2] = (message_id >> 8) & 0xFF;
  ack_response[3] = message_id & 0xFF;

  if (recipient_client) {
    size_t sender_len = strlen(client->username);
    time_t timestamp = time(NULL);

    uint8_t *message_payload = malloc(4 + 1 + sender_len + 4 + 2 + message_len);
    if (!message_payload) {
      ack_response[4] = 0;
      send_network_message(client->socket_fd, MSG_MESSAGE_ACK, ack_response,
                           sizeof(ack_response));
      return -1;
    }

    message_payload[0] = (message_id >> 24) & 0xFF;
    message_payload[1] = (message_id >> 16) & 0xFF;
    message_payload[2] = (message_id >> 8) & 0xFF;
    message_payload[3] = message_id & 0xFF;

    message_payload[4] = (uint8_t)sender_len;
    memcpy(&message_payload[5], client->username, sender_len);

    uint32_t ts = (uint32_t)timestamp;
    message_payload[5 + sender_len] = (ts >> 24) & 0xFF;
    message_payload[5 + sender_len + 1] = (ts >> 16) & 0xFF;
    message_payload[5 + sender_len + 2] = (ts >> 8) & 0xFF;
    message_payload[5 + sender_len + 3] = ts & 0xFF;

    message_payload[5 + sender_len + 4] = (message_len >> 8) & 0xFF;
    message_payload[5 + sender_len + 5] = message_len & 0xFF;

    memcpy(&message_payload[5 + sender_len + 6], encrypted_message,
           message_len);

    if (send_network_message(recipient_client->socket_fd, MSG_INCOMING_MESSAGE,
                             message_payload,
                             4 + 1 + sender_len + 4 + 2 + message_len) == 0) {
      ack_response[4] = 1;
      log_info("Message %u delivered from %s to %s", message_id,
               client->username, recipient);
    } else {
      ack_response[4] = 2;
      queue_message(recipient, client->username, encrypted_message,
                    message_len);
      log_info("Message %u queued from %s to %s (delivery failed)", message_id,
               client->username, recipient);
    }

    sodium_memzero(message_payload, 4 + 1 + sender_len + 4 + 2 + message_len);
    free(message_payload);
  } else {
    queue_message(recipient, client->username, encrypted_message, message_len);
    ack_response[4] = 2;
    log_info("Message %u queued from %s to %s (recipient offline)", message_id,
             client->username, recipient);
  }

  return send_network_message(client->socket_fd, MSG_MESSAGE_ACK, ack_response,
                              sizeof(ack_response));
}

int handle_get_messages(client_connection_t *client, const uint8_t *payload,
                        uint32_t payload_len) {
  (void)payload;
  (void)payload_len;

  return deliver_queued_messages(client);
}

int handle_set_status(client_connection_t *client, const uint8_t *payload,
                      uint32_t payload_len) {
  if (!payload || payload_len < 1) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT, "Invalid status data");
    return -1;
  }

  user_status_t new_status = (user_status_t)payload[0];
  if (new_status > STATUS_AWAY) {
    send_error(client->socket_fd, ERR_INVALID_FORMAT, "Invalid status value");
    return -1;
  }

  user_record_t *user = find_user(client->username);
  if (user) {
    pthread_mutex_lock(&user->mutex);
    user->status = new_status;
    user->last_seen = time(NULL);
    pthread_mutex_unlock(&user->mutex);

    pthread_mutex_lock(&client->mutex);
    client->status = new_status;
    pthread_mutex_unlock(&client->mutex);

    broadcast_status_update(client->username, new_status);

    log_info("User %s changed status to %d", client->username, new_status);
  }

  return 0;
}

int handle_list_users(client_connection_t *client, const uint8_t *payload,
                      uint32_t payload_len) {
  (void)payload;
  (void)payload_len;

  pthread_mutex_lock(&server.users_mutex);

  size_t total_size = 2;
  for (int i = 0; i < server.user_count; i++) {
    if (server.users[i].is_registered) {
      total_size += 1 + strlen(server.users[i].username) + 1;
    }
  }

  uint8_t *response = malloc(total_size);
  if (!response) {
    pthread_mutex_unlock(&server.users_mutex);
    send_error(client->socket_fd, ERR_SERVER_ERROR, "Memory allocation failed");
    return -1;
  }

  uint16_t user_count = 0;
  size_t offset = 2;

  for (int i = 0; i < server.user_count; i++) {
    if (server.users[i].is_registered) {
      size_t username_len = strlen(server.users[i].username);

      response[offset] = (uint8_t)username_len;
      memcpy(&response[offset + 1], server.users[i].username, username_len);

      pthread_mutex_lock(&server.users[i].mutex);
      response[offset + 1 + username_len] = (uint8_t)server.users[i].status;
      pthread_mutex_unlock(&server.users[i].mutex);

      offset += 1 + username_len + 1;
      user_count++;
    }
  }

  response[0] = (user_count >> 8) & 0xFF;
  response[1] = user_count & 0xFF;

  pthread_mutex_unlock(&server.users_mutex);

  int result = send_network_message(client->socket_fd, MSG_USER_LIST_RESPONSE,
                                    response, offset);

  sodium_memzero(response, total_size);
  free(response);

  log_debug("Sent user list to %s (%u users)", client->username, user_count);
  return result;
}

int handle_logout(client_connection_t *client, const uint8_t *payload,
                  uint32_t payload_len) {
  (void)payload;
  (void)payload_len;

  log_info("User %s requested logout", client->username);

  pthread_mutex_lock(&client->mutex);
  client->connected = false;
  pthread_mutex_unlock(&client->mutex);

  return 0;
}