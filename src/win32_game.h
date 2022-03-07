#ifndef WIN32_GAME_H
#define WIN32_GAME_H

struct Win32_Display_Output {
    int32 width;
    int32 height;
};

struct Debug_Audio_Marker {
    DWORD play_cursor;
    uint32 write_cursor;
    uint32 flip_play_cursor;
    uint32 flip_write_cursor;
};

struct Win32_Sound_Output {
    int32 samples_per_second;
    int32 bit_depth;
    int32 bytes_per_sample; // this includes the bytes for all channels (i.e., bit_depth / 8 * num_channels)
    LPDIRECTSOUNDBUFFER sound_buffer;
    int32 buffer_size;
    bool32 is_playing;
    uint32 current_sample_index;
    int16 *accumulated_sound_buffer;
    DWORD current_play_cursor;
    DWORD current_write_cursor;
    DWORD last_write_cursor;
    //uint32 marker_index;
    //Debug_Audio_Marker markers[TARGET_FRAMERATE];
};

#endif
