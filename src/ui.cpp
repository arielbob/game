#include "ui.h"

UI_Widget *make_widget(UI_id id, uint32 flags) {
    UI_Widget *widget = (UI_Widget *) allocate(&ui_manager->frame_arena, sizeof(UI_Widget));

    *widget = {};
    widget->id = id;
    widget->flags = flags;

    // TODO: just have initial values for these, so we don't have to do these checks
    if (ui_manager->background_color_stack) {
        widget->background_color = ui_manager->background_color_stack->background_color;
    }

    if (ui_manager->hot_background_color_stack) {
        widget->hot_background_color = ui_manager->hot_background_color_stack->background_color;
    }

    if (ui_manager->active_background_color_stack) {
        widget->active_background_color = ui_manager->active_background_color_stack->background_color;
    }
    
    if (ui_manager->size_stack) {
        widget->semantic_size = ui_manager->size_stack->size;
    }

    if (ui_manager->position_stack) {
        widget->semantic_position = ui_manager->position_stack->position;
    }

    if (ui_manager->position_type_stack) {
        widget->position_type = ui_manager->position_type_stack->type;
    }

    if (ui_manager->layout_type_stack) {
        widget->layout_type = ui_manager->layout_type_stack->type;
    }
    
    if (ui_manager->size_type_stack) {
        widget->size_type = ui_manager->size_type_stack->type;
    }

    if (ui_manager->font_stack) {
        widget->font = ui_manager->font_stack->font;
    }
    
    if (ui_manager->text_color_stack) {
        widget->text_color = ui_manager->text_color_stack->color;
    }
    
    return widget;
}

uint32 get_hash(UI_id id, uint32 bucket_size) {
    // don't use the pointer here for hashing, since that'll always be even
    String_Iterator it = make_string_iterator(make_string(id.string_ptr));
    uint32 sum = 0;
    char c = get_next_char(&it);
    while (c) {
        sum += c;
        c = get_next_char(&it);
    }
    sum += id.index;

    it = make_string_iterator(make_string(id.parent_string_ptr));
    c = get_next_char(&it);
    while (c) {
        sum += c;
        c = get_next_char(&it);
    }
    sum += id.parent_index;
    
    uint32 hash = sum % bucket_size;

    #if 0
    if (id.parent) {
        hash += get_hash(id.parent->id, bucket_size);
        hash %= bucket_size;
    }
    #endif
    
    return hash;
}

UI_Widget *ui_table_get(UI_Widget **table, UI_id id) {
    uint32 hash = get_hash(id, NUM_WIDGET_BUCKETS);

    UI_Widget *current = table[hash];
    while (current) {
        if (ui_id_equals(current->id, id)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void ui_table_add(UI_Widget **table, UI_Widget *widget) {
    if (!widget->id.string_ptr) return;
    
    uint32 hash = get_hash(widget->id, NUM_WIDGET_BUCKETS);

    UI_Widget *current = table[hash];
    if (!current) {
        widget->table_next = NULL;
        table[hash] = widget;
        return;
    }
    
    while (true) {
        if (ui_id_equals(current->id, widget->id)) {
            assert(!"Widget already exists in table.");
            return;
        }

        if (!current->table_next) {
            current->table_next = widget;
            widget->table_next = NULL;
            return;
        }

        current = current->table_next;
    }
}

void ui_table_clear(UI_Widget **table) {
    for (int32 i = 0; i < NUM_WIDGET_BUCKETS; i++) {
        table[i] = NULL;
    }
}

UI_Widget *ui_add_widget(UI_Widget *widget) {
    assert(ui_manager->widget_stack); // ui should be initted with a root node (call ui_frame_init)
    
    UI_Widget *parent = ui_manager->widget_stack->widget;
    widget->id.parent_string_ptr = parent->id.string_ptr;
    widget->id.parent_index      = parent->id.index;
    widget->parent = parent;

    if (widget->position_type != UI_POSITION_FLOAT) {
        if (widget->size_type[UI_WIDGET_X_AXIS] != UI_SIZE_FILL_REMAINING) {
            parent->num_sized_children++;
        } else {
            parent->num_fill_children++;
        }
    }
    
    if (!parent->last) {
        assert(!parent->first);
        parent->first = widget;
        parent->last = widget;
    } else {
        parent->last->next = widget;
        widget->prev = parent->last;
        parent->last = widget;
    }

    ui_table_add(ui_manager->widget_table, widget);

    return widget;
}

// NOTE: both ui_add_widget and ui_push_widget add widgets to the hierarchy. the only difference is that
//       ui_push_widget also adds the widget to the widget stack, so that next calls to ui_add_widget or
//       ui_push_widget will have their widget added as a child of the widget at the top of the widget
//       stack.
UI_Widget *ui_add_widget(UI_id id, uint32 flags) {
    UI_Widget *widget = make_widget(id, flags);
    ui_add_widget(widget);

    return widget;
}

UI_Widget *ui_add_widget(char *id_string_ptr, uint32 flags = 0) {
    return ui_add_widget(make_ui_id(id_string_ptr), flags);
}

UI_Widget *ui_push_widget(UI_Widget *widget) {
    // push to the stack
    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&ui_manager->frame_arena, sizeof(UI_Stack_Widget));

    entry->widget = widget;
    entry->next = ui_manager->widget_stack;
    
    ui_manager->widget_stack = entry;

    return widget;
}

UI_Widget *ui_push_widget(UI_id id, uint32 flags = 0) {
    UI_Widget *widget = make_widget(id, flags);
    ui_add_widget(widget);

    return ui_push_widget(widget);
}

UI_Widget *ui_push_widget(char *id_string_ptr, uint32 flags = 0) {
    return ui_push_widget(make_ui_id(id_string_ptr), flags);
}

void ui_pop_widget() {
    assert(ui_manager->widget_stack);
    ui_manager->widget_stack = ui_manager->widget_stack->next;
}

// TODO: layout types, and calculating positions (should be done after size calculations)
// TODO: stack popping procedures

void ui_push_position(Vec2 position) {
    UI_Style_Position *entry = (UI_Style_Position *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_Position));

    entry->position = position;
    entry->next = ui_manager->position_stack;
    
    ui_manager->position_stack = entry;
}

void ui_push_position_type(UI_Position_Type type) {
    UI_Style_Position_Type *entry = (UI_Style_Position_Type *) allocate(&ui_manager->frame_arena,
                                                                        sizeof(UI_Style_Position_Type));

    entry->type = type;
    entry->next = ui_manager->position_type_stack;
    
    ui_manager->position_type_stack = entry;
}

void ui_push_size(Vec2 size) {
    UI_Style_Size *entry = (UI_Style_Size *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_Size));

    entry->size = size;
    entry->next = ui_manager->size_stack;
    
    ui_manager->size_stack = entry;
}

