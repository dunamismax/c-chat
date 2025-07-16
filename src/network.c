#include "c-chat.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

static int server_socket = -1;
static bool connected_to_server = false;

static cchat_error_t reconnect_to_server(void) {
  if (connected_to_server) {
    return CCHAT_SUCCESS;
  }

  printf("Attempting to reconnect to server...\n");

  if (server_socket >= 0) {
    close(server_socket);
    server_socket = -1;
  }

  return connect_to_server();
}

static int send_network_message(uint8_t msg_type, const uint8_t *payload,
                                uint32_t payload_len) {
  if (server_socket < 0 || !connected_to_server) {
    return -1;
  }

  uint8_t header[5];
  header[0] = (payload_len >> 24) & 0xFF;
  header[1] = (payload_len >> 16) & 0xFF;
  header[2] = (payload_len >> 8) & 0xFF;
  header[3] = payload_len & 0xFF;
  header[4] = msg_type;

  ssize_t sent = send(server_socket, header, sizeof(header), MSG_NOSIGNAL);
  if (sent != sizeof(header)) {
    if (sent < 0 && (errno == EPIPE || errno == ECONNRESET)) {
      connected_to_server = false;
    }
    return -1;
  }

  if (payload_len > 0 && payload) {
    sent = send(server_socket, payload, payload_len, MSG_NOSIGNAL);
    if (sent != (ssize_t)payload_len) {
      if (sent < 0 && (errno == EPIPE || errno == ECONNRESET)) {
        connected_to_server = false;
      }
      return -1;
    }
  }

  return 0;
}

static int receive_network_message(uint8_t *msg_type, uint8_t **payload,
                                   uint32_t *payload_len) {
  if (server_socket < 0) {
    return -1;
  }

  uint8_t header[5];
  if (recv(server_socket, header, sizeof(header), MSG_WAITALL) !=
      sizeof(header)) {
    return -1;
  }

  *payload_len = ((uint32_t)header[0] << 24) | ((uint32_t)header[1] << 16) |
                 ((uint32_t)header[2] << 8) | ((uint32_t)header[3]);
  *msg_type = header[4];

  if (*payload_len > 0) {
    *payload = malloc(*payload_len);
    if (!*payload) {
      return -1;
    }

    if (recv(server_socket, *payload, *payload_len, MSG_WAITALL) !=
        (ssize_t)*payload_len) {
      free(*payload);
      *payload = NULL;
      return -1;
    }
  } else {
    *payload = NULL;
  }

  return 0;
}

cchat_error_t connect_to_server(void) {
  printf("Connecting to server at %s:%d...\n", SERVER_HOST, SERVER_PORT);

  if (connected_to_server) {
    return CCHAT_SUCCESS;
  }

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("Failed to create socket");
    return CCHAT_ERROR_NETWORK;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(server_socket);
    server_socket = -1;
    return CCHAT_ERROR_NETWORK;
  }

  if (connect(server_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) < 0) {
    perror("Failed to connect to server");
    close(server_socket);
    server_socket = -1;
    return CCHAT_ERROR_NETWORK;
  }

  connected_to_server = true;
  printf("Connected to C-Chat server successfully\n");
  return CCHAT_SUCCESS;
}

cchat_error_t disconnect_from_server(void) {
  if (server_socket >= 0) {
    if (connected_to_server) {
      send_network_message(0x08, NULL, 0);
    }
    close(server_socket);
    server_socket = -1;
    connected_to_server = false;
    printf("Disconnected from server\n");
  }

  return CCHAT_SUCCESS;
}

