#ifndef STRING_H
#define STRING_H

#include "memory.h"

// NOTE: in both String and String_Buffer, contents is NOT null-terminated.
//       this is useful for doing string operations on arbitrary data, such as when parsing text files.
//       for example, when parsing mesh files, we often create strings that point to the start of a
//       token and set length to the length of the token string. using this method, we can do string
//       operations such as comparing to other strings without either modifying the original data buffer
//       to have a null-terminator or allocating memory and doing a copy and adding a null-terminator
//       there.

int32 string_length(char* str) {
    if (!str) return 0;
    int32 count = 0;
    while (*(str++) != '\0') {
        count++;
    }
    return count;
}

struct String {
    // allocator should be NULL when contents are in read-only memory
    Allocator *allocator = NULL;
    char *contents;
    int32 length;
};

struct String_Buffer {
    char *contents;
    int32 current_length;
    int32 size;
    Allocator *allocator;
};

struct String_Iterator {
    String string;
    int32 index;
};

inline bool32 is_empty(String string) {
    return (string.length == 0);
}

inline bool32 is_empty(String_Buffer string) {
    return (string.current_length == 0);
}

// NOTE: max_size does NOT include null-terminator
// NOTE: copies a null-terminated src string into a buffer, dest, of size max_size
//       for example, if dest is 5 bytes, max_size = 5, and src is a null-terminated char array hello\0,
//       this is fine. i.e. destination does NOT need to include space for the null-terminator.
void copy_string(char *dest, char *src, int32 max_size) {
    assert(string_length(src) <= max_size);
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
}

void copy_string(String_Buffer *dest, String_Buffer *src) {
    assert(dest->size >= src->size);

    for (int32 i = 0; i < src->current_length; i++) {
        dest->contents[i] = src->contents[i];
    }

    dest->current_length = src->current_length;
}

void copy_string(String_Buffer *dest, String src) {
    assert(dest->size >= src.length);

    memcpy(dest->contents, src.contents, src.length);
    dest->current_length = src.length;
}

String copy(Allocator *allocator, String src) {
    String result;
    result.allocator = allocator;
    result.length = src.length;
    result.contents = (char *) allocate(allocator, src.length);
    memcpy(result.contents, src.contents, src.length);
    return result;
}

/*
// TODO: this may be outdated
void copy_string(String *dest, String src, uint32 max_size) {
    // uint32 final_length = a.length + b.length;
    assert(src.length <= max_size);
    for (uint32 i = 0; i < src.length; i++) {
        dest->contents[i] = src.contents[i];
    }
    
    dest->length = src.length;
}
*/

String_Buffer make_empty_string_buffer(char *buffer, uint32 size_of_buffer) {
    String_Buffer result;
    result.contents = buffer;
    result.current_length = 0;
    result.size = size_of_buffer;
    // deallocate shouldn't be called on these string buffers, since it's assumed that buffer is handling
    // the allocation and deallocation of the memory
    result.allocator = NULL;
    return result;
}

String_Buffer make_string_buffer(Allocator *allocator, uint32 size) {
    String_Buffer buffer;
    char *contents = (char *) allocate(allocator, size);
    buffer.contents = contents;
    buffer.current_length = 0;
    buffer.size = size;
    buffer.allocator = allocator;
    return buffer;
}

// NOTE: size does NOT include a null-terminator, since String does not include a null-terminator in contents.
// NOTE: creates a String_Buffer with initial contents (this does a copy)
// NOTE: string is the null-terminated initial contents of the buffer
String_Buffer make_string_buffer(Allocator *allocator, char *string, uint32 size) {
    String_Buffer buffer;

    char *contents = (char *) allocate(allocator, size);
    uint32 length = string_length(string);
    assert(length <= size);
    copy_string(contents, string, size);

    buffer.contents = contents;
    buffer.current_length = length;
    buffer.size = size;
    buffer.allocator = allocator;

    return buffer;
}

// TODO: maybe rename this to make_string_buffer_copy, so it's more clear that the contents of initial value are
//       being copied using the allocator.
String_Buffer make_string_buffer(Allocator *allocator, String initial_value, int32 size) {
    assert(initial_value.length <= size);
    String_Buffer buffer;

    char *contents = (char *) allocate(allocator, size);
    buffer.contents = contents;
    buffer.size = size;
    buffer.allocator = allocator;

    copy_string(&buffer, initial_value);

    return buffer;
}

String_Buffer copy(Allocator *allocator, String_Buffer source_buffer) {
    String_Buffer buffer = {};

    char *contents = (char *) allocate(allocator, source_buffer.size);
    memcpy(contents, source_buffer.contents, source_buffer.current_length);

    buffer.allocator = allocator;
    buffer.contents = contents;
    buffer.size =  source_buffer.size;
    buffer.current_length = source_buffer.current_length;

    return buffer;
}

