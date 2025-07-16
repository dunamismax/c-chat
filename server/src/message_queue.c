#include "../include/c-chat-server.h"

int queue_message(const char *recipient, const char *sender,
                  const unsigned char *encrypted_data, size_t encrypted_len) {
  if (!recipient || !sender || !encrypted_data || encrypted_len == 0) {
    return -1;
  }

  client_connection_t *recipient_client = find_client_by_username(recipient);
  if (!recipient_client) {
    log_error("Cannot queue message: recipient %s not found", recipient);
    return -1;
  }

  pthread_mutex_lock(&recipient_client->queue_mutex);

  if (recipient_client->queue_count >= MESSAGE_QUEUE_SIZE) {
    pthread_mutex_unlock(&recipient_client->queue_mutex);
    log_error("Message queue full for user %s", recipient);
    return -1;
  }

  stored_message_t *msg =
      &recipient_client->message_queue[recipient_client->queue_tail];

  pthread_mutex_lock(&server.message_id_mutex);
  msg->message_id = server.next_message_id++;
  pthread_mutex_unlock(&server.message_id_mutex);

  strncpy(msg->sender, sender, MAX_USERNAME_LEN - 1);
  msg->sender[MAX_USERNAME_LEN - 1] = '\0';

  strncpy(msg->recipient, recipient, MAX_USERNAME_LEN - 1);
  msg->recipient[MAX_USERNAME_LEN - 1] = '\0';

  msg->timestamp = time(NULL);
  msg->encrypted_len = encrypted_len;
  msg->delivered = false;

  msg->encrypted_data = malloc(encrypted_len);
  if (!msg->encrypted_data) {
    pthread_mutex_unlock(&recipient_client->queue_mutex);
    log_error("Failed to allocate memory for queued message");
    return -1;
  }

  memcpy(msg->encrypted_data, encrypted_data, encrypted_len);

  recipient_client->queue_tail =
      (recipient_client->queue_tail + 1) % MESSAGE_QUEUE_SIZE;
  recipient_client->queue_count++;

  pthread_mutex_unlock(&recipient_client->queue_mutex);

  log_debug("Message %u queued for %s from %s", msg->message_id, recipient,
            sender);
  return 0;
}

int deliver_queued_messages(client_connection_t *client) {
  if (!client || !client->authenticated) {
    return -1;
  }

  pthread_mutex_lock(&client->queue_mutex);

  int delivered_count = 0;

  while (client->queue_count > 0) {
    stored_message_t *msg = &client->message_queue[client->queue_head];

    if (msg->delivered) {
      client->queue_head = (client->queue_head + 1) % MESSAGE_QUEUE_SIZE;
      client->queue_count--;
      continue;
    }

    size_t sender_len = strlen(msg->sender);
    size_t total_size = 4 + 1 + sender_len + 4 + 2 + msg->encrypted_len;

    uint8_t *payload = malloc(total_size);
    if (!payload) {
      log_error("Failed to allocate memory for message delivery");
      break;
    }

    payload[0] = (msg->message_id >> 24) & 0xFF;
    payload[1] = (msg->message_id >> 16) & 0xFF;
    payload[2] = (msg->message_id >> 8) & 0xFF;
    payload[3] = msg->message_id & 0xFF;

    payload[4] = (uint8_t)sender_len;
    memcpy(&payload[5], msg->sender, sender_len);

    uint32_t timestamp = (uint32_t)msg->timestamp;
    payload[5 + sender_len] = (timestamp >> 24) & 0xFF;
    payload[5 + sender_len + 1] = (timestamp >> 16) & 0xFF;
    payload[5 + sender_len + 2] = (timestamp >> 8) & 0xFF;
    payload[5 + sender_len + 3] = timestamp & 0xFF;

    uint16_t msg_len = (uint16_t)msg->encrypted_len;
    payload[5 + sender_len + 4] = (msg_len >> 8) & 0xFF;
    payload[5 + sender_len + 5] = msg_len & 0xFF;

    memcpy(&payload[5 + sender_len + 6], msg->encrypted_data,
           msg->encrypted_len);

    if (send_network_message(client->socket_fd, MSG_INCOMING_MESSAGE, payload,
                             total_size) == 0) {
      msg->delivered = true;
      delivered_count++;

      log_debug("Delivered queued message %u to %s from %s", msg->message_id,
                client->username, msg->sender);
    } else {
      log_error("Failed to deliver queued message %u to %s", msg->message_id,
                client->username);
      sodium_memzero(payload, total_size);
      free(payload);
      break;
    }

    sodium_memzero(payload, total_size);
    free(payload);

    if (msg->encrypted_data) {
      sodium_memzero(msg->encrypted_data, msg->encrypted_len);
      free(msg->encrypted_data);
      msg->encrypted_data = NULL;
    }

    client->queue_head = (client->queue_head + 1) % MESSAGE_QUEUE_SIZE;
    client->queue_count--;
  }

  pthread_mutex_unlock(&client->queue_mutex);

  if (delivered_count > 0) {
    log_info("Delivered %d queued messages to %s", delivered_count,
             client->username);
  }

  return delivered_count;
}