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

Font load_font(Memory *memory,
               Game_State *game_state,
               char *font_filename, char *font_name,
               real32 font_height_pixels,
               int32 font_texture_width, int32 font_texture_height) {
    Font font = {};
    font.height_pixels = font_height_pixels;
    font.texture_width = font_texture_width;
    font.texture_height = font_texture_height;

    stbtt_fontinfo font_info;

    File_Data font_file_data;
    String font_filename_string = make_string(font_filename);
    if (!hash_table_find(game_state->font_file_table, font_filename_string, &font_file_data)) {
        font_file_data = platform_open_and_read_file((Allocator *) &memory->font_arena,
                                                     font_filename);
        hash_table_add(&game_state->font_file_table, font_filename_string, font_file_data);
    }

    // get font info
    // NOTE: this assumes that the TTF file only has a single font and is at index 0, or else
    //       stbtt_GetFontOffsetForIndex will return a negative value.
    // NOTE: font_info uses the raw data from the file contents, so the file data allocation should NOT
    //       be temporary.
    stbtt_InitFont(&font_info, (uint8 *) font_file_data.contents,
                   stbtt_GetFontOffsetForIndex((uint8 *) font_file_data.contents, 0));
    font.scale_for_pixel_height = stbtt_ScaleForPixelHeight(&font_info, font_height_pixels);
    stbtt_GetFontVMetrics(&font_info, &font.ascent, &font.descent, &font.line_gap);
    font.font_info = font_info;
    font.file_data = font_file_data;

    int32 first_char = 32;
    int32 num_chars = 96;
    font.cdata = (stbtt_bakedchar *) arena_push(&memory->font_arena, num_chars * sizeof(stbtt_bakedchar), false);
    font.first_char = first_char;
    font.num_chars = num_chars;
    font.name = make_string_buffer((Allocator *) &memory->string_arena, font_name, FONT_NAME_MAX_SIZE);

    return font;
}

