#ifndef STRING_H
#define STRING_H

uint32 string_length(char* str) {
    if (!str) return 0;
    uint32 count = 0;
    while (*(str++) != '\0') {
        count++;
    }
    return count;
}

struct String {
    char *contents;
    uint32 length;
};

struct String_Iterator {
    String string;
    uint32 index;
};

String make_string(char *contents, uint32 length) {
    String result = {};
    result.contents = contents;
    result.length = length;
    return result;
}

// NOTE: creates a string from a null-terminated char array
String make_string(char *contents) {
    String result = {};
    result.contents = contents;
    result.length = string_length(contents);
    return result;
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
        for (uint32 i = 0; i < a.length; i++) {
            if (a.contents[i] != b.contents[i]) {
                return false;
            }
        }
    }

    return true;
}

void append_string(String *result, String a, String b, uint32 max_size) {
    uint32 final_length = a.length + b.length;
    assert(final_length <= max_size);

    uint32 current_index = 0;
    for (uint32 i = 0; i < a.length; i++) {
        result->contents[current_index++] = a.contents[i];
    }
    for (uint32 i = 0; i < b.length; i++) {
        result->contents[current_index++] = b.contents[i];
    }

    result->length = final_length;
}

void copy_string(String *dest, String src, uint32 max_size) {
    /* uint32 final_length = a.length + b.length; */
    assert(src.length <= max_size);
    for (uint32 i = 0; i < src.length; i++) {
        dest->contents[i] = src.contents[i];
    }
    
    dest->length = src.length;
}

void to_char_array(String string, char *buffer, uint32 buffer_size) {
    assert(buffer_size >= string.length + 1);
    for (uint32 i = 0; i < string.length; i++) {
        buffer[i] = string.contents[i];
    }
    
    buffer[string.length] = '\0';
}

#endif
