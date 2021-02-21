#include "filetools.h"
#include "stdio.h"

char* LineReader_Init(LineReader *lr, char* beginning) {
    lr->currentLine = beginning;
    lr->nextLine = strchr(lr->currentLine, '\n'); 
    if (lr->nextLine) *lr->nextLine = '\0';  
    return lr->currentLine;         
}

char* LineReader_Get(LineReader *lr) {
    lr->currentLine = lr->nextLine ? (lr->nextLine+1) : NULL;
    lr->nextLine = strchr(lr->currentLine, '\n'); 
    if (lr->nextLine) *lr->nextLine = '\0';
    return lr->currentLine;
}

uint8_t LineReader_HasNext(LineReader *lr) {
    return lr->nextLine != NULL;
}

sArray_t* sArray_Number(uint32_t num) {
    char buf[20];
    sprintf(&buf[0], "%d", num);
    return sArray_String(&buf[0], TRUE);
}

sArray_t* sArray_String(char* text, uint8_t copy) {
    return sArray_Data(text, strlen(text), copy);
}

sArray_t* sArray_Data(char* data, int len, uint8_t copy) {
    sArray_t *ret = (sArray_t*) malloc(sizeof(sArray_t));
    ret->len = len;

    if (TRUE == copy) {
        ret->data = (char*) malloc(len);
        memcpy(ret->data, data, len);       
    }
    else
    {
        ret->data = data;
    }
    
    return ret;
}

void sArray_free(sArray_t* ar) {
    if (ar->data != NULL) {
        free(ar->data);
    }
    free(ar);
}