/***********************************************************************************************************************
 * INCLUDES
 **********************************************************************************************************************/

#include "sqlite3.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SQLiteH.h"

/***********************************************************************************************************************
 * DEFINES
 **********************************************************************************************************************/

#define LOG_ERROR(logLevel, msg, ...) printf((msg), ##__VA_ARGS__);
#define LOG_PRIO(logLevel, msg, ...) printf((msg), ##__VA_ARGS__);
#define LOG(logLevel, msg, ...) printf((msg), ##__VA_ARGS__);
#define SQLITEH_DATABASE "test.db"

/***********************************************************************************************************************
 * TYPEDEFS
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * PROTOTYPES
 ***********************************************************************************************************************/

static SQLiteH_ReturnType sqliteH_getTextValueForTextKey(const char *table, const char *column, const char *primKey, const char *keyVal, char **outVal);
static int callback(void *data, int argc, char **argv, char **azColName);

/***********************************************************************************************************************
 * VARIABLES
 **********************************************************************************************************************/
uint8_t initialized = FALSE;
/***********************************************************************************************************************
 * PRIVATE METHODS
 ***********************************************************************************************************************/
static int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        LOG_PRIO(1, "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

static int sqlite3_execN(
    sqlite3 *db,                                    /* An open database */
    const char *sql,                                /* SQL to be evaluated */
    int (*callback)(void *, int, char **, char **), /* Callback function */
    void *arg,                                      /* 1st argument to callback */
    char **errmsg,                                  /* Error msg written here */
    uint8_t N                                       /* Number of retries */
)

{
    uint8_t retry = N;
    uint8_t ret = 0;

    while (((ret = sqlite3_exec(db, sql, callback, arg, errmsg)) != 0) && (retry > 0))
    {
        retry--;

        if (0u == retry)
        {
            LOG_ERROR(1, "Could not execute SQLite command with %d retries", N);
            LOG_ERROR(1, "QUERY: %s", sql)
        }
        else
        {
            sqlite3_free(*errmsg);
            *errmsg = NULL;
            g_usleep(1000);
        }
    }
    //LOG_PRIO(1, "RETVAL: %d", ret)
    return ret;
}

static int sqlite3_prepare_v2N(
    sqlite3 *db,           /* Database handle */
    const char *zSql,      /* SQL statement, UTF-8 encoded */
    int nByte,             /* Maximum length of zSql in bytes. */
    sqlite3_stmt **ppStmt, /* OUT: Statement handle */
    const char **pzTail,   /* OUT: Pointer to unused portion of zSql */
    uint8_t N              /* Number of retries */
)

{
    uint8_t retry = N;
    uint8_t ret = 0;

    while (((ret = sqlite3_prepare_v2(db, zSql, nByte, ppStmt, pzTail)) != 0) && (retry > 0))
    {
        retry--;

        if (0u == retry)
        {
            LOG_ERROR(1, "Could not prepare SQLite command with %d retries", N);
            LOG_ERROR(1, "QUERY: %s", zSql)
        }
        else
        {
            g_usleep(1000);
        }
    }
    //LOG_PRIO(1, "RETVAL: %d", ret)

    return ret;
}

/***********************************************************************************************************************
 * PUBLIC METHODS
 ***********************************************************************************************************************/
uint8_t sqliteH_init(void)
{
    if (initialized == TRUE)
    {
        return TRUE;
    }
    //Check if Database is available
    if (access(SQLITEH_DATABASE, F_OK) == -1)
    {
        LOG_ERROR(1, "Database-File %s doesn't excists", SQLITEH_DATABASE)
        return FALSE;
    }
    //Check if Database is opanable
    sqlite3 *uaccDatabase;
    int errCode;
    errCode = sqlite3_open(SQLITEH_DATABASE, &uaccDatabase);
    if (errCode)
    {
        LOG_ERROR(1, "Failed to open Database. ErrCode: %d", errCode)
        return FALSE;
    }
    sqlite3_close(uaccDatabase);
    initialized = TRUE;
    return TRUE;
}

uint8_t sqliteH_printAllUseraccounts(void)
{
    if (initialized == FALSE)
    {
        if (sqliteH_init() == FALSE)
        {
            LOG_ERROR(1, "Failed to insert UserAccount")
            return FALSE;
        }
    }
    sqlite3 *db;
    int err;
    char *zErrMsg = 0;
    err = sqlite3_open(SQLITEH_DATABASE, &db);
    if (err)
    {
        LOG_ERROR(1, "Failed to open Database. ErrCode: %d", err)
        return FALSE;
    }
    char *execString = "SELECT * FROM userAccounts";
    err = sqlite3_execN(db, execString, callback, NULL, &zErrMsg, 10u);
    sqlite3_close(db);
    if (err != SQLITE_OK)
    {
        LOG_ERROR(1, "Failed to read Database. ErrCode: %d", err)
        sqlite3_free(zErrMsg);
        return FALSE;
    }
    return TRUE;
}

cJSON *sqliteH_readCustomJsonFromDb(char *customQuery)
{

    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open(SQLITEH_DATABASE, &db);

    if (rc != SQLITE_OK)
    {

        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return cJSON_CreateArray();
    }

    rc = sqlite3_prepare_v2N(db, customQuery, -1, &res, 0, 10u);

    if (rc == SQLITE_OK)
    {

        //sqlite3_bind_int(res, 1, 3);
    }
    else
    {

        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    cJSON *root = cJSON_CreateArray();

    int step;
    while ((step = sqlite3_step(res)) == SQLITE_ROW)
    {
        cJSON *row = cJSON_CreateObject();
        for (int i = 0; i < sqlite3_column_count(res); i++)
        {
            if (sqlite3_column_type(res, i) == SQLITE_INTEGER)
            {
                cJSON_AddNumberToObject(row, sqlite3_column_name(res, i), sqlite3_column_int(res, i));
            }
            else if (sqlite3_column_type(res, i) == SQLITE_TEXT)
            {
                cJSON_AddStringToObject(row, sqlite3_column_name(res, i), (const char *)sqlite3_column_text(res, i));
            }
            else if (sqlite3_column_type(res, i) == SQLITE_NULL)
            {
                cJSON_AddStringToObject(row, sqlite3_column_name(res, i), "");
            }
        }
        cJSON_AddItemToArray(root, row);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return root;
}

uint8_t sqliteH_customQuery(char *queryString)
{
    if (initialized == FALSE)
    {
        if (sqliteH_init() == FALSE)
        {
            LOG_ERROR(1, "Failed to insert UserAccount")
            return FALSE;
        }
    }
    sqlite3 *db;
    int err;
    char *zErrMsg = 0;

    err = sqlite3_open(SQLITEH_DATABASE, &db);
    if (err)
    {
        LOG_ERROR(1, "Failed to open Database. ErrCode: %d", err)
        return FALSE;
    }
    err = sqlite3_execN(db, queryString, NULL, 0, &zErrMsg, 10u);
    sqlite3_close(db);
    if (err != SQLITE_OK)
    {
        LOG_ERROR(1, "SQL error: %s", zErrMsg)
        sqlite3_free(zErrMsg);
        return FALSE;
    }
    return TRUE;
}

uint8_t sqliteH_dropTableData(char *tableName)
{
    if (initialized == FALSE)
    {
        if (sqliteH_init() == FALSE)
        {
            LOG_ERROR(1, "Failed to insert UserAccount")
            return FALSE;
        }
    }
    char *execString = g_strdup_printf("DELETE FROM %s", tableName);
    sqlite3 *db;
    int err;
    char *zErrMsg = 0;

    err = sqlite3_open(SQLITEH_DATABASE, &db);
    if (err)
    {
        LOG_ERROR(1, "Failed to open Database. ErrCode: %d", err)
        return FALSE;
    }
    err = sqlite3_execN(db, execString, NULL, 0, &zErrMsg, 10u);
    free(execString);
    sqlite3_close(db);
    if (err != SQLITE_OK)
    {
        LOG_ERROR(1, "SQL error: %s", zErrMsg)
        sqlite3_free(zErrMsg);
        return FALSE;
    }
    return TRUE;
}

uint8_t sqliteH_deleteAppAccountByUserAccountId(char *userAccId)
{
    if (initialized == FALSE)
    {
        if (sqliteH_init() == FALSE)
        {
            LOG_ERROR(1, "Failed to insert UserAccount")
            return FALSE;
        }
    }
    char *execString = g_strdup_printf("DELETE FROM appAccounts WHERE userAccId='%s'", userAccId);
    sqlite3 *db;
    int err;
    char *zErrMsg = 0;

    err = sqlite3_open(SQLITEH_DATABASE, &db);
    if (err)
    {
        LOG_ERROR(1, "Failed to open Database. ErrCode: %d", err)
        return FALSE;
    }
    err = sqlite3_execN(db, execString, NULL, 0, &zErrMsg, 10u);
    sqlite3_close(db);
    free(execString);
    if (err != SQLITE_OK)
    {
        LOG_ERROR(1, "SQL error: %s", zErrMsg)
        sqlite3_free(zErrMsg);
        return FALSE;
    }
    return TRUE;
}

uint8_t sqliteH_deleteTableRowById(char *id, int id_int)
{
    char *tableName = "";
    char *columnName = "";
    //sqliteH_getTableAndColumnname(&columnName, &tableName, table);
    LOG_PRIO(5, "columnName: %s", columnName)
    LOG_PRIO(5, "tableName: %s", tableName)

    if (initialized == FALSE)
    {
        if (sqliteH_init() == FALSE)
        {
            LOG_ERROR(1, "Failed to insert UserAccount")
            return FALSE;
        }
    }
    char *execString = NULL;
    if (id != NULL)
    {
        execString = g_strdup_printf("DELETE FROM %s WHERE %s='%s'", tableName, columnName, id);
    }
    else if (id_int >= 0)
    {
        execString = g_strdup_printf("DELETE FROM %s WHERE %s=%d", tableName, columnName, id_int);
    }
    sqlite3 *db;
    int err;
    char *zErrMsg = 0;

    err = sqlite3_open(SQLITEH_DATABASE, &db);
    if (err)
    {
        LOG_ERROR(1, "Failed to open Database. ErrCode: %d", err)
        return FALSE;
    }
    err = sqlite3_execN(db, execString, NULL, 0, &zErrMsg, 10u);
    sqlite3_close(db);
    free(execString);
    if (err != SQLITE_OK)
    {
        LOG_ERROR(1, "SQL error: %s", zErrMsg)
        sqlite3_free(zErrMsg);
        return FALSE;
    }
    return TRUE;
}

void sqliteH_updateDatabaseText(const char *id, const char *entry, const char *value)
{
    char *tableName, *columnName;
    //sqliteH_getTableAndColumnname(&columnName, &tableName, table);

    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open("/data/ipdoor/UserAccounts.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char sql[200];
    //sprintf(&sql[0], "UPDATE userAccounts SET %s=? WHERE idUniqueId=?", entry);
    sprintf(&sql[0], "UPDATE %s SET %s=? WHERE %s=?", tableName, entry, columnName);
    rc = sqlite3_prepare_v2N(db, sql, -1, &res, 0, 10u);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(res, 1, value, -1, 0);
        sqlite3_bind_text(res, 2, id, -1, 0);
    }
    else
    {
        LOG_ERROR(1, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
    sqlite3_close(db);
}

void sqliteH_updateDatabaseInt(const char *id, const char *entry, const int value)
{
    char *tableName, *columnName;
    //sqliteH_getTableAndColumnname(&columnName, &tableName, table);
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open("/data/ipdoor/UserAccounts.db", &db);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Cannot open database: %s\n", sqlite3_errmsg(db))
        sqlite3_close(db);
        return;
    }
    char sql[200];
    sprintf(&sql[0], "UPDATE %s SET %s=? WHERE %s=?", tableName, entry, columnName);
    rc = sqlite3_prepare_v2N(db, sql, -1, &res, 0, 10u);
    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(res, 2, id, -1, 0);
        sqlite3_bind_int(res, 1, (int)value);
    }
    else
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
    sqlite3_close(db);
}

void sqliteH_updateJson(const char *table, cJSON *json, const char *primKey)
{
    sqlite3 *db;
    GString *sql;
    char *error;
    cJSON *tmp;
    if (table == NULL || json == NULL || primKey == NULL)
    {
        LOG_ERROR(1, "sqliteH_updateJason: Invalid parameters")
        return;
    }
    int rc = sqlite3_open("/data/ipdoor/UserAccounts.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        LOG_ERROR(1, "sqliteH_updateJason: Failed to open Database")
        return;
    }
    sql = g_string_new("UPDATE");
    g_string_append_printf(sql, " %s SET", table);
    tmp = json->child;
    while (tmp)
    {
        if (0u != g_strcmp0(primKey, tmp->string))
        {
            if (cJSON_IsNumber(tmp))
            {
                g_string_append_printf(sql, " %s=%d,", tmp->string, tmp->valueint);
            }
            else if (cJSON_IsBool(tmp))
            {
                g_string_append_printf(sql, " %s=%d,", tmp->string, tmp->valueint);
            }
            else if (cJSON_IsString(tmp))
            {
                g_string_append_printf(sql, " %s=\"%s\",", tmp->string, tmp->valuestring);
            }
        }
        tmp = tmp->next;
    }
    g_string_truncate(sql, sql->len - 1);
    tmp = cJSON_GetObjectItem(json, primKey);
    if (!cJSON_IsString(tmp))
    {
        LOG_ERROR(1, "sqliteH_updateJason: primaryKey isn't string")
        return;
    }
    if (cJSON_IsNumber(tmp))
    {
        g_string_append_printf(sql, " WHERE %s=%d", primKey, tmp->valueint);
    }
    else if (cJSON_IsString(tmp))
    {
        g_string_append_printf(sql, " WHERE %s=\"%s\"", primKey, tmp->valuestring);
    }
    if (sqlite3_execN(db, sql->str, NULL, NULL, &error, 10u) != 0)
    {
        LOG_ERROR(1, "Error: %s\n", error);
        sqlite3_free(error);
    }
    sqlite3_close(db);
}

uint8_t sqliteH_getColumValByKeyText(char *column, char *uuid, char **retVal)
{
    char *tableName = "";
    char *columnName = "";
    int _retVal;
    //sqliteH_getTableAndColumnname(&columnName, &tableName, table);
    _retVal = sqliteH_getTextValueForTextKey(tableName, column, columnName, uuid, retVal);
    if (*retVal == NULL)
    {
        **retVal = 0;
    }
    else if (strcmp(*retVal, "(null)") == 0)
    {
        **retVal = 0;
    }

    return _retVal;
}

uint8_t sqliteH_getColumValByKeyInt(char *column, char *uuid, int *retVal)
{
    //char *tableName, *columnName;
    //sqliteH_getTableAndColumnname(&columnName, &tableName, table);

    return TRUE;
}

static SQLiteH_ReturnType sqliteH_getTextValueForTextKey(const char *table, const char *column, const char *primKey, const char *keyVal, char **outVal)
{
    if ((NULL == table) || (NULL == column) || (NULL == primKey) || (NULL == keyVal))
    {
        if (table == NULL)
        {
            LOG_ERROR(1, "table NULL")
        }
        if (column == NULL)
        {
            LOG_ERROR(1, "column NULL")
        }
        if (primKey == NULL)
        {
            LOG_ERROR(1, "primKey NULL")
        }
        if (keyVal == NULL)
        {
            LOG_ERROR(1, "keyVal NULL")
        }
        LOG_ERROR(1, "Invalid parameters")
        return SQLITEH_NOK;
    }
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open(SQLITEH_DATABASE, &db);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Cannot open database: %s\n", sqlite3_errmsg(db))
        sqlite3_close(db);
        return SQLITEH_NOK;
    }
    char sql[1000];
    sprintf(&sql[0], "SELECT %s FROM %s WHERE %s=\"%s\"", column, table, primKey, keyVal);
    LOG_ERROR(5, "SQL_QUERY: %s", sql)
    rc = sqlite3_prepare_v2N(db, sql, -1, &res, 0, 10u);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Failed to execute statement: %s\n", sqlite3_errmsg(db))
        return SQLITEH_NOK;
    }
    if (sqlite3_step(res) == SQLITE_ROW)
    {
        if (sqlite3_column_count(res) != 1)
        {
            LOG_ERROR(1, "Number of results not one")
            return SQLITEH_NOK;
        }
        if (outVal != NULL)
        {
            *outVal = g_strdup_printf("%s", sqlite3_column_text(res, 0));
        }
    }
    else
    {
        return SQLITEH_NOK;
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return SQLITEH_OK;
}

SQLiteH_ReturnType sqliteH_getIntValueForTextKey(const char *table, const char *column, const char *primKey, const char *keyVal, int *outVal)
{
    if ((NULL == table) || (NULL == column) || (NULL == primKey) || (NULL == keyVal))
    {
        if (table == NULL)
        {
            LOG_ERROR(1, "table NULL")
        }
        if (column == NULL)
        {
            LOG_ERROR(1, "column NULL")
        }
        if (primKey == NULL)
        {
            LOG_ERROR(1, "primKey NULL")
        }
        if (keyVal == NULL)
        {
            LOG_ERROR(1, "keyVal NULL")
        }
        if (outVal == NULL)
        {
            LOG_ERROR(1, "outVal NULL")
        }
        LOG_ERROR(1, "Invalid parameters")
        return SQLITEH_NOK;
    }
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open(SQLITEH_DATABASE, &db);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Cannot open database: %s\n", sqlite3_errmsg(db))
        sqlite3_close(db);
        return SQLITEH_NOK;
    }
    char sql[1000];
    sprintf(&sql[0], "SELECT %s FROM %s WHERE %s=\"%s\"", column, table, primKey, keyVal);
    rc = sqlite3_prepare_v2N(db, sql, -1, &res, 0, 10u);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Failed to execute statement: %s\n", sqlite3_errmsg(db))
        return SQLITEH_NOK;
    }
    if (sqlite3_step(res) == SQLITE_ROW)
    {
        if (sqlite3_column_count(res) != 1)
        {
            LOG_ERROR(1, "Number of results not one")
            return SQLITEH_NOK;
        }
        if (outVal != NULL)
        {
            *outVal = sqlite3_column_int(res, 0);
        }
    }
    else
    {
        return SQLITEH_NOK;
    }
    sqlite3_finalize(res);
    sqlite3_close(db);
    return SQLITEH_OK;
}

