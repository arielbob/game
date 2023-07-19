#include "ui.h"

bool32 ui_command_should_coalesce(UI_Render_Command command) {
    // we only coalesce with the last command for now.
    // coalescing with anything before can be confusing.

    if (ui_manager->num_render_commands > 0) {
        int32 last_index = ui_manager->num_render_commands - 1;
        UI_Render_Command *last_command = &ui_manager->render_commands[last_index];

        if ((command.rendering_mode != last_command->rendering_mode) ||
            command.rendering_mode == UI_Rendering_Mode::TRIANGLE_FAN) {
            return false;
        }
        
        if (command.shader_type != UI_Shader_Type::NONE ||
            last_command->shader_type != UI_Shader_Type::NONE) {
            // we're using uniforms, and it's annoying to have to check those, so just
            // don't coalesce whenever we use a custom shader.
            // also, make sure to check previous because even if the current one is
            // NONE, we don't want to coalesce with the previous and inadvertently
            // use their shader.
            return false;
        }

        if (last_command->texture_type != command.texture_type) {
            return false;
        }

        if (last_command->use_scissor) {
            if (command.use_scissor) {
                // if both use scissor regions, make sure that they're equal
                if ((last_command->scissor_position != command.scissor_position) ||
                    (last_command->scissor_dimensions != command.scissor_dimensions)) {
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

        last_command->num_indices += num_indices;
    } else {
        UI_Render_Command *new_command = &ui_manager->render_commands[ui_manager->num_render_commands];
        *new_command = command;
        new_command->indices_start = indices_start;
        new_command->num_indices = num_indices;

        ui_manager->num_render_commands++;
    }
}

UI_Scissor_Rect get_current_scissor(UI_Scissor_State *scissor_state) {
    assert(scissor_state->num_scissors > 0);
    return scissor_state->stack[scissor_state->num_scissors - 1];
}

void set_scissor(UI_Render_Command *command, UI_Widget *widget,
                 UI_Scissor_State *scissor_state) {
    // note that this currently only gets called if you have the flag UI_WIDGET_DRAW or
    // UI_WIDGET_USE_CUSTOM_SHADER. i.e. only if the widget makes the renderer add
    // vertices.
    
    if (widget->scissor_type == UI_SCISSOR_NONE) {
        command->use_scissor = false;
        return;
    }

    // the current scissor is set based on whether the widget is computed or inherited before
    // this function (set_scissor()) is called
    UI_Scissor_Rect scissor_rect = get_current_scissor(scissor_state);
    
    command->use_scissor = true;
    command->scissor_position = scissor_rect.position;
    command->scissor_dimensions = scissor_rect.dimensions;

    widget->computed_scissor_position = command->scissor_position;
    widget->computed_scissor_dimensions = command->scissor_dimensions;

#if 0
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
                break;
            } else if (current->scissor_type == UI_SCISSOR_NONE) {
                assert(!"UI_SCISSOR_INHERIT cannot be used on a widget that does not have a parent with a scissor region.");
                return;
            } else if (current->scissor_type == UI_SCISSOR_INHERIT) {
                // need to keep going until we find what we're inheriting from
            }

            current = current->parent;
        }

        if (!current) {
            // this should only be reached if we exit the loop because current is null
            assert(!"UI_SCISSOR_INHERIT cannot be used on root or on a widget that does not have a parent without a scissor region.");
            return;
        }
    }
#endif

#if 0
    // bound the scissor rect to be within the window or else weird things happen
    Display_Output *display_output = &game_state->render_state.display_output;

    assert(command->scissor_dimensions.x >= 0);
    assert(command->scissor_dimensions.y >= 0);

    int32 left = command->scissor_position.x;
    int32 right = command->scissor_position.x + command->scissor_dimensions.x;

    int32 top = command->scissor_position.y;
    int32 bottom = command->scissor_position.y + command->scissor_dimensions.y;

    left   = clamp(left, 0, display_output->width);
    right  = clamp(right, 0, display_output->width);
    top    = clamp(top, 0, display_output->height);
    bottom = clamp(bottom, 0, display_output->height);

    assert(right >= left);
    assert(bottom >= top);

    command->scissor_position = { left, top };
    command->scissor_dimensions = { right - left, bottom - top };
#endif
}

