#include "c-chat.h"
#ifdef _WIN32
#include <windows.h>
#endif

void print_usage(const char *program_name) {
  printf("C-Chat - Secure End-to-End Encrypted CLI Chat\n\n");
  printf("Usage: %s [OPTIONS]\n\n", program_name);
  printf("Options:\n");
  printf("  -r, --register <username>   Register a new user account\n");
  printf("  -l, --login <username>      Login to your account\n");
  printf("  -u, --list-users            List all registered users\n");
  printf("  -h, --help                  Show this help message\n");
  printf("  -v, --version               Show version information\n\n");
  printf("Examples:\n");
  printf("  %s --register alice         Register user 'alice'\n", program_name);
  printf("  %s --login alice            Login as 'alice'\n", program_name);
  printf("  %s --list-users             List all users\n\n", program_name);
  printf("In-Chat Commands:\n");
  printf("  chat <username>             Start encrypted chat with user\n");
  printf("  /exit                       Exit current chat session\n");
  printf("  /quit                       Quit c-chat application\n\n");
}

void print_version(void) {
  printf("C-Chat v1.0.0\n");
  printf("Built with pure C for maximum security and performance\n");
  printf("Copyright (c) 2025 - MIT License\n");
}

// Global current user for session management
extern chat_session_t current_session;

void run_chat_interface(void) {
  char input[MAX_MESSAGE_LEN];
  char command[MAX_COMMAND_LEN];
  char target_user[MAX_USERNAME_LEN];
  bool running = true;

  printf(
      "\nWelcome to C-Chat! Type 'chat <username>' to start a conversation.\n");
  printf("Available commands: chat, /exit, /quit\n\n");

  while (running) {
    printf("c-chat> ");
    fflush(stdout);

    if (!fgets(input, sizeof(input), stdin)) {
      break;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Parse command
    if (sscanf(input, "%s %s", command, target_user) >= 1) {
      if (strcmp(command, "/quit") == 0) {
        running = false;
        printf("Goodbye!\n");
      } else if (strcmp(command, "chat") == 0) {
        if (strlen(target_user) > 0) {
          printf("Starting chat with %s... (Type /exit to return)\n",
                 target_user);
          start_chat(target_user);
        } else {
          printf("Usage: chat <username>\n");
        }
      } else if (strcmp(command, "/exit") == 0) {
        printf("No active chat session to exit.\n");
      } else {
        printf("Unknown command. Available commands: chat, /exit, /quit\n");
      }
    }
  }
}

cchat_error_t secure_clear_screen(void) {
#ifdef _WIN32
  // Use Windows API instead of system() call
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole == INVALID_HANDLE_VALUE) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  COORD coordScreen = {0, 0};
  DWORD cCharsWritten;
  DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

  if (!FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen,
                                  &cCharsWritten)) {
    return CCHAT_ERROR_INVALID_ARGS;
  }

  if (!SetConsoleCursorPosition(hConsole, coordScreen)) {
    return CCHAT_ERROR_INVALID_ARGS;
  }
#else
  // Use ANSI escape sequences instead of system() call
  printf("\033[2J\033[H");
  fflush(stdout);
#endif
  return CCHAT_SUCCESS;
}