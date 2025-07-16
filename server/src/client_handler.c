#include "../include/c-chat-server.h"

void *client_handler(void *arg) {
  client_connection_t *client = (client_connection_t *)arg;
  network_message_t msg;

  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client->address.sin_addr, client_ip, INET_ADDRSTRLEN);

  log_info("Client handler started for %s:%d", client_ip,
           ntohs(client->address.sin_port));

  while (client->connected && server.running) {
    memset(&msg, 0, sizeof(msg));

    int result = receive_network_message(client->socket_fd, &msg);
    if (result == -2) {
      log_info("Client %s disconnected", client_ip);
      break;
    } else if (result < 0) {
      log_error("Failed to receive message from client %s", client_ip);
      break;
    }

    if (!check_rate_limit(client)) {
      log_error("Rate limit exceeded for client %s", client_ip);
      send_error(client->socket_fd, ERR_RATE_LIMIT, "Rate limit exceeded");
      free_network_message(&msg);
      break;
    }

    update_rate_limit(client);

    switch (msg.type) {
    case MSG_REGISTER_USER:
      if (handle_register_user(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle register user from %s", client_ip);
      }
      break;

    case MSG_LOGIN_USER:
      if (handle_login_user(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle login user from %s", client_ip);
      }
      break;

    case MSG_GET_PUBLIC_KEY:
      if (!client->authenticated) {
        send_error(client->socket_fd, ERR_AUTH_FAILED, "Not authenticated");
        break;
      }
      if (handle_get_public_key(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle get public key from %s", client_ip);
      }
      break;

    case MSG_SEND_MESSAGE:
      if (!client->authenticated) {
        send_error(client->socket_fd, ERR_AUTH_FAILED, "Not authenticated");
        break;
      }
      if (handle_send_message(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle send message from %s", client_ip);
      }
      break;

    case MSG_GET_MESSAGES:
      if (!client->authenticated) {
        send_error(client->socket_fd, ERR_AUTH_FAILED, "Not authenticated");
        break;
      }
      if (handle_get_messages(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle get messages from %s", client_ip);
      }
      break;

    case MSG_SET_STATUS:
      if (!client->authenticated) {
        send_error(client->socket_fd, ERR_AUTH_FAILED, "Not authenticated");
        break;
      }
      if (handle_set_status(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle set status from %s", client_ip);
      }
      break;

    case MSG_LIST_USERS:
      if (!client->authenticated) {
        send_error(client->socket_fd, ERR_AUTH_FAILED, "Not authenticated");
        break;
      }
      if (handle_list_users(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle list users from %s", client_ip);
      }
      break;

    case MSG_LOGOUT:
      if (handle_logout(client, msg.payload, msg.length) < 0) {
        log_error("Failed to handle logout from %s", client_ip);
      }
      break;

    default:
      log_error("Unknown message type 0x%02X from client %s", msg.type,
                client_ip);
      send_error(client->socket_fd, ERR_INVALID_FORMAT, "Unknown message type");
      break;
    }

    free_network_message(&msg);
  }

  pthread_mutex_lock(&client->mutex);

  if (client->authenticated && strlen(client->username) > 0) {
    user_record_t *user = find_user(client->username);
    if (user) {
      pthread_mutex_lock(&user->mutex);
      user->status = STATUS_OFFLINE;
      user->last_seen = time(NULL);
      pthread_mutex_unlock(&user->mutex);

      broadcast_status_update(client->username, STATUS_OFFLINE);
      log_info("User %s logged out", client->username);
    }
  }

  if (client->socket_fd >= 0) {
    close(client->socket_fd);
    client->socket_fd = -1;
  }

  client->connected = false;
  client->authenticated = false;
  memset(client->username, 0, sizeof(client->username));

  pthread_mutex_lock(&client->queue_mutex);
  for (int i = 0; i < client->queue_count; i++) {
    if (client->message_queue[i].encrypted_data) {
      sodium_memzero(client->message_queue[i].encrypted_data,
                     client->message_queue[i].encrypted_len);
      free(client->message_queue[i].encrypted_data);
    }
  }
  client->queue_count = 0;
  client->queue_head = 0;
  client->queue_tail = 0;
  pthread_mutex_unlock(&client->queue_mutex);

  pthread_mutex_unlock(&client->mutex);

  log_info("Client handler terminated for %s:%d", client_ip,
           ntohs(client->address.sin_port));
  return NULL;
}