void ui_push_background_color(Vec4 color) {
    UI_Style_BG_Color *entry = (UI_Style_BG_Color *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_BG_Color));

    entry->background_color = color;
    entry->next = ui_manager->background_color_stack;
    
    ui_manager->background_color_stack = entry;
}

void ui_push_hot_background_color(Vec4 color) {
    UI_Style_BG_Color *entry = (UI_Style_BG_Color *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_BG_Color));

    entry->background_color = color;
    entry->next = ui_manager->hot_background_color_stack;
    
    ui_manager->hot_background_color_stack = entry;
}

void ui_push_active_background_color(Vec4 color) {
    UI_Style_BG_Color *entry = (UI_Style_BG_Color *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_BG_Color));

    entry->background_color = color;
    entry->next = ui_manager->active_background_color_stack;
    
    ui_manager->active_background_color_stack = entry;
}

void ui_push_layout_type(UI_Layout_Type type) {
    UI_Style_Layout_Type *entry = (UI_Style_Layout_Type *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_Layout_Type));

    entry->type = type;
    entry->next = ui_manager->layout_type_stack;
    
    ui_manager->layout_type_stack = entry;
}

void ui_pop_layout_type() {
    assert(ui_manager->layout_type_stack);
    ui_manager->layout_type_stack = ui_manager->layout_type_stack->next;
}

void ui_push_size_type(Vec2_UI_Size_Type type) {
    UI_Style_Size_Type *entry = (UI_Style_Size_Type *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_Size_Type));

    entry->type = type;
    entry->next = ui_manager->size_type_stack;
    
    ui_manager->size_type_stack = entry;
}

void ui_pop_size_type() {
    assert(ui_manager->size_type_stack);
    ui_manager->size_type_stack = ui_manager->size_type_stack->next;
}

void ui_pop_size() {
    assert(ui_manager->size_stack);
    ui_manager->size_stack = ui_manager->size_stack->next;
}

void ui_pop_position() {
    assert(ui_manager->position_stack);
    ui_manager->position_stack = ui_manager->position_stack->next;
}

void ui_pop_position_type() {
    assert(ui_manager->position_type_stack);
    ui_manager->position_type_stack = ui_manager->position_type_stack->next;
}

void ui_pop_background_color() {
    assert(ui_manager->background_color_stack);
    ui_manager->background_color_stack = ui_manager->background_color_stack->next;
}

void ui_pop_text_color() {
    assert(ui_manager->text_color_stack);
    ui_manager->text_color_stack = ui_manager->text_color_stack->next;
}

void ui_push_text_color(Vec4 color) {
    UI_Style_Text_Color *entry = (UI_Style_Text_Color *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_Text_Color));

    entry->color = color;
    entry->next = ui_manager->text_color_stack;
    
    ui_manager->text_color_stack = entry;
}

void ui_push_font(char *font) {
    UI_Style_Font *entry = (UI_Style_Font *) allocate(&ui_manager->frame_arena, sizeof(UI_Style_Font));

    entry->font = font;
    entry->next = ui_manager->font_stack;
    
    ui_manager->font_stack = entry;
}

