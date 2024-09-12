#include <stdarg.h>
#include <stdio.h>

#include "logger.h"
#include "malloc_utils.h"
#include "xmlparser.h"


void logger_error(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[e] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

void logger_warn(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[!] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

void logger_info(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[i] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}

void logger_log(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    fputs("[~] ", stdout);
    vfprintf(stdout, fmt, argv);
    fputc('\n', stdout);
    va_end(argv);
}


void logger_error_xml(const char* fmt, const void* xml_parser_node, ...) {
    va_list argv;
    va_start(argv, xml_parser_node);

    char* outerXML;

    if (xml_parser_node)
        outerXML = xmlparser_get_outerXml((XmlNode)xml_parser_node);
    else
        outerXML = NULL;

    fputs("[e] ", stdout);
    vfprintf(stdout, fmt, argv);
    va_end(argv);

    if (xml_parser_node) {
        fputc('\n', stdout);
        if (outerXML) {
            fputs(outerXML, stdout);
            free_chk(outerXML);
        } else {
            fprintf(stdout, "{%s: %p}", xmlparser_get_tag_name(xml_parser_node), xml_parser_node);
        }
    }

    fputc('\n', stdout);
}

void logger_warn_xml(const char* fmt, const void* xml_parser_node, ...) {
    va_list argv;
    va_start(argv, xml_parser_node);

    char* outerXML;

    if (xml_parser_node)
        outerXML = xmlparser_get_outerXml((XmlNode)xml_parser_node);
    else
        outerXML = NULL;

    fputs("[!] ", stdout);
    vfprintf(stdout, fmt, argv);
    va_end(argv);

    if (xml_parser_node) {
        fputc('\n', stdout);
        if (outerXML) {
            fputs(outerXML, stdout);
            free_chk(outerXML);
        } else {
            fprintf(stdout, "{%s: %p}", xmlparser_get_tag_name(xml_parser_node), xml_parser_node);
        }
    }

    fputc('\n', stdout);
}

void logger_info_xml(const char* fmt, const void* xml_parser_node, ...) {
    va_list argv;
    va_start(argv, xml_parser_node);

    char* outerXML;

    if (xml_parser_node)
        outerXML = xmlparser_get_outerXml((XmlNode)xml_parser_node);
    else
        outerXML = NULL;

    fputs("[i] ", stdout);
    vfprintf(stdout, fmt, argv);
    va_end(argv);

    if (xml_parser_node) {
        fputc('\n', stdout);
        if (outerXML) {
            fputs(outerXML, stdout);
            free_chk(outerXML);
        } else {
            fprintf(stdout, "{%s: %p}", xmlparser_get_tag_name(xml_parser_node), xml_parser_node);
        }
    }

    fputc('\n', stdout);
}

void logger_log_xml(const char* fmt, const void* xml_parser_node, ...) {
    va_list argv;
    va_start(argv, xml_parser_node);

    char* outerXML;

    if (xml_parser_node)
        outerXML = xmlparser_get_outerXml((XmlNode)xml_parser_node);
    else
        outerXML = NULL;

    fputs("[~] ", stdout);
    vfprintf(stdout, fmt, argv);
    va_end(argv);

    if (xml_parser_node) {
        fputc('\n', stdout);
        if (outerXML) {
            fputs(outerXML, stdout);
            free_chk(outerXML);
        } else {
            fprintf(stdout, "{%s: %p}", xmlparser_get_tag_name(xml_parser_node), xml_parser_node);
        }
    }

    fputc('\n', stdout);
}
