#ifndef HASH_TABLE_2_H
#define HASH_TABLE_2_H

#include "string.h"

#define HASH_TABLE_SIZE 64

#define FOR_ENTRY_POINTERS(key_type, value_type, hash_table)            \
    Hash_Table_Iterator<key_type, value_type> iterator = make_hash_table_iterator(hash_table); \
    for (Hash_Table_Entry<key_type, value_type> *entry = get_next_entry_pointer(&iterator); \
         entry != NULL;                                                 \
         entry = get_next_entry_pointer(&iterator))                     \

#define FOR_VALUE_POINTERS(key_type, value_type, hash_table)            \
    Hash_Table_Iterator<key_type, value_type> iterator = make_hash_table_iterator(hash_table); \
    for (value_type *value = get_next_value_pointer(&iterator); \
         value != NULL;                                                 \
         value = get_next_value_pointer(&iterator))                     \

bool32 int32_equals(int32 a, int32 b) {
    return a == b;
}

int32 copy(Allocator *allocator, int32 n) {
    return n;
}

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

template <class Key_Type, class Value_Type>
struct Hash_Table_Iterator {
    int32 index;
    int32 num_checked;
    Hash_Table<Key_Type, Value_Type> hash_table;
};

template <class Key_Type, class Value_Type>
Hash_Table_Iterator<Key_Type, Value_Type> make_hash_table_iterator(Hash_Table<Key_Type, Value_Type> hash_table) {
    Hash_Table_Iterator<Key_Type, Value_Type> iterator;
    iterator.hash_table = hash_table;
    iterator.index = 0;
    iterator.num_checked = 0;
    return iterator;
}

template <class Key_Type, class Value_Type>
bool32 is_finished(Hash_Table_Iterator<Key_Type, Value_Type> iterator) {
    Hash_Table<Key_Type, Value_Type> hash_table = iterator.hash_table;
    return ((iterator.index >= hash_table.max_entries) ||
            (iterator.num_checked >= hash_table.num_entries));
}

template <class Key_Type, class Value_Type>
Hash_Table_Entry<Key_Type, Value_Type> *get_next_entry_pointer(Hash_Table_Iterator<Key_Type, Value_Type> *iterator) {
    Hash_Table<Key_Type, Value_Type> hash_table = iterator->hash_table;
    if ((iterator->index >= hash_table.max_entries) || (iterator->num_checked >= hash_table.num_entries)) {
        return NULL;
    }

    while ((iterator->index < hash_table.max_entries) &&
           (iterator->num_checked < hash_table.num_entries)) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry_pointer = &hash_table.entries[iterator->index];
        if (entry_pointer->is_occupied) {
            iterator->num_checked++;
            iterator->index++;

            return entry_pointer;
        }

        iterator->index++;
    }

    return NULL;
}

template <class Key_Type, class Value_Type>
Value_Type *get_next_value_pointer(Hash_Table_Iterator<Key_Type, Value_Type> *iterator) {
    Hash_Table<Key_Type, Value_Type> hash_table = iterator->hash_table;
    if ((iterator->index >= hash_table.max_entries) || (iterator->num_checked >= hash_table.num_entries)) {
        return NULL;
    }

    while ((iterator->index < hash_table.max_entries) &&
           (iterator->num_checked < hash_table.num_entries)) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry_pointer = &hash_table.entries[iterator->index];
        if (entry_pointer->is_occupied) {
            iterator->num_checked++;
            iterator->index++;

            return &entry_pointer->value;
        }

        iterator->index++;
    }

    return NULL;
}

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

// NOTE: creates a new hash table with the entries allocated using the specified allocator.
//       this just does memcpy of the entries array into a new entries array. it does not copy values at
//       pointers.
template <class Key_Type, class Value_Type>
Hash_Table<Key_Type, Value_Type> copy_hash_table(Allocator *allocator,
                                                 Hash_Table<Key_Type, Value_Type> src_table) {
    Hash_Table<Key_Type, Value_Type> new_table = src_table;
    uint32 entry_size = sizeof(Hash_Table_Entry<Key_Type, Value_Type>);
    uint32 table_storage_size = entry_size * src_table.max_entries;
    new_table.entries = (Hash_Table_Entry<Key_Type, Value_Type> *) allocate(allocator, table_storage_size);
    new_table.allocator = allocator;
    memcpy((void *) new_table.entries, (void *) src_table.entries, table_storage_size);

    return new_table;
}

