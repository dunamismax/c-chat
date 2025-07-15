#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/**
 * File utilities application
 * Demonstrates: file I/O, directory operations, system calls
 *
 * SECURITY: Implements path validation and access controls
 */

#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 256

/**
 * Validate path to prevent directory traversal attacks
 * Returns 0 if path is safe, -1 if dangerous
 */
static int validate_path(const char *path) {
  if (!path) {
    return -1;
  }

  // Check for null bytes (directory traversal attempt)
  if (strlen(path) != strcspn(path, "\0")) {
    return -1;
  }

  // Check for directory traversal patterns
  if (strstr(path, "../") != NULL || strstr(path, "/..") != NULL ||
      strcmp(path, "..") == 0 || strncmp(path, "../", 3) == 0) {
    return -1;
  }

  // Check for absolute paths to system directories
  const char *dangerous_prefixes[] = {"/etc/",  "/usr/", "/bin/",  "/sbin/",
                                      "/boot/", "/sys/", "/proc/", "/dev/",
                                      "/root/", "/var/", NULL};

  for (int i = 0; dangerous_prefixes[i]; i++) {
    if (strncmp(path, dangerous_prefixes[i], strlen(dangerous_prefixes[i])) ==
        0) {
      return -1;
    }
  }

  // Check path length
  if (strlen(path) >= MAX_PATH_LENGTH) {
    return -1;
  }

  // Check for control characters
  for (const char *p = path; *p; p++) {
    if (*p < 32 && *p != '\t') {
      return -1;
    }
  }

  return 0;
}

/**
 * Get real path and validate it's within allowed directories
 * Returns malloc'd canonical path or NULL if invalid
 */
static char *get_safe_path(const char *path) {
  if (validate_path(path) != 0) {
    printf("Error: Invalid or dangerous path\n");
    return NULL;
  }

  char *real_path = realpath(path, NULL);
  if (!real_path) {
    if (errno == ENOENT) {
      printf("Error: File or directory does not exist\n");
    } else {
      printf("Error: Cannot resolve path\n");
    }
    return NULL;
  }

  // Additional validation on resolved path
  if (validate_path(real_path) != 0) {
    printf("Error: Resolved path is not safe\n");
    free(real_path);
    return NULL;
  }

  return real_path;
}

/**
 * Safe filename validation
 */
static int validate_filename(const char *filename) {
  if (!filename || strlen(filename) == 0) {
    return -1;
  }

  if (strlen(filename) >= MAX_FILENAME_LENGTH) {
    return -1;
  }

  // Check for dangerous characters
  const char *dangerous_chars = "<>:\"|?*\0";
  if (strpbrk(filename, dangerous_chars) != NULL) {
    return -1;
  }

  // Check for reserved names on various systems
  const char *reserved_names[] = {
      "CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2", "COM3", "COM4",
      "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3",
      "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9", NULL};

  for (int i = 0; reserved_names[i]; i++) {
    if (strcasecmp(filename, reserved_names[i]) == 0) {
      return -1;
    }
  }

  return 0;
}

/**
 * Display file information with security validation
 */
void show_file_info(const char *filename) {
  char *safe_path = get_safe_path(filename);
  if (!safe_path) {
    return;
  }

  struct stat file_stat;

  if (stat(safe_path, &file_stat) != 0) {
    printf("Error: Cannot access file '%s'\n", filename);
    free(safe_path);
    return;
  }

  printf("\n=== File Information: %s ===\n", filename);
  printf("Size: %lld bytes\n", (long long)file_stat.st_size);
  printf("Type: ");

  if (S_ISREG(file_stat.st_mode)) {
    printf("Regular file\n");
  } else if (S_ISDIR(file_stat.st_mode)) {
    printf("Directory\n");
  } else if (S_ISLNK(file_stat.st_mode)) {
    printf("Symbolic link\n");
  } else {
    printf("Other\n");
  }

  printf("Permissions: ");
  printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
  printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
  printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
  printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
  printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
  printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
  printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
  printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
  printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
  printf("\n");

  // Convert timestamps
  printf("Last modified: %s", ctime(&file_stat.st_mtime));
  printf("Last accessed: %s", ctime(&file_stat.st_atime));

  free(safe_path);
}

