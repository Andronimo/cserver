/**
 * @file      [file name].c
 * @authors   [author]
 * @copyright [copy write holder]
 *
 * @brief [description]
 */
/*******************************************************************************
* Includes
*******************************************************************************/

#include "hash_table.h"
#include "string.h"
#include "stdio.h"

/*******************************************************************************
* Defines
*******************************************************************************/

/*******************************************************************************
* Local Types and Typedefs
*******************************************************************************/

/*******************************************************************************
* Global Variables
*******************************************************************************/

/*******************************************************************************
* Static Function Prototypes
*******************************************************************************/

/*******************************************************************************
* Static Variables
*******************************************************************************/

/*******************************************************************************
* Functions
*******************************************************************************/

void hash_table_init(hash_table_t * table, kv_pair_t* kv_pairs, uint8_t max_pairs, hash_table_data_t* data, uint32_t cap)
{
    table->size = 0;
    table->pairs = 0;
    table->kv_pairs = kv_pairs;
    table->max_pairs = max_pairs;
    table->data = data;
    table->max_size = cap;
}

static inline uint8_t* hash_table_insert_single(hash_table_t * table, hash_table_data_t* data, uint32_t length)
{
    if (table->size + length <= table->max_size) {
        hash_table_data_t* pos = table->data + table->size;
        memcpy(pos, data, length);
        table->size += length;
        return pos;
    }
   
    return NULL;
}

void hash_table_insert_string(hash_table_t * table, hash_table_data_t* key, hash_table_data_t* value)
{
    hash_table_insert_string_with_tag(table, key, value, 0u);
}

void hash_table_insert_string_with_tag(hash_table_t * table, hash_table_data_t* key, hash_table_data_t* value, uint8_t tag)
{
    hash_table_insert_data_with_tag(table, key, value, strlen(value)+1, tag);
}

void hash_table_insert_data_with_tag(hash_table_t * table, hash_table_data_t* key, hash_table_data_t* value, uint32_t length, uint8_t tag)
{
    if (table->pairs < table->max_pairs) {
        uint32_t size;
        table->kv_pairs[table->pairs].key = hash_table_insert_single(table, key, strlen(key)+1);
        size = table->size;
        table->kv_pairs[table->pairs].value = hash_table_insert_single(table, value, length);
        table->kv_pairs[table->pairs].tag = tag;
        table->kv_pairs[table->pairs].length = table->size - size;
        table->pairs++;
    }
}

hash_table_data_t* hash_table_get_data_with_tag(hash_table_t * table, hash_table_data_t* key, uint32_t* length, uint8_t tag)
{
    uint32_t piece = 0u;

    while (piece < table->pairs)
    {
        if ((tag == table->kv_pairs[piece].tag) && (0u == strcmp(key, table->kv_pairs[piece].key)))
        {
            *length = table->kv_pairs[piece].length;
            return table->kv_pairs[piece].value;
        }

        piece++;
    }

    return NULL;
}

hash_table_data_t* hash_table_get_with_tag(hash_table_t * table, hash_table_data_t* key, uint8_t tag)
{
    uint32_t piece = 0u;

    while (piece < table->pairs)
    {
        if ((tag == table->kv_pairs[piece].tag) && (0u == strcmp(key, table->kv_pairs[piece].key)))
        {
            return table->kv_pairs[piece].value;
        }

        piece++;
    }

    return NULL;
}

hash_table_data_t* hash_table_get(hash_table_t * table, hash_table_data_t* key)
{
    uint32_t piece = 0u;

    while (piece < table->pairs)
    {
        if (0u == strcmp(key, table->kv_pairs[piece].key))
        {
            return table->kv_pairs[piece].value;
        }

        piece++;
    }

    return NULL;
}

void hash_table_iterate(hash_table_t * table, void (*it_func)(hash_table_data_t*, hash_table_data_t*, p_void_t), p_void_t user_data)
{
    uint32_t piece = 0u;

    while (piece < table->pairs)
    {
        it_func(table->kv_pairs[piece].key, table->kv_pairs[piece].value, user_data);
        piece++;
    }
}

void hash_table_iterate_with_tag(hash_table_t * table, uint8_t tag, void (*it_func)(hash_table_data_t*, hash_table_data_t*, p_void_t), p_void_t user_data)
{
    uint32_t piece = 0u;

    while (piece < table->pairs)
    {
        if (tag == table->kv_pairs[piece].tag) 
        {
            it_func(table->kv_pairs[piece].key, table->kv_pairs[piece].value, user_data);
        }
       
        piece++;
    }
}
