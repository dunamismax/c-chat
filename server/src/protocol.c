#include "../include/c-chat-server.h"

int send_network_message(int socket_fd, message_type_t type,
                         const uint8_t *payload, uint32_t payload_len) {
  uint8_t header[5];

  header[0] = (payload_len >> 24) & 0xFF;
  header[1] = (payload_len >> 16) & 0xFF;
  header[2] = (payload_len >> 8) & 0xFF;
  header[3] = payload_len & 0xFF;
  header[4] = (uint8_t)type;

  if (send(socket_fd, header, sizeof(header), MSG_NOSIGNAL) != sizeof(header)) {
    log_error("Failed to send message header: %s", strerror(errno));
    return -1;
  }

  if (payload_len > 0 && payload) {
    if (send(socket_fd, payload, payload_len, MSG_NOSIGNAL) !=
        (ssize_t)payload_len) {
      log_error("Failed to send message payload: %s", strerror(errno));
      return -1;
    }
  }

  log_debug("Sent message type 0x%02X with %u bytes payload", type,
            payload_len);
  return 0;
}

int receive_network_message(int socket_fd, network_message_t *msg) {
  if (!msg) {
    return -1;
  }

  uint8_t header[5];
  ssize_t received = recv(socket_fd, header, sizeof(header), MSG_WAITALL);

  if (received == 0) {
    log_debug("Client disconnected");
    return -2;
  }

  if (received != sizeof(header)) {
    log_error(
        "Failed to receive message header: received %zd bytes, expected %zu",
        received, sizeof(header));
    return -1;
  }

  msg->length = ((uint32_t)header[0] << 24) | ((uint32_t)header[1] << 16) |
                ((uint32_t)header[2] << 8) | ((uint32_t)header[3]);
  msg->type = header[4];

  if (msg->length > MAX_MESSAGE_LEN * 2) {
    log_error("Message too large: %u bytes", msg->length);
    return -1;
  }

  if (msg->length > 0) {
    msg->payload = malloc(msg->length);
    if (!msg->payload) {
      log_error("Failed to allocate memory for message payload");
      return -1;
    }

    received = recv(socket_fd, msg->payload, msg->length, MSG_WAITALL);
    if (received != (ssize_t)msg->length) {
      log_error(
          "Failed to receive message payload: received %zd bytes, expected %u",
          received, msg->length);
      free(msg->payload);
      msg->payload = NULL;
      return -1;
    }
  } else {
    msg->payload = NULL;
  }

  log_debug("Received message type 0x%02X with %u bytes payload", msg->type,
            msg->length);
  return 0;
}

void free_network_message(network_message_t *msg) {
  if (msg && msg->payload) {
    if (msg->length > 0) {
      sodium_memzero(msg->payload, msg->length);
    }
    free(msg->payload);
    msg->payload = NULL;
  }
}

int send_error(int socket_fd, error_code_t error_code,
               const char *error_message) {
  size_t msg_len = error_message ? strlen(error_message) : 0;
  uint8_t *payload = malloc(3 + msg_len);
  if (!payload) {
    return -1;
  }

  payload[0] = error_code;
  payload[1] = (msg_len >> 8) & 0xFF;
  payload[2] = msg_len & 0xFF;

  if (msg_len > 0) {
    memcpy(&payload[3], error_message, msg_len);
  }

  int result = send_network_message(socket_fd, MSG_ERROR, payload, 3 + msg_len);

  sodium_memzero(payload, 3 + msg_len);
  free(payload);

  return result;
}

int validate_username_server(const char *username) {
  if (!username) {
    return -1;
  }

  size_t len = strlen(username);
  if (len == 0 || len >= MAX_USERNAME_LEN) {
    return -1;
  }

  for (size_t i = 0; i < len; i++) {
    char c = username[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_')) {
      return -1;
    }
  }

  return 0;
}

bool check_rate_limit(client_connection_t *client) {
  time_t now = time(NULL);

  if (now - client->rate_limit.window_start >= RATE_LIMIT_WINDOW) {
    client->rate_limit.window_start = now;
    client->rate_limit.request_count = 0;
  }

  return client->rate_limit.request_count < RATE_LIMIT_MAX_REQUESTS;
}

void update_rate_limit(client_connection_t *client) {
  client->rate_limit.request_count++;
}