void generate_vertices(UI_Widget *widget, Allocator *allocator,
                       UI_Vertex **vertices, int32 *num_vertices,
                       uint32 **indices, int32 *num_indices,
                       Vec4 color,
                       UI_Rendering_Mode *rendering_mode) {
    if ((widget->flags & UI_WIDGET_USE_CUSTOM_SHAPE) && widget->shape_type == UI_Shape_Type::CIRCLE) {
        *num_vertices = 32;
        *vertices = (UI_Vertex *) allocate(allocator, (*num_vertices)*sizeof(UI_Vertex));

        *num_indices = *num_vertices;
        *indices = (uint32 *) allocate(allocator, (*num_indices)*sizeof(uint32));

        *rendering_mode = UI_Rendering_Mode::TRIANGLE_FAN;
        
        real32 angle_delta = 2.0f*PI / (*num_vertices);
        Vec2 center = {
            widget->computed_position.x + widget->computed_size.x / 2.0f,
            widget->computed_position.y + widget->computed_size.y / 2.0f
        };
        real32 y_scale = widget->computed_size.y / widget->computed_size.x;
        real32 radius = widget->computed_size.x / 2.0f;

        for (int32 i = 0; i < *num_vertices; i++) {
            // do 2*PI - delta*i because winding order needs to be clockwise
            real32 unit_x = cosf(2*PI - angle_delta * i);
            real32 x = unit_x * radius;
            real32 unit_y = sinf(2*PI - angle_delta * i);
            real32 y = -unit_y * radius * y_scale; // opposite y coordinate-space
            Vec2 vertex_pos = center + make_vec2(x, y);
            Vec2 uv = make_vec2((unit_x + 1) / 2.0f, (unit_y + 1) / 2.0f);
            (*vertices)[i] = {
                vertex_pos,
                uv,
                color
            };
            (*indices)[i] = i;
        }

    } else {
        *num_vertices = 4;
        *vertices = (UI_Vertex *) allocate(allocator, (*num_vertices)*sizeof(UI_Vertex));

        *num_indices = 6;
        *indices = (uint32 *) allocate(allocator, (*num_indices)*sizeof(uint32));

        Vec2 computed_position = widget->computed_position;
        Vec2 computed_size = widget->computed_size;
        
        (*vertices)[0] = {
            { computed_position.x, computed_position.y },
            { 0.0f, 1.0f },
            color
        };
        (*vertices)[1] = {
            { computed_position.x + computed_size.x, computed_position.y },
            { 1.0f, 1.0f },
            color
        };
        (*vertices)[2] = {
            { computed_position.x + computed_size.x, computed_position.y + computed_size.y },
            { 1.0f, 0.0f },
            color
        };
        (*vertices)[3] = {
            { computed_position.x, computed_position.y + computed_size.y },
            { 0.0f, 0.0f },
            color
        };

        uint32 indices_to_copy[] = { 0, 1, 2, 0, 2, 3 };
        memcpy(*indices, indices_to_copy, *num_indices * sizeof(uint32));

        *rendering_mode = UI_Rendering_Mode::TRIANGLES;
    }
}