// TODO: we need to use the last frame's hierarchy since the actual visual positions are not calculated until
//       the end of the update procedure. so basically just store the last frame's hierarchy and use that for..
//       well actually, we want to be able to get the widgets without having to go through the tree. so maybe
//       just store them in a hash table, keyed by the widget IDs.
UI_Interact_Result ui_interact(UI_Widget *semantic_widget) {
    UI_Widget *computed_widget = ui_table_get(ui_manager->last_frame_widget_table, semantic_widget->id);
    if (!computed_widget) return {};
    //assert(computed_widget);
    
    Controller_State *controller_state = Context::controller_state;
    Vec2 mouse_pos = controller_state->current_mouse;

    UI_Interact_Result result = {};

    result.relative_mouse = mouse_pos - computed_widget->computed_position;
    result.relative_mouse_percentage = { result.relative_mouse.x / computed_widget->computed_size.x,
                                         result.relative_mouse.y / computed_widget->computed_size.y };
    
    UI_id id = computed_widget->id;
    if (computed_widget->flags & UI_WIDGET_IS_CLICKABLE) {
        if (in_bounds(mouse_pos, computed_widget->computed_position, computed_widget->computed_size)) {
            ui_manager->hot = id;

            if (just_pressed(controller_state->left_mouse)) {
                // we check for !was_down to avoid setting a button active if we click and hold outside then
                // move into the button
                ui_manager->active = id;
                ui_manager->active_t = 0.0f;
            } else if (is_active(computed_widget) && just_lifted(controller_state->left_mouse)) {
                result.clicked = true;
                result.click_t = ui_manager->active_t;
                ui_manager->active = {};
            }
        } else {
            if (is_hot(computed_widget)) {
                ui_manager->hot = {};
            }

            if (is_active(computed_widget) && !controller_state->left_mouse.is_down) {
                ui_manager->active = {};
            }
        }

        // must be active, since active can only be started when starting click inside the bounds.
        // we don't want to be able to start dragging by holding down outside and moving into bounds.
        if (is_active(computed_widget)) {
            if (being_held(controller_state->left_mouse)) {
                result.holding = true;
            }
        }
    }

    if (computed_widget->flags & UI_WIDGET_IS_FOCUSABLE) {
        if (in_bounds(mouse_pos, computed_widget->computed_position, computed_widget->computed_size)) {
            ui_manager->hot = id;
            if (just_pressed(controller_state->left_mouse)) {
                if (!is_focus(computed_widget)) {
                    ui_manager->focus = id;
                    result.focused = true;
                    ui_manager->focus_t = 0;
                }
            } else if (just_pressed(controller_state->key_enter)) {
                ui_manager->focus = {};
                result.lost_focus = true;
            }
        } else {
            if (is_hot(computed_widget)) {
                ui_manager->hot = {};
            }

            if (just_pressed(controller_state->left_mouse) || just_pressed(controller_state->key_enter)) {
                if (is_focus(computed_widget)) {
                    ui_manager->focus = {};
                    result.lost_focus = true;
                }
            }
        }
    }
    
    return result;
}

void calculate_standalone_size(Asset_Manager *asset, UI_Widget *widget, UI_Widget_Axis axis) {
    UI_Size_Type size_type = widget->size_type[axis];
    real32 axis_semantic_size = widget->semantic_size[axis];
    real32 *axis_computed_size = &widget->computed_size[axis];
    
    if (size_type == UI_SIZE_ABSOLUTE) {
        *axis_computed_size = axis_semantic_size;
    } else if (size_type == UI_SIZE_FIT_TEXT) {
        Font font = get_font(asset, widget->font);
        if (axis == UI_WIDGET_X_AXIS) {
            *axis_computed_size = get_width(font, widget->text);
        } else {
            *axis_computed_size = font.height_pixels;
        }
    } else if (size_type == UI_SIZE_FIT_CHILDREN) {
        *axis_computed_size = axis_semantic_size;
    }
}

void ui_calculate_standalone_sizes(Asset_Manager *asset) {
    UI_Widget *current = ui_manager->root;
    
    while (true) {
        UI_Widget *parent = current->parent;

        calculate_standalone_size(asset, current, UI_WIDGET_X_AXIS);
        calculate_standalone_size(asset, current, UI_WIDGET_Y_AXIS);
                
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) return;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) return; // root
                    current_ancestor = current_ancestor->parent;
                }

                current = current_ancestor->next;
            }
        }
    }
}

void calculate_ancestor_dependent_size(UI_Widget *widget, UI_Widget_Axis axis) {
    UI_Widget *parent = widget->parent;

    UI_Size_Type size_type = widget->size_type[axis];
    real32 axis_semantic_size = widget->semantic_size[axis];
    real32 *axis_computed_size = &widget->computed_size[axis];
    
    if (size_type == UI_SIZE_PERCENTAGE) {
        assert(parent); // root node cannot be percentage based
        if (parent->size_type[axis] == UI_SIZE_PERCENTAGE ||
            parent->size_type[axis] == UI_SIZE_ABSOLUTE) {
            // since this is pre-order traversal, the parent should already have a computed size
            *axis_computed_size = parent->computed_size[axis] * axis_semantic_size;
        }
    }
    
    // we could add an else block here that just sets computed size to semantic size. this could be useful
    // if we wanted to be able to specify minimum sizes for widgets that use fit_children sizing.
    // when we compute children based sizes, we do max(parent->computed_size.x current->computed_size.x), so
    // if we never set it, it'll just take the max child's computed size.
    // ideally it would be clear if we're using pixels to specify the minimum size or a percentage. that's
    // why i'm not doing it now, since it might result in unexpected behaviour.
}

void ui_calculate_ancestor_dependent_sizes() {
    UI_Widget *current = ui_manager->root;

    while (true) {
        UI_Widget *parent = current->parent;

        calculate_ancestor_dependent_size(current, UI_WIDGET_X_AXIS);
        calculate_ancestor_dependent_size(current, UI_WIDGET_Y_AXIS);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) return;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) return; // root
                    current_ancestor = current_ancestor->parent;
                }

                current = current_ancestor->next;
            }
        }
    }
}

