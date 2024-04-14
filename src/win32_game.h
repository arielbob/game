#ifndef WIN32_GAME_H
#define WIN32_GAME_H

#include "memory.h"

#define PLATFORM_MAX_PATH MAX_PATH

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

#pragma pack(push, 1)
struct Wav_Data {
    char chunk_id[4];
    uint32 chunk_size;
    char format[4];
    
    char subchunk_1_id[4];
    uint32 subchunk_1_size;
    uint16 audio_format;
    uint16 num_channels;
    uint32 sample_rate;
    uint32 byte_rate;
    uint16 block_align;
    uint16 bits_per_sample;
    
    char subchunk_2_id[4];
    uint32 subchunk_2_size;
    
    void *data;
};
#pragma pack(pop)

struct Win32_Directory_Watcher_Data {
    Arena_Allocator *arena;
    void *dir_changes_buffer;
    int32 dir_changes_buffer_size;
};

#endif
