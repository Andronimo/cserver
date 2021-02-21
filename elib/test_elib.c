#include "hash_table.h"
#include "stdio.h"

void print_table(hash_table_data_t* key, hash_table_data_t* value, p_void_t user_data)
{
    printf("%s: %s\n", key, value);
}

void main() {
    HASH_TABLE_NEW(table, 20, 1000);

    hash_table_insert_string(&table, "KEY1", "VALUE2");
    hash_table_insert_string(&table, "KEY2", "VALUE6");
    hash_table_insert_string(&table, "KEY3", "VALUE4");
    hash_table_insert_string(&table, "KEY4", "VALUE8");
    hash_table_insert_data_with_tag(&table, "KEY11", "VALUE8", 7, 1);
    hash_table_insert_string(&table, "KEY5", "VALUE11");
    hash_table_insert_string_with_tag(&table, "KEY4", "VALUE9asdasd", 1);

    hash_table_iterate_with_tag(&table, 0u, print_table, NULL);
    uint32_t length;
    printf("Hallo %s %d\n", hash_table_get_data_with_tag(&table, "KEY4", &length, 1), length);
    printf("Hallo %d\n", length);
}