void calculate_child_dependent_size(UI_Widget *widget, UI_Widget_Axis axis) {
    if (widget->position_type == UI_POSITION_FLOAT) return;
    
    UI_Widget *parent = widget->parent;

    if (parent) {
        // we also check percentage since if the parent of the percentage based widget is FIT_CHILDREN,
        // we need to bubble up the child width to the FIT_CHILDREN widget. we do this by setting the
        // percentage based computed size as if it were a FIT_CHILDREN widget. this computed size will be
        // overwritten later to be based on its parent's size.
        if (parent->size_type[axis] == UI_SIZE_FIT_CHILDREN || parent->size_type[axis] == UI_SIZE_PERCENTAGE) {
            // if the current axis matches with the parent's layout axis, then increase the parent's
            // size on that axis.
            bool32 axis_matches_with_parent_layout = ((axis == UI_WIDGET_X_AXIS && parent->layout_type == UI_LAYOUT_HORIZONTAL) ||
                                                      (axis == UI_WIDGET_Y_AXIS && parent->layout_type == UI_LAYOUT_VERTICAL));
            if (axis_matches_with_parent_layout) {
                parent->computed_size[axis] += widget->computed_size[axis];
            } else {
                parent->computed_size[axis] = max(parent->computed_size[axis], widget->computed_size[axis]);
            }
        }

        if (axis == UI_WIDGET_X_AXIS &&
            (widget->size_type[axis] != UI_SIZE_FILL_REMAINING) &&
            (parent->layout_type == UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN ||
             parent->layout_type == UI_LAYOUT_HORIZONTAL)) {
            parent->computed_child_size_sum[axis] += widget->computed_size[axis];
        }
    }
}

void ui_calculate_child_dependent_sizes() {
    UI_Widget *current = ui_manager->root;

    bool32 revisiting = false;
    while (current) {
        UI_Widget *parent = current->parent;

        if (current->first && !revisiting) {
            current = current->first;
        } else {
            if (!current->first || revisiting) {
                // since this is post-order, we need to go back up to process the node. we pass by it to get to
                // the leaf nodes at first, but we only process it when we're either going back up (revisiting)
                // or if we're at a leaf node (since leaf nodes will never get revisited since they have no
                // children.

                // TODO: we may want to do something where size types are applied per axis. for example, if we
                //       wanted something to fit children horizontally, but have y be a fixed height.
                //       you can kind of do that now, but only when the cross-axis size is larger than the child's
                //       size on that same axis. i can't really think of a case where you would want the cross-axis
                //       size to be smaller than the largest child's on that cross-axis.

                calculate_child_dependent_size(current, UI_WIDGET_X_AXIS);
                calculate_child_dependent_size(current, UI_WIDGET_Y_AXIS);
                
                revisiting = false;
            }
            
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) return;

                current = parent;
                revisiting = true;
            }
        }
    }
}

void calculate_ancestor_dependent_sizes_part_2(UI_Widget *widget, UI_Widget_Axis axis) {
    UI_Widget *parent = widget->parent;
    
    if (widget->size_type[axis] == UI_SIZE_PERCENTAGE) {
        assert(parent); // root node cannot be percentage based
        widget->computed_size[axis] = parent->computed_size[axis] * widget->semantic_size[axis];
    } else if (widget->size_type[axis] == UI_SIZE_FILL_REMAINING) {
        assert(widget->position_type != UI_POSITION_FLOAT);
        assert(parent);
        if (axis == UI_WIDGET_X_AXIS) {
            widget->computed_size[axis] = ((parent->computed_size[axis] - parent->computed_child_size_sum[axis]) /
                                           parent->num_fill_children);
        } else {
            assert(!"UI_SIZE_FIT_REMAINING only supported for x-axis.");
        }
        
    }
}

// this is to resolve percentage widths that are children of components that have child-dependent sizes
// for example, if the parent of a percentage width widget is a FIT_CHILDREN widget, then we have to wait
// until the parent widget has a computed width, then we can set the computed width of the percantage
// based widget.
void ui_calculate_ancestor_dependent_sizes_part_2() {
    UI_Widget *current = ui_manager->root;

    while (true) {
        UI_Widget *parent = current->parent;

        calculate_ancestor_dependent_sizes_part_2(current, UI_WIDGET_X_AXIS);
        calculate_ancestor_dependent_sizes_part_2(current, UI_WIDGET_Y_AXIS);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) return;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) return; // root
                    current_ancestor = current_ancestor->parent;
                }

                current = current_ancestor->next;
            }
        }
    }
}

