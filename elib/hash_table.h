/**
 * @file      [file name].c
 * @authors   [author]
 * @copyright [copy write holder]
 *
 * @brief [description]
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/*******************************************************************************
* Includes
*******************************************************************************/

#include "types.h"

/*******************************************************************************
* Defines
*******************************************************************************/

/*******************************************************************************
* Types and Typedefs
*******************************************************************************/

typedef char hash_table_data_t;

typedef struct kv_pair
{
    hash_table_data_t* key;
    hash_table_data_t* value;
    uint8_t tag;
    uint32_t length;
} kv_pair_t;

typedef struct hash_table {
    uint32_t max_size;
    uint32_t size;
    uint32_t max_pairs;
    uint32_t pairs;
    kv_pair_t* kv_pairs;
    hash_table_data_t* data;
} hash_table_t;

/*******************************************************************************
* Global Variables
*******************************************************************************/

/*******************************************************************************
* Functions
*******************************************************************************/

#define HASH_TABLE_NEW(name, maxpairs, maxsize) \
        kv_pair_t name##kv_pairs[maxpairs]; \
        hash_table_data_t name##data[maxsize]; \
        hash_table_t name; \
        hash_table_init(&name, &name##kv_pairs[0], maxpairs, &name##data[0], maxsize);

extern void hash_table_init(hash_table_t * table, kv_pair_t* kv_pairs, uint8_t max_pairs, hash_table_data_t* data, uint32_t cap);
extern void hash_table_insert_string(hash_table_t * table, hash_table_data_t* key, hash_table_data_t* value);
extern void hash_table_insert_string_with_tag(hash_table_t * table, hash_table_data_t* key, hash_table_data_t* value, uint8_t tag);
extern void hash_table_insert_data_with_tag(hash_table_t * table, hash_table_data_t* key, hash_table_data_t* value, uint32_t length, uint8_t tag);
extern hash_table_data_t* hash_table_get(hash_table_t * table, hash_table_data_t* key);
extern hash_table_data_t* hash_table_get_with_tag(hash_table_t * table, hash_table_data_t* key, uint8_t tag);
extern hash_table_data_t* hash_table_get_data_with_tag(hash_table_t * table, hash_table_data_t* key, uint32_t* length, uint8_t tag);
extern void hash_table_iterate(hash_table_t * table, void (*it_func)(hash_table_data_t*, hash_table_data_t*, p_void_t), p_void_t user_data);
extern void hash_table_iterate_with_tag(hash_table_t * table, uint8_t tag, void (*it_func)(hash_table_data_t*, hash_table_data_t*, p_void_t), p_void_t user_data);
#endif