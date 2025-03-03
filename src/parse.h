#ifndef PARSE_H
#define PARSE_H

#include "platform.h"

struct Tokenizer {
    char *current;
    char *contents; // start pointer of the contents we're tokenizing
    int32 index;
    int32 size;
    int32 line; // 1-indexed
};

Tokenizer make_tokenizer(File_Data file_data) {
    Tokenizer tokenizer = {};
    tokenizer.current = (char *) file_data.contents;
    tokenizer.contents = (char *) file_data.contents;
    tokenizer.index = 0;
    tokenizer.size = file_data.size;
    tokenizer.line = 1;
    return tokenizer;
}

inline bool32 is_digit(char c) {
    return (c >= 48 && c <= 57);
}

inline uint32 ascii_to_uint32(char c) {
    assert(c >= 48 && c <= 57);
    return c - 48;
}

// TODO: test this
int32 string_to_int32(char *str, uint32 length) {
    int32 result = 0;
    uint32 place_value = 1;

    int32 i = (int32) length - 1;
    for (; i > 0; i--) {
        result += place_value * ascii_to_uint32(str[i]);
        place_value *= 10;
    }

    if (length > 0) {
        // guaranteed i = 0 here
        if (str[i] == '-') {
            result *= -1;
        } else {
            result += place_value * ascii_to_uint32(str[i]);
            place_value *= 10;
        }
    }

    return result;
}

bool32 string_to_int32(char *str, uint32 length, int32 *result_out) {
    int32 result = 0;
    uint32 place_value = 1;

    int32 i = (int32) length - 1;
    for (; i > 0; i--) {
        char c = str[i];
        if (!is_digit(c)) {
            return false;
        }
        result += place_value * ascii_to_uint32(c);
        place_value *= 10;
    }

    if (length > 0) {
        // guaranteed i = 0 here
        char c = str[i];
        if (c == '-') {
            result *= -1;
        } else {
            if (!is_digit(c)) {
                return false;
            }
            result += place_value * ascii_to_uint32(c);
            place_value *= 10;
        }
    }

    *result_out = result;
    return true;
}

inline int32 string_to_int32(String string, int32 *result) {
    return string_to_int32(string.contents, string.length, result);
}

int32 string_to_uint32(char *str, uint32 length) {
    uint32 result = 0;
    uint32 place_value = 1;

    for (int32 i = (int32) length - 1; i >= 0; i--) {
        result += place_value * ascii_to_uint32(str[i]);
        place_value *= 10;
    }

    return result;
}

inline uint32 string_to_uint32(String string) {
    return string_to_uint32(string.contents, string.length);
}

bool32 string_to_uint32(char *str, uint32 length, uint32 *result_out) {
    uint32 result = 0;
    uint32 place_value = 1;

    for (int32 i = (int32) length - 1; i >= 0; i--) {
        char c = str[i];
        if (!is_digit(c)) {
            return false;
        }
        result += place_value * ascii_to_uint32(str[i]);
        place_value *= 10;
    }

    *result_out = result;
    return true;
}

inline bool32 string_to_uint32(String string, uint32 *result) {
    return string_to_uint32(string.contents, string.length, result);
}

real32 string_to_real32(char *str, uint32 length) {
    real32 result = 0;
    bool32 has_decimal = false;
    real32 decimal_denom = 10.0f;
    bool32 is_negative = false;

    uint32 i = 0;
    if (length > 0) {
        if (str[0] == '-') {
            is_negative = true;
            i++;
        }
    }
    
    for (; i < length; i++) {
        char c = str[i];

        if (c == '.') {
            assert(has_decimal == false);
            has_decimal = true;
        } else {
            if (has_decimal) {
                result += (real32) ascii_to_uint32(str[i]) / decimal_denom;
                decimal_denom *= 10;
            } else {
                result = result*10 + ascii_to_uint32(str[i]);
            }            
        }
    }

    if (is_negative) {
        result = -result;
    }

    return result;
}

inline real32 string_to_real32(String string) {
    return string_to_real32(string.contents, string.length);
}

