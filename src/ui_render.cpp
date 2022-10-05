#include "ui.h"

void ui_push_draw_command(UI_Draw_Command command) {
    assert(ui_manager->num_draw_commands < UI_MAX_DRAW_COMMANDS);
    ui_manager->draw_commands[ui_manager->num_draw_commands++] = command;
}

void ui_reset_render_group(UI_Render_Group *group) {
    group->num_vertices = 0;
    group->num_indices = 0;
}

void ui_push_triangle_list(UI_Render_Group *group,
                           UI_Vertex *vertices, int32 num_vertices,
                           uint32 *indices,     int32 num_indices) {
    assert(num_indices % 3 == 0); // make sure we're doing triangles
    int32 indices_base = ui_manager->num_vertices;

    assert(group->num_vertices + num_vertices <= UI_MAX_VERTICES);
    assert(group->num_indices + num_indices <= UI_MAX_INDICES);
    
    memcpy(&group->vertices[group->num_vertices], vertices, num_vertices*sizeof(UI_Vertex));
    group->num_vertices += num_vertices;

    // TODO: try also adding and copying at the same time?
    for (int32 i = 0; i < num_indices; i++) {
        // assert(indices[i] < num_vertices);
        indices[i] += indices_base;
    }

    memcpy(&group->indices[group->num_indices], indices, num_indices*sizeof(uint32));
    group->num_indices += num_indices;
}

// assumes clockwise order of vertices
void ui_push_quad(UI_Render_Group *group, Vec2 vertices[4], Vec2 uvs[4], Vec4 color) {
    UI_Vertex ui_vertices[4];
    for (int32 i = 0; i < 4; i++) {
        ui_vertices[i] = { vertices[i], uvs[i], color };
    }

    uint32 indices[] = { 0, 1, 2, 0, 2, 3 };
    ui_push_triangle_list(group, ui_vertices, 4, indices, 6);
}

// pushes render group data and creates and pushes a render command to draw the group
void ui_push_render_group(UI_Render_Group *group) {
    if (group->num_vertices == 0) return;
    
    // indices in group are based off of vertices array. we save the number of vertices before
    // adding the vertices so that we can offset the indices so that they're relative to the
    // ui_manager's vertices array that we'are appending to.
    int32 indices_offset = ui_manager->num_vertices;
    
    // copy vertices
    assert(ui_manager->num_vertices + group->num_vertices <= UI_MAX_VERTICES);
    memcpy(&ui_manager->vertices[ui_manager->num_vertices], group->vertices,
           group->num_vertices*sizeof(UI_Vertex));
    ui_manager->num_vertices += group->num_vertices;

    // add offset to indices
    for (int32 i = 0; i < group->num_indices; i++) {
        group->indices[i] += indices_offset;
    }
    
    // copy indices
    int32 indices_start = ui_manager->num_indices;
    assert(ui_manager->num_indices + group->num_indices <= UI_MAX_INDICES);
    memcpy(&ui_manager->indices[ui_manager->num_indices], group->indices,
           group->num_indices*sizeof(uint32));
    ui_manager->num_indices += group->num_indices;

    // create draw command
    UI_Draw_Command command;
    command.texture_type = group->texture_type;
    switch (command.texture_type) {
        case UI_Texture_Type::UI_TEXTURE_NONE: {} break;
        case UI_Texture_Type::UI_TEXTURE_IMAGE: {
            command.texture_name = copy(frame_arena, group->texture_name);
        } break;
        case UI_Texture_Type::UI_TEXTURE_FONT: {
            command.font_name = copy(frame_arena, group->font_name);
        } break;
        default: {
            assert(!"Invalid UI texture type.");
        } break;
    }
    command.num_indices = group->num_indices;
    command.indices_start = indices_start;

    ui_push_draw_command(command);
}