void ui_render_widget_to_commands(UI_Widget *widget, UI_Scissor_State *scissor_state) {
    Vec2 computed_position = widget->computed_position;
    Vec2 computed_size = widget->computed_size;

    // we don't need to add anything here for when flags is 0, i.e when we don't draw anything and only
    // have a scissor region, because we only need scissors when we actually draw something, and when we
    // draw, that'll automatically visit the widgets with scissors to inherit from them and set the
    // scissor correctly. i'm pretty sure we won't ever need to scissor without drawing. scissoring is
    // handled for user input before rendering, i.e. we don't have to wait for rendering for the scissor
    // to affect interaction, so i'm pretty sure this is fine.

    // ok, instead we can update scissor_state based on the widget, then set_scissor() sets it on
    // the command.
    // the GL drawing is really basic and basically just sets the scissor if the command has use_scissor
    // and sets the scissor to whatever's in it. we might be able to make it so the GL code knows about
    // the scissor stack so we don't scissor unnecessarily, but the way we're doing it now seems simpler
    // for now.
    
    // TODO: finish this - see gl_draw_ui_widget() for the rest of the stuff

    if (widget->flags & UI_WIDGET_USE_CUSTOM_SHADER) {
        UI_Render_Command command = {};

        command.shader_type = widget->shader_type;
        command.shader_uniforms = widget->shader_uniforms;

        Allocator *temp_region = begin_region();
        UI_Vertex *vertices;
        int32 num_vertices, num_indices;
        uint32 *indices;
        UI_Rendering_Mode rendering_mode;

        generate_vertices(widget, temp_region,
                          &vertices, &num_vertices,
                          &indices, &num_indices,
                          {},
                          &rendering_mode);

#if 0
        UI_Vertex vertices[4] = {
            {
                { computed_position.x, computed_position.y },
                { 0.0f, 1.0f },
                {}
            },
            {
                { computed_position.x + computed_size.x, computed_position.y },
                { 1.0f, 1.0f },
                {}
            },
            {
                { computed_position.x + computed_size.x, computed_position.y + computed_size.y },
                { 1.0f, 0.0f },
                {}
            },
            {
                { computed_position.x, computed_position.y + computed_size.y },
                { 0.0f, 0.0f },
                {}
            }
        };
        uint32 indices[] = { 0, 1, 2, 0, 2, 3 };
#endif

        command.shader_type = widget->shader_type;
        command.shader_uniforms = widget->shader_uniforms;

        set_scissor(&command, widget, scissor_state);

        command.rendering_mode = rendering_mode;
        
        ui_push_command(command, vertices, num_vertices, indices, num_indices);

        end_region(temp_region);
    } else if (widget->flags & UI_WIDGET_DRAW_BACKGROUND) {
        UI_Render_Command command = {};
        
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

        Allocator *temp_region = begin_region();
        UI_Vertex *vertices;
        int32 num_vertices, num_indices;
        uint32 *indices;
        UI_Rendering_Mode rendering_mode;

        generate_vertices(widget, temp_region,
                          &vertices, &num_vertices,
                          &indices, &num_indices,
                          color,
                          &rendering_mode);

        if (widget->flags & UI_WIDGET_USE_TEXTURE) {
            assert(widget->texture_name);
            Texture *texture = get_texture(make_string(widget->texture_name));
            assert(texture);

            command.texture_type = UI_Texture_Type::UI_TEXTURE_IMAGE;
            command.texture_name = texture->name;
        } else {
            command.texture_type = UI_Texture_Type::UI_TEXTURE_NONE;
        }
        
        set_scissor(&command, widget, scissor_state);

        command.rendering_mode = rendering_mode;
        
        ui_push_command(command, vertices, num_vertices, indices, num_indices);

        end_region(temp_region);
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
                set_scissor(&command, widget, scissor_state);

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

LRTB_Rect_int32 get_lrtb_rect(UI_Scissor_Rect scissor_rect) {
    assert(!scissor_rect.empty);
    assert(scissor_rect.dimensions.x >= 0);
    assert(scissor_rect.dimensions.y >= 0);
    
    int32 left = scissor_rect.position.x;
    int32 right = left + scissor_rect.dimensions.x;

    int32 top = scissor_rect.position.y;
    int32 bottom = top + scissor_rect.dimensions.y;

    return { left, right, top, bottom };
};

void push_scissor(UI_Scissor_State *scissor_state, Vec2_int32 position, Vec2_int32 dimensions) {
    assert(scissor_state->num_scissors < UI_MAX_SCISSOR_STATES);

    UI_Scissor_Rect rect_to_add = {};

    bool cut_rect = false;
    if (scissor_state->num_scissors > 0) {
        UI_Scissor_Rect current_scissor = get_current_scissor(scissor_state);
        if (!current_scissor.empty) {
            cut_rect = true;
        }
    }
    
    if (cut_rect) {
        // cut into the current one
        LRTB_Rect_int32 current_rect = get_lrtb_rect(get_current_scissor(scissor_state));
        LRTB_Rect_int32 new_rect = get_lrtb_rect({ false, position, dimensions });

        int32 left = max(current_rect.left, new_rect.left);
        int32 right = min(current_rect.right, new_rect.right);

        int32 top = max(current_rect.top, new_rect.top);
        int32 bottom = min(current_rect.bottom, new_rect.bottom);

        if (left > right || top > bottom) {
            // the rects aren't intersecting at all, so just set everything to 0
            rect_to_add.position = {};
            rect_to_add.dimensions = {};
        } else {
            rect_to_add.position = { left, top };
            rect_to_add.dimensions = { right - left, bottom - top };
        }
    } else {
        // we either don't have a scissor, or the current scissor is an empty one,
        // so just push whatever was passed in.
        rect_to_add.position = position;
        rect_to_add.dimensions = dimensions;
    }

    scissor_state->stack[scissor_state->num_scissors++] = rect_to_add;
}

void push_null_scissor(UI_Scissor_State *scissor_state) {
    assert(scissor_state->num_scissors < UI_MAX_SCISSOR_STATES);
    scissor_state->stack[scissor_state->num_scissors++] = { true, {}, {} };
}

void pop_scissor(UI_Scissor_State *scissor_state) {
    assert(scissor_state->num_scissors > 0);
    scissor_state->num_scissors--;
}

void pop_scissor_if_necessary(UI_Scissor_State *scissor_state, UI_Widget *widget) {
    if (widget->scissor_type != UI_SCISSOR_INHERIT) {
        pop_scissor(scissor_state);
    }
}

void ui_create_render_commands() {
    UI_Scissor_State scissor_state = {};

    Display_Output *display_output = &game_state->render_state.display_output;
    push_scissor(&scissor_state, {}, { display_output->width, display_output->height });
    
    // TODO: what if the default scissor state is UI_SCISSOR_INHERIT?
    // - but what if we want to not scissor something?
    // - i guess UI_SCISSOR_NONE can put in a special entry
    //   - and then any UI_SCISSOR_INHERIT's will also not be scissored
    // if it's computed, we push onto the stack
    // if it's inherited, we get the last on the stack
    // if it's none, we push a special "none" state on the stack
    // when we go up, if it was computed or none, we pop off the stack
    // if it's NONE, then COMPUTED, uh, maybe just assert?
    // if it's COMPUTED, then COMPUTED, we get the intersected area between the two
    // if it's (NONE | COMPUTED), then INHERIT, we just get last on the stack
    
    UI_Widget *current = ui_manager->root;

    int32 num_visited = 0;
    
    // depth-first traversal (pre-order)
    while (current) {
        current->rendering_index = num_visited;
        num_visited++;

        bool32 skip_widget_and_children = false;

        if (current->flags & UI_WIDGET_HIDE) {
            skip_widget_and_children = true;
        }
        
        // don't add anything if the current node is root
        if (current != ui_manager->root && !skip_widget_and_children) {
            if (current->scissor_type == UI_SCISSOR_COMPUTED_SIZE) {
                push_scissor(&scissor_state,
                             make_vec2_int32(current->computed_position),
                             make_vec2_int32(current->computed_size));
            } else if (current->scissor_type == UI_SCISSOR_NONE) {
                push_null_scissor(&scissor_state);
            }

            ui_render_widget_to_commands(current, &scissor_state);
        }
        
        if (current->first && !skip_widget_and_children) {
            current = current->first;
        } else {
            if (current->next) {
                // we're done with current, so pop its scissor if it pushed one
                if (!skip_widget_and_children) {
                    pop_scissor_if_necessary(&scissor_state, current);
                }
                current = current->next;
            } else {
                // if there's no sibling, then go up the tree until we find a node
                // that has a sibling or until we hit the root
                if (!current->parent) {
                    break;
                }

                UI_Widget *current_ancestor = current;

                do {
                    // note that we do this before we go up
                    if (!skip_widget_and_children) {
                        pop_scissor_if_necessary(&scissor_state, current_ancestor);
                    }

                    current_ancestor = current_ancestor->parent;

                    // we only ever visit the hidden node itself and none if its children, so if
                    // we go up, we have to set skip_widget_and_children to false
                    skip_widget_and_children = false;
                    
                    if (!current_ancestor->parent) {
                        // root
                        // this will break out of outer loop as well, since root->next is NULL
                        break;
                    }
                } while (!current_ancestor->next);

                pop_scissor_if_necessary(&scissor_state, current_ancestor);
                current = current_ancestor->next;
            }
        }
    }

    assert(scissor_state.num_scissors == 1); // the window scissor
};
