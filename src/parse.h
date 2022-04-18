#ifndef PARSE_H
#define PARSE_H

struct Tokenizer {
    char *current;
    uint32 index;
    uint32 size;
};

inline uint32 ascii_to_uint32(char c) {
    assert(c >= 48 && c <= 57);
    return c - 48;
}

inline uint32 string_to_uint32(char *str, uint32 length) {
    uint32 result = 0;
    uint32 place_value = 1;

    for (int32 i = (int32) length - 1; i >= 0; i--) {
        result += place_value * ascii_to_uint32(str[i]);
        place_value *= 10;
    }

    return result;
}

inline real32 string_to_real32(char *str, uint32 length) {
    real32 result = 0;
    bool32 has_decimal = false;
    real32 decimal_denom = 10.0f;
    bool32 is_negative = false;
    
    for (uint32 i = 0; i < length; i++) {
        char c = str[i];

        if (c == '.') {
            assert(has_decimal == false);
            has_decimal = true;
        } else if (c == '-') {
            assert(is_negative == false);
            is_negative = true;
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

inline bool32 is_digit(char c) {
    return (c >= 48 && c <= 57);
}

inline bool32 is_line_end(char *c) {
    // we don't actually need to check for null-terminator as long as we're checking the charcaters in order,
    // i.e. if we're looking for \r\n, then the condition should be (*c == 'r') && (*(c+1) == '\n').
    // this is because it'll short circuit if the first one is the null-terminator, so we don't have to
    // worry about reading out of bounds.
    if (*c == '\r' && *(c + 1) == '\n') {
        return true;
    } else if (*c == '\n') {
        return true;
    } else {
        return false;
    }
}

inline bool32 is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

inline bool32 is_letter(char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z'));
}

inline void increment_tokenizer(Tokenizer *tokenizer, int32 amount = 1) {
    tokenizer->current += amount;
    tokenizer->index += amount;
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
