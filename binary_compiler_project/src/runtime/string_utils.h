/**
 * @file string_utils.h
 * @brief 字符串工具函数 / String Utility Functions
 * @author newLISP Binary Compiler Team
 * @date 2024
 * 
 * 提供统一的字符串操作函数，解决strdup兼容性问题
 * Provides unified string operations, solving strdup compatibility issues
 */

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 安全的字符串复制函数 / Safe string duplication function
 * @param str 要复制的字符串 / String to duplicate
 * @return 复制的字符串指针，失败返回NULL / Duplicated string pointer, NULL on failure
 */
char* safe_strdup(const char* str);

/**
 * @brief 安全的字符串复制函数（带长度限制） / Safe string duplication with length limit
 * @param str 要复制的字符串 / String to duplicate
 * @param max_len 最大长度 / Maximum length
 * @return 复制的字符串指针，失败返回NULL / Duplicated string pointer, NULL on failure
 */
char* safe_strndup(const char* str, size_t max_len);

/**
 * @brief 安全的字符串连接 / Safe string concatenation
 * @param dest 目标字符串 / Destination string
 * @param src 源字符串 / Source string
 * @return 连接后的字符串指针，失败返回NULL / Concatenated string pointer, NULL on failure
 */
char* safe_strcat(const char* dest, const char* src);

/**
 * @brief 检查字符串是否为空 / Check if string is empty
 * @param str 字符串 / String
 * @return 空字符串返回true / Returns true if string is empty
 */
bool string_is_empty(const char* str);

/**
 * @brief 字符串长度（安全版本） / String length (safe version)
 * @param str 字符串 / String
 * @return 字符串长度 / String length
 */
size_t safe_strlen(const char* str);

/**
 * @brief 安全的字符串比较 / Safe string comparison
 * @param str1 第一个字符串 / First string
 * @param str2 第二个字符串 / Second string
 * @return 比较结果 / Comparison result
 */
int safe_strcmp(const char* str1, const char* str2);

/**
 * @brief 安全的字符串复制 / Safe string copy
 * @param dest 目标缓冲区 / Destination buffer
 * @param src 源字符串 / Source string
 * @param dest_size 目标缓冲区大小 / Destination buffer size
 * @return 成功返回true / Returns true on success
 */
bool safe_strcpy(char* dest, const char* src, size_t dest_size);

/**
 * @brief 格式化字符串（安全版本） / Safe string formatting
 * @param format 格式字符串 / Format string
 * @param ... 参数 / Arguments
 * @return 格式化后的字符串 / Formatted string
 */
char* safe_sprintf(const char* format, ...);

/**
 * @brief 字符串转整数（安全版本） / Safe string to integer conversion
 * @param str 字符串 / String
 * @param result 结果指针 / Result pointer
 * @return 成功返回true / Returns true on success
 */
bool safe_str_to_int(const char* str, long long* result);

/**
 * @brief 字符串转浮点数（安全版本） / Safe string to float conversion
 * @param str 字符串 / String
 * @param result 结果指针 / Result pointer
 * @return 成功返回true / Returns true on success
 */
bool safe_str_to_float(const char* str, double* result);

#ifdef __cplusplus
}
#endif

#endif // STRING_UTILS_H
