#ifndef HASH_TABLE_2_H
#define HASH_TABLE_2_H

#include "string.h"

#define HASH_TABLE_SIZE 64

int32 get_hash(int32 id, uint32 bucket_size) {
    return id % bucket_size;
}

template <class Key_Type, class Value_Type>
struct Hash_Table_Entry {
    bool32 is_occupied;
    Key_Type key;
    Value_Type value;
};

template <class Key_Type, class Value_Type>
struct Hash_Table {
    Hash_Table_Entry<Key_Type, Value_Type> *entries;
    int32 total_added_ever;
    int32 num_entries;
    int32 max_entries;
    Allocator *allocator;
    bool32 (*key_equals) (Key_Type, Key_Type);
};

// create a UI_Element_Variant struct which is a union of all derived types
template <class Key_Type, class Value_Type>
Hash_Table<Key_Type, Value_Type> make_hash_table(Allocator *allocator,
                                                 int32 max_entries,
                                                 bool32 (*key_equals) (Key_Type, Key_Type)) {
    Hash_Table<Key_Type, Value_Type> hash_table;
    uint32 entry_size = sizeof(Hash_Table_Entry<Key_Type, Value_Type>);
    hash_table.entries = ((Hash_Table_Entry<Key_Type, Value_Type> *)
                          allocate(allocator, entry_size * max_entries));
    hash_table.total_added_ever = 0;
    hash_table.num_entries = 0;
    hash_table.max_entries = max_entries;
    hash_table.allocator = allocator;
    hash_table.key_equals = key_equals;

    return hash_table;
}

template <class Key_Type, class Value_Type>
void hash_table_add(Hash_Table<Key_Type, Value_Type> *hash_table, Key_Type key, Value_Type value) {
    assert(hash_table->num_entries < hash_table->max_entries);

    uint32 hash = get_hash(key, hash_table->max_entries);
    
    int32 num_checked = 0;
    while (num_checked < hash_table->max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> entry = hash_table->entries[hash];
        if (entry.is_occupied) {
            hash++;
            hash %= hash_table->max_entries;
            num_checked++;
        } else {
            Hash_Table_Entry<Key_Type, Value_Type> new_entry = { true, key, value };
            hash_table->entries[hash] = new_entry;
            hash_table->num_entries++;
            hash_table->total_added_ever++;
            return;
        }
    }

    assert(!"Should be unreachable");
}

template <class Key_Type, class Value_Type>
void hash_table_remove(Hash_Table<Key_Type, Value_Type> *hash_table, Key_Type key) {
    uint32 hash = get_hash(key, hash_table->max_entries);

    int32 num_checked = 0;
    while (num_checked < hash_table->max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry = &hash_table->entries[hash];
        if (entry->is_occupied && hash_table->key_equals(key, entry->key)) {
            entry->is_occupied = false;
            deallocate(entry->value);
            hash_table->num_entries--;
            return;
        }

        hash++;
        hash %= hash_table->max_entries;
        num_checked++;
    }

    assert(!"Entry to be removed does not exist.");
}

template <class Key_Type, class Value_Type>
bool32 hash_table_remove_if_exists(Hash_Table<Key_Type, Value_Type> *hash_table, Key_Type key) {
    uint32 hash = get_hash(key, hash_table->max_entries);

    int32 num_checked = 0;
    while (num_checked < hash_table->max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry = &hash_table->entries[hash];
        if (entry->is_occupied && hash_table->key_equals(key, entry->key)) {
            entry->is_occupied = false;
            deallocate(entry->value);
            return true;
        }

        hash++;
        hash %= hash_table->max_entries;
        num_checked++;
    }

    return false;
}

template <class Key_Type, class Value_Type>
bool32 hash_table_exists(Hash_Table<Key_Type, Value_Type> hash_table, Key_Type key) {
    uint32 hash = get_hash(key, hash_table.max_entries);

    int32 num_checked = 0;
    while (num_checked < hash_table.max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> entry = hash_table.entries[hash];

        if (entry.is_occupied && hash_table.key_equals(key, entry.key)) {
            return true;
        }

        hash++;
        hash %= hash_table.max_entries;
        num_checked++;
    }

    return false;
}

// NOTE: hash_table_get() will assert if the key does not exist in the table.
//       hash_table_find() will not assert if the key does not exist, but instead returns a boolean of
//       whether or not the key was found in the table.
template <class Key_Type, class Value_Type>
Value_Type hash_table_get(Hash_Table<Key_Type, Value_Type> hash_table, Key_Type key) {
    uint32 hash = get_hash(key, hash_table.max_entries);

    int32 num_checked = 0;
    while (num_checked < hash_table.max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> entry = hash_table.entries[hash];
        if (entry.is_occupied && hash_table.key_equals(key, entry.key)) {
            return entry.value;
        }

        hash++;
        hash %= hash_table.max_entries;
        num_checked++;
    }

    assert("!Key does not exist in table.");
    return {};
}

template <class Key_Type, class Value_Type>
bool32 hash_table_find(Hash_Table<Key_Type, Value_Type> hash_table, Key_Type key, Value_Type *value_result) {
    uint32 hash = get_hash(key, hash_table.max_entries);

    int32 num_checked = 0;
    while (num_checked < hash_table.max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> entry = hash_table.entries[hash];
        if (entry.is_occupied && hash_table.key_equals(key, entry.key)) {
            *value_result = entry.value;
            return true;
        }

        hash++;
        hash %= hash_table.max_entries;
        num_checked++;
    }

    return false;
}

template <class Key_Type, class Value_Type>
bool32 hash_table_find_pointer(Hash_Table<Key_Type, Value_Type> hash_table, Key_Type key, Value_Type **value_result) {
    uint32 hash = get_hash(key, hash_table.max_entries);

    int32 num_checked = 0;
    while (num_checked < hash_table.max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry = &hash_table.entries[hash];
        if (entry->is_occupied && hash_table.key_equals(key, entry->key)) {
            *value_result = &entry->value;
            return true;
        }

        hash++;
        hash %= hash_table.max_entries;
        num_checked++;
    }

    return false;
}

#endif
