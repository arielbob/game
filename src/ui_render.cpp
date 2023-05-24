#include "ui.h"

bool32 ui_command_should_coalesce(UI_Render_Command *a, UI_Render_Command *b) {
    if (a->texture_type != b->texture_type) {
        return false;
    }

    switch (a->texture_type) {
        case UI_Texture_Type::UI_TEXTURE_NONE: {
            return true;
        } break;
        case UI_Texture_Type::UI_TEXTURE_IMAGE: {
            return string_equals(a->texture_name, b->texture_name);
        } break;
        case UI_Texture_Type::UI_TEXTURE_FONT: {
            return string_equals(a->font_name, b->font_name);
        } break;
        default: {
            assert(!"Unhandled UI_Render_Command texture type");
            return false;
        }
    }
}

void ui_push_command(UI_Render_Command_List *command_list, UI_Render_Command *command, bool32 maintain_order) {
    // check if we can coalesce
    UI_Render_Command *current;

    if (maintain_order) {
        current = command_list->last;
    } else {
        current = command_list->first;
    }
    
    while (current) {
        if (ui_command_should_coalesce(current, command)) {
            break;
        }

        current = current->next;
    }

    // we always copy data here, since we allocate the command on a temp region
    if (current) {
        // coalesce

        assert(current->num_vertices + command->num_vertices < current->max_vertices);
        assert(current->num_indices + command->num_indices < current->max_indices);

        // offset the indices
        int32 indices_base = current->num_vertices;
        for (int32 i = 0; i < command->num_indices; i++) {
            command->indices[i] += indices_base;
        }
        
        memcpy(&current->vertices[current->num_vertices], command->vertices, command->num_vertices*sizeof(UI_Vertex));
        memcpy(&current->indices[current->num_indices],   command->indices,  command->num_indices*sizeof(int32));

        current->num_vertices += command->num_vertices;
        current->num_indices  += command->num_indices;
    } else {
        UI_Render_Command *new_entry = (UI_Render_Command *) allocate(frame_arena, sizeof(UI_Render_Command));
        *new_entry = *command;
        new_entry->next = NULL;

        assert(command->num_vertices < UI_MAX_VERTICES);
        assert(command->num_indices < UI_MAX_INDICES);
        
        int32 vertex_buffer_size = UI_MAX_VERTICES*sizeof(UI_Vertex);
        int32 index_buffer_size = UI_MAX_INDICES*sizeof(int32);
        if (command->texture_type == UI_Texture_Type::UI_TEXTURE_IMAGE) {
            // TODO: this is kinda ugly, but since we don't coalesce UI_TEXTURE_IMAGE quads, we don't need to
            //       allocate a large buffer
            assert(command->num_vertices == 4);
            assert(command->num_indices == 6);
            vertex_buffer_size = command->num_vertices*sizeof(UI_Vertex);
            index_buffer_size = command->num_indices*sizeof(int32);
        }
        
        new_entry->vertices = (UI_Vertex *) allocate(frame_arena, vertex_buffer_size);
        new_entry->indices  = (uint32 *) allocate(frame_arena, index_buffer_size);
                                       
        memcpy(new_entry->vertices, command->vertices, command->num_vertices*sizeof(UI_Vertex));
        memcpy(new_entry->indices,  command->indices,  command->num_indices*sizeof(int32));
        
        // couldn't coalesce with anything, so we just append command to end of list
        if (command_list->last) {
            command_list->last->next = new_entry;
        } else {
            // command list is empty
            command_list->first = new_entry;
            command_list->last  = new_entry;
        }

        command_list->last = new_entry;
    }
}

// assumes clockwise order of vertices
void ui_push_quad(UI_Render_Command_List *command_list, 
                  Vec2 vertices[4], Vec2 uvs[4], Vec4 color,
                  bool32 maintain_order) {
    UI_Vertex ui_vertices[4];
    for (int32 i = 0; i < 4; i++) {
        ui_vertices[i] = { vertices[i], uvs[i], color };
    }

    uint32 indices[] = { 0, 1, 2, 0, 2, 3 };

    Marker m = begin_region();

    UI_Render_Command *command = (UI_Render_Command *) allocate(temp_region, sizeof(UI_Render_Command));
    command->texture_type = UI_Texture_Type::UI_TEXTURE_NONE;
    command->next = NULL;
    
    command->vertices = ui_vertices;
    command->num_vertices = 4;
    command->max_vertices = UI_MAX_VERTICES;

    command->indices = indices;
    command->num_indices = 6;
    command->max_indices = UI_MAX_INDICES;
    
    ui_push_command(command_list, command, maintain_order);
    end_region(m);
}