void calculate_position(UI_Widget *widget, UI_Widget_Axis axis) {
    UI_Widget *parent = widget->parent;

    if (parent) {
        UI_Widget *prev = widget->prev;

        if (widget->position_type == UI_POSITION_FLOAT) {
            widget->computed_position[axis] = (parent->computed_position[axis] +
                                               widget->semantic_position[axis]);
            return;
        }
        
        if (parent->layout_type == UI_LAYOUT_HORIZONTAL) {
            if (axis == UI_WIDGET_X_AXIS) {
                if (!prev) {
                    widget->computed_position[axis] = parent->computed_position[axis];
                } else {
                    widget->computed_position[axis] = prev->computed_position[axis] + prev->computed_size[axis];
                }
            } else {
                widget->computed_position[axis] = parent->computed_position[axis];
            }
        } else if (parent->layout_type == UI_LAYOUT_VERTICAL) {
            if (axis == UI_WIDGET_Y_AXIS) {
                if (!prev) {
                    widget->computed_position[axis] = parent->computed_position[axis];
                } else {
                    widget->computed_position[axis] = prev->computed_position[axis] + prev->computed_size[axis];
                }
            } else {
                widget->computed_position[axis] = parent->computed_position[axis];
            }
        } else if (parent->layout_type == UI_LAYOUT_CENTER) {
            widget->computed_position[axis] = (parent->computed_position[axis] + (parent->computed_size[axis] / 2.0f) -
                                               (widget->computed_size[axis] / 2.0f));
        } else if (parent->layout_type == UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN) {
            if (axis == UI_WIDGET_X_AXIS) {
                real32 d = ((parent->computed_size.x - parent->computed_child_size_sum.x) /
                            (parent->num_sized_children - 1));
                if (prev) {
                    widget->computed_position.x = prev->computed_position.x + prev->computed_size.x + d;
                } else {
                    widget->computed_position[axis] = parent->computed_position[axis];
                }
            } else {
                widget->computed_position[axis] = parent->computed_position[axis];
            }
        } else {
            widget->computed_position[axis] = widget->semantic_position[axis];
        }
    } else {
        widget->computed_position[axis] = widget->semantic_position[axis];
    }
}

void ui_calculate_positions() {
    UI_Widget *current = ui_manager->root;

    // pre-order traversal
    
    while (true) {
        UI_Widget *parent = current->parent;
        
        calculate_position(current, UI_WIDGET_X_AXIS);
        calculate_position(current, UI_WIDGET_Y_AXIS);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) return;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) return; // root
                    current_ancestor = current_ancestor->parent;
                }

                current = current_ancestor->next;
            }
        }
    }
}

