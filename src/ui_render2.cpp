#include "ui.h"

bool32 ui_command_should_coalesce(UI_Render_Command command) {
    // we only coalesce with the last command for now.
    // coalescing with anything before can be confusing.

    if (ui_manager->num_render_commands > 0) {
        int32 last_index = ui_manager->num_render_commands - 1;
        UI_Render_Command *last_command = &ui_manager->render_commands[last_index];

        if (last_command->texture_type != command.texture_type) {
            return false;
        }

        if (last_command->use_scissor) {
            if (command.use_scissor) {
                // if both use scissor regions, make sure that they're equal
                if (last_command->scissor_position != command.scissor_position) {
                    return false;
                } else {
                    // they're equal, so we go on to the texture_type checks
                }
            } else {
                // we need to remove the scissor, so this needs to be a new draw call
                return false;
            }
        } else {
            if (command.use_scissor) {
                // if we aren't using a scissor region, but this command needs one, then
                // we need to start a new scissor
                return false;
            }
        }

        if (last_command->num_indices == 0) {
            // if the command is empty (this happens if we're doing something like just setting a
            // scissor, but not drawing anything), then we can definitely coalesce.
            return true;
        }
        
        switch (command.texture_type) {
            case UI_Texture_Type::UI_TEXTURE_NONE: {
                return true;
            } break;
            case UI_Texture_Type::UI_TEXTURE_IMAGE: {
                assert(!"Not implemented yet");
                //return string_equals(last_command->texture_name, texture_name);
                return false;
            } break;
            case UI_Texture_Type::UI_TEXTURE_FONT: {
                return string_equals(last_command->font_name, command.font_name);
            } break;
            default: {
                assert(!"Unhandled UI_Render_Command texture type");
                return false;
            }
        }
    }

    return false;
}

// TODO: we don't do textures yet
// we just use the command argument here for the command type and the parameter for that
// command type, for example, a font or a texture name.
// we don't use the indices or scissor region members.
void ui_push_command(UI_Render_Command command,
                     UI_Vertex *vertices, int32 num_vertices,
                     uint32 *indices, int32 num_indices) {
    assert(ui_manager->num_render_commands < UI_MAX_RENDER_COMMANDS);

    UI_Render_Data *render_data = &ui_manager->render_data;

    // copy vertices
    assert(render_data->num_vertices + num_vertices < UI_MAX_VERTICES);
    int32 initial_num_vertices = render_data->num_vertices;
    memcpy(&render_data->vertices[render_data->num_vertices], vertices, num_vertices*sizeof(UI_Vertex));
    render_data->num_vertices += num_vertices;

    assert(render_data->num_indices + num_indices < UI_MAX_INDICES);
    int32 indices_start = render_data->num_indices;
    // offset local indices to position in the render_data vertices array
    for (int32 i = 0; i < num_indices; i++) {
        indices[i] += initial_num_vertices;
    }
    // copy indices
    memcpy(&render_data->indices[render_data->num_indices], indices, num_indices*sizeof(uint32));
    render_data->num_indices += num_indices;

    if (ui_command_should_coalesce(command)) {
        // we check this in the should_coalesce function, but just being safe..
        assert(ui_manager->num_render_commands > 0);
        UI_Render_Command *last_command = &ui_manager->render_commands[ui_manager->num_render_commands - 1];

        if (last_command->num_indices == 0) {
            // since the last command isn't drawing anything, then we should just copy the whole command.
            // note that ui_command_should_coalesce() verifies that the scissor regions are the same, so
            // we don't have to worry about overwriting it.
            *last_command = command;
        } else {
            last_command->num_indices += num_indices;
        }
    } else {
        UI_Render_Command *new_command = &ui_manager->render_commands[ui_manager->num_render_commands];
        *new_command = command;
        new_command->indices_start = indices_start;
        new_command->num_indices = num_indices;

        ui_manager->num_render_commands++;
    }
}

void set_scissor(UI_Render_Command *command, UI_Widget *widget) {
    if (widget->scissor_type == UI_SCISSOR_COMPUTED_SIZE) {
        command->use_scissor = true;
        command->scissor_position = make_vec2_int32(widget->computed_position);
        command->scissor_dimensions = make_vec2_int32(widget->computed_size);

        widget->computed_scissor_position = command->scissor_position;
        widget->computed_scissor_dimensions = command->scissor_dimensions;
    } else if (widget->scissor_type == UI_SCISSOR_INHERIT) {
        UI_Widget *current = widget->parent;
        // TODO: we could maybe optimize this by caching the computed scissor positions/dimensions for
        //       the widgets we've already visited that have UI_SCISSOR_INHERIT, idk, we'd have to go
        //       back down after finding the UI_SCISSOR_COMPUTED_SIZE widget.
        while (current) {
            if (current->scissor_type == UI_SCISSOR_COMPUTED_SIZE) {
                command->use_scissor = true;
                command->scissor_position = make_vec2_int32(current->computed_position);
                command->scissor_dimensions = make_vec2_int32(current->computed_size);

                widget->computed_scissor_position = command->scissor_position;
                widget->computed_scissor_dimensions = command->scissor_dimensions;
                return;
            } else if (current->scissor_type == UI_SCISSOR_NONE) {
                assert(!"UI_SCISSOR_INHERIT cannot be used on a widget that does not have a parent with a scissor region.");
                return;
            } else if (current->scissor_type == UI_SCISSOR_INHERIT) {
                // need to keep going until we find what we're inheriting from
            }

            current = current->parent;
        }
        
        assert(!"UI_SCISSOR_INHERIT cannot be used on root or on a widget that does not have a parent without a scissor region.");
    }
}

