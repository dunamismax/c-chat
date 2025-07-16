#include "../include/c-chat-server.h"
#include <stdarg.h>

server_state_t server = {0};

void log_info(const char *format, ...) {
  time_t now;
  struct tm *tm_info;
  char timestamp[64];

  time(&now);
  tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  printf("[%s] INFO: ", timestamp);

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
  fflush(stdout);
}

void log_error(const char *format, ...) {
  time_t now;
  struct tm *tm_info;
  char timestamp[64];

  time(&now);
  tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  fprintf(stderr, "[%s] ERROR: ", timestamp);

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
  fflush(stderr);
}

void log_debug(const char *format, ...) {
#ifdef DEBUG
  time_t now;
  struct tm *tm_info;
  char timestamp[64];

  time(&now);
  tm_info = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

  printf("[%s] DEBUG: ", timestamp);

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
  fflush(stdout);
#else
  (void)format;
#endif
}

void signal_handler(int sig) {
  log_info("Received signal %d, shutting down server gracefully", sig);

  pthread_mutex_lock(&server.running_mutex);
  server.running = false;
  pthread_mutex_unlock(&server.running_mutex);

  if (server.server_socket >= 0) {
    close(server.server_socket);
  }
}

int init_server(void) {
  log_info("Initializing C-Chat Server");

  if (sodium_init() < 0) {
    log_error("Failed to initialize libsodium");
    return -1;
  }

  memset(&server, 0, sizeof(server));
  server.server_socket = -1;
  server.running = true;
  server.next_message_id = 1;

  if (pthread_mutex_init(&server.users_mutex, NULL) != 0 ||
      pthread_mutex_init(&server.clients_mutex, NULL) != 0 ||
      pthread_mutex_init(&server.message_id_mutex, NULL) != 0 ||
      pthread_mutex_init(&server.running_mutex, NULL) != 0) {
    log_error("Failed to initialize mutexes");
    return -1;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (pthread_mutex_init(&server.users[i].mutex, NULL) != 0 ||
        pthread_mutex_init(&server.clients[i].mutex, NULL) != 0 ||
        pthread_mutex_init(&server.clients[i].queue_mutex, NULL) != 0) {
      log_error("Failed to initialize client mutex %d", i);
      return -1;
    }
    server.clients[i].socket_fd = -1;
  }

  server.server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server.server_socket < 0) {
    log_error("Failed to create server socket: %s", strerror(errno));
    return -1;
  }

  int opt = 1;
  if (setsockopt(server.server_socket, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt)) < 0) {
    log_error("Failed to set socket options: %s", strerror(errno));
    close(server.server_socket);
    return -1;
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(SERVER_PORT);

  if (bind(server.server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    log_error("Failed to bind server socket: %s", strerror(errno));
    close(server.server_socket);
    return -1;
  }

  if (listen(server.server_socket, SERVER_BACKLOG) < 0) {
    log_error("Failed to listen on server socket: %s", strerror(errno));
    close(server.server_socket);
    return -1;
  }

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGPIPE, SIG_IGN);

  log_info("Server initialized successfully on port %d", SERVER_PORT);
  return 0;
}

void cleanup_server(void) {
  log_info("Cleaning up server resources");

  pthread_mutex_lock(&server.running_mutex);
  server.running = false;
  pthread_mutex_unlock(&server.running_mutex);

  if (server.server_socket >= 0) {
    close(server.server_socket);
  }

  pthread_mutex_lock(&server.clients_mutex);
  for (int i = 0; i < server.client_count; i++) {
    if (server.clients[i].connected && server.clients[i].socket_fd >= 0) {
      close(server.clients[i].socket_fd);
      server.clients[i].connected = false;
    }

    for (int j = 0; j < server.clients[i].queue_count; j++) {
      if (server.clients[i].message_queue[j].encrypted_data) {
        sodium_memzero(server.clients[i].message_queue[j].encrypted_data,
                       server.clients[i].message_queue[j].encrypted_len);
        free(server.clients[i].message_queue[j].encrypted_data);
      }
    }
  }
  pthread_mutex_unlock(&server.clients_mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    pthread_mutex_destroy(&server.users[i].mutex);
    pthread_mutex_destroy(&server.clients[i].mutex);
    pthread_mutex_destroy(&server.clients[i].queue_mutex);
  }

  pthread_mutex_destroy(&server.users_mutex);
  pthread_mutex_destroy(&server.clients_mutex);
  pthread_mutex_destroy(&server.message_id_mutex);
  pthread_mutex_destroy(&server.running_mutex);

  log_info("Server cleanup completed");
}

int main(void) {
  log_info("Starting C-Chat Server v1.0.0");

  if (init_server() < 0) {
    log_error("Failed to initialize server");
    return EXIT_FAILURE;
  }

  log_info("Server listening on port %d", SERVER_PORT);

  while (server.running) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(server.server_socket,
                               (struct sockaddr *)&client_addr, &client_len);
    if (client_socket < 0) {
      if (errno == EINTR && !server.running) {
        break;
      }
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        log_error("Failed to accept client connection: %s", strerror(errno));
      }
      continue;
    }

    pthread_mutex_lock(&server.clients_mutex);

    int client_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!server.clients[i].connected) {
        client_index = i;
        break;
      }
    }

    if (client_index == -1) {
      pthread_mutex_unlock(&server.clients_mutex);
      log_error("Maximum client connections reached, rejecting client");
      close(client_socket);
      continue;
    }

    memset(&server.clients[client_index], 0, sizeof(client_connection_t));
    server.clients[client_index].socket_fd = client_socket;
    server.clients[client_index].address = client_addr;
    server.clients[client_index].connected = true;
    server.clients[client_index].authenticated = false;
    server.clients[client_index].status = STATUS_ONLINE;
    server.clients[client_index].connected_time = time(NULL);
    server.clients[client_index].queue_head = 0;
    server.clients[client_index].queue_tail = 0;
    server.clients[client_index].queue_count = 0;

    randombytes_buf(server.clients[client_index].challenge, CHALLENGE_SIZE);

    if (client_index >= server.client_count) {
      server.client_count = client_index + 1;
    }

    pthread_mutex_unlock(&server.clients_mutex);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    log_info("New client connected from %s:%d (slot %d)", client_ip,
             ntohs(client_addr.sin_port), client_index);

    if (pthread_create(&server.clients[client_index].thread_id, NULL,
                       client_handler, &server.clients[client_index]) != 0) {
      log_error("Failed to create client thread: %s", strerror(errno));
      close(client_socket);
      server.clients[client_index].connected = false;
    }
  }

  log_info("Server shutting down");
  cleanup_server();
  return EXIT_SUCCESS;
}