/**
 * Count lines, words, and characters in a file with security validation
 */
void count_file_content(const char *filename) {
  char *safe_path = get_safe_path(filename);
  if (!safe_path) {
    return;
  }

  FILE *file = fopen(safe_path, "r");
  if (!file) {
    printf("Error: Cannot open file '%s'\n", filename);
    free(safe_path);
    return;
  }

  int lines = 0, words = 0, chars = 0;
  int in_word = 0;
  int c;

  while ((c = fgetc(file)) != EOF) {
    chars++;

    if (c == '\n') {
      lines++;
    }

    if (c == ' ' || c == '\t' || c == '\n') {
      in_word = 0;
    } else if (!in_word) {
      in_word = 1;
      words++;
    }
  }

  // If file doesn't end with newline, still count the last line
  if (chars > 0 && c != '\n') {
    lines++;
  }

  fclose(file);

  printf("\n=== File Statistics: %s ===\n", filename);
  printf("Lines: %d\n", lines);
  printf("Words: %d\n", words);
  printf("Characters: %d\n", chars);

  free(safe_path);
}

/**
 * List directory contents with security validation
 */
void list_directory(const char *dirname) {
  char *safe_path = get_safe_path(dirname);
  if (!safe_path) {
    return;
  }

  DIR *dir = opendir(safe_path);
  if (!dir) {
    printf("Error: Cannot open directory '%s'\n", dirname);
    free(safe_path);
    return;
  }

  printf("\n=== Directory Contents: %s ===\n", dirname);

  struct dirent *entry;
  int count = 0;

  while ((entry = readdir(dir)) != NULL) {
    // Skip hidden files starting with '.'
    if (entry->d_name[0] == '.') {
      continue;
    }

    printf("%s", entry->d_name);

    // Try to get more info about the entry
    char full_path[MAX_PATH_LENGTH];
    int ret = snprintf(full_path, sizeof(full_path), "%s/%s", safe_path,
                       entry->d_name);
    if (ret >= sizeof(full_path) || ret < 0) {
      printf("Error: Path too long for entry %s\n", entry->d_name);
      continue;
    }

    struct stat entry_stat;
    if (stat(full_path, &entry_stat) == 0) {
      if (S_ISDIR(entry_stat.st_mode)) {
        printf("/");
      } else if (S_ISREG(entry_stat.st_mode)) {
        printf(" (%lld bytes)", (long long)entry_stat.st_size);
      }
    }

    printf("\n");
    count++;
  }

  printf("\nTotal entries: %d\n", count);
  closedir(dir);
  free(safe_path);
}

/**
 * Copy file contents with security validation and atomic operations
 */