SQLiteH_ReturnType sqliteH_getNumberDatabaseEntrys(const char *table, int *retVal)
{
    if (table == NULL)
    {
        LOG_ERROR(1, "table NULL")
        return SQLITEH_NOK;
    }
    sqlite3 *db;
    sqlite3_stmt *res;
    char *queryString = NULL;
    int rc = sqlite3_open(SQLITEH_DATABASE, &db);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Cannot open database: %s\n", sqlite3_errmsg(db))
        sqlite3_close(db);
        return SQLITEH_NOK;
    }
    queryString = g_strdup_printf("SELECT COUNT (*) FROM %s;", table);
    rc = sqlite3_prepare_v2N(db, queryString, -1, &res, 0, 10);

    if (rc == SQLITE_OK)
    {
    }
    else
    {

        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    free(queryString);

    int step;
    if (((step = sqlite3_step(res)) == SQLITE_ROW))
    {
        *retVal = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        sqlite3_close(db);
        return SQLITEH_OK;
    }
    else
    {
        sqlite3_finalize(res);
        sqlite3_close(db);
        return SQLITEH_NOK;
    }
}

SQLiteH_ReturnType sqliteH_getNumberEntrysCustomQuery(const char *cQuery, int *retVal)
{
    if (cQuery == NULL)
    {
        LOG_ERROR(1, "cQuery NULL")
        return SQLITEH_NOK;
    }
    sqlite3 *db;
    sqlite3_stmt *res;
    int rc = sqlite3_open(SQLITEH_DATABASE, &db);
    if (rc != SQLITE_OK)
    {
        LOG_ERROR(1, "Cannot open database: %s\n", sqlite3_errmsg(db))
        sqlite3_close(db);
        return SQLITEH_NOK;
    }
    rc = sqlite3_prepare_v2N(db, cQuery, -1, &res, 0, 10);

    if (rc == SQLITE_OK)
    {

        //sqlite3_bind_int(res, 1, 3);
    }
    else
    {

        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int step;
    if (((step = sqlite3_step(res)) == SQLITE_ROW))
    {
        *retVal = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        sqlite3_close(db);
        return SQLITEH_OK;
    }
    else
    {
        sqlite3_finalize(res);
        sqlite3_close(db);
        return SQLITEH_NOK;
    }
}

SQLiteH_ReturnType sqliteH_insert(cJSON* element)
{
    sqlite3 *db;
    sqlite3_stmt * stmt;
    int err;
    cJSON* it;
    uint8_t count = 1;
    SQLiteH_ReturnType ret = SQLITEH_NOK;

    if (element == NULL) {
        return SQLITEH_BAD_JSON;
    }

    GString *execString = g_string_new("INSERT INTO Persons (");
    GString *values = g_string_new("");

    for (it = element->child; it != NULL; it = it->next)
    {
        if (it->string != NULL) {
            g_string_append_printf(execString, "%s,", it->string); \
            g_string_append_printf(values, "?,");
        } else {
            ret = SQLITEH_BAD_JSON;
            goto end;
        }   
    }

    g_string_truncate(execString, strlen(execString->str) - 1);
    g_string_truncate(values, strlen(values->str) - 1);
    g_string_append_printf(execString, ") VALUES (%s);", values->str);

    err = sqlite3_open(SQLITEH_DATABASE, &db);
    if (err)
    {
        ret = SQLITEH_DATABASE_NOT_FOUND;
        goto end;
    }

    err = sqlite3_prepare_v2(db, execString->str, execString->len, &stmt, NULL);
    if (err)
    {
        ret = SQLITEH_BAD_JSON;
        goto end;
    }

    for (it = element->child; it != NULL; it = it->next)
    {
       switch (it->type) 
       {
           case cJSON_String:
           {
               sqlite3_bind_text(stmt, count, it->valuestring, strlen(it->string), 0);
               break;
           }
           case cJSON_Number:
           {
               sqlite3_bind_int(stmt, count, it->valueint);
               break;
           }
           default:
           {
               ret = SQLITEH_BAD_JSON;
               goto end;
           }
       }
       count++;
    }
    
    err = sqlite3_step (stmt);
    
    sqlite3_close(db);
    
    if (SQLITE_DONE == err) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        ret = SQLITEH_OK;
    } 

end:
    g_string_free(values, TRUE);
    g_string_free(execString, TRUE);
    sqlite3_finalize(stmt);

    return ret;
}