void ui_push_text_quad(UI_Render_Command_List *command_list,
                       Vec2 vertices[4], Vec2 uvs[4],
                       Font *font, Vec4 color,
                       bool32 maintain_order) {
    UI_Vertex ui_vertices[4];
    for (int32 i = 0; i < 4; i++) {
        ui_vertices[i] = { vertices[i], uvs[i], color };
    }

    uint32 indices[] = { 0, 1, 2, 0, 2, 3 };

    Marker m = begin_region();

    UI_Render_Command *command = (UI_Render_Command *) allocate(temp_region, sizeof(UI_Render_Command));
    command->texture_type = UI_Texture_Type::UI_TEXTURE_FONT;
    command->font_name = font->name;
    command->next = NULL;
    
    command->vertices = ui_vertices;
    command->num_vertices = 4;
    command->max_vertices = UI_MAX_VERTICES;

    command->indices = indices;
    command->num_indices = 6;
    command->max_indices = UI_MAX_INDICES;
    
    ui_push_command(command_list, command, maintain_order);
    end_region(m);
}

void ui_render_widget_to_group(UI_Render_Group *group, UI_Widget *widget) {
    Vec2 computed_position = widget->computed_position;
    Vec2 computed_size = widget->computed_size;
    
    // TODO: finish this - see gl_draw_ui_widget() for the rest of the stuff
    if (widget->flags & UI_WIDGET_DRAW_BACKGROUND) {
        Vec4 color = widget->background_color;
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
        
        Vec2 vertices[4] = {
            { computed_position.x,                   computed_position.y },
            { computed_position.x + computed_size.x, computed_position.y },
            { computed_position.x + computed_size.x, computed_position.y + computed_size.y },
            { computed_position.x,                   computed_position.y + computed_size.y },
        };
        Vec2 uvs[4] = {
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
            { 1.0f, 0.0f },
            { 0.0f, 0.0f }
        };
        
        ui_push_quad(&group->triangles, vertices, uvs, color, true);
    }

    if (widget->flags & UI_WIDGET_DRAW_TEXT) {
        Font *font = get_font(widget->font);

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

                Vec2 vertices[4] = {
                    { q.x0, q.y0 },
                    { q.x1, q.y0 },
                    { q.x1, q.y1 },
                    { q.x0, q.y1 }
                };
                Vec2 uvs[4] = {
                    { q.s0, q.t0 },
                    { q.s1, q.t0 },
                    { q.s1, q.t1 },
                    { q.s0, q.t1 }
                };

                ui_push_text_quad(&group->text_quads, vertices, uvs, font, widget->text_color, false);
            } else if (*text == '\n') {
                text_pos.x = initial_text_pos.x;
                text_pos.y += line_advance;
            }
        
            text++;
            i++;
        }
    }
}

void ui_create_render_groups() {
    Marker m = begin_region();

    // every direct child of the root is a group
    
    UI_Widget *current = ui_manager->root;

    int32 *num_groups = &ui_manager->num_render_groups;
    UI_Render_Group *groups = ui_manager->render_groups;
    UI_Render_Group *current_group = NULL;

    // depth-first traversal
    while (current) {
        if (current->parent == ui_manager->root) {
            assert(*num_groups < UI_MAX_GROUPS);
            current_group = &groups[*num_groups];
            
            (*num_groups)++;
            *current_group = {};
        }

        // don't add anything if the current node is root
        if (current != ui_manager->root) {
            assert(current_group);
            ui_render_widget_to_group(current_group, current);
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

    ui_manager->render_groups = groups;
    
    end_region(m);
};

#if 0
void ui_create_render_lists() {
    Marker m = begin_region();

    // every direct child of the root is a group
    
    UI_Widget *current = ui_manager->root;

    int32 *num_groups = &ui_manager->num_render_groups;
    UI_Render_Group *groups = ui_manager->render_groups;
    UI_Render_Group *current_group = NULL;
    
    while (current) {
        if (current->parent == ui_manager->root) {
            assert(*num_groups < UI_MAX_GROUPS);
            current_group = &groups[*num_groups];
            
            (*num_groups)++;
            *current_group = {};
        }

        // don't add anything if the current node is root
        if (current != ui_manager->root) {
            assert(current_group);
            ui_render_widget_to_group(current_group, current);
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

    ui_manager->render_groups = groups;
    
    end_region(m);
}
#endif