void ui_render_widget_to_groups(UI_Render_Group *quads, UI_Render_Group *text_quads,
                                UI_Widget *widget) {
    // TODO: finish this - see gl_draw_ui_widget() for the rest of the stuff
    if (widget->flags & UI_WIDGET_DRAW_BACKGROUND) {
        Vec4 color = widget->background_color;
        if (widget->flags & UI_WIDGET_IS_CLICKABLE) {
            real32 transition_time = 0.1f;
            
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

        Vec2 computed_position = widget->computed_position;
        Vec2 computed_size = widget->computed_size;
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
        
        ui_push_quad(quads, vertices, uvs, color);
    }
}

/*
- root children are sorted from back to front
- no need for ordering within a window because widgets inside a window can never be floating, and also they
already will be in traversed in the correct render order
- within a window, text will always be on top of quads. so we can group by quads and text, and render in that
order.
- for textured quads, we should basically just the group around the textured quad. we can't group by textured
quads and render them after because other quads can be rendered on top of the textured quad.
- for now, we can just group by quads and single font. we don't need to handle multiple fonts yet. we don't even
need to handle textured quads yet.


we assume that a depth first search from left to right on a widget tree is render-order, i.e. from back to front.
we assume that the ui manager sorts the widgets in this way before calling ui_create_render_lists().
we group by window: [quads text] [quads text]

 */

void ui_create_render_lists() {
    Marker m = begin_region();

    // TODO: for each window, create two UI render groups: one for quads and one for glyph quads
    //       - in the future we may want to dynamically create these render groups so we can have more groups per
    //         window, like multiple fonts/quads with different textures.
    //
    //       - for both the render groups, copy its vertices to the UI vertex buffer. for the indices, go through
    //         the render group's indices array and add the total amount of vertices in the UI vertex buffer so
    //         that the indices are correctly offset. then, copy the render group's indices to the index buffer.
    //         (we could maybe try adding and setting? but i think memcpy might be faster and adding to each
    //         index will probably be auto-vectorized.)
    //
    //       - for each render group, create a UI_Draw_Command struct that has the render group info and the
    //         indices start and end index within the UI's indices buffer.
    //
    //       - delete the UI_Render_Group structs
    // 
    //       - we don't do textured quads yet; only if they're textured with a font, and we only handle a single
    //         fontt
    
    // TODO: render the UI with the array of UI_Draw_Command structs

    // every direct child of the root is a group
    
    UI_Widget *current = ui_manager->root;

    bool32 inside_group = false;
    UI_Render_Group *group_quads      = (UI_Render_Group *) allocate(temp_region, sizeof(UI_Render_Group));
    UI_Render_Group *group_text_quads = (UI_Render_Group *) allocate(temp_region, sizeof(UI_Render_Group));
    
    while (current) {
        if (current->parent == ui_manager->root) {
            if (inside_group) {
                ui_push_render_group(group_quads);
                ui_push_render_group(group_text_quads);

                ui_reset_render_group(group_quads);
                ui_reset_render_group(group_text_quads);
            }

            inside_group = true;
        }

        // don't add anything if the current node is root
        // TODO: we could probably avoid this if we just start at root->first?
        if (current != ui_manager->root) {
            // TODO: push quad to text group

            ui_render_widget_to_groups(group_quads, group_text_quads, current);
        }
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
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

    if (inside_group) {
        // add last render group if we finished inside one
        ui_push_render_group(group_quads);
        ui_push_render_group(group_text_quads);

        ui_reset_render_group(group_quads);
        ui_reset_render_group(group_text_quads);

        inside_group = false;
    }

    end_region(m);
}

#if 0
void ui_push_triangle_list(UI_Vertex *vertices, int32 num_vertices,
                           uint32 *indices,     int32 num_indices) {
    assert(num_indices % 3 == 0);
    int32 num_triangles = num_indices / 3;

    int32 indices_base = ui_manager->num_vertices;
    for (int32 i = 0; i < num_vertices; i++) {
        assert(ui_manager->num_vertices < UI_MAX_VERTICES);
        ui_manager->vertices[ui_manager->num_vertices++] = vertices[i];
    }

    for (int32 i = 0; i < num_triangles; i++) {
        assert((ui_manager->num_indices / 3) < UI_MAX_TRIANGLES);
        // convert the indices to be relative to the ui_manager's vertex list instead of relative to
        // the vertices argument
        uint32 i1 = indices_base + indices[3*i + 0];
        uint32 i2 = indices_base + indices[3*i + 1];
        uint32 i3 = indices_base + indices[3*i + 2];
        assert(i1 < (uint32) num_vertices);
        assert(i2 < (uint32) num_vertices);
        assert(i3 < (uint32) num_vertices);
        ui_manager->indices[ui_manager->num_indices++] = i1;
        ui_manager->indices[ui_manager->num_indices++] = i2;
        ui_manager->indices[ui_manager->num_indices++] = i3;
    }
}
#endif

/*
we go through the UI widget tree and for each window, we create lists of basic quads, textured quads, and font quads
and we push UI_Draw_Commands for all the different groups.
we can't really put the draw commands in ui_push_quad or whatever. it would have to be in a grouping function.
 */

#if 0
void ui_push_quad(Vec2 vertices[4], Vec2 uvs[4], Vec4 color) {
    UI_Vertex ui_vertices[4];
    for (int32 i = 0; i < 4; i++) {
        ui_vertices[i] = { vertices[i], uvs[i], color };
    }

    int32 num_triangles = 2;
    uint32 indices[] = { 0, 1, 2, 0, 2, 3 };
    ui_push_triangle_list(ui_vertices, 4, indices, num_triangles*3);
}
#endif
