#ifndef FONT_H
#define FONT_H

struct Font {
    String_Buffer name;
    File_Data file_data;
    stbtt_fontinfo font_info;
    stbtt_bakedchar *cdata;
    real32 height_pixels;
    real32 scale_for_pixel_height;
    int32 ascent;
    int32 descent;
    int32 line_gap;
    int32 texture_width;
    int32 texture_height;
    int32 first_char;
    int32 num_chars;
    bool32 is_baked;
};

real32 get_width(Font font, char *text);

#endif
