#include "c-chat.h"
#include <termios.h>
#include <pwd.h>

void safe_strncpy(char* dest, const char* src, size_t size) {
    if (!dest || !src || size == 0) {
        return;
    }
    
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

void get_password_input(const char* prompt, char* password, size_t max_len) {
    if (!prompt || !password || max_len == 0) {
        return;
    }
    
    printf("%s", prompt);
    fflush(stdout);
    
    // Disable echo for password input
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    // Read password
    if (fgets(password, max_len, stdin)) {
        // Remove newline
        password[strcspn(password, "\n")] = '\0';
    }
    
    // Restore echo
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    printf("\n");
}

char* get_home_directory(void) {
    char* home = getenv("HOME");
    if (home) {
        return strdup(home);
    }
    
    // Fallback to passwd entry
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return strdup(pw->pw_dir);
    }
    
    return NULL;
}

cchat_error_t secure_file_permissions(const char* filepath) {
    if (!filepath) {
        return CCHAT_ERROR_INVALID_ARGS;
    }
    
    // Set file permissions to owner read/write only (600)
    if (chmod(filepath, S_IRUSR | S_IWUSR) != 0) {
        perror("Failed to set secure file permissions");
        return CCHAT_ERROR_PERMISSION_DENIED;
    }
    
    return CCHAT_SUCCESS;
}