void copy_file(const char *source, const char *dest) {
  // Validate source path
  char *safe_source = get_safe_path(source);
  if (!safe_source) {
    return;
  }

  // Validate destination path (allow non-existent files)
  if (validate_path(dest) != 0) {
    printf("Error: Invalid destination path\n");
    free(safe_source);
    return;
  }

  // Validate destination filename
  char *dest_copy = strdup(dest);
  if (!dest_copy) {
    printf("Error: Memory allocation failed\n");
    free(safe_source);
    return;
  }

  char *dest_filename = basename(dest_copy);
  if (validate_filename(dest_filename) != 0) {
    printf("Error: Invalid destination filename\n");
    free(dest_copy);
    free(safe_source);
    return;
  }
  free(dest_copy);

  // Check if source and destination are the same
  struct stat src_stat, dst_stat;
  if (stat(safe_source, &src_stat) != 0) {
    printf("Error: Cannot access source file '%s'\n", source);
    free(safe_source);
    return;
  }

  if (stat(dest, &dst_stat) == 0) {
    if (src_stat.st_dev == dst_stat.st_dev &&
        src_stat.st_ino == dst_stat.st_ino) {
      printf("Error: Source and destination are the same file\n");
      free(safe_source);
      return;
    }
  }

  FILE *src = fopen(safe_source, "rb");
  if (!src) {
    printf("Error: Cannot open source file '%s'\n", source);
    free(safe_source);
    return;
  }

  // Create temporary file for atomic copy
  char temp_dest[MAX_PATH_LENGTH];
  int ret = snprintf(temp_dest, sizeof(temp_dest), "%s.tmp.%d", dest, getpid());
  if (ret >= sizeof(temp_dest) || ret < 0) {
    printf("Error: Destination path too long\n");
    fclose(src);
    free(safe_source);
    return;
  }

  FILE *dst = fopen(temp_dest, "wb");
  if (!dst) {
    printf("Error: Cannot create destination file '%s'\n", dest);
    fclose(src);
    free(safe_source);
    return;
  }

  // Set restrictive permissions on temporary file
  if (chmod(temp_dest, S_IRUSR | S_IWUSR) != 0) {
    printf("Warning: Could not set secure permissions on temporary file\n");
  }

  char buffer[4096];
  size_t bytes_read;
  size_t total_bytes = 0;

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
    size_t bytes_written = fwrite(buffer, 1, bytes_read, dst);
    if (bytes_written != bytes_read) {
      printf("Error: Write failed\n");
      break;
    }
    total_bytes += bytes_written;
  }

  fclose(src);
  fclose(dst);

  // Atomically rename temporary file to final destination
  if (rename(temp_dest, dest) != 0) {
    printf("Error: Failed to finalize copy operation\n");
    unlink(temp_dest); // Clean up temporary file
    free(safe_source);
    return;
  }

  printf("Successfully copied %zu bytes from '%s' to '%s'\n", total_bytes,
         source, dest);
  free(safe_source);
}

/**
 * Display help information
 */
void show_help() {
  printf("\n=== File Utils Help ===\n");
  printf("Commands:\n");
  printf("  info <file>        - Show file information\n");
  printf("  count <file>       - Count lines, words, chars\n");
  printf("  list <directory>   - List directory contents\n");
  printf("  copy <src> <dst>   - Copy file\n");
  printf("  help               - Show this help\n");
  printf("  quit               - Exit program\n");
  printf("\nExample: info myfile.txt\n");
  printf("Example: list /home/user\n\n");
}

int main() {
  char input[512];
  char command[64];
  char arg1[256], arg2[256];

  printf("=== File Utilities ===\n");
  printf("Type 'help' for commands or 'quit' to exit\n\n");

  while (1) {
    printf("fileutils> ");

    // Read input line
    if (!fgets(input, sizeof(input), stdin)) {
      break;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Skip empty lines
    if (strlen(input) == 0) {
      continue;
    }

    // Parse command and arguments with bounds checking
    int parsed = sscanf(input, "%63s %255s %255s", command, arg1, arg2);

    if (parsed < 1) {
      continue;
    }

    if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
      printf("Goodbye!\n");
      break;
    } else if (strcmp(command, "help") == 0) {
      show_help();
    } else if (strcmp(command, "info") == 0) {
      if (parsed < 2) {
        printf("Usage: info <filename>\n");
      } else {
        show_file_info(arg1);
      }
    } else if (strcmp(command, "count") == 0) {
      if (parsed < 2) {
        printf("Usage: count <filename>\n");
      } else {
        count_file_content(arg1);
      }
    } else if (strcmp(command, "list") == 0) {
      if (parsed < 2) {
        list_directory(".");
      } else {
        list_directory(arg1);
      }
    } else if (strcmp(command, "copy") == 0) {
      if (parsed < 3) {
        printf("Usage: copy <source> <destination>\n");
      } else {
        copy_file(arg1, arg2);
      }
    } else {
      printf("Unknown command: %s\n", command);
      printf("Type 'help' for available commands.\n");
    }

    printf("\n");
  }

  return 0;
}