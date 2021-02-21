#include "response.h"
#include "output.h"

// response_t* response_create()
// {
//     response_t* res = (response_t*) malloc(sizeof(response_t));

//     res->returncode = HTTP_OK;
//     res->options = options_create();
//     res->body = NULL;

//     options_http_insert(res->options, "Content-Type", "text/html");
//     options_http_insert(res->options, "Server", "cServer/0.0.1");
//     options_http_insert(res->options, "Date", "Thu, 23 Apr 2020 09:09:44 GMT");

//     return res;
// }

static void response_http_params(hash_table_data_t* key, hash_table_data_t* val, p_void_t ssl);

void response_set_returncode(response_t* res, uint32_t code) {
    res->returncode = code;
}

void response_set_body(response_t* res, char* data, uint32_t len, uint8_t copy)
{
    hash_table_insert_data_with_tag(&res->options, "body", data, len, OPTIONS_TAG_PAGE);
}

// void response_free(response_t* res)
// {
//     options_destroy(res->options);
//     if (res->body != NULL) {
//         sArray_free(res->body);
//     }
//     free(res);
// }

static void response_http_params(hash_table_data_t* key, hash_table_data_t* val, p_void_t ssl)
{
    output_string(0, key, ssl);
    output_string(0, ": ", ssl);
    output_string(0, val, ssl);
    output_string(0, "\n", ssl);
}

void response_print_header(response_t* res, SSL *ssl)
{
    uint32_t length;
    (void)hash_table_get_data_with_tag(&res->options, "body", &length, OPTIONS_TAG_PAGE);

    if (HTTP_OK == res->returncode) {
        output_string(0, "HTTP/1.1 200 Ok\n", ssl);
    } else {
        output_string(0, "HTTP/2 404 Not Found\n", ssl);
    }
    hash_table_iterate_with_tag(&res->options, OPTIONS_TAG_HTTP, response_http_params, ssl);
    output_string(0, "Content-Length: ", ssl);
    output_number(0, length, ssl);
    output_string(0, "\n\n", ssl);
}

void response_print_body(response_t* res, SSL *ssl)
{
    uint32_t length;
    char* body = hash_table_get_data_with_tag(&res->options, "body", &length, OPTIONS_TAG_PAGE);
    SSL_write(ssl, body, length);
}

void response_set_cookie(response_t* res, char* key, char* value)
{
    char buf[100];
    sprintf(buf, "%s=%s", key, value);
    hash_table_insert_string_with_tag(&res->options, "Set-Cookie", &buf[0], OPTIONS_TAG_HTTP);
}

void response_get_file_response(char* filename, response_t* response) {
    gsize length;
    GError *errorRed = NULL;
    gchar* contents;

    g_file_get_contents(filename, &contents, &length, &errorRed);
    if (NULL != errorRed)
    {
        printf(" <Not Found>\n");
        response_set_returncode(response, 404);
        response_set_body(response, "Page not found", 14, TRUE);
        return;
    }

    response_set_body(response, contents, length, FALSE);
}