// int main (int argc, char ** argv)
// {
//     sqlite3 * db;
//     char * sql;
//     sqlite3_stmt * stmt;
//     int nrecs;
//     char * errmsg;
//     int i;
//     int row = 0;
//     int arg;

//     CALL_SQLITE (open ("/home/ben/test.db", & db));
//     sql = "SELECT * FROM t WHERE xyz = ?";
//     CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, & stmt, NULL));

//     for (arg = 1; arg < argc; arg++) {
//         CALL_SQLITE (bind_text (stmt,
//                                 1, /* The number of the argument. */
//                                 argv[arg],
//                                 strlen (argv[arg]),
//                                 0 /* The callback. */
//                                 ));
//         while (1) {
//             int s;

//             s = sqlite3_step (stmt);
//             if (s == SQLITE_ROW) {
//                 int bytes;
//                 const unsigned char * text;
//                 bytes = sqlite3_column_bytes (stmt, 0);
//                 text  = sqlite3_column_text (stmt, 0);
//                 printf ("%d: %s\n", row, text);
//                 row++;
//             }
//             else if (s == SQLITE_DONE) {
//                 break;
//             }
//             else {
//                 fprintf (stderr, "Failed.\n");
//                 exit (1);
//             }
//         }
//         CALL_SQLITE (reset (stmt));
//         CALL_SQLITE (clear_bindings (stmt));
//     }
//     return 0;
// }

cJSON *sqliteH_readJsonFromDb(char* db_file, const char *sql)
{

    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(db_file, &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return cJSON_CreateArray();
    }

    rc = sqlite3_prepare_v2N(db, sql, -1, &res, 0, 10u);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    cJSON *root = cJSON_CreateArray();

    int step;
    while ((step = sqlite3_step(res)) == SQLITE_ROW)
    {
        cJSON *row = cJSON_CreateObject();
        for (int i = 0; i < sqlite3_column_count(res); i++)
        {
            if (sqlite3_column_type(res, i) == SQLITE_INTEGER)
            {
                cJSON_AddNumberToObject(row, sqlite3_column_name(res, i), sqlite3_column_int(res, i));
            }
            else if (sqlite3_column_type(res, i) == SQLITE_TEXT)
            {
                cJSON_AddStringToObject(row, sqlite3_column_name(res, i), (const char *)sqlite3_column_text(res, i));
            }
            else if (sqlite3_column_type(res, i) == SQLITE_NULL)
            {
                cJSON_AddStringToObject(row, sqlite3_column_name(res, i), "");
            }
        }
        cJSON_AddItemToArray(root, row);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return root;
}