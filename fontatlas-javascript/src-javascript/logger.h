#ifndef __logger_h
#define __logger_h

#include "number_format_specifiers.h"

void logger_error(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void logger_warn(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void logger_info(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void logger_log(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

void logger_error_xml(const char* fmt, const void* xml_parser_node, ...) __attribute__((format(printf, 1, 3)));
void logger_warn_xml(const char* fmt, const void* xml_parser_node, ...) __attribute__((format(printf, 1, 3)));
void logger_info_xml(const char* fmt, const void* xml_parser_node, ...) __attribute__((format(printf, 1, 3)));
void logger_log_xml(const char* fmt, const void* xml_parser_node, ...) __attribute__((format(printf, 1, 3)));

#endif