void ui_init(Arena_Allocator *arena) {
    uint32 max_padding = 8 * 3;
    uint32 persistent_heap_size = MEGABYTES(64);
    uint32 frame_arena_size = MEGABYTES(64);
    uint32 last_frame_arena_size = MEGABYTES(64) - max_padding;

    {
        uint32 size = persistent_heap_size;
        void *base = arena_push(arena, size, false);
        ui_manager->persistent_heap = make_heap_allocator(base, size);
    }
    {
        uint32 size = frame_arena_size;
        void *base = arena_push(arena, size, false);
        ui_manager->frame_arena = make_arena_allocator(base, size);
    }
    {
        uint32 size = last_frame_arena_size;
        void *base = arena_push(arena, size, false);
        ui_manager->last_frame_arena = make_arena_allocator(base, size);
    }

    ui_manager->widget_table            = (UI_Widget **) arena_push(&ui_manager->frame_arena,
                                                                 sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
    ui_manager->last_frame_widget_table = (UI_Widget **) arena_push(&ui_manager->last_frame_arena,
                                                                 sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
    ui_manager->state_table             = (UI_Widget_State **) heap_allocate(&ui_manager->persistent_heap,
                                                                          sizeof(UI_Widget_State *) * NUM_WIDGET_BUCKETS, true);
}

void ui_frame_init(Display_Output *display_output, real32 dt) {
    ui_push_position({ 0.0f, 0.0f });
    ui_push_layout_type(UI_LAYOUT_NONE);
    ui_push_size_type({ UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE });
    ui_push_size({ (real32) display_output->width, (real32) display_output->height });

    UI_Widget *widget = make_widget(make_ui_id("root"), 0);

    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&ui_manager->frame_arena, sizeof(UI_Stack_Widget));
    assert(ui_manager->widget_stack == NULL);
    entry->widget = widget;
    entry->next = ui_manager->widget_stack;
    ui_manager->widget_stack = entry;

    ui_manager->root = widget;

    ui_manager->widget_table = (UI_Widget **) arena_push(&ui_manager->frame_arena,
                                                      sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
    
    ui_table_add(ui_manager->widget_table, widget);

    ui_manager->focus_t += dt;
    ui_manager->active_t += dt;
}

// TODO: don't clear.. we want to keep state actually
void ui_frame_end() {
    ui_manager->last_frame_root = ui_manager->root;
    ui_manager->root = NULL;

    // swap allocators
    Arena_Allocator temp = ui_manager->last_frame_arena;
    ui_manager->last_frame_arena = ui_manager->frame_arena;
    ui_manager->frame_arena = temp;
    clear_arena(&ui_manager->frame_arena);

    // swap tables
    UI_Widget **temp_widget_table = ui_manager->last_frame_widget_table;
    ui_manager->last_frame_widget_table = ui_manager->widget_table;
    ui_manager->widget_table = temp_widget_table;
    
    ui_manager->widget_stack = NULL;
    ui_manager->background_color_stack = NULL;
    ui_manager->hot_background_color_stack = NULL;
    ui_manager->active_background_color_stack = NULL;
    ui_manager->layout_type_stack = NULL;
    ui_manager->size_stack = NULL;
    ui_manager->position_stack = NULL;
    ui_manager->size_type_stack = NULL;
    ui_manager->text_color_stack = NULL;
}

bool32 ui_has_focus() {
    return false;
}

void ui_disable_input() {
    ui_manager->hot = {};
    ui_manager->active = {};
    ui_manager->is_disabled = true;
}

void ui_enable_input() {
    ui_manager->is_disabled = false;
}

inline bool32 is_hot(UI_Widget *widget) {
    return ui_id_equals(ui_manager->hot, widget->id);
}

inline bool32 is_active(UI_Widget *widget) {
    return ui_id_equals(ui_manager->active, widget->id);
}

inline bool32 is_focus(UI_Widget *widget) {
    return ui_id_equals(ui_manager->focus, widget->id);
}

inline bool32 ui_has_hot() {
    return (ui_manager->hot.string_ptr != NULL);
}

inline bool32 ui_has_active() {
    return (ui_manager->active.string_ptr != NULL);
}

inline bool32 in_bounds(Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max) {
    return (p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max);
}

inline bool32 in_bounds(Vec2 p, Vec2 widget_position, Vec2 widget_size) {
    return in_bounds(p,
                     widget_position.x, widget_position.x + widget_size.x,
                     widget_position.y, widget_position.y + widget_size.y);
}

inline bool32 ui_id_equals(UI_id id1, UI_id id2) {
    return ((id1.string_ptr == id2.string_ptr) && (id1.index == id2.index) &&
            (id1.parent_string_ptr == id2.parent_string_ptr) && (id1.parent_index == id2.parent_index));
}

inline real32 get_adjusted_font_height(Font font) {
    return font.height_pixels + (font.scale_for_pixel_height * font.line_gap);
}

inline real32 get_center_x_offset(real32 container_width, real32 element_width) {
    return (container_width / 2.0f - element_width / 2.0f);
}

// this is different from get_center_y_offset because text is drawn from the bottom left corner and goes up
// i.e. it's drawn going up from its baseline, instead of like for quads from the top left going down
inline real32 get_center_baseline_offset(real32 container_height, real32 text_height) {
    return 0.5f * (container_height + text_height);
}

inline real32 get_center_y_offset(real32 height, real32 box_height) {
    return 0.5f * (height - box_height);
}

UI_Widget_State *ui_get_state(UI_id id) {
    uint32 hash = get_hash(id, NUM_WIDGET_BUCKETS);

    UI_Widget_State *current = ui_manager->state_table[hash];
    while (current) {
        if (ui_id_equals(current->id, id)) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

void _ui_add_state(UI_Widget_State *state) {
    if (!state->id.string_ptr) return;
    
    uint32 hash = get_hash(state->id, NUM_WIDGET_BUCKETS);

    UI_Widget_State **state_table = ui_manager->state_table;
    UI_Widget_State *current = state_table[hash];
    if (!current) {
        state->next = NULL;
        state_table[hash] = state;
        return;
    }
    
    while (true) {
        if (ui_id_equals(current->id, state->id)) {
            assert(!"Widget state already exists in table.");
            return;
        }

        if (!current->next) {
            current->next = state;
            state->next = NULL;
            return;
        }

        current = current->next;
    }
    
    assert(!"Should be unreachable.");
}

UI_Widget_State *ui_make_widget_state() {
    UI_Widget_State *result = (UI_Widget_State *) heap_allocate(&ui_manager->persistent_heap,
                                                                sizeof(UI_Widget_State), true);
    return result;
}

UI_Window_State *ui_add_window_state(UI_id id, Vec2 position) {
    UI_Widget_State *state = ui_make_widget_state();
    state->id = id;
    state->type = UI_STATE_WINDOW;
    state->window = { position };

    _ui_add_state(state);

    return &state->window;
}

UI_Text_Field_Slider_State *ui_add_text_field_slider_state(UI_id id, real32 value) {
    UI_Widget_State *state = ui_make_widget_state();
    state->id = id;
    state->type = UI_STATE_TEXT_FIELD_SLIDER;
    String_Buffer buffer = make_string_buffer((Allocator *) &ui_manager->persistent_heap, 64);
    state->text_field_slider = { buffer, false, false, 0 };
    
    _ui_add_state(state);

    return &state->text_field_slider;
}


// COMPOUND WIDGETS

void do_text(char *text, char *id, uint32 flags = 0, int32 index = 0) {
    ui_push_size_type({ UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT });
    ui_push_background_color({ 1.0f, 0.0f, 0.0f, 1.0f });
    //UI_Widget *text_widget = ui_add_widget(make_ui_id(id, index), UI_WIDGET_DRAW_TEXT | UI_WIDGET_DRAW_BACKGROUND);
    UI_Widget *text_widget = ui_add_widget(make_ui_id(id, index), UI_WIDGET_DRAW_TEXT);
    text_widget->text = text;
    ui_pop_background_color();
    ui_pop_size_type();
}

bool32 do_button(UI_id id) {
    UI_Widget *widget = ui_add_widget(id, UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    UI_Interact_Result interact_result = ui_interact(widget);
    
    return interact_result.clicked;
}

bool32 do_text_button(char *text, real32 padding, UI_id id) {
    ui_push_size({});
    ui_push_size_type({ UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN });
    ui_push_layout_type(UI_LAYOUT_VERTICAL);
    UI_Widget *button = ui_push_widget(id,
                                       UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    {
        ui_push_size({ 0.0f, padding });
        ui_add_widget("");

        ui_push_layout_type(UI_LAYOUT_HORIZONTAL);
        ui_push_widget("", 0);
        {
            // inner row
            ui_push_size({ padding, 0.0f });
            ui_add_widget("");
            //ui_pop_size_type();

            ui_push_size_type({ UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT });
            UI_Widget *text_widget = ui_add_widget(make_ui_id(NULL), UI_WIDGET_DRAW_TEXT);
            text_widget->text = text;
            ui_pop_size_type();

            ui_add_widget("");
            ui_pop_size();
        }
        ui_pop_widget();
        ui_pop_layout_type();
        
        ui_add_widget("");
        ui_pop_size();
    }
    ui_pop_widget();
    ui_pop_layout_type();
    ui_pop_size_type();
    ui_pop_size();
    
    
    UI_Interact_Result interact_result = ui_interact(button);
#if 0
    ui_pop_size_type();
    
    ui_push_size_type(UI_SIZE_FIT_TEXT);

    // TODO: we should probably use a hashing method for storing UI IDs, so that we can generate IDs dynamically.
    //       right now we just use pointers, which is fine for read-only memory, but if we create new strings,
    //       then the string addresses will not be consistent.
    // TODO: scope UI IDs. have a parent UI_id be included in all UI_ids. that way, we can do stuff like this
    //       without having collisions. actually, i don't think that'll work. try and use a hash. actually,
    //       i think this is fine, since we always have the parent ID use some unique string.
    UI_Widget *text_widget = ui_add_widget(make_ui_id("button-text"), UI_WIDGET_DRAW_TEXT);
    text_widget->text = text;

    ui_pop_layout_type();
    ui_pop_size_type();
    ui_pop_widget();
#endif

    return interact_result.clicked;
}

inline bool32 do_text_button(char *text, real32 padding, char *id, int32 index = 0) {
    return do_text_button(text, padding, make_ui_id(id, index));
}

void ui_x_pad(real32 width) {
    ui_push_size_type({ UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE });
    ui_push_size({ width, 0.0f });
    ui_add_widget("");
    ui_pop_size();
    ui_pop_size_type();
}

void ui_add_slider_bar(real32 value, real32 min, real32 max) {
    ui_push_size_type({ UI_SIZE_PERCENTAGE, UI_SIZE_PERCENTAGE });
    ui_push_position_type(UI_POSITION_FLOAT);
    ui_push_position({});
    ui_push_size({ clamp((value - min) / (max - min), 0.0f, 1.0f), 1.0f });
    
    ui_add_widget("", UI_WIDGET_DRAW_BACKGROUND);

    ui_pop_size();
    ui_pop_position();
    ui_pop_position_type();
    ui_pop_size_type();
}

// TODO: add slider
real32 do_text_field_slider(Asset_Manager *asset, real32 value,
                            real32 min_value, real32 max_value,
                            bool32 show_slider,
                            char *id_string, int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Text_Field_Slider_State *state;
    if (!state_variant) {
        state = ui_add_text_field_slider_state(id, value);
    } else {
        state = &state_variant->text_field_slider;
    }
    
    // size, position, background color, text color should be set before this is called
    ui_push_layout_type(UI_LAYOUT_CENTER);
    UI_Widget *textbox = ui_push_widget(id_string,
                                        UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE | UI_WIDGET_IS_FOCUSABLE);
    UI_Interact_Result interact = ui_interact(textbox);

    real32 x_delta = fabsf((get_mouse_delta()).x);
    real32 deadzone = 1.0f;

    if (!state->is_using) {
        state->is_sliding = is_active(textbox) && (x_delta > deadzone);

        if (state->is_sliding) {
            state->is_using = true;
        } else if (interact.clicked) {
            state->is_using = true;

            char *value_text = string_format((Allocator *) &ui_manager->frame_arena, "%f", value);
            set_string_buffer_text(&state->buffer, value_text);
            state->cursor_index = state->buffer.current_length;
        }
    }


    if (state->is_using) {
        if (state->is_sliding) {
            if (!is_active(textbox)) {
                state->is_using = false;
            }
        } else {
            if (interact.lost_focus) {
                state->is_using = false;

                real32 result;
                bool32 conversion_result = string_to_real32(make_string(state->buffer), &result);
                if (conversion_result) {
                    value = result;
                    //return result;
                }
            }
        }
    }
    
    if (state->is_using && state->is_sliding) {
        value = interact.relative_mouse_percentage.x * (max_value - min_value);
        value = clamp(value, min_value, max_value);
    }
    
    if (state->is_using && !state->is_sliding) {
        Controller_State *controller_state = Context::controller_state;
        int32 original_cursor_index = state->cursor_index;
        
        if (just_pressed_or_repeated(controller_state->key_left)) {
            state->cursor_index--;
            state->cursor_index = max(state->cursor_index, 0);
        }

        if (just_pressed_or_repeated(controller_state->key_right)) {
            state->cursor_index++;
            state->cursor_index = min(state->cursor_index, state->buffer.current_length);
            state->cursor_index = min(state->cursor_index, state->buffer.size);
        }

        String_Buffer *buffer = &state->buffer;
        for (int32 i = 0; i < controller_state->num_pressed_chars; i++) {
            char c = controller_state->pressed_chars[i];
            if (c == '\b') { // backspace key
                splice(&state->buffer, state->cursor_index - 1);
                state->cursor_index--;
                state->cursor_index = max(state->cursor_index, 0);
            } else if ((buffer->current_length < buffer->size) && (state->cursor_index < buffer->size)) {
                if (c >= 32 && c <= 126) {
                    splice_insert(&state->buffer, state->cursor_index, c);
                    state->cursor_index++;
                }
            }
        }

        if (state->cursor_index != original_cursor_index) {
            ui_manager->focus_t = 0.0f;
        }
    }

    if (show_slider) {
        if (!state->is_using || state->is_sliding) {
            ui_push_background_color({ 0.0f, 0.0f, 1.0f, 1.0f });
            ui_add_slider_bar(value, min_value, max_value);
            ui_pop_background_color();
        }
    }
    
    {
        ui_push_size_type({ UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN });
        ui_push_layout_type(UI_LAYOUT_HORIZONTAL);
        ui_push_size({ 1.0f, 0.0f });

        ui_push_widget("");
        {
            ui_x_pad(5.0f);
            ui_push_size_type({ UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN });
            ui_push_layout_type(UI_LAYOUT_HORIZONTAL);
            ui_push_background_color({ 0.0f, 1.0f, 0.0f, 1.0f });
            ui_push_widget("");
            {
                if (state->is_using && !state->is_sliding) {
                    //if (false) {
                    char *str = to_char_array((Allocator *) &ui_manager->frame_arena, state->buffer);
                    do_text(str, "");

                    // draw cursor

                    ui_push_size_type({ UI_SIZE_ABSOLUTE, UI_SIZE_PERCENTAGE });
                    ui_push_size({ 1.0f, 1.0f });
                    ui_push_position_type(UI_POSITION_FLOAT);

                    Font font = get_font(asset, textbox->font);
                    real32 width_to_index = get_width(font, str, state->cursor_index);
                    ui_push_position({ floorf(width_to_index), 0.0f });

                    ui_push_background_color({ 0.0f, 0.0f, 0.0f, 1.0f });
                    real32 time_to_switch = 0.5f; // time spent in either state
                    bool32 show_background = ((int32) (ui_manager->focus_t*(1.0f / time_to_switch)) + 1) % 2;
                    
                    UI_Widget *cursor = ui_add_widget("", show_background ? UI_WIDGET_DRAW_BACKGROUND : 0);
                    ui_pop_background_color();
                    
                    ui_pop_position();
                    ui_pop_position_type();
                    ui_pop_size();
                    ui_pop_size_type();    
                } else {
                    char *buf = string_format((Allocator *) &ui_manager->frame_arena, "%f", value);
                    do_text(buf, "");
                }
            }
            ui_pop_widget();
            ui_pop_background_color();
            ui_pop_layout_type();
            ui_pop_size_type();
            
            ui_x_pad(5.0f);
        }
        ui_pop_widget();

        ui_pop_size();
        ui_pop_layout_type();
        ui_pop_size_type();
    }
    ui_pop_widget();
    ui_pop_layout_type();

    // we do validation that it's a number, but any putting value into bounds should be done by the caller
    #if 0
    if (interact.lost_focus) {
        real32 result;
        bool32 conversion_result = string_to_real32(make_string(state->buffer), &result);
        if (conversion_result) {
            return result;
        }
    }
    #endif
    
    return value;
}

// TODO: this should be push_window and should move all the popping calls to a pop_window procedure
// TODO: add state and window dragging
void do_window(char *text, char *id_string, int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Window_State *state;
    if (!state_variant) {
        state = ui_add_window_state(id, { 100.0f, 200.0f });
    } else {
        state = &state_variant->window;
    }
    
    
    #if 0
    UI_Widget_State *state_variant = get_state(id);
    UI_Window_State *state;
    if (!state_variant) {
        state = &(add_state(id))->window_state;
        state.position = { 200.0f, 200.0f };
    } else {
        state = &state_variant->window_state;
    }
    #endif
    
    ui_push_widget(ui_manager->root);
    ui_push_position(state->position);
    
    ui_push_size({});
    ui_push_layout_type(UI_LAYOUT_VERTICAL);
    ui_push_size_type({ UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN });
    //ui_push_size_type({ UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE });

    ui_push_text_color({ 1.0f, 1.0f, 1.0f, 1.0f });
    #if 1
    ui_push_widget(id, UI_WIDGET_DRAW_BACKGROUND);
    {
        ui_push_size_type({ UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN });
        ui_push_size({ 1.0f, 20.0f });
        ui_push_layout_type(UI_LAYOUT_CENTER);
        ui_push_background_color({ 1.0f, 0.0f, 0.0f, 1.0f });

        // title bar
        UI_Widget *title_bar = ui_push_widget("window-title-bar",
                                              UI_WIDGET_IS_CLICKABLE | UI_WIDGET_DRAW_BACKGROUND);
        UI_Interact_Result title_bar_interact = ui_interact(title_bar);
        {
            do_text(text, "");
        }
        ui_pop_widget();

        if (title_bar_interact.holding) {
            Vec2 delta = get_mouse_delta();
            state->position += delta;
        }
        
        ui_pop_background_color();
        
        ui_push_size({ 200.0f, 200.0f });
        ui_push_size_type({ UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE });
        ui_push_widget("", UI_WIDGET_DRAW_BACKGROUND);
        {
        }
        ui_pop_size();
        ui_pop_size_type();
        ui_pop_size();
        ui_pop_layout_type();
        ui_pop_size_type();
    }
    ui_pop_widget();
    #endif
    ui_pop_text_color();

    ui_pop_position();
    ui_pop_widget();
}
