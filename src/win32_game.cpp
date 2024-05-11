// TODO (done): open window
// TODO (done): basic directsound
// TODO (done): play sounds with directsound

#define OEMRESOURCE

#include <windows.h>
#include <shlwapi.h>
#include <stdint.h>
#include <stdio.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <dsound.h>
#include <math.h>

#undef near
#undef far

typedef WCHAR wchar16;

#define TARGET_FRAMERATE 60
#define SAMPLE_RATE 44100
// NOTE: sound buffer holds a 10th of a second of audio
// FIXME: there is a bug where there is a delay in audio. if you set SOUND_BUFFER_SAMPLE_COUNT to hold a
//        second's worth of sample, you'll see the delay clearly. if we just keep it to 1/10th of a second,
//        it's not that noticeable.
#define SOUND_BUFFER_SAMPLE_COUNT SAMPLE_RATE / 10

#include "common.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include "win32_threads.h"
#include "win32_game.h"

#include "utils.h"
#include "memory.h"

//global_variable Allocator *temp_region;
global_variable Allocator *frame_arena;

#include "string.h"

#include "math.h"
//#include "entity.h"
#include "game.h"
#include "platform.h"

global_variable Memory memory;
global_variable UI_Manager *ui_manager;
global_variable Asset_Manager *asset_manager;
global_variable Game_State *game_state;
global_variable Render_State *render_state;
global_variable Win32_Directory_Watcher_Manager directory_watcher_manager;

#include "memory.cpp"
#include "math.cpp"
#include "mesh.cpp"
#include "render.cpp"
#include "render_queue.cpp"
#include "font.cpp"
#include "animation.cpp"
#include "asset.cpp"
#include "ui.cpp"
#include "ui_render2.cpp"
#include "entity.cpp"
#include "level.cpp"
#include "level_import.cpp"
#include "level_export.cpp"
#include "gizmo.cpp"
//#include "editor_actions.cpp"
//#include "editor_ui.cpp"
#include "asset_library.cpp"
#include "collision_debugger.cpp"
#include "editor.cpp"
//#include "tree-test.cpp"
#include "game.cpp"



global_variable int64 perf_counter_frequency;
global_variable bool32 is_running = true;
global_variable bool32 is_paused = true;
global_variable bool32 is_cursor_visible = true;
global_variable HWND window;

typedef char GLchar;
typedef signed long long int khronos_ssize_t;
typedef int * khronos_intptr_t;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;

