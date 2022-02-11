// TODO (done): open window
// TODO: play sounds with directsound

#include <windows.h>
#include <stdint.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "common.h"

#undef near
#undef far

global_variable bool32 is_running = true;

typedef char GLchar;
typedef signed long long int khronos_ssize_t;
typedef khronos_ssize_t GLsizeiptr;

typedef void GL_GEN_VERTEX_ARRAYS(GLsizei n, GLuint *arrays);
typedef void GL_GEN_BUFFERS (GLsizei n, GLuint *buffers);
typedef void GL_BIND_BUFFER (GLenum target, GLuint buffer);
typedef void GL_BUFFER_DATA (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
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
typedef void GL_GET_SHADER_INFO_LOG(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void GL_GET_SHADERIV(GLuint shader, GLenum pname, GLint *params);
typedef void GL_GET_PROGRAM_INFO_LOG (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void GL_GET_PROGRAMIV (GLuint program, GLenum pname, GLint *params);
typedef void GL_UNIFORM_3FV(GLint location, GLsizei count, const GLfloat *value);
typedef void GL_UNIFORM_4FV(GLint location, GLsizei count, const GLfloat *value);

GL_GEN_VERTEX_ARRAYS *glGenVertexArrays;
GL_GEN_BUFFERS *glGenBuffers;
GL_BIND_BUFFER *glBindBuffer;
GL_BUFFER_DATA *glBufferData;
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
GL_UNIFORM_3FV *glUniform3fv;
GL_UNIFORM_4FV *glUniform4fv;

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
                    glBindBuffer = (GL_BIND_BUFFER *) wglGetProcAddress("glBindBuffer");
                    glBufferData = (GL_BUFFER_DATA *) wglGetProcAddress("glBufferData");
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
                    glUniform3fv = (GL_UNIFORM_3FV *) wglGetProcAddress("glUniform3fv");
                    glUniform4fv = (GL_UNIFORM_4FV *) wglGetProcAddress("glUniform4fv");
                    
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

struct Win32_Display_Output {
    int32 width;
    int32 height;
};


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

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd) {
    UINT desired_min_timer_resolution_ms = 1;
    bool32 sleep_is_granular = timeBeginPeriod(desired_min_timer_resolution_ms) == TIMERR_NOERROR;

    Win32_Display_Output display_output = {};
    display_output.width = 1280;
    display_output.height = 720;
    
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
        HWND window = CreateWindowEx(0,
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
            
            MSG message;

            POINT center_point = { display_output.width / 2, display_output.height / 2 };
            ClientToScreen(window, &center_point);
            SetCursorPos(center_point.x, center_point.y);

            POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            ScreenToClient(window, &cursor_pos);
            cursor_pos.x = cursor_pos.x - (display_output.width / 2);
            cursor_pos.y = -cursor_pos.y + (display_output.height / 2);
            
            ShowCursor(0);
            
            if (opengl_is_valid) {
                while (is_running) {
                    if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
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
                                    // win32_process_keyboard_input(was_down, is_down, vk_code, &controller_state);
                                }
                            } break;
                            case WM_LBUTTONDOWN:
                            {
                            } break;
                            case WM_LBUTTONUP:
                            {
                            } break;
                            case WM_RBUTTONDOWN:
                            {
                            } break;
                            case WM_RBUTTONUP:
                            {
                            } break;
                            case WM_MBUTTONDOWN:
                            {
                            } break;
                            case WM_MBUTTONUP:
                            {
                            } break;
                            default:
                            {
                                TranslateMessage(&message);
                                DispatchMessage(&message);
                            }
                        }
                    }

                    // real32 work_time = win32_get_elapsed_time(last_wall_clock_time, win32_get_wall_clock_time());
                    // real32 target_frame_time = 1.0f / REFRESH_RATE;
                    // if (work_time < target_frame_time) {
                    //     if (sleep_is_granular) {
                    //         DWORD sleep_ms = (DWORD) ((target_frame_time - work_time) * 1000);
                    //         Sleep(sleep_ms);
                    //     }

                    //     while (work_time < target_frame_time) {
                    //         work_time = win32_get_elapsed_time(last_wall_clock_time, win32_get_wall_clock_time());
                    //     }
                    // } else {
                    //     // TODO: logging, missed frame
                    // }
                    
                    SwapBuffers(hdc);
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
