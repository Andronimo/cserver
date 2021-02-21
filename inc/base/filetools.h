#ifndef FILETOOLS_HEADER
#define FILETOOLS_HEADER

#include "glib.h"
#include "stdint.h"

#define OPTIONS_TAG_HTTP 1u
#define OPTIONS_TAG_PAGE 2u
#define OPTIONS_TAG_POST 3u

typedef struct sArray {
    char* data;
    guint32 len;
} sArray_t;

#define REGISTER_HTML_FULL(url, name) \
if (strcmp(file, url) == 0) { \
    GString* page = g_string_new_len("", 0); \
    Output_##name(page); \
    response_set_body(response, page->str, page->len, FALSE); \
    g_string_free(page, FALSE); \
    return; \
}

#define REGISTER_HTML(name) REGISTER_HTML_FULL("/"#name".html", name)



typedef struct LineReader_t {
    char* currentLine;
    char* nextLine;
} LineReader;

extern char* LineReader_Init(LineReader *lr, char* beginning);
extern char* LineReader_Get(LineReader *lr);
extern uint8_t LineReader_HasNext(LineReader *lr);

extern sArray_t* sArray_Number(uint32_t num);
extern sArray_t* sArray_Data(char* data, int len, uint8_t copy);
extern sArray_t* sArray_String(char* text, uint8_t copy);
extern void sArray_free(sArray_t* ar);

#endif