typedef void GL_GEN_VERTEX_ARRAYS(GLsizei n, GLuint *arrays);
typedef void GL_GEN_BUFFERS (GLsizei n, GLuint *buffers);
typedef void GL_GEN_FRAMEBUFFERS (GLsizei n, GLuint *framebuffers);
typedef void GL_BIND_FRAMEBUFFER (GLenum target, GLuint framebuffer);
typedef void GL_BIND_BUFFER (GLenum target, GLuint buffer);
typedef void GL_BUFFER_DATA (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void GL_BUFFER_SUB_DATA (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void GL_VERTEX_ATTRIB_POINTER (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void GL_VERTEX_ATTRIB_I_POINTER (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void GL_ENABLE_VERTEX_ATTRIB_ARRAY (GLuint index);
typedef void GL_BIND_VERTEX_ARRAY (GLuint array);
typedef GLuint GL_CREATE_SHADER (GLenum type);
typedef void GL_SHADER_SOURCE (GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void GL_COMPILE_SHADER (GLuint shader);
typedef GLuint GL_CREATE_PROGRAM (void);
typedef void GL_ATTACH_SHADER (GLuint program, GLuint shader);
typedef void GL_LINK_PROGRAM (GLuint program);
typedef void GL_DELETE_SHADER (GLuint shader);
typedef void GL_USE_PROGRAM (GLuint program);
typedef GLint GL_GET_UNIFORM_LOCATION (GLuint program, const GLchar *name);
typedef void GL_UNIFORM_MATRIX_4FV (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void GL_GET_SHADER_INFO_LOG (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void GL_GET_SHADERIV (GLuint shader, GLenum pname, GLint *params);
typedef void GL_GET_PROGRAM_INFO_LOG (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void GL_GET_PROGRAMIV (GLuint program, GLenum pname, GLint *params);
typedef void GL_UNIFORM_1I (GLint location, GLint value);
typedef void GL_UNIFORM_1UI (GLint location, GLint v0);
typedef void GL_UNIFORM_1F (GLint location, GLfloat v0);
typedef void GL_UNIFORM_2FV (GLint location, GLsizei count, const GLfloat *value);
typedef void GL_UNIFORM_3FV (GLint location, GLsizei count, const GLfloat *value);
typedef void GL_UNIFORM_4FV (GLint location, GLsizei count, const GLfloat *value);
typedef void GL_FRAMEBUFFER_TEXTURE_2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void GL_GEN_RENDERBUFFERS (GLsizei n, GLuint *renderbuffers);
typedef void GL_BIND_RENDERBUFFER (GLenum target, GLuint renderbuffer);
typedef void GL_RENDERBUFFER_STORAGE (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_FRAMEBUFFER_RENDERBUFFER (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef GLenum GL_CHECK_FRAMEBUFFER_STATUS (GLenum target);
typedef void GL_DELETE_FRAMEBUFFERS (GLsizei n, const GLuint *framebuffers);
typedef void GL_DELETE_RENDERBUFFERS (GLsizei n, const GLuint *renderbuffers);
typedef void GL_BIND_BUFFER_BASE (GLenum target, GLuint index, GLuint buffer);
typedef GLuint GL_GET_UNIFORM_BLOCK_INDEX (GLuint program, const GLchar *uniformBlockName);
typedef void GL_UNIFORM_BLOCK_BINDING (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void GL_BIND_BUFFER_RANGE (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void GL_DELETE_BUFFERS (GLsizei n, const GLuint *buffers);
typedef void GL_DELETE_VERTEX_ARRAYS (GLsizei n, const GLuint *arrays);
typedef void GL_DRAW_ELEMENTS_INSTANCED (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
typedef void GL_TEX_IMAGE_2D_MULTISAMPLE (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void GL_RENDER_BUFFER_STORAGE_MULTISAMPLE (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void GL_BLIT_FRAMEBUFFER (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void GL_VERTEX_ATTRIB_DIVISOR (GLuint index, GLuint divisor);
typedef void GL_ACTIVE_TEXTURE (GLenum texture);
typedef void GL_BLEND_EQUATION_PROC (GLenum mode);

GL_GEN_VERTEX_ARRAYS *glGenVertexArrays;
GL_GEN_BUFFERS *glGenBuffers;
GL_GEN_FRAMEBUFFERS *glGenFramebuffers;
GL_BIND_FRAMEBUFFER *glBindFramebuffer;
GL_BIND_BUFFER *glBindBuffer;
GL_BUFFER_DATA *glBufferData;
GL_BUFFER_SUB_DATA *glBufferSubData;
GL_VERTEX_ATTRIB_POINTER *glVertexAttribPointer;
GL_VERTEX_ATTRIB_I_POINTER *glVertexAttribIPointer;
GL_ENABLE_VERTEX_ATTRIB_ARRAY *glEnableVertexAttribArray;
GL_BIND_VERTEX_ARRAY *glBindVertexArray;
GL_CREATE_SHADER *glCreateShader;
GL_SHADER_SOURCE *glShaderSource;
GL_COMPILE_SHADER *glCompileShader;
GL_CREATE_PROGRAM *glCreateProgram;
GL_ATTACH_SHADER *glAttachShader;
GL_LINK_PROGRAM *glLinkProgram;
GL_DELETE_SHADER *glDeleteShader;
GL_USE_PROGRAM *glUseProgram;
GL_GET_UNIFORM_LOCATION *glGetUniformLocation;
GL_UNIFORM_MATRIX_4FV *glUniformMatrix4fv;
GL_GET_SHADER_INFO_LOG *glGetShaderInfoLog;
GL_GET_SHADERIV *glGetShaderiv;
GL_GET_PROGRAM_INFO_LOG *glGetProgramInfoLog;
GL_GET_PROGRAMIV *glGetProgramiv;
GL_UNIFORM_1I *glUniform1i;
GL_UNIFORM_1UI *glUniform1ui;
GL_UNIFORM_1F *glUniform1f;
GL_UNIFORM_2FV *glUniform2fv;
GL_UNIFORM_3FV *glUniform3fv;
GL_UNIFORM_4FV *glUniform4fv;
GL_FRAMEBUFFER_TEXTURE_2D *glFramebufferTexture2D;
GL_GEN_RENDERBUFFERS *glGenRenderbuffers;
GL_BIND_RENDERBUFFER *glBindRenderbuffer;
GL_RENDERBUFFER_STORAGE *glRenderbufferStorage;
GL_FRAMEBUFFER_RENDERBUFFER *glFramebufferRenderbuffer;
GL_CHECK_FRAMEBUFFER_STATUS *glCheckFramebufferStatus;
GL_DELETE_FRAMEBUFFERS *glDeleteFramebuffers;
GL_DELETE_RENDERBUFFERS *glDeleteRenderbuffers;
GL_BIND_BUFFER_BASE *glBindBufferBase;
GL_GET_UNIFORM_BLOCK_INDEX *glGetUniformBlockIndex;
GL_UNIFORM_BLOCK_BINDING *glUniformBlockBinding;
GL_BIND_BUFFER_RANGE *glBindBufferRange;
GL_DELETE_BUFFERS *glDeleteBuffers;
GL_DELETE_VERTEX_ARRAYS *glDeleteVertexArrays;
GL_DRAW_ELEMENTS_INSTANCED *glDrawElementsInstanced;
GL_TEX_IMAGE_2D_MULTISAMPLE *glTexImage2DMultisample;
GL_RENDER_BUFFER_STORAGE_MULTISAMPLE *glRenderbufferStorageMultisample;
GL_BLIT_FRAMEBUFFER *glBlitFramebuffer;
GL_VERTEX_ATTRIB_DIVISOR *glVertexAttribDivisor;
GL_ACTIVE_TEXTURE *glActiveTexture;
GL_BLEND_EQUATION_PROC *glBlendEquation;

#include "game_gl.cpp"

internal int64 win32_get_perf_counter() {
    LARGE_INTEGER perf_counter;
    QueryPerformanceCounter(&perf_counter);
    return perf_counter.QuadPart;
}

internal real32 win32_get_elapsed_time(int64 start_perf_counter) {
    return (real32) (win32_get_perf_counter() - start_perf_counter) / perf_counter_frequency;
}

// gets wall clock time in seconds
real64 platform_get_wall_clock_time() {
    LARGE_INTEGER perf_counter;
    
    QueryPerformanceCounter(&perf_counter);
    return (real64) perf_counter.QuadPart / perf_counter_frequency;
}

void debug_print(char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int32 num_chars_no_null = vsnprintf(NULL, 0, format, args);
    int32 n = num_chars_no_null + 1;

    // TODO: this is not thread safe...
    Allocator *temp_region = begin_region();
    char *buf = (char *) allocate(temp_region, n);

    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    OutputDebugStringA(buf);
    end_region(temp_region);
    
    va_end(args);
}

void string_format(char *buf, int32 n, char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    va_end(args);
}

char *string_format(Allocator *allocator, char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int32 num_chars_no_null = vsnprintf(NULL, 0, format, args);
    int32 n = num_chars_no_null + 1;
    char *buf = (char *) allocate(allocator, n);

    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    va_end(args);
    
    return buf;
}

char *string_format(Allocator *allocator, int32 n, char *format, ...) {
    char *buf = (char *) allocate(allocator, n);
    va_list args;
    va_start(args, format);
    
    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    va_end(args);

    return buf;
}

// buffer should be a String_Buffer with contents and size already set.
// it's being used here like a container for (char *buf, int n) arguments like
// would usually be passed in a function like this.
void string_format(String_Buffer *buffer, char *format, ...) {
    assert(buffer->contents);

    va_list args;
    va_start(args, format);
    
    int32 num_chars_no_null = vsnprintf(NULL, 0, format, args);
    int32 n = num_chars_no_null + 1;

    Allocator *temp_region = begin_region();
    char *temp_buf = (char *) allocate(temp_region, n);

    int32 num_chars_outputted = vsnprintf(temp_buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    // copy the temp text to the buffer.
    // note that this will assert if there isn't enough space.
    set_string_buffer_text(buffer, temp_buf);

    end_region(temp_region);

    va_end(args);
}

void platform_get_relative_path(char *absolute_path,
                                char *relative_path_buffer, int32 relative_path_buffer_size) {
    assert(relative_path_buffer_size >= MAX_PATH);

    char current_directory[MAX_PATH];
    // NOTE: this should be the root directory and should not change. if for some reason files can't be found
    //       then it could be the case that some other procedure modified the current directory.
    DWORD get_current_directory_result = GetCurrentDirectoryA(MAX_PATH, current_directory);
    // we do < MAX_PATH, since the return value on success, which is the amount of characters written,
    // does not include the null terminator.
    assert(get_current_directory_result > 0 && get_current_directory_result < MAX_PATH);

    // NOTE: PathRelativePathToA fails if the two paths do not share a common prefix. for example, two
    //       absolute paths on different drives, for example, one has the prefix C:/ and the other D:/, would
    //       fail.
    bool32 path_relative_path_to_result = PathRelativePathToA(relative_path_buffer,
                                                              current_directory,
                                                              FILE_ATTRIBUTE_DIRECTORY,
                                                              absolute_path,
                                                              FILE_ATTRIBUTE_NORMAL);
    assert(path_relative_path_to_result);
}

void platform_get_absolute_path(char *relative_path,
                                char *absolute_path_buffer, int32 absolute_path_buffer_size) {
    assert(absolute_path_buffer_size >= MAX_PATH);

    DWORD path_length_without_null = GetFullPathNameA(relative_path,
                                                      absolute_path_buffer_size, absolute_path_buffer, NULL);
    assert(path_length_without_null > 0 && path_length_without_null < MAX_PATH);
}

void platform_get_absolute_path(wchar16 *relative_path,
                                wchar16 *absolute_path_buffer, uint32 absolute_path_buffer_size) {
    assert(sizeof(TCHAR) == 1);
    assert(absolute_path_buffer_size >= MAX_PATH*sizeof(wchar16));

    // the docs say that the 2nd argument is the size of the output buffer in TCHARs..
    // i assumed that since our TCHAR is 1 (we don't compile with -DUNICODE or -D_UNICODE), this argument
    // would be MAX_PATH*sizeof(WCHAR), so 260*2, but doing this results in a buffer overrun.
    // which is weird, because i would assume that if we put just MAX_PATH, it would be too small and
    // result in a buffer overrun.
    // the return is supposed to be size in TCHARs as well, but it acts like TCHAR is wide (2 bytes),
    // so i guess TCHAR is always considered wide when using W functions, despite not compiling with
    // UNICODE defined?
    DWORD path_length_without_null = GetFullPathNameW(relative_path,
                                                      MAX_PATH, absolute_path_buffer, NULL);
    assert(path_length_without_null > 0 && path_length_without_null < absolute_path_buffer_size);
}

// NOTE: we create a file handle so that we deny other processes from writing to it before we're done with it.
//       this is done with the FILE_SHARE_READ flag. other processes can only read it, but not write or delete it
//       until we close the file.
//       this is also useful if the file is not null-terminated. to read a file like that, we would first
//       get the file size, allocate memory of that size, then put the contents of the file into the allocated
//       memory. to loop through it, we need to know the size of it, since it is not null-terminated, so we use
//       the value we got from the call to GetFileSize. we do not want the file size or its contents to change
//       while we're using it, or else our file size we're using to read the file would be out of date.
bool32 platform_open_file(char *filename, Platform_File *file_result) {
    // TODO: may want to handle long paths
    // TODO: this does not handle unicode paths; would have to use GetFullPathNameW
    char path_result[MAX_PATH];
    DWORD path_length_without_null = GetFullPathNameA(filename, MAX_PATH, path_result, NULL);
    assert(path_length_without_null > 0 && path_length_without_null < MAX_PATH);

    HANDLE file_handle = CreateFile(path_result, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        file_result->file_handle = file_handle;
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size)) {
            // TODO: we only support up to 4.2 GB files right now (32 bits)
            // will have to stream it in if we want more
            assert(file_size.QuadPart <= 0xffffffff);
            uint32 file_size_32 = file_size.u.LowPart;
            file_result->file_size = file_size_32;
            return true;
        }
    }

    DWORD last_error = GetLastError();

    return false;
}

// given a filepath (relative or absolute), remove the file and backslash, if they exist.
// this also converts relative filepaths to absolute.
// note that if you have something like "test/", you'll just get "test".
// if you have something like "a/test", you'll get "a".
// so basically, if you're using this function, you should ensure that what you're passing
// in is in fact a path to a file.
void platform_get_folder_path(char *path, char *abs_folder_path, int32 abs_folder_path_size) {
    assert(abs_folder_path_size >= string_length(path));

    char abs_path[MAX_PATH];
    // get the absolute path because this function also converts non-windows paths to
    // windows paths, i.e. converts / to \. PathRemoveFileSpecA() doesn't work correctly
    // with non-windows paths
    platform_get_absolute_path(path, abs_path, MAX_PATH);
    PathRemoveFileSpecA(abs_path);

    copy_string(abs_folder_path, abs_path, abs_folder_path_size);
}

String platform_get_folder_path(Allocator *allocator, String path) {
    char *abs_path = (char *) allocate(allocator, MAX_PATH);

    // get the absolute path because this function also converts non-windows paths to
    // windows paths, i.e. converts / to \. PathRemoveFileSpecA() doesn't work correctly
    // with non-windows paths

    char path_c_str[MAX_PATH];
    to_char_array(path, path_c_str, MAX_PATH);
    
    platform_get_absolute_path(path_c_str, abs_path, MAX_PATH);
    PathRemoveFileSpecA(abs_path);

    String ret = {};
    ret.allocator = allocator;
    ret.contents = abs_path;
    ret.length = string_length(abs_path);

    return ret;
}

bool32 platform_path_is_directory(char *path) {
    char abs_path[MAX_PATH];
    platform_get_absolute_path(path, abs_path, MAX_PATH);
    
    DWORD file_attributes = GetFileAttributesA(abs_path);
    assert(file_attributes != INVALID_FILE_ATTRIBUTES);

    return (file_attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool32 platform_path_is_directory(wchar16 *path) {
    WCHAR abs_path[MAX_PATH];
    platform_get_absolute_path(path, abs_path, MAX_PATH*sizeof(WCHAR));
    
    DWORD file_attributes = GetFileAttributesW(abs_path);
    assert(file_attributes != INVALID_FILE_ATTRIBUTES);

    return (file_attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool32 platform_read_file(Platform_File platform_file, File_Data *file_data) {
    assert(platform_file.file_handle);
    uint32 file_size_32 = platform_file.file_size;
    HANDLE file_handle = platform_file.file_handle;

    DWORD number_of_bytes_read;
    assert(file_data->contents);
    // NOTE: file_data->contents will NOT be null-terminated
    if (ReadFile(file_handle, file_data->contents, file_size_32, &number_of_bytes_read, 0)) {
        // NOTE: we make sure that the number of bytes read is the same as the file size we got when we opened
        //       the file. if this is not the case, the file has been modified, and we should error (this should
        //       not happen, since our platform_open_file procedure prevents the file handle from being modified
        //       while we have it opened).
        assert(number_of_bytes_read == file_size_32);
        // NOTE: we save the file size again since we don't want to keep having to use platform_file.
        //       platform_file is only used when we need the handle. once we have the file's contents we don't
        //       need it anymore.
        file_data->size = file_size_32;
        return true;
    } else {
        file_data->contents = 0;
        debug_print("Could not read file\n");
        return false;
    }
}

void platform_close_file(Platform_File platform_file) {
    assert(platform_file.file_handle);
    CloseHandle(platform_file.file_handle);
}

bool32 platform_file_exists(char *filename) {
    Platform_File file;

    bool32 result = platform_open_file(filename, &file);

    if (result) {
        platform_close_file(file);
    }

    return result;
}

bool32 platform_file_exists(String filename) {
    Allocator *temp_region = begin_region();

    char *filename_c_str = to_char_array(temp_region, filename);
    bool32 result = platform_file_exists(filename_c_str);
    
    end_region(temp_region);
    
    return result;
}

File_Data platform_open_and_read_file(Allocator *allocator, char *filename) {
    Platform_File platform_file;
    bool32 file_exists = platform_open_file(filename, &platform_file);
    assert(file_exists);

    File_Data file_data = {};
    file_data.contents = (char *) allocate(allocator, platform_file.file_size, false);

    bool32 result = platform_read_file(platform_file, &file_data);
    assert(result);

    platform_close_file(platform_file);

    return file_data;
}

File_Data platform_open_and_read_file(Allocator *allocator, String filename) {
    char *filename_c_string = to_char_array(allocator, filename);
    File_Data result = platform_open_and_read_file(allocator, filename_c_string);
    return result;
}

internal bool32 win32_init_opengl(HDC hdc) {
    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    desired_pixel_format.nVersion   = 1;
    desired_pixel_format.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 24;
    desired_pixel_format.cDepthBits = 32;

    bool32 opengl_valid = false;
    int32 suggested_pixel_format_index = ChoosePixelFormat(hdc, &desired_pixel_format);
    PIXELFORMATDESCRIPTOR suggested_pixel_format = {};
    DescribePixelFormat(hdc,
                        suggested_pixel_format_index,
                        sizeof(PIXELFORMATDESCRIPTOR),
                        &suggested_pixel_format);
    HGLRC opengl_rendering_context;
    if (suggested_pixel_format_index) {
        if (SetPixelFormat(hdc, suggested_pixel_format_index, &suggested_pixel_format)) {
            opengl_rendering_context = wglCreateContext(hdc);
            if (opengl_rendering_context) {
                if (wglMakeCurrent(hdc, opengl_rendering_context)) {
                    // TODO: check if these actually exist and handle errors
                    // TODO: i think there's an easier way to do this where just check for an opengl version
                    
                    glGenVertexArrays = (GL_GEN_VERTEX_ARRAYS *) wglGetProcAddress("glGenVertexArrays");
                    glGenBuffers = (GL_GEN_BUFFERS *) wglGetProcAddress("glGenBuffers");
                    glGenFramebuffers = (GL_GEN_FRAMEBUFFERS *) wglGetProcAddress("glGenFramebuffers");
                    glBindFramebuffer = (GL_BIND_FRAMEBUFFER *) wglGetProcAddress("glBindFramebuffer");
                    glBindBuffer = (GL_BIND_BUFFER *) wglGetProcAddress("glBindBuffer");
                    glBufferData = (GL_BUFFER_DATA *) wglGetProcAddress("glBufferData");
                    glBufferSubData = (GL_BUFFER_SUB_DATA *) wglGetProcAddress("glBufferSubData");
                    glVertexAttribPointer = (GL_VERTEX_ATTRIB_POINTER *) wglGetProcAddress("glVertexAttribPointer");
                    glVertexAttribIPointer = (GL_VERTEX_ATTRIB_I_POINTER *) wglGetProcAddress("glVertexAttribIPointer");
                    glEnableVertexAttribArray = (GL_ENABLE_VERTEX_ATTRIB_ARRAY *) wglGetProcAddress("glEnableVertexAttribArray");
                    glBindVertexArray = (GL_BIND_VERTEX_ARRAY *) wglGetProcAddress("glBindVertexArray");
                    glCreateShader = (GL_CREATE_SHADER *) wglGetProcAddress("glCreateShader");
                    glShaderSource = (GL_SHADER_SOURCE *) wglGetProcAddress("glShaderSource");
                    glCompileShader = (GL_COMPILE_SHADER *) wglGetProcAddress("glCompileShader");
                    glCreateProgram = (GL_CREATE_PROGRAM *) wglGetProcAddress("glCreateProgram");
                    glAttachShader = (GL_ATTACH_SHADER *) wglGetProcAddress("glAttachShader");
                    glLinkProgram = (GL_LINK_PROGRAM *) wglGetProcAddress("glLinkProgram");
                    glDeleteShader = (GL_DELETE_SHADER *) wglGetProcAddress("glDeleteShader");
                    glUseProgram = (GL_USE_PROGRAM *) wglGetProcAddress("glUseProgram");
                    glGetUniformLocation = (GL_GET_UNIFORM_LOCATION *) wglGetProcAddress("glGetUniformLocation");
                    glUniformMatrix4fv = (GL_UNIFORM_MATRIX_4FV *) wglGetProcAddress("glUniformMatrix4fv");
                    glGetShaderiv = (GL_GET_SHADERIV *) wglGetProcAddress("glGetShaderiv");
                    glGetShaderInfoLog = (GL_GET_SHADER_INFO_LOG *) wglGetProcAddress("glGetShaderInfoLog");
                    glGetProgramiv = (GL_GET_PROGRAMIV *) wglGetProcAddress("glGetProgramiv");
                    glGetProgramInfoLog = (GL_GET_PROGRAM_INFO_LOG *) wglGetProcAddress("glGetProgramInfoLog");
                    glUniform1i = (GL_UNIFORM_1I *) wglGetProcAddress("glUniform1i");
                    glUniform1ui = (GL_UNIFORM_1UI *) wglGetProcAddress("glUniform1ui");
                    glUniform1f = (GL_UNIFORM_1F *) wglGetProcAddress("glUniform1f");
                    glUniform2fv = (GL_UNIFORM_2FV *) wglGetProcAddress("glUniform2fv");
                    glUniform3fv = (GL_UNIFORM_3FV *) wglGetProcAddress("glUniform3fv");
                    glUniform4fv = (GL_UNIFORM_4FV *) wglGetProcAddress("glUniform4fv");  
                    glFramebufferTexture2D = (GL_FRAMEBUFFER_TEXTURE_2D *) wglGetProcAddress("glFramebufferTexture2D");
                    glGenRenderbuffers = (GL_GEN_RENDERBUFFERS *) wglGetProcAddress("glGenRenderbuffers");
                    glBindRenderbuffer = (GL_BIND_RENDERBUFFER *) wglGetProcAddress("glBindRenderbuffer");
                    glRenderbufferStorage = (GL_RENDERBUFFER_STORAGE *) wglGetProcAddress("glRenderbufferStorage");
                    glFramebufferRenderbuffer = (GL_FRAMEBUFFER_RENDERBUFFER *) wglGetProcAddress("glFramebufferRenderbuffer");
                    glCheckFramebufferStatus = (GL_CHECK_FRAMEBUFFER_STATUS *) wglGetProcAddress("glCheckFramebufferStatus");
                    glDeleteFramebuffers = (GL_DELETE_FRAMEBUFFERS *) wglGetProcAddress("glDeleteFramebuffers");
                    glDeleteRenderbuffers = (GL_DELETE_RENDERBUFFERS *) wglGetProcAddress("glDeleteRenderbuffers");
                    glBindBufferBase = (GL_BIND_BUFFER_BASE *) wglGetProcAddress("glBindBufferBase");
                    glGetUniformBlockIndex = (GL_GET_UNIFORM_BLOCK_INDEX *) wglGetProcAddress("glGetUniformBlockIndex");
                    glUniformBlockBinding = (GL_UNIFORM_BLOCK_BINDING *) wglGetProcAddress("glUniformBlockBinding");
                    glBindBufferRange = (GL_BIND_BUFFER_RANGE *) wglGetProcAddress("glBindBufferRange");
                    glDeleteBuffers = (GL_DELETE_BUFFERS *) wglGetProcAddress("glDeleteBuffers");
                    glDeleteVertexArrays = (GL_DELETE_VERTEX_ARRAYS *) wglGetProcAddress("glDeleteVertexArrays");
                    glDrawElementsInstanced = (GL_DRAW_ELEMENTS_INSTANCED *) wglGetProcAddress("glDrawElementsInstanced");
                    glTexImage2DMultisample = (GL_TEX_IMAGE_2D_MULTISAMPLE *) wglGetProcAddress("glTexImage2DMultisample");
                    glRenderbufferStorageMultisample = (GL_RENDER_BUFFER_STORAGE_MULTISAMPLE *) wglGetProcAddress("glRenderbufferStorageMultisample");
                    glBlitFramebuffer = (GL_BLIT_FRAMEBUFFER *) wglGetProcAddress("glBlitFramebuffer");
                    glVertexAttribDivisor = (GL_VERTEX_ATTRIB_DIVISOR *) wglGetProcAddress("glVertexAttribDivisor");
                    glActiveTexture = (GL_ACTIVE_TEXTURE *) wglGetProcAddress("glActiveTexture");
                    glBlendEquation = (GL_BLEND_EQUATION_PROC *) wglGetProcAddress("glBlendEquation");
                    
                    return true;
                } else {
                    // TODO: logging
                    return false;
                }
            } else {
                // TODO: logging
                return false;
            }
        } else {
            // TODO: logging
            return false;
        }
    } else {
        // TODO: logging
        return false;
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DirectSoundCreateProc);

internal bool32 win32_init_directsound(Win32_Sound_Output *win32_sound_output) { 
    HMODULE directsound_library = LoadLibrary("dsound.dll");
    if (directsound_library) {
        DirectSoundCreateProc *direct_sound_create = (DirectSoundCreateProc *) GetProcAddress(directsound_library, "DirectSoundCreate");

        LPDIRECTSOUND DirectSound;
        if (direct_sound_create(NULL, &DirectSound, NULL) == DS_OK) {
            if (DirectSound->SetCooperativeLevel(window, DSSCL_PRIORITY) == DS_OK) {
                // assume bit_depth is evenly divisible by 8
                int32 bytes_per_sample = (win32_sound_output->bit_depth / 8) * 2;

                DSBUFFERDESC primary_buffer_desc = {};
                primary_buffer_desc.dwSize = sizeof(DSBUFFERDESC);
                primary_buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                primary_buffer_desc.guid3DAlgorithm = GUID_NULL;

                LPDIRECTSOUNDBUFFER primary_buffer;
                if (DirectSound->CreateSoundBuffer(&primary_buffer_desc, &primary_buffer, NULL) == DS_OK) {
                    WAVEFORMATEX wave_format = {};
                    wave_format.wFormatTag = WAVE_FORMAT_PCM;
                    wave_format.nChannels = 2;
                    wave_format.nSamplesPerSec = win32_sound_output->samples_per_second;
                    wave_format.wBitsPerSample = (WORD) win32_sound_output->bit_depth;
                    wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
                    wave_format.nAvgBytesPerSec =  wave_format.nSamplesPerSec * wave_format.nBlockAlign;
                    wave_format.cbSize = 0;

                    DSBUFFERDESC secondary_buffer_desc = {};
                    secondary_buffer_desc.dwSize = sizeof(DSBUFFERDESC);
                    secondary_buffer_desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
                    secondary_buffer_desc.dwBufferBytes = SOUND_BUFFER_SAMPLE_COUNT * bytes_per_sample;
                    secondary_buffer_desc.dwReserved = 0;
                    secondary_buffer_desc.lpwfxFormat = &wave_format;
                    secondary_buffer_desc.guid3DAlgorithm = GUID_NULL;
                                
                    if (DirectSound->CreateSoundBuffer(&secondary_buffer_desc, &win32_sound_output->sound_buffer, NULL) == DS_OK) {
                        OutputDebugStringA("Initialized secondary buffer\n");
                        return true;
                    } else {
                        OutputDebugStringA("Could not initialize secondary buffer\n");
                        return false;
                    }    
                } else {
                    OutputDebugStringA("Could not initialize primary buffer\n");
                    return false;
                }
            } else {
                OutputDebugStringA("Could not set DirectSound cooperative level\n");
                return false;
            }
        } else {
            OutputDebugStringA("Could not initialize DirectSound interface\n");
            return false;
        }
    } else {
        OutputDebugStringA("Could not load DirectSound library");
        return false;
    }
}

internal void win32_process_keyboard_input(bool was_down, bool is_down,
                                           uint32 vk_code, uint32 scan_code,
                                           Controller_State *controller_state) {
    // TODO: rebinding?
    // TODO: also store the was down variable to detect clicks
    if (is_down) {
        WORD translated_chars[16];
        char keyboard_state[256];
        if (GetKeyboardState((PBYTE) keyboard_state)) {
            // TODO: we can use ToAsciiEx to specify a specific keyboard layout, but i'm assuming ToAscii
            //       might get the current keyboard layout automatically - should check this
            int32 to_ascii_result = ToAscii(vk_code, scan_code, (PBYTE) keyboard_state, translated_chars, 0);
            if (to_ascii_result >= 1) {
                if (controller_state->num_pressed_chars < MAX_PRESSED_CHARS) {
                    char c = (char) translated_chars[0];
                    // if we get the delete character which can be created by pressing ctrl+backspace,
                    // just say that it's backspace for now.
                    if (c == 127) c = '\b';
                    controller_state->pressed_chars[controller_state->num_pressed_chars++] = c;
                }
            }
            
            //debug_print("ascii key translation: %c\n", translated_chars[0]);
        }
    }

    switch (vk_code) {
        case 'W': {
            controller_state->key_w.is_down = is_down;
            controller_state->key_w.was_down = was_down;
            return;
        }
        case 'A': {
            controller_state->key_a.is_down = is_down;
            controller_state->key_a.was_down = was_down;
            return;
        }
        case 'S': {
            controller_state->key_s.is_down = is_down;
            controller_state->key_s.was_down = was_down;
            return;
        }
        case 'D': {
            controller_state->key_d.is_down = is_down;
            controller_state->key_d.was_down = was_down;
            return;
        }
        case 'E': {
            controller_state->key_e.is_down = is_down;
            controller_state->key_e.was_down = was_down;
            return;
        }
        case 'X': {
            controller_state->key_x.is_down = is_down;
            controller_state->key_x.was_down = was_down;
            return;
        }
        case 'Z': {
            controller_state->key_z.is_down = is_down;
            controller_state->key_z.was_down = was_down;
            return;
        }
        case VK_UP: {
            controller_state->key_up.is_down = is_down;
            controller_state->key_up.was_down = was_down;
            controller_state->key_up.repeat = is_down == was_down;
            return;
        }
        case VK_DOWN: {
            controller_state->key_down.is_down = is_down;
            controller_state->key_down.was_down = was_down;
            controller_state->key_down.repeat = is_down == was_down;
            return;
        }
        case VK_RIGHT: {
            controller_state->key_right.is_down = is_down;
            controller_state->key_right.was_down = was_down;
            controller_state->key_right.repeat = is_down == was_down;
            return;
        }
        case VK_LEFT: {
            controller_state->key_left.is_down = is_down;
            controller_state->key_left.was_down = was_down;
            controller_state->key_left.repeat = is_down == was_down;
            return;
        }
        case VK_SPACE: {
            controller_state->key_space.is_down = is_down;
            controller_state->key_space.was_down = was_down;
            controller_state->key_space.repeat = is_down == was_down;
            return;
        }
        case VK_SHIFT: {
            controller_state->key_shift.is_down = is_down;
            controller_state->key_shift.was_down = was_down;
            return;
        }
        case VK_CONTROL: {
            controller_state->key_ctrl.is_down = is_down;
            controller_state->key_ctrl.was_down = was_down;
            return;
        }
        case VK_RETURN: {
            controller_state->key_enter.is_down = is_down;
            controller_state->key_enter.was_down = was_down;
            return;
        }
        case VK_MENU: {
            controller_state->key_alt.is_down = is_down;
            controller_state->key_alt.was_down = was_down;
            return;
        }
        case VK_TAB: {
            controller_state->key_tab.is_down = is_down;
            controller_state->key_tab.was_down = was_down;
            return;
        }
            // NOTE: you cannot handle VK_F10 here, since F10 sends a WM_SYSKEYDOWN message instead of
            //       WM_KEYDOWN, and we call this procedure only on WM_KEYDOWN and WM_KEYUP messages
        case VK_F5: {
            controller_state->key_f5.is_down = is_down;
            controller_state->key_f5.was_down = was_down;
            return;
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN:
            assert(!"Received WM_KEYDOWN in WindowProc");
            return 0;
        case WM_CREATE:
            return 0;
        case WM_PAINT:
            HDC hdc;
            PAINTSTRUCT ps;
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        case WM_SIZE: 
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0; 
    }

    return DefWindowProc(hwnd, message, wParam, lParam); 
}

global_variable int32 samples_written = 0;

void fill_sound_buffer(Win32_Sound_Output *win32_sound_output,
                       int16 *src_sound_buffer, uint32 num_samples) {
    if (num_samples == 0) return;
    LPDIRECTSOUNDBUFFER dest_sound_buffer = win32_sound_output->sound_buffer;

    DWORD bytes_to_write = num_samples * win32_sound_output->bytes_per_sample;

    LPVOID block1;
    DWORD block1_size;
    LPVOID block2;
    DWORD block2_size;

#if 0
    sound_output.last_write_cursor = sound_output.current_write_cursor;
    DWORD current_play_cursor, current_write_cursor;
    if (sound_output.sound_buffer->GetCurrentPosition(&current_play_cursor,
                                                      &current_write_cursor) == DS_OK) {
        sound_output.current_play_cursor = current_play_cursor;
        sound_output.current_write_cursor = current_write_cursor;
        real32 audio_latency_samples = ((real32) (current_write_cursor - current_play_cursor) /
                                        sound_output.bytes_per_sample);
        real32 audio_latency_ms = audio_latency_samples / sound_output.samples_per_second * 1000.0f;
        debug_print("audio latency; samples: %f, ms: %f\n", audio_latency_samples, audio_latency_ms);
    }
#else
    DWORD current_play_cursor, current_write_cursor;
    if (win32_sound_output->sound_buffer->GetCurrentPosition(&current_play_cursor,
                                                             &current_write_cursor) != DS_OK) {
        debug_print("Could not get current sound buffer position\n");
        assert(false);
    }
#endif

    DWORD byte_to_lock = (win32_sound_output->current_sample_index * win32_sound_output->bytes_per_sample);
    
    HRESULT lock_result = dest_sound_buffer->Lock(byte_to_lock,
                                             bytes_to_write,
                                             &block1, &block1_size,
                                             &block2, &block2_size,
                                             0);

    int16 *original_src_sound_buffer = src_sound_buffer;
    if (lock_result == DS_OK) {
        int16 *buffer_pos_1 = (int16 *) block1;
        for (DWORD i = 0; i < block1_size / win32_sound_output->bytes_per_sample; i++) {
            *(buffer_pos_1++) = *(src_sound_buffer++);
            *(buffer_pos_1++) = *(src_sound_buffer++);
        }

        int16 *buffer_pos_2 = (int16 *) block2;
        for (DWORD i = 0; i < block2_size / win32_sound_output->bytes_per_sample; i++) {
            *(buffer_pos_2++) = *(src_sound_buffer++);
            *(buffer_pos_2++) = *(src_sound_buffer++);
        }

        // copy the src_sound_buffer into the win32_sound_output accumulated sound buffer for debugging purposes
        src_sound_buffer = original_src_sound_buffer;
        int16 *accumulated_sound_buffer_dest = win32_sound_output->accumulated_sound_buffer;
        int32 max_samples = win32_sound_output->buffer_size / win32_sound_output->bytes_per_sample;
        uint32 samples_saved = 0;
        DWORD current_sample = byte_to_lock / win32_sound_output->bytes_per_sample;
        while (samples_saved < num_samples) {
            accumulated_sound_buffer_dest[2*current_sample]     = *(src_sound_buffer++);
            accumulated_sound_buffer_dest[2*current_sample + 1] = *(src_sound_buffer++);

            current_sample++;
            current_sample %= max_samples;
            samples_saved++;
        }

        win32_sound_output->current_sample_index = ((win32_sound_output->current_sample_index + num_samples) %
                                                    max_samples);

        dest_sound_buffer->Unlock(block1, block1_size, block2, block2_size);
    } else {
        assert(false);
        debug_print("Could not lock sound buffer region\n");
    }

    // debug_print("samples written: %d\n", samples_written);
}

bool32 win32_init_memory() {
    uint32 global_stack_size = MEGABYTES(64);
    uint32 hash_table_stack_size = MEGABYTES(8); // TODO: we should remove this
    uint32 game_data_arena_size = GIGABYTES(1);
    uint32 font_arena_size = MEGABYTES(64);
    uint32 frame_arena_size = MEGABYTES(64);

    uint32 ui_arena_size = MEGABYTES(256);
    uint32 editor_arena_size = MEGABYTES(256);
    
    uint32 total_memory_size = (global_stack_size +
                                hash_table_stack_size +
                                game_data_arena_size +
                                font_arena_size +
                                frame_arena_size +
                                ui_arena_size +
                                editor_arena_size);
    void *memory_base = VirtualAlloc(0, total_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (memory_base) {
        void *base = (uint8 *) memory_base;
        Stack_Allocator global_stack = make_stack_allocator(base, global_stack_size);
        memory.global_stack = global_stack;
        base = (uint8 *) base + global_stack_size;

        Arena_Allocator game_data_arena = make_arena_allocator(base, game_data_arena_size);
        memory.game_data = game_data_arena;
        base = (uint8 *) base + game_data_arena_size;

        Arena_Allocator font_arena = make_arena_allocator(base, font_arena_size);
        memory.font_arena = font_arena;
        base = (uint8 *) base + font_arena_size;

        // underscore prefix so we don't shadow the global frame_arena variable
        Arena_Allocator _frame_arena = make_arena_allocator(base, frame_arena_size);
        memory.frame_arena = _frame_arena;
        base = (uint8 *) base + frame_arena_size;

        Arena_Allocator ui_arena = make_arena_allocator(base, ui_arena_size);
        memory.ui_arena = ui_arena;
        base = (uint8 *) base + ui_arena_size;

        Arena_Allocator editor_arena = make_arena_allocator(base, editor_arena_size);
        memory.editor_arena = editor_arena;
        base = (uint8 *) base + editor_arena_size;

        memory.is_initted = true;
        frame_arena = (Allocator *) &memory.frame_arena;

        return true;
    }
    
    return false;
}

void platform_toggle_pause() {
    is_paused = !is_paused;
}

void platform_zero_memory(void *base, uint32 size) {
    ZeroMemory(base, size);
}

Vec2 platform_get_cursor_pos() {
    POINT cursor_pos;
    GetCursorPos(&cursor_pos);
    ScreenToClient(window, &cursor_pos);

    int64 cursor_y = cursor_pos.y;
    int64 cursor_x = cursor_pos.x;

    return make_vec2((real32) cursor_x, (real32) cursor_y);
}

void platform_set_cursor_pos(Vec2 cursor_pos) {
    POINT new_cursor_pos = { (long) cursor_pos.x, (long) cursor_pos.y };
    ClientToScreen(window, &new_cursor_pos);
    SetCursorPos(new_cursor_pos.x, new_cursor_pos.y);
}

void platform_set_cursor_visible(bool32 is_visible) {
    // since cursor visibility on windows is actually dependent on a counter that is incremented and decremented
    // by ShowCursor(), we only call ShowCursor if is_visible is different from is_cursor_visible. this makes
    // the API a lot nicer because you can just show and hide the cursor whenever you want and it'll work how
    // you expect it.
    if (is_visible != is_cursor_visible) {
        ShowCursor(is_visible);
        is_cursor_visible = is_visible;
    }
}

inline bool32 platform_window_has_focus() {
    return (GetFocus() == window);
}

bool32 platform_open_file_dialog(char *filepath, uint32 size) {
    OPENFILENAME ofn = {};       // common dialog box structure

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFile = filepath;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = size;
    ofn.lpstrFilter = "All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    bool32 result = GetOpenFileName(&ofn);
    return result;
}

// file_extensions should be of format "*.png; *.jpg"
bool32 platform_open_file_dialog(char *filepath, char *filetype_name, char *file_extensions, uint32 size) {
    OPENFILENAME ofn = {};       // common dialog box structure

    String filetype_name_string = make_string(filetype_name);
    String file_extensions_string = make_string(file_extensions);

    char filter_buffer[64];
    // NOTE: we do it this way since string_format is messed up when the string contains null
    //       characters that aren't at the end (the format string uses \0 as a separator).
    String_Buffer working_buffer = make_empty_string_buffer(filter_buffer, sizeof(filter_buffer));
    append_string(&working_buffer, filetype_name_string);
    append_string(&working_buffer, make_string("\0", 1));
    append_string(&working_buffer, file_extensions_string);
    append_string(&working_buffer, make_string("\0\0", 2));

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFile = filepath;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = size;
    ofn.lpstrFilter = working_buffer.contents;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    bool32 result = GetOpenFileName(&ofn);
    return result;
}

// NOTE: this only supports a single file extension
bool32 platform_open_save_file_dialog(char *filepath, char *filetype_name, char *file_extension, uint32 size) {
    assert(filetype_name);
    assert(file_extension);

    OPENFILENAME ofn = {};

    String filetype_name_string = make_string(filetype_name);

    char filter_buffer[64];
    // NOTE: we do it this way since string_format is messed up when the string contains null
    //       characters that aren't at the end (the format string uses \0 as a separator).
    String_Buffer working_buffer = make_empty_string_buffer(filter_buffer, sizeof(filter_buffer));
    append_string(&working_buffer, filetype_name_string);
    append_string(&working_buffer, make_string("\0", 1));
    append_string(&working_buffer, file_extension);
    append_string(&working_buffer, make_string("\0\0", 2));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFile = filepath;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = size;
    ofn.lpstrFilter = working_buffer.contents;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    // appends the file_extension to the returned filepath
    ofn.lpstrDefExt = file_extension;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    bool32 result = GetSaveFileName(&ofn);

    return result;
}

bool32 platform_write_file(char *filename, void *buffer, uint32 num_bytes_to_write, bool32 overwrite) {
    char path_result[MAX_PATH];
    DWORD path_length_without_null = GetFullPathNameA(filename, MAX_PATH, path_result, NULL);
    assert(path_length_without_null > 0 && path_length_without_null < MAX_PATH);

    TCHAR temp_path[MAX_PATH];
    TCHAR temp_filename[MAX_PATH];

    DWORD get_temp_path_result = GetTempPathA(MAX_PATH, temp_path);
    if (!get_temp_path_result) {
        debug_print("Could not find temporary file path\n");
        return false;
    }

    if (get_temp_path_result > MAX_PATH) {
        debug_print("Temporary file path too large.\n");
        return false;
    }

    uint32 get_temp_file_name_result = GetTempFileNameA(temp_path, "tmp", 0, temp_filename);
    if (!get_temp_file_name_result) {
        debug_print("Could not generate temporary filename\n");
        return false;
    }

    HANDLE temp_file_handle = CreateFile((LPTSTR) temp_filename,
                                         GENERIC_WRITE,
                                         0,
                                         NULL,
                                         CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
    if (temp_file_handle == INVALID_HANDLE_VALUE) {
        debug_print("Could not create temporary file for writing.\n");
        return false;
    }

    // write temp file
    DWORD temp_num_bytes_written;
    bool32 temp_write_file_result = WriteFile(temp_file_handle,
                                              buffer,
                                              num_bytes_to_write, &temp_num_bytes_written,
                                              NULL);
    if (!temp_write_file_result) {
        debug_print("Could not write to temporary file.\n");
        return false;
    }
    CloseHandle(temp_file_handle);

    DWORD move_file_flags = MOVEFILE_COPY_ALLOWED;
    if (overwrite) {
        move_file_flags |= MOVEFILE_REPLACE_EXISTING;
    }

    // move temp file to destination file
    bool32 move_file_result = MoveFileEx(temp_filename,
                                         path_result,
                                         move_file_flags);
    if (!move_file_result) {
        debug_print("Could not write to destination file.\n");
        return false;
    }

    return true;
}

void file_watcher_completion_routine(DWORD errorCode, DWORD bytesTransferred, LPOVERLAPPED overlapped) {
    OutputDebugStringA("something changed!!!!!\n");

    if (errorCode == ERROR_OPERATION_ABORTED) {
        // CancelIo was called
        OutputDebugStringA("COMPLETION ROUTINE GOT ABORTED SIGNAL!!!\n");
        return;
    }

    assert(bytesTransferred);
    
    Win32_Directory_Watcher_Data *data = (Win32_Directory_Watcher_Data *) overlapped->hEvent;
    assert(data->change_callback);
    
    // TODO: finish this
    FILE_NOTIFY_INFORMATION *event  = (FILE_NOTIFY_INFORMATION *) data->dir_changes_buffer;
    while (event) {
        // event->FileName isn't null-terminated, so just make this WString manually
        WString filename = {};
        filename.contents = event->FileName;
        // FileNameLength is in bytes, WString length is in characters
        filename.length = event->FileNameLength / sizeof(WCHAR);

        Directory_Change_Type change_type = DIR_CHANGE_NONE;
        switch (event->Action) {
            case FILE_ACTION_MODIFIED: {
                OutputDebugString("file modified!\n");
                change_type = DIR_CHANGE_FILE_MODIFIED;
            } break;
            case FILE_ACTION_RENAMED_OLD_NAME: {
                // TODO: we need to be able to convert this to a single event
                // - like we set a flag if we get this, then on the new name event,
                //   we call the callback. we would have to modify the callback to
                //   have an extra optional parameter.
                OutputDebugString("file renamed (old name event)!\n");
            } break;
            case FILE_ACTION_RENAMED_NEW_NAME: {
                OutputDebugString("file renamed (new name event)!\n");
            } break;
        }

        data->change_callback(change_type, filename);
        
        if (event->NextEntryOffset) {
            *((uint8 **) &event) += event->NextEntryOffset;
        } else {
            event = NULL;
            break;
        }
    }

    // make sure to clear this before using it again
    data->overlapped = {};
    data->overlapped.hEvent = data;

    DWORD bytesReturned = 0;

    BOOL success = ReadDirectoryChangesW(
        data->dir_handle,
        data->dir_changes_buffer,
        data->dir_changes_buffer_size,
        false, // not recursive
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
        &bytesReturned,
        &data->overlapped,
        file_watcher_completion_routine
    );

    assert(success);
}

void file_watcher_add_directory_routine(ULONG_PTR param) {
    // the request holds the directory
    Win32_Directory_Watcher_Start_Request *request = (Win32_Directory_Watcher_Start_Request *) param;
    Win32_Directory_Watcher_Manager *manager = request->manager;

    Win32_Directory_Watcher_Data *data = NULL;

    EnterCriticalSection(&manager->critical_section);

    if (!manager->is_running) {
        deallocate(request->abs_filepath);
        deallocate((Allocator *) &manager->heap, request);
        return;
    }
    
    WCHAR abs_filepath[MAX_PATH];
    to_c_string(request->abs_filepath, abs_filepath, MAX_PATH*sizeof(WCHAR));

    deallocate(request->abs_filepath);
    deallocate((Allocator *) &manager->heap, request);

    // see if it exists first
    Win32_Directory_Watcher_Data *current = manager->watchers;
    while (current) {
        if (string_equals(current->dir_abs_path, abs_filepath)) {
            // not an error; just don't add it again
            OutputDebugStringA("Directory is already being watched!");
            LeaveCriticalSection(&manager->critical_section);
            return;
        }
        current = current->next;
    }
    
    if (manager->first_free_watcher) {
        data = manager->first_free_watcher;
        manager->first_free_watcher = data->next_free;
    } else {
        // TODO: assert that we don't hit the pool limit
        Allocator *manager_allocator = (Allocator *) &manager->arena;

        int32 watcher_arena_size = MEGABYTES(1) + KILOBYTES(128);
        void *arena_start = allocate(manager_allocator, watcher_arena_size);
        
        // initialize the stuff
        data = (Win32_Directory_Watcher_Data *) allocate(manager_allocator, sizeof(Win32_Directory_Watcher_Data));

        // add an arena for the watcher data (strings and changes buffer)
        data->arena = make_arena_allocator(arena_start, watcher_arena_size);
    }

    // initialize the changes buffer again, since we clear the watcher's arena when we
    // stop that watcher
    int32 changes_buffer_size = MEGABYTES(1);
    data->dir_changes_buffer_size = changes_buffer_size;
    // changes buffer needs to be DWORD aligned
    data->dir_changes_buffer = allocate(&data->arena, changes_buffer_size, sizeof(DWORD));
    
    Arena_Allocator *watcher_arena = &data->arena;

    // copy absolute filepath string to arena
    data->dir_abs_path = make_wstring((Allocator *) watcher_arena, abs_filepath);

    // create handle to the directory
    data->dir_handle = CreateFileW(abs_filepath,
                                   FILE_LIST_DIRECTORY,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                   NULL);

    //end_region(temp_region);

    assert(data->dir_handle != INVALID_HANDLE_VALUE);

    DWORD bytesReturned = 0;

    // hEvent is not used by ReadDirectoryChangesW, so store our own data in it that we can use
    // in the completion routine
    data->overlapped = {};
    data->overlapped.hEvent = data;
    data->change_callback = request->change_callback;
        
    data->next = manager->watchers;
    manager->watchers = data;
    InterlockedIncrement(&manager->num_watchers);

    // we're sleeping in the function we pass to CreateThread, so no need to sleep here; i'm
    // pretty sure you shouldn't sleep in APCs anyways..
    BOOL success = ReadDirectoryChangesW(
        data->dir_handle,
        data->dir_changes_buffer,
        data->dir_changes_buffer_size,
        false,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
        &bytesReturned,
        &data->overlapped,
        file_watcher_completion_routine
    );
    assert(success);

    // if CreateFile fails.. we might be deadlocked.. but let's just hope that doesn't happen
    LeaveCriticalSection(&manager->critical_section);
}

void file_watcher_end_routine(ULONG_PTR param) {
    Win32_Directory_Watcher_End_Request *end_request = (Win32_Directory_Watcher_End_Request *) param;

    Win32_Directory_Watcher_Manager *manager = end_request->manager;
    Win32_Directory_Watcher_Data *prev = NULL;

    EnterCriticalSection(&manager->critical_section);
    Win32_Directory_Watcher_Data *current = manager->watchers;

    // let's assume that end_request's filepath is a wide string already
#if 0
    // MAX_PATH includes null-terminator
    WCHAR wide_end_dir_path[MAX_PATH];
    // TODO: move this conversion to a platform-independent function
    int mb_to_wide_result = MultiByteToWideChar(CP_ACP, 0,
                                                end_request->filepath.contents, end_request->filepath.length,
                                                wide_end_dir_path, MAX_PATH);
    assert(mb_to_wide_result);
#endif

    WCHAR end_dir_c_string[MAX_PATH];
    to_c_string(end_request->filepath, end_dir_c_string, MAX_PATH*sizeof(WCHAR));
    
    WCHAR end_dir_abs_path[MAX_PATH];
    platform_get_absolute_path(end_dir_c_string, end_dir_abs_path, MAX_PATH*sizeof(WCHAR));

    OutputDebugStringA("unwatching: ");
    OutputDebugStringW(end_dir_abs_path);
    OutputDebugStringA("\n");
    
    deallocate(end_request->filepath);
    deallocate((Allocator *) &manager->heap, end_request);

    while (current) {
        if (string_equals(current->dir_abs_path, end_dir_abs_path)) {
            break;
        }
        prev = current;
        current = current->next;
    }

    if (!current) {
        // we didn't find it.. this isn't really an error.
        // it could happen that you try and stop watching, then the end all watchers
        // loop runs before the queue has ran, so we get two end requests for the same
        // filepath.
        LeaveCriticalSection(&manager->critical_section);
        return;
    }
    
    CancelIo(current->dir_handle);
    CloseHandle(current->dir_handle);

    clear_arena(&current->arena);

    if (prev) {
        // remove from list of watchers
        prev->next = current->next;
    } else {
        // was first in list, so set manager->watchers
        manager->watchers = current->next;
    }

    // add to free-list
    current->next_free = manager->first_free_watcher;
    manager->first_free_watcher = current;

    InterlockedDecrement(&manager->num_watchers);
    LeaveCriticalSection(&manager->critical_section);
}

void file_watcher_wake_routine(ULONG_PTR param) {
    // nothing; just to wake up the thread
}

DWORD watch_files_thread_function(void *param) {
    OutputDebugString("watching files...\n");

    Win32_Directory_Watcher_Manager *manager = (Win32_Directory_Watcher_Manager *) param;

#if 0
    // init changes buffer
    dir_watcher_data->dir_changes_buffer_size = MEGABYTES(32);
    dir_watcher_data->dir_changes_buffer = arena_push(&dir_watcher_data->arena,
                                                      dir_watcher_data->dir_changes_buffer_size,
                                                      false, sizeof(DWORD));

    // init thread stack
    uint32 temp_stack_size = KILOBYTES(64);
    void *stack_base = allocate((Allocator *) &dir_watcher_data->arena, temp_stack_size);
    dir_watcher_data->thread_stack = make_stack_allocator(stack_base, temp_stack_size);
    Allocator *thread_stack = (Allocator *) &dir_watcher_data->thread_stack;

    dir_watcher_data->dir_path = make_string("assets");
    Allocator *temp_region = begin_region(thread_stack);

    char *path_c_string = to_char_array(temp_region, dir_watcher_data->dir_path);
    char abs_path_to_watch[MAX_PATH];
    platform_get_absolute_path(path_c_string, abs_path_to_watch, MAX_PATH);

    // create handle to the directory
    dir_watcher_data->dir_handle = CreateFile(abs_path_to_watch,
                                              FILE_LIST_DIRECTORY,
                                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                              NULL,
                                              OPEN_EXISTING,
                                              FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                                              NULL);

    //end_region(temp_region);

    assert(dir_watcher_data->dir_handle != INVALID_HANDLE_VALUE);

    DWORD bytesReturned = 0;

    dir_watcher_data->overlapped = {};
    dir_watcher_data->is_running = true;

    // hEvent is not used by ReadDirectoryChangesW, so store our own data in it that we can use
    // in the completion routine
    dir_watcher_data->overlapped.hEvent = dir_watcher_data;
    
    BOOL success = ReadDirectoryChangesW(
        dir_watcher_data->dir_handle,
        dir_watcher_data->dir_changes_buffer,
        dir_watcher_data->dir_changes_buffer_size,
        false,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
        &bytesReturned,
        &dir_watcher_data->overlapped,
        file_watcher_completion_routine
    );

    assert(success);

    while (dir_watcher_data->is_running) {
        OutputDebugString("SLEEPING");
        SleepEx(INFINITE, true);
        OutputDebugString("AWAKE!!!");
    }

#endif

    while (manager->num_watchers || manager->is_running) {
        SleepEx(INFINITE, true);
        OutputDebugString("awaken from infinite sleep!\n");
    }
    
    OutputDebugString("ending file watching thread...\n");
    
    return 0;
}

void platform_watch_directory(String directory, Directory_Change_Callback change_callback) {
    assert(directory.length <= MAX_PATH);

    // convert the ascii string to a WCHAR (16-bit chars) string
    // before converting, we need to convert the string to be a null-terminated string.
    // MultiByteToWideChar doesn't automatically add a null-terminator if the original
    // string didn't already include one.
    char filepath_c_str[MAX_PATH];
    to_char_array(directory, filepath_c_str, MAX_PATH);

    // convert to absolute path
    char abs_path_c_str[MAX_PATH];
    platform_get_absolute_path(filepath_c_str, abs_path_c_str, MAX_PATH);

    // convert to wide string
    WCHAR wide_filepath[MAX_PATH];
    int mb_to_wide_result = MultiByteToWideChar(CP_UTF8, 0, abs_path_c_str, -1,
                                                wide_filepath, MAX_PATH);
    
    // verify that it's a directory
    bool32 is_directory = platform_path_is_directory(wide_filepath);
    if (!is_directory) {
        assert(!"Path is not a directory!");
        return;
    }
    
    Allocator *heap = (Allocator *) &directory_watcher_manager.heap;
    WString abs_dir = make_wstring(heap, wide_filepath);

    Win32_Directory_Watcher_Start_Request *start_request = (Win32_Directory_Watcher_Start_Request *) allocate(heap,
                                                                    sizeof(Win32_Directory_Watcher_Start_Request));
    *start_request = {
        &directory_watcher_manager,
        abs_dir,
        change_callback
    };
    QueueUserAPC(file_watcher_add_directory_routine, directory_watcher_manager.thread_handle,
                 (ULONG_PTR) start_request);
}

// notice that we know that we need to convert to wide string, because arg is String
// and not WString
void platform_unwatch_directory(String directory) {
    assert(directory.length <= MAX_PATH);

    // convert the ascii string to a WCHAR (16-bit chars) string
    // before converting, we need to convert the string to be a null-terminated string.
    // MultiByteToWideChar doesn't automatically add a null-terminator if the original
    // string didn't already include one.
    char filepath_c_str[MAX_PATH];
    to_char_array(directory, filepath_c_str, MAX_PATH);

    // convert to absolute path
    char abs_path_c_str[MAX_PATH];
    platform_get_absolute_path(filepath_c_str, abs_path_c_str, MAX_PATH);

    // convert to wide string
    WCHAR wide_filepath[MAX_PATH];
    int mb_to_wide_result = MultiByteToWideChar(CP_UTF8, 0, abs_path_c_str, -1,
                                                wide_filepath, MAX_PATH);
    
    // verify that it's a directory
    bool32 is_directory = platform_path_is_directory(wide_filepath);
    if (!is_directory) {
        assert(!"Path is not a directory!");
        return;
    }
    
    Allocator *heap = (Allocator *) &directory_watcher_manager.heap;
    WString abs_dir = make_wstring(heap, wide_filepath);

    Win32_Directory_Watcher_End_Request *end_request = (Win32_Directory_Watcher_End_Request *) allocate(heap,
                                                                                                        sizeof(Win32_Directory_Watcher_End_Request));
    *end_request = {
        &directory_watcher_manager,
        abs_dir
    };
    QueueUserAPC(file_watcher_end_routine, directory_watcher_manager.thread_handle,
                 (ULONG_PTR) end_request);
}

void platform_wide_char_to_multi_byte(WString wstring, char *buffer, int32 buffer_size) {
    int num_bytes_needed = WideCharToMultiByte(CP_UTF8,
                                               0,
                                               wstring.contents,
                                               wstring.length,
                                               NULL,
                                               0,
                                               NULL,
                                               NULL);
    assert(num_bytes_needed);

    // we need + 1 for null-terminator
    if (buffer_size < (num_bytes_needed + 1)) {
        assert(!"Buffer is too small for conversion!");
        return;
    }

    int result = WideCharToMultiByte(CP_UTF8,
                                     0,
                                     wstring.contents,
                                     wstring.length,
                                     buffer,
                                     buffer_size,
                                     NULL,
                                     NULL);
    assert(result);
};

String platform_wide_char_to_multi_byte(Allocator *allocator, WString wstring) {
    int num_bytes_needed = WideCharToMultiByte(CP_UTF8,
                                               0,
                                               wstring.contents,
                                               wstring.length,
                                               NULL,
                                               0,
                                               NULL,
                                               NULL);
    assert(num_bytes_needed);

    char *buffer = (char *) allocate(allocator, num_bytes_needed);
    int result = WideCharToMultiByte(CP_UTF8,
                                     0,
                                     wstring.contents,
                                     wstring.length,
                                     buffer,
                                     num_bytes_needed,
                                     NULL,
                                     NULL);
    assert(result);

    String ret = {};
    ret.contents = buffer;
    // technically, we shouldn't do this because string_length assumes each character is
    // a single byte. but, we only ever use ascii strings, so we should be fine for now..
    ret.length = string_length(buffer);
    ret.allocator = allocator;

    return ret;
};

void test_crash() {
    WCHAR *wide_test = L"C:\\Users\\Ariel\\source\\game\\assets\\animations\\blender_test.animation";

    // it's this absolute_path code...
    //WCHAR abs_path[MAX_PATH];
    //platform_get_absolute_path(wide_test, abs_path, MAX_PATH*sizeof(WCHAR));

#ifdef UNICODE
    printf("unicode");
#endif

#ifdef _UNICODE
    printf("unicode");
#endif
    
    // sizeof(TCHAR) == 1
    // the second arg in GetFullPathNameW is supposed to be the size of the buffer in TCHARs...
    // so i put in MAX_PATH * sizeof(WCHAR), since that's the size in TCHARs... but for whatever reason
    // it results in a buffer overrun...
    int32 tchar_size = sizeof(TCHAR);
    WCHAR absolute_path_buffer[MAX_PATH];
    int32 absolute_path_buffer_size = sizeof(absolute_path_buffer) / sizeof(TCHAR);//MAX_PATH * sizeof(WCHAR);
    DWORD result = GetFullPathNameW(wide_test,
                                    absolute_path_buffer_size, absolute_path_buffer, NULL);
    assert(result);
    
    DWORD file_attributes = GetFileAttributesW(wide_test);
    assert(file_attributes != INVALID_FILE_ATTRIBUTES);

    bool32 is_directory = (file_attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// filename is allocated on the directory watcher's buffer. don't try and
// keep it because it will get overwritten.
// btw, this gets ran on the directory watcher thread..
void watcher_callback(Directory_Change_Type change_type, WString filename) {
    //assert(change_type != DIR_CHANGE_NONE);

    switch (change_type) {
        case DIR_CHANGE_FILE_MODIFIED:
        {
            OutputDebugString("file modified (in callback)\n");
        } break;
    }
}

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd) {

    #if 0
    char *test = "C:\\Users\\Ariel\\source\\game\\assets\\animations\\blender_test.animation";
    
    WCHAR wide_filepath[MAX_PATH];
    int mb_to_wide_result = MultiByteToWideChar(CP_UTF8, 0, test, -1,
                                                wide_filepath, MAX_PATH);
    #endif

#if 0
    test_crash();
#endif
    
#if 0
    Vec3 v = make_vec3(3.0f, -2.0f, 0.0f);
    real32 test = dot(make_vec3(6.0f, -4.0f, 0.0f), normalize(v) / distance(v));
#endif
#if 0
    char path_result[MAX_PATH];
    // DWORD path_length = GetFullPathNameA("C:/dghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijklasdfghijkl", MAX_PATH, path_result, NULL);
#endif

#if 0
{
    OPENFILENAME ofn = {};       // common dialog box structure
    char szFile[260];            // buffer for file name
    HANDLE hf;                   // file handle

// Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFile = szFile;
// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
// use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

// Display the Open dialog box. 

    if (GetOpenFileName(&ofn) == TRUE) {
        hf = CreateFile(ofn.lpstrFile, 
                        GENERIC_READ,
                        0,
                        (LPSECURITY_ATTRIBUTES) NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        (HANDLE) NULL);
        Platform_File file = { hf, 0 };
        platform_close_file(file);
    }
}
#endif

    LARGE_INTEGER perf_counter_frequency_result;
    QueryPerformanceFrequency(&perf_counter_frequency_result);
    perf_counter_frequency = perf_counter_frequency_result.QuadPart;

    int64 last_perf_counter = win32_get_perf_counter();

    UINT desired_min_timer_resolution_ms = 1;
    bool32 sleep_is_granular = timeBeginPeriod(desired_min_timer_resolution_ms) == TIMERR_NOERROR;

    Win32_Display_Output display_output = {};
    display_output.width = 1280;
    display_output.height = 720;

    Win32_Sound_Output sound_output = {};
    sound_output.samples_per_second = SAMPLE_RATE;
    sound_output.bit_depth = 16;
    sound_output.bytes_per_sample = (sound_output.bit_depth / 8) * 2;
    // NOTE: store a 10th of a second (100ms)
    sound_output.buffer_size = sound_output.bytes_per_sample * SOUND_BUFFER_SAMPLE_COUNT;
    sound_output.is_playing = false;
    int16 accumulated_sound_buffer[SOUND_BUFFER_SAMPLE_COUNT * 2];
    sound_output.accumulated_sound_buffer = accumulated_sound_buffer;
    
    char* window_class_name = "WindowClassName";
    WNDCLASSEX window_class = {};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = (WNDPROC) WindowProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = hInstance;
    window_class.hIcon = 0;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    window_class.lpszMenuName = 0;
    window_class.lpszClassName = window_class_name;
    window_class.hIconSm = 0;

    if (RegisterClassEx(&window_class)) {
        // TODO: add fullscreen support and use multi-monitor API for multi-monitor support
        // NOTE: center the window using the primary monitor's dimensions
        int32 device_width = GetSystemMetrics(SM_CXSCREEN);
        int32 device_height = GetSystemMetrics(SM_CYSCREEN);
        int32 window_left = device_width / 2 - display_output.width / 2;
        int32 window_top = device_height / 2 - display_output.height / 2;
        RECT window_rect = {};
        SetRect(&window_rect,
                window_left, window_top,
                window_left + display_output.width, window_top + display_output.height);
        // we use AdjustWindowRect to get the proper window dimensions such that the inner area has our desired dimensions
        AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, 0);
        
        char* window_name = "Window Name";
        window = CreateWindowEx(0,
                                     window_class_name,
                                     window_name,
                                     WS_OVERLAPPEDWINDOW,
                                     window_rect.left, window_rect.top,
                                     window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
                                     0, 0,
                                     hInstance,
                                     0);
        if (window) {
            ShowWindow(window, nShowCmd);
            UpdateWindow(window);
            
            HDC hdc = GetDC(window);
            bool32 opengl_is_valid = win32_init_opengl(hdc);
            bool32 directsound_is_valid = win32_init_directsound(&sound_output);
            win32_init_critical_sections();

            Platform_Critical_Section cs = platform_make_critical_section();
            platform_enter_critical_section(&cs);
            platform_leave_critical_section(&cs);
            platform_delete_critical_section(&cs);

            bool32 memory_is_valid = win32_init_memory();

            MSG message;

            POINT center_point = { display_output.width / 2, display_output.height / 2 };
            ClientToScreen(window, &center_point);
            SetCursorPos(center_point.x, center_point.y);

            POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            ScreenToClient(window, &cursor_pos);
            //cursor_pos.x = cursor_pos.x - (display_output.width / 2);
            //cursor_pos.y = -cursor_pos.y + (display_output.height / 2);
            
            //ShowCursor(0);
            
            if (memory_is_valid && opengl_is_valid && directsound_is_valid) {
                Display_Output initial_display_output = { display_output.width, display_output.height };
                gl_init(&memory.game_data, initial_display_output);

                Sound_Output game_sound_output = {};
                int16 sound_buffer[SOUND_BUFFER_SAMPLE_COUNT * 2];
                game_sound_output.sound_buffer = sound_buffer;
                game_sound_output.buffer_size = sizeof(sound_buffer);
                game_sound_output.max_samples = game_sound_output.buffer_size / sound_output.bytes_per_sample;
                game_sound_output.samples_per_second = 44100;

                game_state = (Game_State *) arena_push(&memory.game_data, sizeof(Game_State), true);
                game_state->is_initted = false;
                game_state->render_state.display_output = initial_display_output;
                render_state = &game_state->render_state;

                uint32 file_watcher_arena_size = MEGABYTES(128);
                void *file_watcher_arena_start = arena_push(&memory.game_data, file_watcher_arena_size);

                uint32 file_watcher_heap_size = MEGABYTES(8);
                void *file_watcher_heap_start = arena_push(&memory.game_data, file_watcher_heap_size);

                directory_watcher_manager = {
                    make_heap_allocator(file_watcher_heap_start, file_watcher_heap_size),
                    make_arena_allocator(file_watcher_arena_start, file_watcher_arena_size)
                };

                directory_watcher_manager.is_running = true;

                InitializeCriticalSection(&directory_watcher_manager.critical_section);

                // TODO: we need to make sure that before we exit this scope, i.e. after the game
                //       loop ends, but before we exit this scope, we end the file watching thread, so
                //       it doesn't try and access these stack variables.
                //file_watcher->allocator = &file_watcher_arena;
                DWORD thread_id;
                HANDLE file_watcher_thread_handle = CreateThread(
                    NULL,
                    0,
                    watch_files_thread_function,
                    &directory_watcher_manager,
                    0,
                    &thread_id);

                assert(file_watcher_thread_handle);

                directory_watcher_manager.thread_handle = file_watcher_thread_handle;

                platform_watch_directory(make_string("assets"), watcher_callback);
                platform_unwatch_directory(make_string("assets"));

#if 0
                Win32_Directory_Watcher_Start_Request start_request = {
                    &directory_watcher_manager,
                    make_string((Allocator *) &directory_watcher_manager.heap, "assets")
                };
                QueueUserAPC(file_watcher_add_directory_routine, file_watcher_thread_handle,
                             (ULONG_PTR) &start_request);
#endif

#if 0
                Win32_Directory_Watcher_End_Request end_request = {
                    &directory_watcher_manager,
                    make_string((Allocator *) &directory_watcher_manager.heap, "assets")
                };
                QueueUserAPC(file_watcher_end_routine, file_watcher_thread_handle,
                             (ULONG_PTR) &end_request);
#endif
                
                using namespace Context;

                Controller_State init_controller_state = {};
                controller_state = &init_controller_state;
                
                controller_state->current_mouse = platform_get_cursor_pos();

                real64 last_time = platform_get_wall_clock_time();
                real64 fps_timer_start = last_time;
                real64 fps_sum = 0.0f;
                int32 num_fps_samples = 1;

                while (is_running) {
                    for (uint32 i = 0; i < array_length(controller_state->key_states); i++) {
                        controller_state->key_states[i].was_down = controller_state->key_states[i].is_down;
                        controller_state->key_states[i].repeat = false;
                    }
                    controller_state->num_pressed_chars = 0;

                    Display_Output game_display_output = { display_output.width, display_output.height };
                    game_state->render_state.display_output = game_display_output;
                    controller_state->last_mouse = controller_state->current_mouse;
                    controller_state->current_mouse = platform_get_cursor_pos();

                    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
                        if (message.message == WM_QUIT) {
                            is_running = false;
                        }

                        UINT message_code = message.message;
                        switch (message.message) {
                            case WM_KEYDOWN:
                            case WM_KEYUP:
                            {
                                bool was_down = (message.lParam & (1 << 30)) != 0;
                                bool is_down = (message.lParam & (1 << 31)) == 0;
                                
                                uint32 vk_code = (uint32) message.wParam;
                                if (vk_code == VK_ESCAPE) {
                                    is_running = false;
                                } else {
                                    // TODO: i'm not sure if we should be using the repeat count in the
                                    // WM_KEYDOWN message.. (bits 0-15)
                                    win32_process_keyboard_input(was_down, is_down,
                                                                 vk_code, (uint32) (message.lParam >> 16),
                                                                 controller_state);
                                }
                            } break;
                            case WM_LBUTTONDOWN:
                            {
                                controller_state->left_mouse.is_down = true;
                            } break;
                            case WM_LBUTTONUP:
                            {
                                controller_state->left_mouse.is_down = false;
                                controller_state->left_mouse.was_down = true;
                            } break;
                            case WM_RBUTTONDOWN:
                            {
                                controller_state->right_mouse.is_down = true;
                            } break;
                            case WM_RBUTTONUP:
                            {
                                controller_state->right_mouse.is_down = false;
                                controller_state->right_mouse.was_down = true;
                            } break;
                            case WM_MBUTTONDOWN:
                            {
                                controller_state->middle_mouse.is_down = true;
                            } break;
                            case WM_MBUTTONUP:
                            {
                                controller_state->middle_mouse.is_down = false;
                                controller_state->middle_mouse.was_down = true;
                            } break;
                            default:
                            {
                                TranslateMessage(&message);
                                DispatchMessage(&message);
                            }
                        }
                    }

                    sound_output.last_write_cursor = sound_output.current_write_cursor;
                    DWORD current_play_cursor, current_write_cursor;
                    if (sound_output.sound_buffer->GetCurrentPosition(&current_play_cursor,
                                                                      &current_write_cursor) == DS_OK) {
                        sound_output.current_play_cursor = current_play_cursor;
                        sound_output.current_write_cursor = current_write_cursor;
#if 0
                        sound_output.markers[sound_output.marker_index].play_cursor = current_play_cursor;
                        sound_output.markers[sound_output.marker_index].write_cursor = current_write_cursor;
#endif
                        real32 audio_latency_samples = ((real32) (current_write_cursor - current_play_cursor) /
                                                        sound_output.bytes_per_sample);
                        real32 audio_latency_ms = audio_latency_samples / sound_output.samples_per_second * 1000.0f;
                        //debug_print("audio latency; samples: %f, ms: %f\n", audio_latency_samples, audio_latency_ms);
                    }

                    // TODO: we may want to base num_samples instead on how much time is remaining for
                    //       the frame. basically just take the delta between the start of the frame and when
                    //       we fill the audio buffer and subtract that from the expected frame time. use the
                    //       difference to calculate how many samples you need.
                    uint32 bytes_delta;
                    if (sound_output.last_write_cursor <= sound_output.current_write_cursor) {
                        bytes_delta = sound_output.current_write_cursor - sound_output.last_write_cursor;
                        uint32 num_samples = bytes_delta / sound_output.bytes_per_sample;
                        //debug_print("num_samples: %d\n", num_samples);    
                    } else {
                        bytes_delta = (sound_output.current_write_cursor +
                                       (sound_output.buffer_size - sound_output.last_write_cursor));
                        //uint32 num_samples = bytes_delta / sound_output.bytes_per_sample;
                        //debug_print("num_samples: %d\n", num_samples);    
                    }
                        
                    uint32 num_samples = bytes_delta / sound_output.bytes_per_sample;

                    update(controller_state,
                           &game_sound_output, num_samples);
                    
                    fill_sound_buffer(&sound_output, game_sound_output.sound_buffer, num_samples);
                    if (!sound_output.is_playing) {
                        sound_output.sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
                        sound_output.is_playing = true;
                    }

                    gl_render(controller_state,
                              &sound_output);

                    reset_debug_state(&game_state->debug_state);
                    //clear_push_buffer(&game_state->ui_manager.push_buffer);
                    clear_arena(&memory.frame_arena);
                    verify(&memory.global_stack);
                    
                    ui_frame_end();
                    r_queue_frame_end();

                    real64 work_time = win32_get_elapsed_time(last_perf_counter);
                    // debug_print("work time before sleep: %f\n", work_time);

                    real32 target_frame_time = 1.0f / TARGET_FRAMERATE;

#if DEBUG_CAP_FRAMERATE
                    if (work_time < target_frame_time) {
                        if (sleep_is_granular) {
                            DWORD sleep_ms = (DWORD) ((target_frame_time - work_time) * 1000);
                            Sleep(sleep_ms);
                        } else {
                            while (work_time < target_frame_time) {
                                work_time = win32_get_elapsed_time(last_perf_counter);
                            }
                        }
                    } else {
                        //debug_print("MISSED FRAME\n");
                        // TODO: logging, missed frame
                    }
#endif
                    
                    //debug_print("frame time: %f\n", work_time);
                    //debug_print("\n");

#if 0
                    sound_output.marker_index++;
                    sound_output.marker_index %= TARGET_FRAMERATE;

                    sound_output.sound_buffer->GetCurrentPosition(&sound_output.markers[sound_output.marker_index].flip_play_cursor,
                                                                  &sound_output.markers[sound_output.marker_index].flip_write_cursor);
#endif
                    
                    SwapBuffers(hdc);

                    last_perf_counter = win32_get_perf_counter();
                }

                // game loop finished

                // TODO: end all of them
                // TODO: we need to have a lock on num_watchers
                // TODO: we need to figure out how we get the thread to end
                // - each thread will decrement the num_watchers
                // - the main thread needs to maybe just Sleep forever and exists when num_watchers = 0?
                // - or we set a flag?
                // - i'm not sure how we're even running right now since we don't do any loop in  the
                //   function of the thread we create..

                /*
                  looping in thread function, infinite sleep (s0)
                  - call add_directory (d1)
                  - call another add_directory (d2)
                  - set manager->is_running to false
                  - call remove_directory(d1), calls CancelIo
                    - completion routine runs, s0 returns
                    - is_running is false, but num_watchers still > 0
                  - call remove_directory(d2)
                    - completion routine runs, s0 returns
                    - TODO (done): which runs first, the completion routine or what's after s0?
                      - if what's after s0 runs first, then the thread ends.. then what happens to the APC queue?
                        - they just don't get called, which i mean, is fine, i think, because we do all the
                          cleanup before the completion routine for CancelIo gets ran
                        - the documentation says "After the thread is in an alertable state, the thread handles all pending APCs in first in, first out (FIFO) order"
                        - that seems like it runs all the queued APCs and then returns from the function that put
                          the thread in an alertable state, i.e. SleepEx() in our case
                        - if we assume that CancelIo works the same way QueueUserAPC does, then the completion
                          routine should run first before returning from SleepEx
                  - anyways, s0 returns, num_watchers is 0 and manager->is running is false, so thread exits

                  what happens if we try and add a directory while we're ending them???
                  - i think it's fine.. we set is_running=false in the critical section, then in the
                    add directory routine, we just verify that it's still running
                  - if we just had a random Queue(add_routine), it would just add it, but we set is_running
                    to false, so any adds after that will be rejected

                 */

                EnterCriticalSection(&directory_watcher_manager.critical_section);

                directory_watcher_manager.is_running = false;
                Win32_Directory_Watcher_Data *current = directory_watcher_manager.watchers;
                
                while (current) {
                    
                    Allocator *heap = (Allocator *) &directory_watcher_manager.heap;
                    Win32_Directory_Watcher_End_Request *end_request = (Win32_Directory_Watcher_End_Request *) allocate(heap,
                                                                                                                        sizeof(Win32_Directory_Watcher_End_Request));

                    // end all the watchers
                    *end_request = {
                        &directory_watcher_manager,
                        copy((Allocator *) &directory_watcher_manager.heap, current->dir_abs_path)
                    };

                    QueueUserAPC(file_watcher_end_routine, file_watcher_thread_handle,
                                 (ULONG_PTR) end_request);
                    current = current->next;
                }

                LeaveCriticalSection(&directory_watcher_manager.critical_section);

                #if 0
                Win32_Directory_Watcher_Start_Request start_request2 = {
                    &directory_watcher_manager,
                    make_string((Allocator *) &directory_watcher_manager.heap, "assets")
                };
                QueueUserAPC(file_watcher_add_directory_routine, file_watcher_thread_handle,
                             (ULONG_PTR) &start_request2);
                #endif

                // queue something, in case we never queued anything, ex. when there
                // are no watchers, we need to wake up the watching the thread so that
                // it can finish execution.
                QueueUserAPC(file_watcher_wake_routine, file_watcher_thread_handle, NULL);

                // wait for file watcher thread to complete
                WaitForSingleObject(file_watcher_thread_handle, INFINITE);

                DeleteCriticalSection(&directory_watcher_manager.critical_section);

                debug_print("exiting program");
            } else {
                // TODO: logging
            }

            // clean up opengl
            wglMakeCurrent(NULL, NULL);
            
            ReleaseDC(window, hdc);
        } else {
            // TODO: logging
        }
    } else {
        // TODO: logging
    }
    
    return 0;
}
