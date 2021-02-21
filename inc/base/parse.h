#ifndef PARSE_HEADER
#define PARSE_HEADER

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <filetools.h>
#include <hash_table.h>

unsigned char parse(int sock, hash_table_t* options, SSL* ssl, int * ssl_error);

#endif