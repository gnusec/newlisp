/**
 * @file string_utils.c
 * @brief 字符串工具函数实现 / String Utility Functions Implementation
 * @author newLISP Binary Compiler Team
 * @date 2024
 */

#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>

/**
 * @brief 安全的字符串复制函数 / Safe string duplication function
 */
char* safe_strdup(const char* str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char* dup = malloc(len + 1);
    if (!dup) {
        return NULL;
    }
    
    memcpy(dup, str, len + 1);
    return dup;
}

/**
 * @brief 安全的字符串复制函数（带长度限制） / Safe string duplication with length limit
 */
char* safe_strndup(const char* str, size_t max_len) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str);
    if (len > max_len) {
        len = max_len;
    }
    
    char* dup = malloc(len + 1);
    if (!dup) {
        return NULL;
    }
    
    memcpy(dup, str, len);
    dup[len] = '\0';
    return dup;
}

/**
 * @brief 安全的字符串连接 / Safe string concatenation
 */
char* safe_strcat(const char* dest, const char* src) {
    if (!dest && !src) {
        return NULL;
    }
    
    if (!dest) {
        return safe_strdup(src);
    }
    
    if (!src) {
        return safe_strdup(dest);
    }
    
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);
    size_t total_len = dest_len + src_len;
    
    char* result = malloc(total_len + 1);
    if (!result) {
        return NULL;
    }
    
    memcpy(result, dest, dest_len);
    memcpy(result + dest_len, src, src_len);
    result[total_len] = '\0';
    
    return result;
}

/**
 * @brief 检查字符串是否为空 / Check if string is empty
 */
bool string_is_empty(const char* str) {
    return !str || str[0] == '\0';
}

/**
 * @brief 字符串长度（安全版本） / String length (safe version)
 */
size_t safe_strlen(const char* str) {
    return str ? strlen(str) : 0;
}

/**
 * @brief 安全的字符串比较 / Safe string comparison
 */
int safe_strcmp(const char* str1, const char* str2) {
    if (!str1 && !str2) return 0;
    if (!str1) return -1;
    if (!str2) return 1;
    return strcmp(str1, str2);
}

/**
 * @brief 安全的字符串复制 / Safe string copy
 */
bool safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return false;
    }

    size_t src_len = strlen(src);
    if (src_len >= dest_size) {
        return false; // 目标缓冲区太小 / Destination buffer too small
    }

    strcpy(dest, src);
    return true;
}

/**
 * @brief 格式化字符串（安全版本） / Safe string formatting
 */
char* safe_sprintf(const char* format, ...) {
    if (!format) {
        return NULL;
    }

    va_list args;
    va_start(args, format);

    // 计算需要的长度 / Calculate needed length
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        va_end(args);
        return NULL;
    }

    char* result = malloc(needed + 1);
    if (!result) {
        va_end(args);
        return NULL;
    }

    vsnprintf(result, needed + 1, format, args);
    va_end(args);

    return result;
}

/**
 * @brief 字符串转整数（安全版本） / Safe string to integer conversion
 */
bool safe_str_to_int(const char* str, long long* result) {
    if (!str || !result) {
        return false;
    }

    char* endptr;
    errno = 0;
    long long value = strtoll(str, &endptr, 10);

    if (errno != 0 || endptr == str || *endptr != '\0') {
        return false;
    }

    *result = value;
    return true;
}

/**
 * @brief 字符串转浮点数（安全版本） / Safe string to float conversion
 */
bool safe_str_to_float(const char* str, double* result) {
    if (!str || !result) {
        return false;
    }

    char* endptr;
    errno = 0;
    double value = strtod(str, &endptr);

    if (errno != 0 || endptr == str || *endptr != '\0') {
        return false;
    }

    *result = value;
    return true;
}