cchat_error_t register_user_with_server(const char *username,
                                        const unsigned char *public_key) {
  if (!username || !public_key || !connected_to_server) {
    return CCHAT_ERROR_NETWORK;
  }

  size_t username_len = strlen(username);
  if (username_len >= MAX_USERNAME_LEN) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  uint8_t *payload = malloc(1 + username_len + PUBLIC_KEY_SIZE);
  if (!payload) {
    return CCHAT_ERROR_MEMORY;
  }

  payload[0] = (uint8_t)username_len;
  memcpy(&payload[1], username, username_len);
  memcpy(&payload[1 + username_len], public_key, PUBLIC_KEY_SIZE);

  if (send_network_message(0x01, payload, 1 + username_len + PUBLIC_KEY_SIZE) <
      0) {
    free(payload);
    return CCHAT_ERROR_NETWORK;
  }

  free(payload);

  uint8_t response_type;
  uint8_t *response_payload;
  uint32_t response_len;

  if (receive_network_message(&response_type, &response_payload,
                              &response_len) < 0) {
    return CCHAT_ERROR_NETWORK;
  }

  if (response_type != 0x81 || response_len < 2) {
    if (response_payload)
      free(response_payload);
    return CCHAT_ERROR_NETWORK;
  }

  bool success = response_payload[0] == 1;
  uint8_t error_code = response_payload[1];

  free(response_payload);

  if (!success) {
    switch (error_code) {
    case 0x02:
      return CCHAT_ERROR_AUTH;
    case 0x01:
      return CCHAT_ERROR_INVALID_ARGS;
    default:
      return CCHAT_ERROR_NETWORK;
    }
  }

  return CCHAT_SUCCESS;
}

cchat_error_t get_user_public_key(const char *username,
                                  unsigned char *public_key) {
  if (!username || !public_key || !connected_to_server) {
    return CCHAT_ERROR_NETWORK;
  }

  size_t username_len = strlen(username);
  if (username_len >= MAX_USERNAME_LEN) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  uint8_t *payload = malloc(1 + username_len);
  if (!payload) {
    return CCHAT_ERROR_MEMORY;
  }

  payload[0] = (uint8_t)username_len;
  memcpy(&payload[1], username, username_len);

  if (send_network_message(0x03, payload, 1 + username_len) < 0) {
    free(payload);
    return CCHAT_ERROR_NETWORK;
  }

  free(payload);

  uint8_t response_type;
  uint8_t *response_payload;
  uint32_t response_len;

  if (receive_network_message(&response_type, &response_payload,
                              &response_len) < 0) {
    return CCHAT_ERROR_NETWORK;
  }

  if (response_type != 0x83) {
    if (response_payload)
      free(response_payload);
    return CCHAT_ERROR_NETWORK;
  }

  if (response_len < 1 || response_payload[0] == 0) {
    free(response_payload);
    return CCHAT_ERROR_USER_NOT_FOUND;
  }

  if (response_len < 1 + PUBLIC_KEY_SIZE) {
    free(response_payload);
    return CCHAT_ERROR_NETWORK;
  }

  memcpy(public_key, &response_payload[1], PUBLIC_KEY_SIZE);
  free(response_payload);

  return CCHAT_SUCCESS;
}

cchat_error_t send_message_to_server(const char *recipient,
                                     const unsigned char *encrypted_message,
                                     size_t message_len) {
  if (!recipient || !encrypted_message || message_len == 0 ||
      !connected_to_server) {
    return CCHAT_ERROR_NETWORK;
  }

  size_t recipient_len = strlen(recipient);
  if (recipient_len >= MAX_USERNAME_LEN ||
      message_len > MAX_MESSAGE_LEN + 100) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  uint8_t *payload = malloc(1 + recipient_len + 2 + message_len);
  if (!payload) {
    return CCHAT_ERROR_MEMORY;
  }

  payload[0] = (uint8_t)recipient_len;
  memcpy(&payload[1], recipient, recipient_len);
  payload[1 + recipient_len] = (message_len >> 8) & 0xFF;
  payload[1 + recipient_len + 1] = message_len & 0xFF;
  memcpy(&payload[1 + recipient_len + 2], encrypted_message, message_len);

  if (send_network_message(0x04, payload, 1 + recipient_len + 2 + message_len) <
      0) {
    free(payload);
    return CCHAT_ERROR_NETWORK;
  }

  free(payload);

  uint8_t response_type;
  uint8_t *response_payload;
  uint32_t response_len;

  if (receive_network_message(&response_type, &response_payload,
                              &response_len) < 0) {
    return CCHAT_ERROR_NETWORK;
  }

  if (response_type != 0x84 || response_len < 5) {
    if (response_payload)
      free(response_payload);
    return CCHAT_ERROR_NETWORK;
  }

  uint32_t message_id = ((uint32_t)response_payload[0] << 24) |
                        ((uint32_t)response_payload[1] << 16) |
                        ((uint32_t)response_payload[2] << 8) |
                        ((uint32_t)response_payload[3]);
  uint8_t status = response_payload[4];

  free(response_payload);

  if (status == 0) {
    return CCHAT_ERROR_NETWORK;
  }

  return CCHAT_SUCCESS;
}