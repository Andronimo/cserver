#include "output.h"
#include "glib.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>

void _round(char *caption)
{
    static struct timeval oldtime;

    struct timeval time;
    gettimeofday(&time, NULL);
    int diff = (time.tv_usec - oldtime.tv_usec) / 1000;
    oldtime = time;

    printf("%s %d\n", caption, diff);
}

void output_header(int out, char *type, guint32 length, SSL *ssl)
{
    output_string(out, "HTTP/1.1 200 Ok\n", ssl);
    
    if (NULL == type)
    {
        output_string(out, "Content-Type: text/html", ssl);
    } 
    else if ((type != NULL) && (TRUE == g_str_equal(type, "js")))
    {
        output_string(out, "Content-Type: application/javascript", ssl);
    }
    else
    {
        output_string(out, "Content-Type: text/", ssl);
        output_string(out, type, ssl);
    }
    output_string(out, "\n", ssl);
    output_string(out, "Content-Length:", ssl);
    output_number(out, length, ssl);
    output_string(out, "\n", ssl);
    output_string(out, "Set-Cookie: id=413536417a7ad2dc7eb2846987688135\n", ssl);
    output_string(out, "Date: Thu, 23 Apr 2020 09:09:44 GMT\n", ssl);
    output_string(out, "Server: lighttpd/1.4.35\n\n", ssl);
}

void output_not_found(int out, SSL *ssl) {
    output_string(out, "HTTP/2 404 Not Found\n", ssl);
    output_string(out, "content-length: 11\n", ssl);
    output_string(out, "content-type: text/html; charset=iso-8859-1\n\n", ssl);
    output_string(out, "Not Found\n", ssl);
}

void output_number(int fd, guint32 number, SSL *ssl)
{
    char length[10];
    sprintf(length, "%d", number);
    output_string(fd, length, ssl);
}

void output_string(int fd, char *str, SSL *ssl)
{
    // printf("%s", str);
    if (NULL != ssl)
    {
        SSL_write(ssl, str, strlen(str));
    }
    else
    {
        send(fd, str, strlen(str), 0);
    }
}

int output_get_file_length(char *file)
{
    int sz = 0;

    FILE *fp = fopen(file + 1, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0L, SEEK_END);
        sz = ftell(fp);
        fclose(fp);
    }

    return sz;
}

void output_file(int out, sArray_t array, SSL *ssl)
{
    if (NULL != ssl)
    {
        SSL_write(ssl, array.data, array.len);
    }
    else
    {
        send(out, array.data, array.len, 0);
    }

    g_free(array.data);
}
