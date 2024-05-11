#ifndef PLATFORM_H
#define PLATFORM_H

#include "memory.h"

// TODO: i think this file is supposed to be platform-independent...
// - HANDLE is win32-specific. each [platform]_game.h should have the platform-specific
//   implementations for stuff.
struct Platform_File {
    HANDLE file_handle;
    uint32 file_size;
};

struct File_Data {
    char *contents;
    uint32 size;
};

struct Buffer {
    uint8 *data;
    int32 size;
};

#define array_length(array) sizeof(array) / sizeof((array)[0])

void debug_print(char *format, ...);
void string_format(char *buf, int32 n, char *format, ...);
char *string_format(Allocator *buf, int32 n, char *format, ...);
char *string_format(Allocator *buf, char *format, ...);
void string_format(String_Buffer *buffer, char *format, ...);
void platform_get_relative_path(char *absolute_path, char *relative_path_buffer, int32 relative_path_buffer_size);
void platform_get_absolute_path(char *relative_path, char *absolute_path_buffer, int32 absolute_path_buffer_size);
bool32 platform_open_file(char *filename, Platform_File *file_result);
bool32 platform_read_file(Platform_File platform_file, File_Data *file_data);
bool32 platform_write_file(char *filename, void *buffer, uint32 num_bytes_to_write, bool32 overwrite);
void platform_close_file(Platform_File platform_file);
bool32 platform_file_exists(char *filename);
bool32 platform_file_exists(String filename);
void platform_zero_memory(void *base, uint32 size);
File_Data platform_open_and_read_file(Allocator *allocator, char *filename);
File_Data platform_open_and_read_file(Allocator *allocator, String filename);
bool32 platform_open_file_dialog(char *filepath, uint32 size);
bool32 platform_open_file_dialog(char *filepath, char *filetype_name, char *file_extension_no_dot, uint32 size);
bool32 platform_open_save_file_dialog(char *filepath, char *filetype_name, char *file_extension_no_dot, uint32 size);
real64 platform_get_wall_clock_time();
void platform_set_cursor_visible(bool32 is_visible);
Vec2 platform_get_cursor_pos();
void platform_set_cursor_pos(Vec2 cursor_pos);
bool32 platform_window_has_focus();
String platform_get_folder_path(Allocator *allocator, String path);
void platform_watch_directory(String directory, Directory_Change_Callback change_callback);
void platform_wide_char_to_multi_byte(WString wstring, char *buffer, int32 buffer_size);

// TODO: remove this and put the callback in asset.cpp or whatever
void watcher_callback(Directory_Change_Type change_type, WString filename);

#endif
