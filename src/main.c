#include "c-chat.h"

int main(int argc, char *argv[]) {
    int opt;
    char username[MAX_USERNAME_LEN] = {0};
    bool register_mode = false;
    bool login_mode = false;
    bool list_mode = false;
    
    // Command line options
    static struct option long_options[] = {
        {"register", required_argument, 0, 'r'},
        {"login", required_argument, 0, 'l'},
        {"list-users", no_argument, 0, 'u'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, "r:l:uhv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'r':
                register_mode = true;
                safe_strncpy(username, optarg, sizeof(username));
                break;
            case 'l':
                login_mode = true;
                safe_strncpy(username, optarg, sizeof(username));
                break;
            case 'u':
                list_mode = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return CCHAT_SUCCESS;
            case 'v':
                print_version();
                return CCHAT_SUCCESS;
            default:
                print_usage(argv[0]);
                return CCHAT_ERROR_INVALID_ARGS;
        }
    }
    
    // Execute based on mode
    if (register_mode) {
        if (validate_username(username) != CCHAT_SUCCESS) {
            fprintf(stderr, "Error: Invalid username\n");
            return CCHAT_ERROR_INVALID_ARGS;
        }
        
        cchat_error_t result = register_user(username);
        if (result != CCHAT_SUCCESS) {
            fprintf(stderr, "Registration failed\n");
            return result;
        }
        
        printf("User '%s' registered successfully!\n", username);
        return CCHAT_SUCCESS;
    }
    
    if (login_mode) {
        if (validate_username(username) != CCHAT_SUCCESS) {
            fprintf(stderr, "Error: Invalid username\n");
            return CCHAT_ERROR_INVALID_ARGS;
        }
        
        cchat_error_t result = login_user(username);
        if (result != CCHAT_SUCCESS) {
            fprintf(stderr, "Login failed\n");
            return result;
        }
        
        printf("Logged in as '%s'\n", username);
        
        // Initialize session with current user
        extern chat_session_t current_session;
        safe_strncpy(current_session.current_user.username, username, sizeof(current_session.current_user.username));
        current_session.current_user.is_authenticated = true;
        
        run_chat_interface();
        return CCHAT_SUCCESS;
    }
    
    if (list_mode) {
        cchat_error_t result = list_users();
        if (result != CCHAT_SUCCESS) {
            fprintf(stderr, "Failed to list users\n");
            return result;
        }
        return CCHAT_SUCCESS;
    }
    
    // No valid option provided
    print_usage(argv[0]);
    return CCHAT_ERROR_INVALID_ARGS;
}