// TODO: this is actually wrong, since in the case where multiply entries map to the same hash, for example
//       if the array starting from the hash is A, B, C, where all of them have the same hash, then removing
//       B leaves a hole. then, if you were to try to add C, this procedure would put it where B was, making
//       A, C, C, which is incorrect. so the naive solution here is to just always check the entire array
//       before adding, but that can be slow. another solution is, when you remove a node, just search forward
//       for an element with the same hash and fill the hole with that. and then fill the new hole the same
//       way until you run out of elements with the same hash.
template <class Key_Type, class Value_Type>
void hash_table_add(Hash_Table<Key_Type, Value_Type> *hash_table, Key_Type key, Value_Type value) {
    assert(hash_table->num_entries < hash_table->max_entries);

    uint32 hash = get_hash(key, hash_table->max_entries);
    
    int32 num_checked = 0;
    while (num_checked < hash_table->max_entries) {
        Hash_Table_Entry<Key_Type, Value_Type> entry = hash_table->entries[hash];
        if (entry.is_occupied) {
            if (hash_table->key_equals(entry.key, key)) {
                assert(!"Entry already exists with key");
            }

            hash++;
            hash %= hash_table->max_entries;
            num_checked++;
        } else {
            Hash_Table_Entry<Key_Type, Value_Type> new_entry;
            new_entry.is_occupied = true;
            new_entry.key = copy(hash_table->allocator, key);
            new_entry.value = value;
            
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

// NOTE: we call this reset since it doesn't go through all the entries and calls deallocate(entry.value).
//       this procedure assumes the caller is handling the deallocating of any memory that needs to be
//       deallocated. this is useful for when values are using something like an arena for allocations and
//       individual entries in the arena cannot be deallocated separately (arenas are always cleared all
//       at once).
template <class Key_Type, class Value_Type>
void hash_table_reset(Hash_Table<Key_Type, Value_Type> *hash_table) {
    int32 num_checked = 0;
    for (int32 i = 0; i < hash_table->max_entries && num_checked < hash_table->num_entries; i++) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry = &hash_table->entries[i];
        if (entry->is_occupied) {
            // TODO: i'm not sure if we actually need to clear it.. we could just set is_occupied = false like we
            //       do in the procedure below
            *entry = {};
            num_checked++;
        }
    }

    hash_table->num_entries = 0;
    hash_table->total_added_ever = 0;
}

template <class Key_Type, class Value_Type>
void reset_and_deallocate_entries(Hash_Table<Key_Type, Value_Type> *hash_table) {
    int32 num_checked = 0;
    for (int32 i = 0; i < hash_table->max_entries && num_checked < hash_table->num_entries; i++) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry = &hash_table->entries[i];
        if (entry->is_occupied) {
            deallocate(entry->value);
            entry->is_occupied = false;
            num_checked++;
        }
    }

    hash_table->num_entries = 0;
    hash_table->total_added_ever = 0;
}

template <class Key_Type, class Value_Type>
void hash_table_reset_entry(Hash_Table<Key_Type, Value_Type> *hash_table, Key_Type key) {
    int32 num_checked = 0;
    int32 found = false;
    for (int32 i = 0; i < hash_table->max_entries && num_checked < hash_table->num_entries; i++) {
        Hash_Table_Entry<Key_Type, Value_Type> *entry = &hash_table->entries[i];
        if (entry->is_occupied && hash_table.key_equals(key, entry->key)) {
            entry->is_occupied = false;
            found = true;
            break;
        }
    }

    if (found) {
        // NOTE: we could do this in the loop, since we break out early, but i don't want to accidentally
        // eventually add something that messes up the loop.
        hash_table->num_entries--;
        return;
    }

    assert(!"Entry to reset not found.");
}

#endif
