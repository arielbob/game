#include "memory.h"
#include "hash_table.h"
#include "game.h"

#define FONT_NAME_MAX_SIZE 128

real32 get_width(Font font, char *text) {
    real32 width = 0;

    while (*text) {
        int32 advance, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font.font_info, *text, &advance, &left_side_bearing);
        width += (advance) * font.scale_for_pixel_height;
        
        if (*(text + 1)) {
            width += font.scale_for_pixel_height * stbtt_GetCodepointKernAdvance(&font.font_info,
                                                                                  *text, *(text + 1));
        }

        text++;
    }
    
    return width;
}

// get width up to index (non-inclusive, i.e. the width of the character at index is not added to the sum)
real32 get_width(Font font, char *text, int32 index) {
    real32 width = 0;

    int32 i = 0;
    while (*text && i < index) {
        int32 advance, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font.font_info, *text, &advance, &left_side_bearing);
        width += (advance) * font.scale_for_pixel_height;
        
        if (*(text + 1)) {
            width += font.scale_for_pixel_height * stbtt_GetCodepointKernAdvance(&font.font_info,
                                                                                 *text, *(text + 1));
        }

        text++;
        i++;
    }
    
    return width;
}

real32 get_width(Font font, String string) {
    real32 width = 0;

    int32 i = 0;
    char *text = string.contents;
    while (*text && i < string.length) {
        int32 advance, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font.font_info, *text, &advance, &left_side_bearing);
        width += (advance) * font.scale_for_pixel_height;
        
        if (*(text + 1)) {
            width += font.scale_for_pixel_height * stbtt_GetCodepointKernAdvance(&font.font_info,
                                                                                  *text, *(text + 1));
        }

        text++;
        i++;
    }
    
    return width;
}

inline real32 get_width(Font font, String_Buffer buffer) {
    return get_width(font, make_string(buffer));
}
