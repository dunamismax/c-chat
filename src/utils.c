#include "c-chat.h"

void safe_strncpy(char* dest, const char* src, size_t size) {
    if (!dest || !src || size == 0) {
        return;
    }
    
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}