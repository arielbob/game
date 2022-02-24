#include "platform.h"
#include "game_gl.h"

real32 vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f,
};

void gl_init(Memory *memory, Win32_Display_Output display_output) {
#if 1
    Marker m = begin_region(memory);

    Platform_File platform_file;
    bool32 file_exists = platform_open_file("../src/shaders/basic.vs", &platform_file);
    assert(file_exists);

    File_Data shader_file_data = {};
    shader_file_data.contents = (char *) region_push(memory, platform_file.file_size);
    bool32 result = platform_read_file(platform_file, &shader_file_data);
    assert(result);

    // NOTE: should not trust that contents is null terminated - this is just for testing
    debug_print(shader_file_data.contents);
    
    end_region(memory, m);
    platform_close_file(platform_file);

#endif
    // uint32 file_size = platform_get_file_size("basic.vs");
    // Arena memory = push_memory(file_size);
    // File_Data vertex_shader_source = platform_read_file("basic.vs", memory);
    
    // end_region(m);

    // File_Data vertex_shader_file = platform_read_file("basic.vs");
    // File_Data fragment_shader_file = platform_read_file("fragment.vs");
}

void gl_draw_triangle(Win32_Display_Output display_output) {
    // TODO: draw a triangle in clip space
    //       https://learnopengl.com/Getting-started/Hello-Triangle
    uint32 vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glUseProgram(0);

    // TODO: need to implement file reading to load shaders
    // (platform_get_file and platform_read_file, or maybe we want a different way of doing it)
}
