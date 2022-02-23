#ifndef PLATFORM_H
#define PLATFORM_H

struct Platform_File {
    HANDLE file_handle;
    uint32 file_size;
};

struct File_Data {
    char *contents;
    uint32 size;
};

void debug_print(char *format, ...);
bool32 platform_open_file(char *filename, Platform_File *file_result);
bool32 platform_read_file(Platform_File platform_file, File_Data *file_data, Arena *arena);
void platform_close_file(Platform_File platform_file);
void platform_zero_memory(void *base, uint32 size);

#endif
