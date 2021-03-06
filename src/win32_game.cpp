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

#include "win32_game.h"
#include "math.h"
#include "entity.h"
#include "game.h"
#include "memory.h"
#include "platform.h"

global_variable Memory memory;
global_variable Allocator *temp_region;

#include "memory.cpp"
#include "math.cpp"
#include "mesh.cpp"
#include "render.cpp"
#include "font.cpp"
#include "asset.cpp"
#include "ui.cpp"
#include "level.cpp"
#include "entity.cpp"
#include "gizmo.cpp"
#include "editor_actions.cpp"
#include "editor_ui.cpp"
#include "editor.cpp"
#include "game.cpp"

global_variable int64 perf_counter_frequency;
global_variable bool32 is_running = true;
global_variable bool32 is_paused = true;
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

GL_GEN_VERTEX_ARRAYS *glGenVertexArrays;
GL_GEN_BUFFERS *glGenBuffers;
GL_GEN_FRAMEBUFFERS *glGenFramebuffers;
GL_BIND_FRAMEBUFFER *glBindFramebuffer;
GL_BIND_BUFFER *glBindBuffer;
GL_BUFFER_DATA *glBufferData;
GL_BUFFER_SUB_DATA *glBufferSubData;
GL_VERTEX_ATTRIB_POINTER *glVertexAttribPointer;
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

    Marker m = begin_region();
    char *buf = (char *) allocate(temp_region, n);

    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    OutputDebugStringA(buf);
    end_region(m);
    
    va_end(args);
}

void string_format(char *buf, int32 n, char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);
}

char *string_format(Allocator *allocator, char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int32 num_chars_no_null = vsnprintf(NULL, 0, format, args);
    int32 n = num_chars_no_null + 1;
    char *buf = (char *) allocate(allocator, n);

    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    return buf;
}

