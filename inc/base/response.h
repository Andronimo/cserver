#ifndef RESPONSE_HEADER
#define RESPONSE_HEADER

#include "hash_table.h"
#include <filetools.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP_OK         (200u)
#define HTTP_NOT_FOUND  (404u)

typedef struct response
{
    hash_table_t options;
    uint32_t returncode;
} response_t;

#define RESPONSE_NEW(name, maxpairs, maxsize) \
        response_t name; \
        name.returncode = HTTP_OK; \
        kv_pair_t name##kv_pairs[maxpairs]; \
        hash_table_data_t name##data[maxsize]; \
        hash_table_init(&(name.options), &name##kv_pairs[0], maxpairs, &name##data[0], maxsize); \
        hash_table_insert_string_with_tag(&(name.options), "Content-Type", "text/html", OPTIONS_TAG_HTTP); \
        hash_table_insert_string_with_tag(&(name.options), "Server", "cServer/0.0.1", OPTIONS_TAG_HTTP); \
        hash_table_insert_string_with_tag(&(name.options), "Date", "Thu, 23 Apr 2020 09:09:44 GMT", OPTIONS_TAG_HTTP); \

//extern response_t* response_create(void);
extern void response_set_body(response_t* res, char* data, uint32_t len, uint8_t copy);
// extern void response_free(response_t* res);
//extern void response_http_params(gpointer key, gpointer val, gpointer ssl);
extern void response_print_header(response_t* res, SSL *ssl);
extern void response_print_body(response_t* res, SSL *ssl);
extern void response_get_file_response(char* filename, response_t* response);
extern void response_set_cookie(response_t* res, char* key, char* value);

#endif