void set_string_buffer_text(String_Buffer *string_buffer, char *text) {
    int32 len = string_length(text);
    assert(len < string_buffer->size);

    memcpy(string_buffer->contents, text, len);
    string_buffer->current_length = len;
}

void deallocate(String_Buffer string_buffer) {
    deallocate(string_buffer.allocator, string_buffer.contents);
}

String make_string(char *contents, uint32 length) {
    String result = {};
    result.allocator = read_only_allocator;
    result.contents = contents;
    result.length = length;
    return result;
}

String make_string(String_Buffer string_buffer) {
    String result = {};
    result.contents = string_buffer.contents;
    result.length = string_buffer.current_length;
    result.allocator = string_buffer.allocator;
    return result;
}

// NOTE: creates a string from a null-terminated char array
String make_string(char *contents) {
    String result = {};
    result.allocator = read_only_allocator;
    result.contents = contents;
    result.length = string_length(contents);
    return result;
}

// this copies
String make_string(Allocator *allocator, char *contents) {
    String result = {};
    result.allocator = allocator;

    int32 len = string_length(contents);
    result.length = len;

    char *buf = (char *) allocate(allocator, len);
    memcpy(buf, contents, len);
    result.contents = buf;

    return result;
}

void deallocate(String string) {
    deallocate(string.allocator, string.contents);
}

void replace_with_copy(Allocator *allocator, String *old_string, String new_string) {
    deallocate(*old_string);
    *old_string = copy(allocator, new_string);
}

String_Iterator make_string_iterator(String string) {
    String_Iterator it = {};
    it.string = string;
    it.index = 0;
    return it;
}

char get_next_char(String_Iterator *it) {
    if (it->index < it->string.length) {
        char result = it->string.contents[it->index];
        it->index++;
        return result;
    } else {
        return '\0';
    }
}

char peek_next_char(String_Iterator *it) {
    if (it->index < it->string.length) {
        char result = it->string.contents[it->index];
        return result;
    } else {
        return '\0';
    }
}

void increment_index(String_Iterator *it) {
    it->index++;
}

bool32 string_equals(String a, String b) {
    if (a.length != b.length) {
        return false;
    } else {
        for (int32 i = 0; i < a.length; i++) {
            if (a.contents[i] != b.contents[i]) {
                return false;
            }
        }
    }

    return true;
}

// NOTE: b is a null-terminated string
inline bool32 string_equals(String a, char *b) {
    return string_equals(a, make_string(b));
}

bool32 string_contains(String string, char chars[], int32 num_chars) {
    for (int32 i = 0; i < string.length; i++) {
        for (int32 j = 0; j < num_chars; j++) {
            if (string.contents[i] == chars[j]) return true;
        }
    }

    return false;
}

void append_string(String *result, String a, String b, uint32 max_size) {
    uint32 final_length = a.length + b.length;
    assert(final_length <= max_size);

    uint32 current_index = 0;
    for (int32 i = 0; i < a.length; i++) {
        result->contents[current_index++] = a.contents[i];
    }
    for (int32 i = 0; i < b.length; i++) {
        result->contents[current_index++] = b.contents[i];
    }

    result->length = final_length;
}

void append_string(String_Buffer *buffer, String to_append) {
    assert(buffer->current_length + to_append.length <= buffer->size);

    memcpy(&buffer->contents[buffer->current_length], to_append.contents, to_append.length);
    buffer->current_length += to_append.length;
}

inline void append_string(String_Buffer *buffer, char *to_append_c_str) {
    String to_append = make_string(to_append_c_str);
    append_string(buffer, to_append);
}

inline void append_string(String_Buffer *buffer, String_Buffer to_append_buffer) {
    String to_append = make_string(to_append_buffer);
    append_string(buffer, to_append);
}

void to_char_array(String string, char *buffer, int32 buffer_size) {
    assert(buffer_size >= string.length + 1);
    for (int32 i = 0; i < string.length; i++) {
        buffer[i] = string.contents[i];
    }
    
    buffer[string.length] = '\0';
}

char *to_char_array(Allocator *allocator, String string) {
    char *buf = (char *) allocate(allocator, string.length + 1);

    memcpy(buf, string.contents, string.length);
    buf[string.length] = '\0';

    return buf;
}

char *to_char_array(Allocator *allocator, String_Buffer string) {
    char *buf = (char *) allocate(allocator, string.current_length + 1);

    for (int32 i = 0; i < string.current_length; i++) {
        buf[i] = string.contents[i];
    }
    
    buf[string.current_length] = '\0';
    return buf;
}

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

#endif
