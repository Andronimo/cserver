#include "parse.h" 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h> 
#include <unistd.h>
#include <sys/socket.h>
#include "sys/time.h"

#define MAX_REQUEST 10000

static char* parse_param(char* key, hash_table_t* options);
static void parse_url(char* page, hash_table_t* options);
static void parse_request(char* req, hash_table_t* options);
static uint8_t parse_line(char* line, hash_table_t* options);
static void parse_form_data(char *body, char* boundary, hash_table_t* options);

static void socket_timeout_receive_set(int handle, uint32_t milliseconds)
{
    if (handle == 0) {
        return;
    }
    
    struct timeval tv = { milliseconds / 1000, (milliseconds % 1000) * 1000 };
    setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
}

unsigned char parse(int sock, hash_table_t* options, SSL *ssl, int * ssl_error) {
    char fullRequest[MAX_REQUEST];
    ssize_t maxRead = 0;
    ssize_t singleRead;

    socket_timeout_receive_set(sock, 1000);
    while (((singleRead = SSL_read(ssl, &fullRequest[maxRead], MAX_REQUEST-maxRead))) > 0 && (maxRead < MAX_REQUEST)) {
        socket_timeout_receive_set(sock, 100);
        maxRead += singleRead;
    }    

    if ((0 == maxRead) && (0 == singleRead)) {
        *ssl_error = 0; 
        return FALSE;
    }

    if ((0 == maxRead) && (singleRead == -1)) {
        return FALSE;
    } 

    LineReader lr;
    parse_request(LineReader_Init(&lr, &fullRequest[0]), options);    

    uint8_t header = TRUE;
    while ((TRUE == LineReader_HasNext(&lr)) && (TRUE == header)) {
        header = parse_line(LineReader_Get(&lr), options);       
    }

    if (TRUE == LineReader_HasNext(&lr)) {
        char* contentLength = hash_table_get_with_tag(options, "Content-Length", OPTIONS_TAG_HTTP);
        if (contentLength != NULL) {
            char* body = lr.nextLine;
            hash_table_insert_data_with_tag(options, "body", body, atoi(contentLength), OPTIONS_TAG_PAGE);

            char *contentType = hash_table_get_with_tag(options, "Content-Type", OPTIONS_TAG_HTTP);
            if ((contentType != NULL) && (NULL != strstr(contentType, "multipart/form-data"))) {       
                char* boundary = strstr(contentType, "boundary=");
                if (boundary != NULL) {
                    boundary+=9;
                }
                parse_form_data(body, boundary, options);
                //options_show_all(options);
            }
        } 
    }

    return TRUE;
}

static void parse_form_data(char *body, char* boundary, hash_table_t* options) {

    char* curPos = strstr(body+1, boundary+30);
    while (curPos != NULL) {
        curPos = strstr(curPos+1, "name=\"");
        if (curPos != NULL) {
            char* endKey;
            char* keyPos = curPos+=6;
            endKey = strchr(keyPos, '"');

            if (endKey != NULL) {
                curPos = strstr(endKey, "\r\n\r\n");

                if (curPos != NULL)
                {
                    char* valPos = curPos+4;
                    curPos = strstr(valPos, boundary);

                    if (curPos != NULL) {
                        curPos -= 4;
                        *endKey = '\0';

                        hash_table_insert_data_with_tag(options, keyPos, valPos, curPos - valPos, OPTIONS_TAG_POST);
                        //options_post_insert(options, keyPos, sArray_Data(valPos, curPos - valPos, TRUE));
                        curPos = strstr(curPos+1, boundary);
                    }             
                }
            }
        }
    } 
}

static char* parse_param(char* key, hash_table_t* options) {
    char* value = strchr(key, '=');
    char* next = NULL;

    if (value != NULL) {
        *(value++) = 0u;

        next = strpbrk(value, "&");

        if (next != NULL) {
            *(next++) = 0u;
        }

        hash_table_insert_string_with_tag(options, key, value, OPTIONS_TAG_PAGE);
    }
    return next;
}

static void parse_url(char* page, hash_table_t* options) {

    char* paramStart = strchr(page, '?');

    if (paramStart != NULL) {
        *(paramStart++) = 0u;

        while ((paramStart = parse_param(paramStart, options)) != NULL);
    }

    hash_table_insert_string_with_tag(options, "page", page, OPTIONS_TAG_HTTP);

    char* seek = page;

    while ((*seek != 0u) && (*seek != 0xd)) {
        seek++;
    }
    *seek = 0u;

    while ((seek != page) && (*seek != '.')) {
        seek--;
    }

    if (seek != page) {
        hash_table_insert_string_with_tag(options, "type", seek+1, OPTIONS_TAG_HTTP);
    }
}

static void parse_request(char* req, hash_table_t* options) {
    char* url = strchr(req, ' ');

    if (url != NULL) {
        *(url++) = 0u;

        char* http_type = strchr(url, ' ');
        if (http_type != NULL) {
            *(http_type++) = 0u;

            char* end = strchr(http_type, 0xdu);
            if (end != NULL) {
                *end = 0u;

                hash_table_insert_string_with_tag(options, "method", req, OPTIONS_TAG_HTTP);
                hash_table_insert_string_with_tag(options, "http_type", http_type, OPTIONS_TAG_HTTP);

                parse_url(url, options);
            }
        }
    }
}

static uint8_t parse_line(char* line, hash_table_t* options) {

    //printf("Parse Line(%ld): %s\n", strlen(line), line);

    if (line[1] == '\0') {
        return FALSE;
    }

    char* argument = strchr(line, ':');

    if (argument != NULL) {
        *argument = 0;
        while (*(++argument) == ' ');
        char* end = strchr(argument, 0xdu);
        if (end != NULL) {
            *end = 0u;
        }
        
        hash_table_insert_string_with_tag(options, line, argument, OPTIONS_TAG_HTTP);
    }

    return TRUE;
}