#include "request.h"
#include "stdio.h"
#include "chtml_files.h"

int str2int(const char* str, int len)
{
    int i;
    int ret = 0;
    for(i = 0; i < len; ++i)
    {
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}

void Request_Handle(hash_table_t* request, response_t* response) {
    char *file = hash_table_get_with_tag(request, "page", OPTIONS_TAG_HTTP);
    char *type = hash_table_get_with_tag(request, "method", OPTIONS_TAG_HTTP);
    printf("%s %s %d/%d %d/%d\n", type, file, request->pairs, request->max_pairs, request->size, request->max_size);
    //options_show_all(options);

    // REGISTER_HTML(gwplan)
    // REGISTER_HTML_FULL("/",gwplan)
    REGISTER_HTML(ctest)
    // REGISTER_HTML(date)

    if (strcmp(file, "/hello") == 0) { 
        sArray_t data;
        data.data = hash_table_get_data_with_tag(request, "a", &data.len, OPTIONS_TAG_POST);
        
        if (data.data != NULL) {
            uint32_t num = str2int(data.data, data.len);
            char buf[10];
            sprintf(buf, "%d", num*37);
            response_set_cookie(response, "id", buf);
            response_set_body(response, buf, strlen(buf), TRUE);
        } else {
            response_set_body(response, "Error", 5, TRUE);
        } 
        return;
    }

    GString* page = g_string_new(file);
    g_string_insert_c(page,0,'.');
    
    response_get_file_response(page->str, response);
}