void ui_render_widget_to_commands(UI_Widget *widget) {
    Vec2 computed_position = widget->computed_position;
    Vec2 computed_size = widget->computed_size;

    // TODO: need to add something here for no flags, i.e. for scissor commands that don't need drawing
    
    // TODO: finish this - see gl_draw_ui_widget() for the rest of the stuff
    if (widget->flags & UI_WIDGET_DRAW_BACKGROUND) {
        // if we're drawing the background, i guess we just ignore the color stuff
        Vec4 color = {};
        if (!(widget->flags & UI_WIDGET_USE_TEXTURE)) {
            // no texture
            color = widget->background_color;
            if (widget->flags & UI_WIDGET_IS_CLICKABLE) {
                real32 transition_time = 0.1f;

                // TODO: the mix needs to be gamma correct
                if (is_hot(widget)) {
                    real32 t = min(ui_manager->hot_t / transition_time, 1.0f);
                    t = 1.0f-(1.0f-t)*(1.0f-t);
                    color = mix(color, widget->hot_background_color, t);
                }

                if (is_active(widget)) {
                    real32 t = min(ui_manager->active_t / transition_time, 1.0f);
                    t = 1.0f-(1.0f-t)*(1.0f-t);
                    color = mix(color, widget->active_background_color, t);
                }
            }
        }
        
        UI_Vertex vertices[4] = {
            {
                { computed_position.x, computed_position.y },
                { 0.0f, 1.0f },
                color,
            },
            {
                { computed_position.x + computed_size.x, computed_position.y },
                { 1.0f, 1.0f },
                color,
            },
            {
                { computed_position.x + computed_size.x, computed_position.y + computed_size.y },
                { 1.0f, 0.0f },
                color,
            },
            {
                { computed_position.x, computed_position.y + computed_size.y },
                { 0.0f, 0.0f },
                color
            }
        };
        uint32 indices[] = { 0, 1, 2, 0, 2, 3 };

        UI_Render_Command command = {};
        if (widget->flags & UI_WIDGET_USE_TEXTURE) {
            assert(widget->texture_name);
            Texture *texture = get_texture(make_string(widget->texture_name));
            assert(texture);

            command.texture_type = UI_Texture_Type::UI_TEXTURE_IMAGE;
            command.texture_name = texture->name;
        } else {
            command.texture_type = UI_Texture_Type::UI_TEXTURE_NONE;
        }
        
        set_scissor(&command, widget);
                
        ui_push_command(command, vertices, 4, indices, 6);
    }

    if (widget->flags & UI_WIDGET_DRAW_TEXT) {
        assert(widget->font);
        Font *font = get_font(widget->font);
        assert(font);

        real32 line_advance = font->scale_for_pixel_height * (font->ascent - font->descent + font->line_gap);

        // initial text_pos is based on bottom left corner and computed_position is top left, so we just
        // add the computed height - line gap to get proper text_pos.y.
        Vec2 text_pos = computed_position;
        text_pos.y = computed_position.y + computed_size.y - (font->scale_for_pixel_height*font->line_gap);
        
        Vec2 initial_text_pos = text_pos;
        
        char *text = widget->text;
        int32 i = 0;
        while (*text) {
            if (*text >= 32 && *text < 128 || *text == '-') {
                stbtt_aligned_quad q;
                stbtt_GetBakedQuad(font->cdata, 512, 512, *text - 32, &text_pos.x, &text_pos.y, &q, 1);

                UI_Vertex vertices[4] = {
                    {
                        { q.x0, q.y0 }, { q.s0, q.t0 }, widget->text_color
                    },
                    {
                        { q.x1, q.y0 }, { q.s1, q.t0 }, widget->text_color
                    },
                    {
                        { q.x1, q.y1 }, { q.s1, q.t1 }, widget->text_color
                    },
                    {
                        { q.x0, q.y1 }, { q.s0, q.t1 }, widget->text_color
                    }
                };
                uint32 indices[] = { 0, 1, 2, 0, 2, 3 };

                // TODO: we should probably extract this command stuff and just put it at the
                //       end of this function. we need to also have some check that makes
                //       sure we aren't adding commands unnecessarily.
                UI_Render_Command command = {};
                command.texture_type = UI_Texture_Type::UI_TEXTURE_FONT;
                command.font_name = font->name;
                set_scissor(&command, widget);

                ui_push_command(command, vertices, 4, indices, 6);
            } else if (*text == '\n') {
                text_pos.x = initial_text_pos.x;
                text_pos.y += line_advance;
            }
        
            text++;
            i++;
        }
    }
}

void ui_create_render_commands() {
    UI_Widget *current = ui_manager->root;

    // depth-first traversal
    while (current) {
        // don't add anything if the current node is root
        if (current != ui_manager->root) {
            //assert(current_command);
            ui_render_widget_to_commands(current);
        }
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                // if there's no sibling, then go up the tree until we find a node
                // that has a sibling or until we hit the root
                if (!current->parent) {
                    break;
                }

                UI_Widget *current_ancestor = current;

                do {
                    current_ancestor = current_ancestor->parent;

                    if (!current_ancestor->parent) {
                        // root
                        // this will break out of outer loop as well, since root->next is NULL
                        break;
                    }
                } while (!current_ancestor->next);

                current = current_ancestor->next;
            }
        }
    }
};

