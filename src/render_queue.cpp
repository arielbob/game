#include "render.h"

void r_add_command(Command command) {
    Command_Queue *queue = &render_state->command_queue;
    assert(queue->num_commands < MAX_COMMANDS);
    
    queue->commands[queue->num_commands] = command;
}

// font names should always be in read-only memory
void r_load_font(char *font_name) {
    Command command = { Command_Type::LOAD_FONT };
    command.load_font = { font_name };
    r_add_command(command);
}

void r_load_mesh(String mesh_name) {
    assert(mesh_name.allocator == frame_arena);

    Command command = { Command_Type::LOAD_MESH };
    command.load_mesh = { mesh_name };
    r_add_command(command);
}

void r_unload_mesh(String mesh_name) {
    assert(mesh_name.allocator == frame_arena);

    Command command = { Command_Type::UNLOAD_MESH };
    command.unload_mesh = { mesh_name };
    r_add_command(command);
}

void r_load_texture(String texture_name) {
    assert(texture_name.allocator == frame_arena);

    Command command = { Command_Type::LOAD_TEXTURE };
    command.load_texture = { texture_name };
    r_add_command(command);
}

void r_unload_texture(String texture_name) {
    assert(texture_name.allocator == frame_arena);

    Command command = { Command_Type::UNLOAD_TEXTURE };
    command.unload_texture = { texture_name };
    r_add_command(command);
}
