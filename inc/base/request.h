#ifndef REQUEST_HEADER
#define REQUEST_HEADER

#include "response.h"
#include "filetools.h"
#include "hash_table.h"

extern void Request_Handle(hash_table_t* request, response_t* response);

#endif