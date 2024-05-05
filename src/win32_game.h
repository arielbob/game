#ifndef WIN32_GAME_H
#define WIN32_GAME_H

#include "memory.h"
#include "string.h"

#define PLATFORM_MAX_PATH MAX_PATH
#define DIRECTORY_WATCHER_MAX_WATCHERS 256

// TODO: these probably should be in platform.h, but oh well..
#define MAX_FILE_CHANGES 128
enum Directory_Change_Type {
    DIR_CHANGE_NONE,
    DIR_CHANGE_FILE_MODIFIED,
    DIR_CHANGE_FILE_RENAMED
};
typedef void (*Directory_Change_Callback)(Directory_Change_Type, WString);

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
    Arena_Allocator arena;
    //Stack_Allocator thread_stack;
    int32 dir_changes_buffer_size;
    void *dir_changes_buffer;
    HANDLE dir_handle;
    WString dir_abs_path;
    Directory_Change_Callback change_callback;
    OVERLAPPED overlapped;
    Win32_Directory_Watcher_Data *next_free;
    Win32_Directory_Watcher_Data *next; // to iterate through them
};

struct Win32_Directory_Watcher_Manager {
    Heap_Allocator heap; // when we need to share things from main thread to watcher thread
    Arena_Allocator arena;
    CRITICAL_SECTION critical_section;
    HANDLE thread_handle;
    bool32 is_running;
    volatile uint32 num_watchers;
    Win32_Directory_Watcher_Data *first_free_watcher;
    Win32_Directory_Watcher_Data *watchers;
};

struct Win32_Directory_Watcher_Start_Request {
    Win32_Directory_Watcher_Manager *manager;
    WString abs_filepath;
    Directory_Change_Callback change_callback;
};

struct Win32_Directory_Watcher_End_Request {
    Win32_Directory_Watcher_Manager *manager;
    WString filepath; // TODO: we may want to convert some of our stuff use wide strings...
};

#endif
