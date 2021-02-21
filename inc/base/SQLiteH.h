#ifndef SQLITEH_H
#define SQLITEH_H

#include <stdint.h>
#include <glib.h>
#include "cJSON.h"

typedef enum e_SQLITEH_ReturnType
{
    SQLITEH_OK = 0,
    SQLITEH_NOK,
    SQLITEH_BAD_JSON,
    SQLITEH_DATABASE_NOT_FOUND
} SQLiteH_ReturnType;

extern SQLiteH_ReturnType sqliteH_insert(cJSON* element);
extern cJSON *sqliteH_readJsonFromDb(char* db_file, const char *sql);

#endif