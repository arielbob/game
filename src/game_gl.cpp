#include "platform.h"
#include "game_gl.h"

real32 vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f,
};

uint32 triangle_shader;
uint32 triangle_vao;

uint32 gl_create_shader(char *shader_source, uint32 shader_source_size, Shader_Type shader_type) {
    GLenum type = 0;
    if (shader_type == VERTEX) {
        type = GL_VERTEX_SHADER;
    } else if (shader_type == FRAGMENT) {
        type = GL_FRAGMENT_SHADER;
    }

    uint32 shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, (const char **) &shader_source, (int32 *) &shader_source_size);
    glCompileShader(shader_id);

    int32 success;
    char buffer[256];
    char error_buffer[256];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(shader_id, 256, NULL, error_buffer);
    if (!success) {
        snprintf(buffer, 256, "%s\n", error_buffer);
        debug_print(error_buffer);
        assert(success);
    }
    
    return shader_id;
}

uint32 gl_compile_and_link_shaders(char *vertex_shader_source, uint32 vertex_shader_source_size,
                                   char *fragment_shader_source, uint32 fragment_shader_source_size) {
    uint32 vertex_shader_id = gl_create_shader(vertex_shader_source, vertex_shader_source_size, VERTEX);
    uint32 fragment_shader_id = gl_create_shader(fragment_shader_source, fragment_shader_source_size, FRAGMENT);
    
    uint32 gl_program_id = glCreateProgram();
    glAttachShader(gl_program_id, vertex_shader_id);
    glAttachShader(gl_program_id, fragment_shader_id);
    glLinkProgram(gl_program_id);
    
    int32 success;
    char error_buffer[256];
    glGetProgramiv(gl_program_id, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(gl_program_id, 256, NULL, error_buffer);
    if (!success) {
        snprintf(error_buffer, 256, "%s\n", error_buffer);
        debug_print(error_buffer);
        assert(success);
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return gl_program_id;
}

void gl_init(Memory *memory, Win32_Display_Output display_output) {
    uint32 vbo;
    glGenVertexArrays(1, &triangle_vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(triangle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glUseProgram(0);

    Marker m = begin_region(memory);

    // NOTE: vertex shader
    Platform_File platform_file;
    bool32 file_exists = platform_open_file("../src/shaders/basic.vs", &platform_file);
    assert(file_exists);

    File_Data vertex_shader_file_data = {};
    vertex_shader_file_data.contents = (char *) region_push(memory, platform_file.file_size);
    bool32 result = platform_read_file(platform_file, &vertex_shader_file_data);
    assert(result);

    platform_close_file(platform_file);

    // NOTE: fragment shader
    file_exists = platform_open_file("../src/shaders/basic.fs", &platform_file);
    assert(file_exists);

    File_Data fragment_shader_file_data = {};
    fragment_shader_file_data.contents = (char *) region_push(memory, platform_file.file_size);
    result = platform_read_file(platform_file, &fragment_shader_file_data);
    assert(result);

    triangle_shader = gl_compile_and_link_shaders(vertex_shader_file_data.contents,
                                                  vertex_shader_file_data.size,
                                                  fragment_shader_file_data.contents,
                                                  fragment_shader_file_data.size);

    end_region(memory, m);
}

void gl_draw_triangle(Win32_Display_Output display_output) {
    glUseProgram(triangle_shader);
    glBindVertexArray(triangle_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
