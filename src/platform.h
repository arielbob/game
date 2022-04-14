#ifndef PLATFORM_H
#define PLATFORM_H

#include "memory.h"

struct Platform_File {
    HANDLE file_handle;
    uint32 file_size;
};

struct File_Data {
    char *contents;
    uint32 size;
};

#define array_length(array) sizeof(array) / sizeof((array)[0])

void debug_print(char *format, ...);
void string_format(char *buf, int32 n, char *format, ...);
char *string_format(Allocator *buf, int32 n, char *format, ...);
bool32 platform_open_file(char *filename, Platform_File *file_result);
bool32 platform_read_file(Platform_File platform_file, File_Data *file_data);
bool32 platform_write_file(char *filename, void *buffer, uint32 num_bytes_to_write, bool32 overwrite);
void platform_close_file(Platform_File platform_file);
void platform_zero_memory(void *base, uint32 size);
File_Data platform_open_and_read_file(Allocator *allocator, char *filename);
bool32 platform_open_file_dialog(char *filepath, uint32 size);
bool32 platform_open_save_file_dialog(char *filepath, char *filetype_name, char *file_extension_no_dot, uint32 size);
real64 platform_get_wall_clock_time();
void platform_set_cursor_visible(bool32 is_visible);
Vec2 platform_get_cursor_pos();
void platform_set_cursor_pos(Vec2 cursor_pos);
bool32 platform_window_has_focus();

#endif
