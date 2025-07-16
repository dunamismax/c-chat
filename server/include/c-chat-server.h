#ifndef C_CHAT_SERVER_H
#define C_CHAT_SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sodium.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 1024
#define MAX_CLIENTS 1000
#define SERVER_PORT 8080
#define SERVER_BACKLOG 50
#define MESSAGE_QUEUE_SIZE 100
#define RATE_LIMIT_WINDOW 60
#define RATE_LIMIT_MAX_REQUESTS 100

#define PUBLIC_KEY_SIZE crypto_box_PUBLICKEYBYTES
#define PRIVATE_KEY_SIZE crypto_box_SECRETKEYBYTES
#define SIGNATURE_SIZE crypto_sign_BYTES
#define CHALLENGE_SIZE 32

typedef enum {
  MSG_REGISTER_USER = 0x01,
  MSG_LOGIN_USER = 0x02,
  MSG_GET_PUBLIC_KEY = 0x03,
  MSG_SEND_MESSAGE = 0x04,
  MSG_GET_MESSAGES = 0x05,
  MSG_SET_STATUS = 0x06,
  MSG_LIST_USERS = 0x07,
  MSG_LOGOUT = 0x08,

  MSG_REGISTER_RESPONSE = 0x81,
  MSG_LOGIN_RESPONSE = 0x82,
  MSG_PUBLIC_KEY_RESPONSE = 0x83,
  MSG_MESSAGE_ACK = 0x84,
  MSG_INCOMING_MESSAGE = 0x85,
  MSG_USER_LIST_RESPONSE = 0x86,
  MSG_STATUS_UPDATE = 0x87,
  MSG_ERROR = 0x88
} message_type_t;

typedef enum {
  STATUS_OFFLINE = 0,
  STATUS_ONLINE = 1,
  STATUS_AWAY = 2
} user_status_t;

typedef enum {
  ERR_INVALID_USERNAME = 0x01,
  ERR_USER_EXISTS = 0x02,
  ERR_USER_NOT_FOUND = 0x03,
  ERR_AUTH_FAILED = 0x04,
  ERR_INVALID_FORMAT = 0x05,
  ERR_RATE_LIMIT = 0x06,
  ERR_SERVER_ERROR = 0x07,
  ERR_CONNECTION_TERMINATED = 0x08
} error_code_t;

typedef struct {
  uint32_t length;
  uint8_t type;
  uint8_t *payload;
} network_message_t;

typedef struct {
  char username[MAX_USERNAME_LEN];
  unsigned char public_key[PUBLIC_KEY_SIZE];
  user_status_t status;
  time_t last_seen;
  bool is_registered;
  pthread_mutex_t mutex;
} user_record_t;

typedef struct {
  uint32_t message_id;
  char sender[MAX_USERNAME_LEN];
  char recipient[MAX_USERNAME_LEN];
  time_t timestamp;
  size_t encrypted_len;
  unsigned char *encrypted_data;
  bool delivered;
} stored_message_t;

typedef struct {
  time_t window_start;
  int request_count;
} rate_limit_t;

typedef struct {
  int socket_fd;
  struct sockaddr_in address;
  char username[MAX_USERNAME_LEN];
  bool authenticated;
  bool connected;
  user_status_t status;
  unsigned char challenge[CHALLENGE_SIZE];
  time_t connected_time;
  rate_limit_t rate_limit;

  stored_message_t message_queue[MESSAGE_QUEUE_SIZE];
  int queue_head;
  int queue_tail;
  int queue_count;
  pthread_mutex_t queue_mutex;

  pthread_t thread_id;
  pthread_mutex_t mutex;
} client_connection_t;

typedef struct {
  user_record_t users[MAX_CLIENTS];
  int user_count;
  pthread_mutex_t users_mutex;

  client_connection_t clients[MAX_CLIENTS];
  int client_count;
  pthread_mutex_t clients_mutex;

  uint32_t next_message_id;
  pthread_mutex_t message_id_mutex;

  int server_socket;
  bool running;
  pthread_mutex_t running_mutex;
} server_state_t;

extern server_state_t server;

int init_server(void);
void cleanup_server(void);
void *client_handler(void *arg);
void signal_handler(int sig);

int send_network_message(int socket_fd, message_type_t type,
                         const uint8_t *payload, uint32_t payload_len);
int receive_network_message(int socket_fd, network_message_t *msg);
void free_network_message(network_message_t *msg);
int send_error(int socket_fd, error_code_t error_code,
               const char *error_message);

int handle_register_user(client_connection_t *client, const uint8_t *payload,
                         uint32_t payload_len);
int handle_login_user(client_connection_t *client, const uint8_t *payload,
                      uint32_t payload_len);
int handle_get_public_key(client_connection_t *client, const uint8_t *payload,
                          uint32_t payload_len);
int handle_send_message(client_connection_t *client, const uint8_t *payload,
                        uint32_t payload_len);
int handle_get_messages(client_connection_t *client, const uint8_t *payload,
                        uint32_t payload_len);
int handle_set_status(client_connection_t *client, const uint8_t *payload,
                      uint32_t payload_len);
int handle_list_users(client_connection_t *client, const uint8_t *payload,
                      uint32_t payload_len);
int handle_logout(client_connection_t *client, const uint8_t *payload,
                  uint32_t payload_len);

user_record_t *find_user(const char *username);
client_connection_t *find_client_by_username(const char *username);
int add_user(const char *username, const unsigned char *public_key);
int authenticate_user(client_connection_t *client, const char *username,
                      const unsigned char *signature);

int queue_message(const char *recipient, const char *sender,
                  const unsigned char *encrypted_data, size_t encrypted_len);
int deliver_queued_messages(client_connection_t *client);

bool check_rate_limit(client_connection_t *client);
void update_rate_limit(client_connection_t *client);

int validate_username_server(const char *username);
void broadcast_status_update(const char *username, user_status_t status);

void log_info(const char *format, ...);
void log_error(const char *format, ...);
void log_debug(const char *format, ...);

#endif // C_CHAT_SERVER_H