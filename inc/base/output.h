#ifndef OUTPUT_HEADER
#define OUTPUT_HEADER

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "glib.h"
#include "filetools.h"

#define STREAM GString*
#define OUT(...) g_string_append_printf(stream, __VA_ARGS__)

#define OUTPUT_FILE_READ_BUFFER_SIZE 100

extern void output_header(int out, char* type, guint32 length, SSL* ssl);
extern void output_not_found(int out, SSL *ssl);
extern void output_string(int fd, char* str, SSL* ssl);
extern void output_number(int fd, guint32 number, SSL *ssl);
extern int output_get_file_length(char *file);
extern void output_file(int fd, sArray_t array, SSL* ssl);

#endif