bool32 string_to_real32(char *str, uint32 length, real32 *result_out) {
    real32 result = 0;
    bool32 has_decimal = false;
    real32 decimal_denom = 10.0f;
    bool32 is_negative = false;

    uint32 i = 0;
    if (length > 0) {
        if (str[0] == '-') {
            is_negative = true;
            i++;
        }
    }
    
    for (; i < length; i++) {
        char c = str[i];

        if (c == '.') {
            if (has_decimal) return false;
            has_decimal = true;
        } else if (is_digit(c)) {
            if (has_decimal) {
                result += (real32) ascii_to_uint32(str[i]) / decimal_denom;
                decimal_denom *= 10;
            } else {
                result = result*10 + ascii_to_uint32(str[i]);
            }            
        } else {
            return false;
        }
    }

    if (is_negative) {
        result = -result;
    }

    *result_out = result;

    return true;
}

inline bool32 string_to_real32(String string, real32 *result_out) {
    return string_to_real32(string.contents, string.length, result_out);
}

// NOTE: we have these tokenizer_equals procedures since the tokenizer contents does not guarantee a
//       null-terminator, since they can come from file reading, and files don't necessarily have null-terminators.
//       these procedures just add bounds checking so that we don't read past the char array bounds.x
inline bool32 tokenizer_equals(Tokenizer *tokenizer, char c) {
    if (c == '\0' && (tokenizer->index >= tokenizer->size)) {
        return true;
    }

    return (*tokenizer->current == c);
}

inline bool32 tokenizer_in_range(Tokenizer *tokenizer, char c_min, char c_max) {
    assert(c_max >= c_min);

    if (tokenizer->index >= tokenizer->size) return false;

    char c = *tokenizer->current;
    return ((c >= c_min) && (c <= c_max));
}

inline bool32 tokenizer_equals(Tokenizer *tokenizer, char *string) {
    int32 len = string_length(string);

    // ensure that we have at least len characters left, so we don't read past array bounds
    if ((tokenizer->size - tokenizer->index) < len) {
        return false;
    }
    
    String a = make_string(tokenizer->current, len);
    String b = make_string(string, len);

    return string_equals(a, b);
}

inline bool32 is_digit(Tokenizer *tokenizer) {
    return tokenizer_in_range(tokenizer, '0', '9');
}

inline bool32 is_line_end(char *c) {
    if (*c == '\r' && *(c + 1) == '\n') {
        return true;
    } else if (*c == '\n') {
        return true;
    } else {
        return false;
    }
}

inline bool32 is_line_end(Tokenizer *tokenizer) {
    return (tokenizer_equals(tokenizer, "\r\n") || tokenizer_equals(tokenizer, '\n'));
}

inline bool32 is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

inline bool32 is_whitespace(Tokenizer *tokenizer) {
    if (tokenizer->index >= tokenizer->size) return false;
    return is_whitespace(*tokenizer->current);
}

inline bool32 is_letter(char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z'));
}

inline bool32 is_letter(Tokenizer *tokenizer) {
    if (tokenizer->index >= tokenizer->size) return false;
    return is_letter(*tokenizer->current);
}

inline void increment_tokenizer(Tokenizer *tokenizer, int32 amount = 1) {
    if (amount < 0) {
        amount = abs(amount);
        for (int i = 0; i < amount && tokenizer->index >= 0; i++) {
            if (tokenizer->index < tokenizer->size) {
                if (*tokenizer->current == '\n') {
                    tokenizer->line--;
                }
            }
            tokenizer->current--;
            tokenizer->index--;
        }
    } else {
        for (int i = 0; i < amount && tokenizer->index < tokenizer->size; i++) {
            if (*tokenizer->current == '\n') {
                tokenizer->line++;
            }
            tokenizer->current++;
            tokenizer->index++;
        }
    }
}

internal bool32 is_end(Tokenizer *tokenizer) {
    return (tokenizer->index >= tokenizer->size);
}

internal void consume_leading_whitespace(Tokenizer *tokenizer) {
    while(!is_end(tokenizer) &&
          is_whitespace(*tokenizer->current)) {
        increment_tokenizer(tokenizer);
    }
}

#endif
