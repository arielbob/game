#include "render.h"

void r_add_command(Command command) {
    Command_Queue *queue = &render_state->command_queue;
    assert(queue->num_commands < MAX_COMMANDS);
    
    queue->commands[queue->num_commands++] = command;
}

void r_load_font(String font_name) {
    font_name = copy(frame_arena, font_name);
    
    Command command = { Command_Type::LOAD_FONT };
    command.load_font = { font_name };
    r_add_command(command);
}

void r_load_mesh(int32 mesh_id) {
    Command command = { Command_Type::LOAD_MESH };
    command.load_mesh = { mesh_id };
    r_add_command(command);
}

void r_unload_mesh(int32 mesh_id) {
    Command command = { Command_Type::UNLOAD_MESH };
    command.unload_mesh = { mesh_id };
    r_add_command(command);
}

void r_load_texture(int32 texture_id) {
    Command command = { Command_Type::LOAD_TEXTURE };
    command.load_texture = { texture_id };
    r_add_command(command);
}

void r_unload_texture(int32 texture_id) {
    Command command = { Command_Type::UNLOAD_TEXTURE };
    command.unload_texture = { texture_id };
    r_add_command(command);
}

void r_queue_frame_end() {
    Command_Queue *queue = &render_state->command_queue;
    queue->num_commands = 0;
}
