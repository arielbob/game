#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "string.h"

#define HASH_TABLE_BUCKETS 64

uint32 get_hash(String name, uint32 bucket_size) {
    String_Iterator it = make_string_iterator(name);
    uint32 sum = 0;
    char c = get_next_char(&it);
    while (c) {
        sum += c;
        c = get_next_char(&it);
    }

    uint32 hash = sum % bucket_size;
    
    return hash;
}

template <class Value_Type>
struct Hash_Table_Bucket_Node {
    String key;
    Value_Type value;
    Hash_Table_Bucket_Node<Value_Type> *next;
};

template <class Value_Type>
struct Hash_Table_Linked_List {
    Hash_Table_Bucket_Node<Value_Type> *start;
};

template <class Value_Type>
struct Hash_Table {
    Hash_Table_Linked_List<Value_Type> buckets[HASH_TABLE_BUCKETS];
    Allocator *allocator;
};

// TODO: we may want to be able to specify how many buckets we want
template <class Value_Type>
Hash_Table<Value_Type> make_hash_table(Allocator *allocator) {
    Hash_Table<Value_Type> hash_table = {};
    hash_table.allocator = allocator;
    return hash_table;
}

template <class Value_Type>
bool32 hash_table_find(Hash_Table<Value_Type> hash_table, String key, Value_Type *value_result) {
    uint32 hash = get_hash(key, HASH_TABLE_BUCKETS);
    Hash_Table_Linked_List<Value_Type> bucket = hash_table.buckets[hash];
    Hash_Table_Bucket_Node<Value_Type> *current = bucket.start;

    while (current) {
        if (string_equals(current->key, key)) {
            *value_result = current->value;
            return true;
        }
        current = current->next;
    }

    return false;
}

template <class Value_Type>
internal void hash_table_add(Hash_Table<Value_Type> *hash_table, String key, Value_Type value) {
    uint32 hash = get_hash(key, HASH_TABLE_BUCKETS);
    Hash_Table_Linked_List<Value_Type> *bucket = &hash_table->buckets[hash];

    if (bucket->start) {
        Hash_Table_Bucket_Node<Value_Type> *current = bucket->start;
        
        while (current) {
            if (string_equals(current->key, key)) {
                assert(!"Entry with that key already exists");
            }

            if (!current->next) {
                Hash_Table_Bucket_Node<Value_Type> *node =
                    (Hash_Table_Bucket_Node<Value_Type> *) allocate(hash_table->allocator,
                                                                    sizeof(Hash_Table_Bucket_Node<Value_Type>));
                node->key = key;
                node->value = value;
                node->next = NULL;
                current->next = node;

                return;
            } else {
                current = current->next;
            }
        }

        // should be unreachable
        assert(false);
    } else {
        Hash_Table_Bucket_Node<Value_Type> *node =
            (Hash_Table_Bucket_Node<Value_Type> *) allocate(hash_table->allocator,
                                                            sizeof(Hash_Table_Bucket_Node<Value_Type>));
        node->key = key;
        node->value = value;
        node->next = NULL;

        bucket->start = node;
    }
}

#endif