char *string_format(Allocator *allocator, int32 n, char *format, ...) {
    char *buf = (char *) allocate(allocator, n);
    va_list args;
    va_start(args, format);
    
    int32 num_chars_outputted = vsnprintf(buf, n, format, args);
    assert(num_chars_outputted > 0 && num_chars_outputted < n);

    return buf;
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

    if (was_down != is_down) {
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
                return;
            }
            case VK_DOWN: {
                controller_state->key_down.is_down = is_down;
                controller_state->key_down.was_down = was_down;
                return;
            }
            case VK_RIGHT: {
                controller_state->key_right.is_down = is_down;
                controller_state->key_right.was_down = was_down;
                return;
            }
            case VK_LEFT: {
                controller_state->key_left.is_down = is_down;
                controller_state->key_left.was_down = was_down;
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
    uint32 hash_table_stack_size = MEGABYTES(8);
    uint32 game_data_arena_size = GIGABYTES(1);
    uint32 font_arena_size = MEGABYTES(64);
    uint32 frame_arena_size = MEGABYTES(64);

    uint32 ui_state_heap_size = MEGABYTES(64);
    uint32 editor_arena_size = MEGABYTES(256);

    uint32 total_memory_size = (global_stack_size +
                                hash_table_stack_size +
                                game_data_arena_size +
                                font_arena_size +
                                frame_arena_size +
                                ui_state_heap_size +
                                editor_arena_size);
    void *memory_base = VirtualAlloc(0, total_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (memory_base) {
        void *base = (uint8 *) memory_base;
        Stack_Allocator global_stack = make_stack_allocator(base, global_stack_size);
        memory.global_stack = global_stack;
        base = (uint8 *) base + global_stack_size;

        Stack_Allocator hash_table_stack = make_stack_allocator(base, hash_table_stack_size);
        memory.hash_table_stack = hash_table_stack;
        base = (uint8 *) base + hash_table_stack_size;

        Arena_Allocator game_data_arena = make_arena_allocator(base, game_data_arena_size);
        memory.game_data = game_data_arena;
        base = (uint8 *) base + game_data_arena_size;

        Arena_Allocator font_arena = make_arena_allocator(base, font_arena_size);
        memory.font_arena = font_arena;
        base = (uint8 *) base + font_arena_size;

        Arena_Allocator frame_arena = make_arena_allocator(base, frame_arena_size);
        memory.frame_arena = frame_arena;
        base = (uint8 *) base + frame_arena_size;

        Heap_Allocator ui_state_heap = make_heap_allocator(base, ui_state_heap_size);
        memory.ui_state_heap = ui_state_heap;
        base = (uint8 *) base + ui_state_heap_size;

        Arena_Allocator editor_arena = make_arena_allocator(base, editor_arena_size);
        memory.editor_arena = editor_arena;
        base = (uint8 *) base + editor_arena_size;

        memory.is_initted = true;
        temp_region = (Allocator *) &memory.global_stack;

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

// NOTE: since cursor visibility on windows is actually dependent on a counter that is incremented and decremented
//       by ShowCursor(), you should NOT call this procedure every loop. we only want the counter to
//       either be at 0 or 1 so that we don't have the counter extremely large or small and so that we
//       don't have to loop to increment/decrement it to have a nice API.
void platform_set_cursor_visible(bool32 is_visible) {
    ShowCursor(is_visible);
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

bool32 platform_open_file_dialog(char *filepath, char *filetype_name, char *file_extension_no_dot, uint32 size) {
    OPENFILENAME ofn = {};       // common dialog box structure

    String filetype_name_string = make_string(filetype_name);
    String file_extension_no_dot_string = make_string(file_extension_no_dot);

    char filter_buffer[64];
    // NOTE: we do it this way since string_format is messed up when the string contains null
    //       characters that aren't at the end (the format string uses \0 as a separator).
    String_Buffer working_buffer = make_empty_string_buffer(filter_buffer, sizeof(filter_buffer));
    append_string(&working_buffer, filetype_name_string);
    append_string(&working_buffer, make_string("\0", 1));
    append_string(&working_buffer, make_string("*.", 2));
    append_string(&working_buffer, file_extension_no_dot_string);
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
bool32 platform_open_save_file_dialog(char *filepath, char *filetype_name, char *file_extension_no_dot, uint32 size) {
    assert(filetype_name);
    assert(file_extension_no_dot);

    OPENFILENAME ofn = {};

    String filetype_name_string = make_string(filetype_name);
    String file_extension_no_dot_string = make_string(file_extension_no_dot);

    char filter_buffer[64];
    // NOTE: we do it this way since string_format is messed up when the string contains null
    //       characters that aren't at the end (the format string uses \0 as a separator).
    String_Buffer working_buffer = make_empty_string_buffer(filter_buffer, sizeof(filter_buffer));
    append_string(&working_buffer, filetype_name_string);
    append_string(&working_buffer, make_string("\0", 1));
    append_string(&working_buffer, make_string("*.", 2));
    append_string(&working_buffer, file_extension_no_dot_string);
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
    ofn.lpstrDefExt = file_extension_no_dot;
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

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd) {
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
                GL_State gl_state = {};
                Display_Output initial_display_output = { display_output.width, display_output.height };
                gl_init(&gl_state, initial_display_output);

                Sound_Output game_sound_output = {};
                int16 sound_buffer[SOUND_BUFFER_SAMPLE_COUNT * 2];
                game_sound_output.sound_buffer = sound_buffer;
                game_sound_output.buffer_size = sizeof(sound_buffer);
                game_sound_output.max_samples = game_sound_output.buffer_size / sound_output.bytes_per_sample;
                game_sound_output.samples_per_second = 44100;

                using namespace Context;

                // TODO: we may want to store this in memory
                Game_State init_game_state = {};
                game_state = &init_game_state;
                game_state->is_initted = false;
                game_state->render_state.display_output = initial_display_output;

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
                    }
                    controller_state->num_pressed_chars = 0;

                    Display_Output game_display_output = { display_output.width, display_output.height };
                    game_state->render_state.display_output = game_display_output;
                    controller_state->last_mouse = controller_state->current_mouse;
                    controller_state->current_mouse = platform_get_cursor_pos();

                    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
                        if (message.message == WM_QUIT) {
                            is_running = false;
                            return 0;
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
                    
                    update(game_state,
                           controller_state,
                           &game_sound_output, num_samples);

                    fill_sound_buffer(&sound_output, game_sound_output.sound_buffer, num_samples);
                    if (!sound_output.is_playing) {
                        sound_output.sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
                        sound_output.is_playing = true;
                    }

                    gl_render(&gl_state, game_state,
                              controller_state,
                              &sound_output);

                    reset_debug_state(&game_state->debug_state);
                    clear_push_buffer(&game_state->ui_manager.push_buffer);
                    clear_arena(&memory.frame_arena);
                    verify(&memory.global_stack);

                    real64 work_time = win32_get_elapsed_time(last_perf_counter);
                    // debug_print("work time before sleep: %f\n", work_time);

                    real32 target_frame_time = 1.0f / TARGET_FRAMERATE;
                    if (work_time < target_frame_time) {
                        if (sleep_is_granular) {
                            //DWORD sleep_ms = (DWORD) ((target_frame_time - work_time) * 1000);
                            //Sleep(sleep_ms);
                        } else {
                            while (work_time < target_frame_time) {
                                work_time = win32_get_elapsed_time(last_perf_counter);
                            }
                        }
                    } else {
                        //debug_print("MISSED FRAME\n");
                        // TODO: logging, missed frame
                    }
                    
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
