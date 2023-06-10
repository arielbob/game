#include "ui.h"

char *default_font = "calibri14";
UI_Theme NULL_THEME = {
    {}, {}, {}, "texture_default",
    {}, default_font, NULL,
    {}, 0, 0, 0, 0,
    UI_LAYOUT_NONE,
    { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN },
    UI_POSITION_NONE,
    { 0.0f, 0.0f },
    { 0.0f, 0.0f }
};

UI_Widget *make_widget(UI_id id, UI_Theme theme, uint32 flags) {
    UI_Widget *widget = (UI_Widget *) allocate(&ui_manager->frame_arena, sizeof(UI_Widget));

    *widget = {};
    widget->id = id;
    widget->flags = flags;

    widget->background_color        = theme.background_color;
    widget->hot_background_color    = theme.hot_background_color;
    widget->active_background_color = theme.active_background_color;
    widget->texture_name            = theme.texture_name;
    
    widget->text_color              = theme.text_color;
    widget->font                    = theme.font;
    widget->text                    = theme.text;

    widget->border_color            = theme.border_color;
    widget->border_flags            = theme.border_flags;
    widget->border_width            = theme.border_width;
    widget->corner_radius           = theme.corner_radius;
    widget->corner_flags            = theme.corner_flags;
    
    widget->layout_type             = theme.layout_type;
    widget->size_type               = theme.size_type;
    widget->position_type           = theme.position_type;
    
    widget->semantic_size           = theme.semantic_size;
    widget->semantic_position       = theme.semantic_position;

    widget->scissor_type            = theme.scissor_type;
    
    return widget;
}

inline UI_Widget *make_widget(char *id_string, UI_Theme theme, uint32 flags) {
    return make_widget(make_ui_id(id_string, 0), theme, flags);
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

#if 0
    it = make_string_iterator(make_string(id.parent_string_ptr));
    c = get_next_char(&it);
    while (c) {
        sum += c;
        c = get_next_char(&it);
    }
    sum += id.parent_index;
#endif
    
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
    if (!id.string_ptr) return NULL;
    
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

// from current frame
inline UI_Widget *ui_get_widget(UI_id id) {
    return ui_table_get(ui_manager->widget_table, id);
}

// from previous frame
inline UI_Widget *ui_get_widget_prev_frame(UI_id id) {
    return ui_table_get(ui_manager->last_frame_widget_table, id);
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
    widget->parent = parent;

    if (widget->position_type != UI_POSITION_FLOAT) {
        if (widget->size_type[UI_WIDGET_X_AXIS] != UI_SIZE_FILL_REMAINING) {
            parent->num_sized_children[UI_WIDGET_X_AXIS]++;
        } else {
            parent->num_fill_children[UI_WIDGET_X_AXIS]++;
        }

        if (widget->size_type[UI_WIDGET_Y_AXIS] != UI_SIZE_FILL_REMAINING) {
            parent->num_sized_children[UI_WIDGET_Y_AXIS]++;
        } else {
            parent->num_fill_children[UI_WIDGET_Y_AXIS]++;
        }
    }
    parent->num_total_children++;

    if (!parent->last) {
        assert(!parent->first);
        parent->first = widget;
        parent->last = widget;
    } else {
        parent->last->next = widget;
        widget->prev = parent->last;
        parent->last = widget;
    }

    if (parent && parent->layout_type == UI_LAYOUT_CENTER && widget->position_type != UI_POSITION_FLOAT) {
        if (widget->prev) {
            assert(!"Widget cannot be added, since parent is LAYOUT_CENTER and can only have a single non-floating child.");
        }
    }
    
    ui_table_add(ui_manager->widget_table, widget);

    return widget;
}

// NOTE: both ui_add_widget and ui_push_widget add widgets to the hierarchy. the only difference is that
//       ui_push_widget also adds the widget to the widget stack, so that next calls to ui_add_widget or
//       ui_push_widget will have their widget added as a child of the widget at the top of the widget
//       stack.
#if 0
UI_Widget *ui_add_widget(UI_id id, uint32 flags) {
    UI_Widget *widget = make_widget(id, flags);
    ui_add_widget(widget);

    return widget;
}
#endif

UI_Widget *ui_add_widget(char *id_string_ptr, UI_Theme theme, uint32 flags = 0) {
    UI_Widget *widget = make_widget(make_ui_id(id_string_ptr, 0), theme, flags);
    ui_add_widget(widget);
    return widget;
}

UI_Widget *ui_add_widget(UI_id id, UI_Theme theme, uint32 flags = 0) {
    UI_Widget *widget = make_widget(id, theme, flags);
    ui_add_widget(widget);
    return widget;
}

#if 0
UI_Widget *ui_add_widget(char *id_string_ptr, uint32 flags = 0) {
    return ui_add_widget(make_ui_id(id_string_ptr), flags);
}
#endif

// should only be used to push widgets that have already been added using add_widget
UI_Widget *ui_push_existing_widget(UI_Widget *widget) {
    // push to the stack
    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&ui_manager->frame_arena, sizeof(UI_Stack_Widget));

    entry->widget = widget;
    entry->next = ui_manager->widget_stack;
    
    ui_manager->widget_stack = entry;

    return widget;
}

UI_Widget *ui_push_widget(UI_id id, UI_Theme theme,uint32 flags = 0) {
    UI_Widget *widget = make_widget(id, theme, flags);
    ui_add_widget(widget);

    return ui_push_existing_widget(widget);
}

UI_Widget *ui_push_widget(char *id_string_ptr, UI_Theme theme, uint32 flags = 0) {
    return ui_push_widget(make_ui_id(id_string_ptr), theme, flags);
}

UI_Widget *ui_add_and_push_widget(UI_Widget *widget) {
    ui_add_widget(widget);
    ui_push_existing_widget(widget);
    return widget;
}

UI_Widget *ui_add_and_push_widget(char *id_string_ptr, UI_Theme theme, uint32 flags = 0) {
    UI_Widget *widget = make_widget(id_string_ptr, theme, flags);
    ui_add_widget(widget);
    ui_push_existing_widget(widget);
    return widget;
}

UI_Widget *ui_add_and_push_widget(UI_id id, UI_Theme theme, uint32 flags = 0) {
    UI_Widget *widget = make_widget(id, theme, flags);
    ui_add_widget(widget);
    ui_push_existing_widget(widget);
    return widget;
}

void ui_pop_widget() {
    assert(ui_manager->widget_stack);
    ui_manager->widget_stack = ui_manager->widget_stack->next;
}

#if 0
void ui_bring_to_top_of_parent(UI_Widget *widget) {
    // widget has to have been added to the tree (idk if this is really necessary, but it's fine, i think)
    assert(ui_get_widget(widget->id));
    assert(widget->parent);

    UI_Widget *parent = widget->parent;

    assert(parent->last);
    if (parent->first == parent->last) {
        // this is the case where widget is the only child
        // in that case, we don't need to do anything
        assert(parent->first == widget);
        return;
    }

    if (!widget->next) {
        // already at the end
        return;
    }

    if (!widget->prev) {
        // first child
        parent->first = widget->next;
    } else {
        widget->prev->next = widget->next;
    }
    
    widget->next->prev = widget->prev;
    parent->last = widget;
}
#endif

// this sets hot based on the rendering_index, which is just when the widget was drawn, higher being later/above.
// we do this so that we set hot to the widget that is above other widgets and don't set hot to a widget that is
// visually below another one at the cursor position.
void set_hot_if_above_current_hot(UI_Widget *widget) {
    UI_Widget *hot_widget = ui_table_get(ui_manager->last_frame_widget_table, ui_manager->hot);
    if (hot_widget) {
        if (widget->rendering_index > hot_widget->rendering_index) {
            ui_manager->hot = widget->id;
            ui_manager->hot_t = 0.0f;
        }
    } else {
        ui_manager->hot = widget->id;
        ui_manager->hot_t = 0.0f;
    }
}

// TODO: we need to use the last frame's hierarchy since the actual visual positions are not calculated until
//       the end of the update procedure. so basically just store the last frame's hierarchy and use that for..
//       well actually, we want to be able to get the widgets without having to go through the tree. so maybe
//       just store them in a hash table, keyed by the widget IDs.
UI_Interact_Result ui_interact(UI_Widget *semantic_widget) {
    // we don't add widgets with NULL IDs to the table, so to interact with a widget, the widget must have a
    // non-null ID.
    UI_Widget *computed_widget = ui_table_get(ui_manager->last_frame_widget_table, semantic_widget->id);

    if (!computed_widget) {
        // no need to assert here, because on the first frame that a widget is added, it won't be
        // in last_frame_widget_table.
        return {};
    }
    
    Controller_State *controller_state = Context::controller_state;
    Vec2 mouse_pos = controller_state->current_mouse;

    UI_Interact_Result result = {};

    result.relative_mouse = mouse_pos - computed_widget->computed_position;
    result.relative_mouse_percentage = { result.relative_mouse.x / computed_widget->computed_size.x,
                                         result.relative_mouse.y / computed_widget->computed_size.y };
    
    UI_id id = computed_widget->id;
    if (computed_widget->flags & UI_WIDGET_IS_CLICKABLE) {
        bool32 in_computed_bounds = in_bounds(mouse_pos,
                                              computed_widget->computed_position, computed_widget->computed_size);
        bool32 in_scissor_bounds = true;
        if (computed_widget->scissor_type != UI_SCISSOR_NONE) {
            in_scissor_bounds = in_bounds(mouse_pos,
                                          computed_widget->computed_scissor_position,
                                          computed_widget->computed_scissor_dimensions);
        }
        
        if (in_computed_bounds && in_scissor_bounds) {
            set_hot_if_above_current_hot(computed_widget);

            if (is_hot(computed_widget)) {
                if (just_pressed(controller_state->left_mouse)) {
                    // we check for !was_down to avoid setting a button active if we click and hold outside then
                    // move into the button
                    result.just_pressed = true;
                    ui_manager->active = id;
                    ui_manager->active_t = 0.0f;
                } else if (is_active(computed_widget) && just_lifted(controller_state->left_mouse)) {
                    result.released = true;
                    result.click_t = ui_manager->active_t;
                    ui_manager->active = {};
                }
            }
        } else {
            if (is_hot(computed_widget)) {
                ui_manager->hot = {};
            }
        }

        // must be active, since active can only be started when starting click inside the bounds.
        // we don't want to be able to start dragging by holding down outside and moving into bounds.
        if (is_active(computed_widget)) {
            if (being_held(controller_state->left_mouse)) {
                result.holding = true;
            }

            if (!controller_state->left_mouse.is_down) {
                ui_manager->active = {};
            }
        }
    }

    // we want focus to be managed globally, since there should be only one widget in focus and handling
    // enter key press should be done here and not by the calling code
    if (computed_widget->flags & UI_WIDGET_IS_FOCUSABLE) {
        if (in_bounds(mouse_pos, computed_widget->computed_position, computed_widget->computed_size)) {
            set_hot_if_above_current_hot(computed_widget);
        } else {
            if (just_pressed(controller_state->left_mouse) || just_pressed(controller_state->key_enter)) {
                if (is_focus(computed_widget)) {
                    ui_manager->focus = {};
                    result.lost_focus = true;
                }
            }
        }

        if (is_hot(computed_widget)) {
            if (just_pressed(controller_state->left_mouse)) {
                if (!is_focus(computed_widget)) {
                    ui_manager->focus = id;
                    result.just_focused = true;
                    ui_manager->focus_t = 0;
                }
            }
        }

        if (is_focus(computed_widget)) {
            if (just_pressed(controller_state->key_enter)) {
                ui_manager->focus = {};
                result.lost_focus = true;
            }
        }
    }
    
    return result;
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

void deallocate(UI_Widget_State *state) {
    switch (state->type) {
        case UI_STATE_WINDOW: {
            // nothing to deallocate
        } break;
        case UI_STATE_TEXT_FIELD: {
            deallocate(state->text_field.buffer);
        } break;
        case UI_STATE_TEXT_FIELD_SLIDER: {
            deallocate(state->text_field_slider.text_field_state.buffer);
        } break;
        case UI_STATE_DROPDOWN: {
            // nothing to deallocate
        } break;
        default: {
            assert(!"Unhandled UI widget state type.");
        }
    }
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

void _ui_delete_state(UI_id id) {
    assert(id.string_ptr);

    uint32 hash = get_hash(id, NUM_WIDGET_BUCKETS);

    UI_Widget_State *current = ui_manager->state_table[hash];
    UI_Widget_State *prev = NULL;

    while (current) {
        if (ui_id_equals(current->id, id)) {
            if (prev) {
                prev->next = current->next;
            } else {
                // if we're first in list, we need to update bucket array when we delete
                ui_manager->state_table[hash] = current->next;
            }
            
            deallocate(current);
            deallocate((Allocator *) &ui_manager->persistent_heap, current);
            
            return;
        }

        current = current->next;
    }

    assert(!"Widget state does not exist.");
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

UI_Text_Field_State *ui_add_text_field_state(UI_id id, String value) {
    UI_Widget_State *state = ui_make_widget_state();
    state->id = id;
    state->type = UI_STATE_TEXT_FIELD;
    String_Buffer buffer = make_string_buffer((Allocator *) &ui_manager->persistent_heap, value, 64);
    state->text_field = { buffer, false, 0.0f, 0.0f };
    
    _ui_add_state(state);

    return &state->text_field;
}

UI_Text_Field_Slider_State *ui_add_text_field_slider_state(UI_id id, real32 value) {
    UI_Widget_State *state = ui_make_widget_state();
    state->id = id;
    state->type = UI_STATE_TEXT_FIELD_SLIDER;

    Marker m = begin_region();
    char *value_text = string_format(temp_region, "%f", value);
    
    UI_Text_Field_Slider_State *main_widget_state = &state->text_field_slider;
    String_Buffer current_text_buffer = make_string_buffer((Allocator *) &ui_manager->persistent_heap,
                                                           value_text, 64);
    main_widget_state->current_text = current_text_buffer;
    
    UI_Text_Field_State *text_field_state = &main_widget_state->text_field_state;
    String_Buffer text_field_buffer = make_string_buffer((Allocator *) &ui_manager->persistent_heap,
                                                         make_string(current_text_buffer),
                                                         current_text_buffer.size);
    *text_field_state = { text_field_buffer, 0, 0.0f, 0.0f };
    
    _ui_add_state(state);

    end_region(m);

    return main_widget_state;
}

UI_Dropdown_State *ui_add_dropdown_state(UI_id id) {
    UI_Widget_State *state = ui_make_widget_state();
    state->id = id;
    state->type = UI_STATE_DROPDOWN;

    UI_Dropdown_State *dropdown_state = &state->dropdown;
    *dropdown_state = {};
    dropdown_state->t = 1.0f;
        
    _ui_add_state(state);

    return dropdown_state;
}

void calculate_standalone_size(UI_Widget *widget, UI_Widget_Axis axis) {
    UI_Size_Type size_type = widget->size_type[axis];
    real32 axis_semantic_size = widget->semantic_size[axis];
    real32 *axis_computed_size = &widget->computed_size[axis];

    if (size_type == UI_SIZE_ABSOLUTE) {
        *axis_computed_size = axis_semantic_size;
    } else if (size_type == UI_SIZE_FIT_TEXT) {
        assert(widget->font);
        Font *font = get_font(widget->font);
        if (axis == UI_WIDGET_X_AXIS) {
            *axis_computed_size = get_width(font, widget->text);
        } else {
            *axis_computed_size = font->height_pixels;
        }
    } else if (size_type == UI_SIZE_FIT_CHILDREN) {
        *axis_computed_size = axis_semantic_size;

        // add an extra pixel for the border if we're size type FIT_CHILDREN.
        // this is fine since it's expected that the parent size will change. it's nice because the child widgets
        // will be sized accordingly and there won't be a missing pixel on any side due to the fact that we draw
        // borders on the inside of widgets.
        // TODO: move this to calculate_positions actually, and just move the first or last child on that axis?
        //       actually, you have to move all of them by one, then also expand it by one. at least that's the
        //       case when you have top and bottom borders. the bottom one doesn't really matter in that case.
        //       when there's two, you offset the position by one, and expand the size by two.
        //       when there's one, you offset only if it's at the start, and expand by one
        //       - so we should expand the size in calculate_child_dependent sizes, checking if it's the first
        //         element, then offset in calculate_positions.
        #if 0
        if (widget->flags & UI_WIDGET_DRAW_BORDER) {
            if (axis == UI_WIDGET_X_AXIS) {
                if (widget->border_flags & BORDER_LEFT) widget->computed_size[axis] += 1.0f;
                if (widget->border_flags & BORDER_RIGHT) widget->computed_size[axis] += 1.0f;
            } else {
                if (widget->border_flags & BORDER_TOP) widget->computed_size[axis] += 1.0f;
                if (widget->border_flags & BORDER_BOTTOM) widget->computed_size[axis] += 1.0f;
            }
        }
        #endif
    }
}

void ui_calculate_standalone_sizes() {
    UI_Widget *current = ui_manager->root;

    int32 num_visited = 0;
    
    while (current) {
        // pre-order

        current->rendering_index = num_visited;
        num_visited++;
        
        UI_Widget *parent = current->parent;
        
        calculate_standalone_size(current, UI_WIDGET_X_AXIS);
        calculate_standalone_size(current, UI_WIDGET_Y_AXIS);
                
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) break;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) break; // root
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

    while (current) {
        UI_Widget *parent = current->parent;

        calculate_ancestor_dependent_size(current, UI_WIDGET_X_AXIS);
        calculate_ancestor_dependent_size(current, UI_WIDGET_Y_AXIS);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) break;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) break; // root
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
            bool32 axis_matches_parent_layout = ((axis == UI_WIDGET_X_AXIS && parent->layout_type == UI_LAYOUT_HORIZONTAL) ||
                                                 (axis == UI_WIDGET_Y_AXIS && parent->layout_type == UI_LAYOUT_VERTICAL));
            if (axis_matches_parent_layout) {
                parent->computed_size[axis] += widget->computed_size[axis];
            } else {
                parent->computed_size[axis] = max(parent->computed_size[axis], widget->computed_size[axis]);
            }

            
        }

#if 1
        // we need need this layout type check here because we only add to the child size sum if the axis
        // and the layout axis match. because when we use the child size sum later for FILL_REMAINING, we only
        // want the child sizes that took up space on that axis.
        if (axis == UI_WIDGET_X_AXIS &&
            (widget->size_type[axis] != UI_SIZE_FILL_REMAINING) &&
            (parent->layout_type == UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN ||
             parent->layout_type == UI_LAYOUT_HORIZONTAL)) {
            parent->computed_child_size_sum[axis] += widget->computed_size[axis];
        } else if (axis == UI_WIDGET_Y_AXIS &&
                   (widget->size_type[axis] != UI_SIZE_FILL_REMAINING) &&
                   parent->layout_type == UI_LAYOUT_VERTICAL) {
            parent->computed_child_size_sum[axis] += widget->computed_size[axis];
        }
#endif

        #if 0
        if (widget->size_type[axis] != UI_SIZE_FILL_REMAINING) {
            parent->computed_child_size_sum[axis] += widget->computed_size[axis];
        }
        #endif
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

        // NOTE: you can have a UI_SIZE_PERCENTAGE inside a UI_SIZE_FIT_CHILDREN, it's just that the percentage will be taken of
        //       the computed size of the parent using its sized children.
        
        widget->computed_size[axis] = parent->computed_size[axis] * widget->semantic_size[axis];
    } else if (widget->size_type[axis] == UI_SIZE_FILL_REMAINING) {
        assert(widget->position_type != UI_POSITION_FLOAT);
        assert(parent);
        if (widget->parent->size_type[axis] == UI_SIZE_FIT_CHILDREN) {
            // since the parent only expands to fit the sized children, there will be no space left for the child.
            // the child will attempt to fill a space of size 0, which can be unexpected, so we assert.
            assert(!"Cannot have widget with axis size type FILL_REMAINING inside a widget who's size type on the same axis is FIT_CHILDREN.");
        }

        // we only need to use the computed sizes when we're on the same axis as the layout axis.
        // for example, if we're laying out on x and the layout is horizontal, then we use the computed child size
        // sum on that axis to calculate how we want to fill on that axis. otherwise, for example, if we're on y,
        // then fill_remaining on that axis with a horizontal layout would just fill the entire height.
        bool32 is_horizontal_match = (axis == UI_WIDGET_X_AXIS && parent->layout_type == UI_LAYOUT_HORIZONTAL);
        bool32 is_vertical_match = (axis == UI_WIDGET_Y_AXIS && parent->layout_type == UI_LAYOUT_VERTICAL);
        
        bool32 axis_matches_parent_layout = is_horizontal_match || is_vertical_match;

        if (axis_matches_parent_layout) {
            widget->computed_size[axis] = ((parent->computed_size[axis] - parent->computed_child_size_sum[axis]) /
                                           parent->num_fill_children[axis]);
            parent->computed_child_size_sum[axis] += widget->computed_size[axis];
        } else {
            widget->computed_size[axis] = parent->computed_size[axis];
        }
    }
}

// this is to resolve percentage widths that are children of components that have child-dependent sizes
// for example, if the parent of a percentage width widget is a FIT_CHILDREN widget, then we have to wait
// until the parent widget has a computed width, then we can set the computed width of the percentage
// based widget.
void ui_calculate_ancestor_dependent_sizes_part_2() {
    UI_Widget *current = ui_manager->root;

    while (current) {
        UI_Widget *parent = current->parent;

        calculate_ancestor_dependent_sizes_part_2(current, UI_WIDGET_X_AXIS);
        calculate_ancestor_dependent_sizes_part_2(current, UI_WIDGET_Y_AXIS);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) break;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) break; // root (has no next, so breaks outer loop as well)
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

        bool32 axis_matches_parent_layout = ((axis == UI_WIDGET_X_AXIS && parent->layout_type == UI_LAYOUT_HORIZONTAL) ||
                                             (axis == UI_WIDGET_Y_AXIS && parent->layout_type == UI_LAYOUT_VERTICAL));
        
        if (parent->layout_type == UI_LAYOUT_HORIZONTAL || parent->layout_type == UI_LAYOUT_VERTICAL) {
            if (axis_matches_parent_layout) {
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
                // all children will be sized at this point.
                // parent->computed_child_size_sum contains all children widths now, even FILL_REMAINING sized ones
                real32 d = ((parent->computed_size.x - parent->computed_child_size_sum.x) /
                            (parent->num_total_children - 1));
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

    if (widget->position_type == UI_POSITION_RELATIVE) {
        widget->computed_position[axis] += widget->semantic_position[axis];
    }
}

void ui_calculate_positions() {
    UI_Widget *current = ui_manager->root;

    // pre-order traversal
    
    while (current) {
        UI_Widget *parent = current->parent;
        
        calculate_position(current, UI_WIDGET_X_AXIS);
        calculate_position(current, UI_WIDGET_Y_AXIS);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!parent) break;

                UI_Widget *current_ancestor = parent;
                while (!current_ancestor->next) {
                    if (!current_ancestor->parent) break; // root
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

void ui_frame_init(real32 dt) {
    Display_Output *display_output = &game_state->render_state.display_output;
    // note that we have a global frame arena and a frame arena for ui (inside ui_manager).
    // frame arena for ui should be used for when we need to reference data in the frame arena
    // in the subsequent frame.
    // the global frame arena is reset every frame and no data is kept, so we use that for
    // rendering. we use the ui_manager's frame arena for ui state stuff.
    // TODO: replace this with NULL_THEME and set the size manually
    UI_Theme root_theme = {
        {}, {}, {}, "texture_default",
        {}, NULL, NULL,
        {}, 0, 0, 0, 0,
        UI_LAYOUT_NONE,
        { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE },
        UI_POSITION_NONE,
        { (real32) display_output->width, (real32) display_output->height },
        { 0.0f, 0.0f },
        UI_SCISSOR_NONE
    };

    UI_Widget *widget = make_widget(make_ui_id("root"), root_theme, 0);

    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&ui_manager->frame_arena, sizeof(UI_Stack_Widget));
    assert(ui_manager->widget_stack == NULL);
    entry->widget = widget;
    entry->next = ui_manager->widget_stack;
    ui_manager->widget_stack = entry;

    ui_manager->root = widget;

    ui_manager->widget_table = (UI_Widget **) arena_push(&ui_manager->frame_arena,
                                                         sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
    
    ui_table_add(ui_manager->widget_table, widget);

    ui_manager->hot_t    += dt;
    ui_manager->focus_t  += dt;
    ui_manager->active_t += dt;

    // reset ui render commands
    ui_manager->num_render_commands = 0;
    
    UI_Render_Data *render_data = &ui_manager->render_data;
    render_data->num_vertices = 0;
    render_data->num_indices = 0;
}

void ui_post_update() {
    ui_calculate_standalone_sizes();
    ui_calculate_ancestor_dependent_sizes();
    ui_calculate_child_dependent_sizes();
    ui_calculate_ancestor_dependent_sizes_part_2();
    ui_calculate_positions();
    
    // TODO: move this somewhere else probably, maybe to win32_game.cpp after update()
    ui_create_render_commands();
}

void ui_frame_end() {
    ui_manager->last_frame_root = ui_manager->root;
    ui_manager->root = NULL;

    UI_id states_to_delete[UI_MAX_STATES_TO_DELETE];
    int32 num_states_to_delete = 0;

    // loop through all the entries of the state table. if we find an entry for a widget
    // that doesn't exist this frame (by checking widget_table), then we delete that
    // widget's state from the state table.
    // note that it isn't enforced that a state is associated with a widget via its ID, since
    // it's kind of annoying to do so. if you have state, it'll probably be keyed with the
    // widget that it's associated with, i.e. a 1:1 relationship. we don't enforce this, but
    // if you don't do this, then this loop won't succeed in deleting state that's associated
    // with all of its widgets. i don't really think this is a case we need to handle right
    // now.
    for (int32 i = 0; i < NUM_WIDGET_BUCKETS; i++) {
        UI_Widget_State *current = ui_manager->state_table[i];
        while (current) {
            if (!ui_get_widget(current->id)) {
                assert(num_states_to_delete < UI_MAX_STATES_TO_DELETE);
                states_to_delete[num_states_to_delete++] = current->id;
            }
            current = current->next;
        }
    }

    // we don't want to delete while we're iterating through the same table, so delete them here
    for (int32 i = 0; i < num_states_to_delete; i++) {
        _ui_delete_state(states_to_delete[i]);
    }
    
    // swap allocators
    Arena_Allocator temp = ui_manager->last_frame_arena;
    ui_manager->last_frame_arena = ui_manager->frame_arena;
    ui_manager->frame_arena = temp;

    // swap tables
    UI_Widget **temp_widget_table = ui_manager->last_frame_widget_table;
    ui_manager->last_frame_widget_table = ui_manager->widget_table;
    ui_manager->widget_table = temp_widget_table;

    clear_arena(&ui_manager->frame_arena);
    
    // make sure we only have a single widget in the stack (should be root)
    assert(ui_manager->widget_stack->next == NULL);
    ui_manager->widget_stack = NULL;
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

inline bool32 in_bounds(Vec2 p, Vec2_int32 widget_position, Vec2_int32 widget_size) {
    return in_bounds(p,
                     (real32) widget_position.x, (real32) widget_position.x + widget_size.x,
                     (real32) widget_position.y, (real32) widget_position.y + widget_size.y);
}

inline bool32 in_bounds(Vec2 p, UI_Widget *widget) {
    bool32 in_computed_bounds = in_bounds(p,
                                          widget->computed_position, widget->computed_size);
    bool32 in_scissor_bounds = true;
    if (widget->scissor_type != UI_SCISSOR_NONE) {
        in_scissor_bounds = in_bounds(p,
                                      widget->computed_scissor_position,
                                      widget->computed_scissor_dimensions);
    }
        
    return (in_computed_bounds && in_scissor_bounds);
}

inline bool32 ui_id_equals(UI_id id1, UI_id id2) {
    return ((id1.string_ptr == id2.string_ptr) && (id1.index == id2.index));
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

// COMPOUND WIDGETS

void do_text(char *text, char *id, uint32 flags = 0, int32 index = 0) {
    UI_Theme text_theme = NULL_THEME;
    text_theme.size_type = { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT };
    text_theme.text_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    //text_theme.background_color = { 1.0f, 0.0f, 0.0f, 1.0f };
    //UI_Widget *text_widget = ui_add_widget(make_ui_id(id, index), text_theme, UI_WIDGET_DRAW_TEXT | UI_WIDGET_DRAW_BACKGROUND);
    UI_Widget *text_widget = ui_add_widget(make_ui_id(id, index), text_theme, UI_WIDGET_DRAW_TEXT);
    text_widget->text = text;
}

void do_text(char *text, char *id, UI_Theme theme, uint32 flags = 0, int32 index = 0) {
    UI_Theme text_theme = NULL_THEME;
    text_theme.size_type = theme.size_type;
    text_theme.semantic_position = theme.semantic_position;
    text_theme.semantic_size = theme.semantic_size;
    text_theme.text_color = theme.text_color;
    text_theme.layout_type = theme.layout_type;
    text_theme.font = theme.font;
    text_theme.scissor_type = theme.scissor_type;
    text_theme.position_type = theme.position_type;
    
    UI_Widget *text_widget = ui_add_widget(make_ui_id(id, index), text_theme, UI_WIDGET_DRAW_TEXT | flags);
    text_widget->text = text;
}

void do_text(char *text, uint32 flags = 0) {
    return do_text(text, "", flags);
}

void do_text(char *text, UI_Theme theme, uint32 flags = 0) {
    return do_text(text, "", theme, flags);
}

bool32 do_button(UI_id id, UI_Theme theme) {
    UI_Widget *widget = ui_add_widget(id, theme, UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    UI_Interact_Result interact_result = ui_interact(widget);
    
    return interact_result.released;
}

bool32 do_text_button(char *text, UI_Button_Theme button_theme, UI_id id) {
    UI_Theme theme = NULL_THEME;
    theme.semantic_position = button_theme.position;
    theme.size_type = button_theme.size_type;
    theme.semantic_size = button_theme.size;
    theme.layout_type = UI_LAYOUT_VERTICAL;
    theme.background_color = button_theme.background_color;
    theme.hot_background_color = button_theme.hot_background_color;
    theme.active_background_color = button_theme.active_background_color;
    theme.corner_radius = 5.0f;
    theme.corner_flags = CORNER_ALL;
    theme.scissor_type = button_theme.scissor_type;

    UI_Widget *button = ui_push_widget(id, theme,
                                       UI_WIDGET_DRAW_BORDER | UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);

    {
        UI_Theme row_theme = NULL_THEME;
        row_theme.layout_type = UI_LAYOUT_CENTER;
        row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
        // TODO: uhh, this doesn't assert because we never run set_scissor with it because this widget
        //       doesn't have any flags. set_scissor does run with this, however, when button_theme has
        //       a non-NONE scissor type, but in that case, we will eventually hit a SCISSOR_COMPUTED, as
        //       long as we laid our stuff out correctly.
        row_theme.scissor_type = UI_SCISSOR_INHERIT;
        
        ui_push_widget("", row_theme);
        {
            UI_Theme text_theme = NULL_THEME;
            text_theme.text_color = button_theme.text_color;
            text_theme.font = button_theme.font;
            text_theme.size_type = { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT };
            text_theme.scissor_type = button_theme.scissor_type;
            do_text(text, text_theme);
        }
        ui_pop_widget();
    }
    ui_pop_widget();
    
    UI_Interact_Result interact_result = ui_interact(button);

    return interact_result.released;
}

inline bool32 do_text_button(char *text, UI_Button_Theme button_theme, char *id, int32 index = 0) {
    return do_text_button(text, button_theme, make_ui_id(id, index));
}

bool32 ui_push_empty_button(UI_Button_Theme button_theme, UI_id id) {
    UI_Theme theme = NULL_THEME;
    theme.semantic_position = button_theme.position;
    theme.size_type = button_theme.size_type;
    theme.semantic_size = button_theme.size;
    theme.layout_type = UI_LAYOUT_VERTICAL;
    theme.background_color = button_theme.background_color;
    theme.hot_background_color = button_theme.hot_background_color;
    theme.active_background_color = button_theme.active_background_color;
    theme.corner_radius = 5.0f;
    theme.corner_flags = CORNER_ALL;
    theme.scissor_type = button_theme.scissor_type;

    UI_Widget *button = ui_push_widget(id, theme,
                                       UI_WIDGET_DRAW_BORDER | UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    UI_Interact_Result interact_result = ui_interact(button);

    return interact_result.released;
}

void ui_x_pad(real32 width) {
    UI_Theme theme = {};
    theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
    theme.semantic_size = { width, 0.0f };
    ui_add_widget("", theme);
}

void ui_y_pad(real32 height) {
    UI_Theme theme = {};
    theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
    theme.semantic_size = { 0.0f, height };
    ui_add_widget("", theme);
}

void ui_push_container(UI_Container_Theme theme, char *id = "") {
    UI_Theme column_theme = {};
    column_theme.size_type = theme.size_type;
    column_theme.semantic_size = theme.size;
    column_theme.layout_type = UI_LAYOUT_VERTICAL;
    column_theme.position_type = theme.position_type;
    column_theme.semantic_position = theme.position;
    column_theme.background_color = theme.background_color;
    column_theme.hot_background_color = theme.background_color;
    column_theme.active_background_color = theme.background_color;
    
    UI_Widget *inner;

    // vertical
    UI_Widget *column = ui_add_and_push_widget(id, column_theme,
                                               UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    {
        ui_y_pad(theme.padding.top);

        UI_Theme row_theme = {};
        row_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
        row_theme.semantic_size = { 1.0f, 1.0f };
        row_theme.layout_type = UI_LAYOUT_HORIZONTAL;

        ui_add_and_push_widget("", row_theme, 0);
        {
            ui_x_pad(theme.padding.left);

            UI_Theme inner_theme = {};
            inner_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
            inner_theme.semantic_size = { 0.0f, 0.0f };
            inner_theme.layout_type = theme.layout_type;
            //inner_theme.background_color = rgb_to_vec4(0, 255, 0);
            inner = ui_add_widget("", inner_theme, 0);

            ui_x_pad(theme.padding.right);
        }
        ui_pop_widget();

        ui_y_pad(theme.padding.bottom);
    }
    ui_pop_widget();

    // just so we get hot state, so that it gets clicks instead of whatever's behind it
    ui_interact(column);
    
    ui_push_existing_widget(inner);
}

struct UI_Window_Theme {
    Vec2 initial_position;
    Vec4 background_color;
    Vec4 title_text_color;
    Vec4 title_bgc;
    Vec4 title_hot_bgc;
    Vec4 title_active_bgc;
    uint32 corner_flags;
    real32 corner_radius;
    uint32 border_flags;
    Vec4 border_color;
    real32 border_width;
    Vec2 semantic_size;
};

void push_window(char *title, UI_Window_Theme theme, char *id_string, int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Window_State *state;
    if (!state_variant) {
        state = ui_add_window_state(id, theme.initial_position);
    } else {
        state = &state_variant->window;
    }
    
    // we only want to add windows to the root node
    assert(ui_manager->widget_stack->widget == ui_manager->root);

    UI_Theme window_theme = {};
    window_theme.semantic_position       = state->position;
    window_theme.size_type               = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };
    window_theme.semantic_size           = theme.semantic_size;
    window_theme.layout_type             = UI_LAYOUT_VERTICAL;
    window_theme.background_color        = theme.background_color;
    window_theme.hot_background_color    = theme.background_color;
    window_theme.active_background_color = theme.background_color;
    //window_theme.text_color              = theme.title_text_color;
    window_theme.corner_flags            = theme.corner_flags;
    window_theme.corner_radius           = theme.corner_radius;
    window_theme.border_flags            = theme.border_flags;
    window_theme.border_color            = theme.border_color;
    window_theme.border_width            = theme.border_width;
    UI_Widget *window = make_widget(id, window_theme,
                                    UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE | UI_WIDGET_IS_FOCUSABLE |
                                    UI_WIDGET_DRAW_BORDER);
    ui_add_and_push_widget(window);
    ui_interact(window);
    
    {
        UI_Theme title_bar_theme = {};
        title_bar_theme.size_type               = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
        title_bar_theme.semantic_size           = { 1.0f, 20.0f };
        title_bar_theme.layout_type             = UI_LAYOUT_CENTER;
        title_bar_theme.background_color        = theme.title_bgc;
        title_bar_theme.hot_background_color    = theme.title_hot_bgc;
        title_bar_theme.active_background_color = theme.title_active_bgc;
        title_bar_theme.text_color              = theme.title_text_color;
        UI_Widget *title_bar = make_widget("window-title-bar", title_bar_theme,
                                           UI_WIDGET_IS_CLICKABLE | UI_WIDGET_DRAW_BACKGROUND);
        ui_add_and_push_widget(title_bar);
        
        UI_Interact_Result title_bar_interact = ui_interact(title_bar);
        do_text(title, "");
        ui_pop_widget();

        if (title_bar_interact.holding) {
            Vec2 delta = get_mouse_delta();
            state->position += delta;
        }
    }
}

void set_is_open(UI_Dropdown_State *state, bool32 is_open) {
    state->t = 0;
    state->start_y_offset = state->y_offset;
    state->is_open = is_open;
}

void ui_push_dropdown(UI_Dropdown_Theme theme, char *button_text,
                      char *button_id_str, char *dropdown_id_str,
                      char *dropdown_inner_id_str, // this is for the content that actually moves
                      bool32 force_reset,
                      int32 index = 0) {
    // TODO: do we check that id is actually valid when creating states?
    // TODO: it's annoying having to pass in 3 different strings. i wish the ui_ids didn't use
    //       pointers, so we can dynamically create the strings. maybe use hashes?
    UI_id button_id         = make_ui_id(button_id_str, index);
    UI_id dropdown_id       = make_ui_id(dropdown_id_str, index);
    UI_id dropdown_inner_id = make_ui_id(dropdown_inner_id_str, index);

    UI_Widget_State *state_variant = ui_get_state(dropdown_id);
    UI_Dropdown_State *state;
    if (!state_variant) {
        // t is initialized to 1.0
        state = ui_add_dropdown_state(dropdown_id);
    } else {
        state = &state_variant->dropdown;
    }

    if (force_reset) {
        _ui_delete_state(dropdown_id);
        UI_Dropdown_State old_state = *state;
        state = ui_add_dropdown_state(dropdown_id);
        state->is_open        = old_state.is_open;
        state->y_offset       = old_state.y_offset;
        state->start_y_offset = old_state.start_y_offset;
        state->t              = old_state.t;
    }

    UI_Widget *computed_widget = ui_get_widget_prev_frame(dropdown_id);
    real32 dropdown_height = 0.0f;
    if (computed_widget) {
        dropdown_height = computed_widget->computed_size.y;
    }

    state->t += game_state->dt;
    
    UI_Theme column_theme = {};
    // these are for the container, basically that holds the button and the dropdown
    column_theme.size_type = theme.size_type;
    column_theme.semantic_size = theme.size;
    column_theme.layout_type = UI_LAYOUT_VERTICAL;

#if 0
    column_theme.position_type = theme.position_type;
    column_theme.semantic_position = theme.position;
    column_theme.background_color = theme.background_color;
    column_theme.hot_background_color = theme.background_color;
    column_theme.active_background_color = theme.background_color;
#endif
    
    UI_Widget *inner;

    // vertical
    UI_Widget *column = ui_add_and_push_widget("", column_theme,
                                               UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    // just so we get hot state, so that it gets clicks instead of whatever's behind it
    //ui_interact(column);
    {
        bool32 dropdown_button_clicked = ui_push_empty_button(theme.button_theme, button_id);
        {
            UI_Theme row_theme = NULL_THEME;
            row_theme.layout_type = UI_LAYOUT_HORIZONTAL;
            row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
            
            ui_push_widget("", row_theme);
            {
                ui_x_pad(5.0f);
                
                UI_Theme text_container = {};
                text_container.layout_type = UI_LAYOUT_CENTER;
                text_container.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
                text_container.scissor_type = UI_SCISSOR_COMPUTED_SIZE;
                
                ui_push_widget("", text_container);
                {
                    UI_Theme text_theme = NULL_THEME;
                    text_theme.text_color = theme.button_theme.text_color;
                    text_theme.font = theme.button_theme.font;
                    text_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_TEXT };
                    text_theme.scissor_type = UI_SCISSOR_INHERIT;
                    do_text(button_text, text_theme);
                }
                ui_pop_widget();

                ui_x_pad(10.0f);

                UI_Theme arrow_container = {};
                arrow_container.layout_type = UI_LAYOUT_CENTER;
                arrow_container.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FILL_REMAINING };

                ui_push_widget("", arrow_container);
                {
                    UI_Theme arrow_theme = {};
                    arrow_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
                    arrow_theme.semantic_size = { 10.0f, 10.0f };
                    arrow_theme.texture_name = "texture_default";

                    ui_add_widget("", arrow_theme, UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_USE_TEXTURE);
                } ui_pop_widget();

                ui_x_pad(5.0f);
            }
            ui_pop_widget();
        }
        ui_pop_widget();
        //bool32 dropdown_button_clicked = do_text_button(button_text, theme.button_theme, button_id);

        if (dropdown_button_clicked) {
            set_is_open(state, !state->is_open);
        }

#if 0
        // the widget that moves
        UI_Widget *computed_inner_widget = ui_get_widget_prev_frame(dropdown_inner_id);
        Vec2 mouse_pos = Context::controller_state->current_mouse;
        if (state->is_open &&
            just_pressed(Context::controller_state->left_mouse) &&
            !in_bounds(mouse_pos, computed_inner_widget)) {
            set_is_open(state, false);
        }
#endif
        
        real32 transition_time = 0.9f;
        // state->t is just the linear percentage we've made it through the transition_time
        state->t = min(state->t / transition_time, 1.0f);
        // t is what we use for the transition timing, but it's based on state->t
        real32 t = 1.0f - (1.0f - state->t) * (1.0f - state->t);

        // TODO: idk if we can use t here, since the position won't be the same on both sides of the curve.
        if (state->is_open) {
            state->y_offset = mix(state->start_y_offset, 0.0f, t);
        } else {
            state->y_offset = mix(state->start_y_offset, -dropdown_height, t);
        }

        // this container is just so that the float container doesn't get put on top of the button.
        UI_Theme content_container_theme = {};
        content_container_theme.layout_type = UI_LAYOUT_VERTICAL;
        content_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
        content_container_theme.semantic_size = { 1.0f, 0.0f }; 
        
        #if 1
        ui_add_and_push_widget("", content_container_theme);
        {
            UI_Theme list_container_theme = {};
            list_container_theme.position_type = UI_POSITION_FLOAT;
            list_container_theme.layout_type = UI_LAYOUT_VERTICAL;
            list_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
            list_container_theme.semantic_size = { 1.0f, 0.0f }; 
            list_container_theme.scissor_type = UI_SCISSOR_COMPUTED_SIZE;
            list_container_theme.background_color = rgb_to_vec4(255, 0, 0);

            UI_Widget *list_container = ui_add_and_push_widget(dropdown_id, list_container_theme);
            {
                UI_Theme inner_theme = {};
                inner_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
                inner_theme.layout_type = UI_LAYOUT_VERTICAL;
                inner_theme.semantic_size = { 1.0f, 0.0f };
                inner_theme.position_type = UI_POSITION_RELATIVE;
                inner_theme.semantic_position = { 0.0f, state->y_offset };
                inner_theme.scissor_type = UI_SCISSOR_INHERIT;
                inner = ui_add_widget(dropdown_inner_id, inner_theme, 0);
            }
            ui_pop_widget();
        }
        ui_pop_widget();
        #endif
    }
    ui_pop_widget();
    
    ui_push_existing_widget(inner);
}

int32 do_dropdown(UI_Dropdown_Theme theme,
                 char **items, int32 num_items,
                 int32 selected_index,
                 char *button_id_str, char *dropdown_id_str,
                 char *dropdown_inner_id_str, // this is for the content that actually moves
                 char *dropdown_item_id_str,
                 bool32 force_reset,
                 int32 index = 0) {
    // TODO: do we check that id is actually valid when creating states?
    // TODO: it's annoying having to pass in 3 different strings. i wish the ui_ids didn't use
    //       pointers, so we can dynamically create the strings. maybe use hashes?
    UI_id button_id         = make_ui_id(button_id_str, index);
    UI_id dropdown_id       = make_ui_id(dropdown_id_str, index);
    UI_id dropdown_inner_id = make_ui_id(dropdown_inner_id_str, index);

    UI_Widget_State *state_variant = ui_get_state(dropdown_id);
    UI_Dropdown_State *state;
    if (!state_variant) {
        // t is initialized to 1.0
        state = ui_add_dropdown_state(dropdown_id);
    } else {
        state = &state_variant->dropdown;
    }

    if (force_reset) {
        _ui_delete_state(dropdown_id);
        UI_Dropdown_State old_state = *state;
        state = ui_add_dropdown_state(dropdown_id);
        state->is_open        = old_state.is_open;
        state->y_offset       = old_state.y_offset;
        state->start_y_offset = old_state.start_y_offset;
        state->t              = old_state.t;
    }

    UI_Widget *computed_widget = ui_get_widget_prev_frame(dropdown_id);
    real32 dropdown_height = 0.0f;
    if (computed_widget) {
        dropdown_height = computed_widget->computed_size.y;
    }

    state->t += game_state->dt;
    
    UI_Theme column_theme = {};
    // these are for the container, basically that holds the button and the dropdown
    column_theme.size_type = theme.size_type;
    column_theme.semantic_size = theme.size;
    column_theme.layout_type = UI_LAYOUT_VERTICAL;

#if 0
    column_theme.position_type = theme.position_type;
    column_theme.semantic_position = theme.position;
    column_theme.background_color = theme.background_color;
    column_theme.hot_background_color = theme.background_color;
    column_theme.active_background_color = theme.background_color;
#endif

    assert(selected_index < num_items);
    int32 selected_item_index = selected_index;

    Controller_State *controller_state = Context::controller_state;
    if (state->is_open) {
        if (just_pressed_or_repeated(controller_state->key_down)) {
            selected_item_index++;
        }

        if (just_pressed_or_repeated(controller_state->key_up)) {
            selected_item_index--;
        }

        if (just_pressed(controller_state->key_enter)) {
            set_is_open(state, false);
        }
    }

    selected_item_index = clamp(selected_item_index, 0, num_items - 1);

    // vertical
    UI_Widget *column = ui_add_and_push_widget("", column_theme,
                                               UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    bool32 dropdown_button_clicked = false;
    // just so we get hot state, so that it gets clicks instead of whatever's behind it
    //ui_interact(column);
    {
        dropdown_button_clicked = ui_push_empty_button(theme.button_theme, button_id);
        {
            UI_Theme row_theme = NULL_THEME;
            row_theme.layout_type = UI_LAYOUT_HORIZONTAL;
            row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
            
            ui_push_widget("", row_theme);
            {
                ui_x_pad(5.0f);
                
                UI_Theme text_container = {};
                text_container.layout_type = UI_LAYOUT_CENTER;
                text_container.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
                text_container.scissor_type = UI_SCISSOR_COMPUTED_SIZE;
                
                ui_push_widget("", text_container);
                {
                    UI_Theme text_theme = NULL_THEME;
                    text_theme.text_color = theme.button_theme.text_color;
                    text_theme.font = theme.button_theme.font;
                    text_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_TEXT };
                    text_theme.scissor_type = UI_SCISSOR_INHERIT;
                    if (selected_index > -1) {
                        do_text(items[selected_index], text_theme);
                    }
                }
                ui_pop_widget();

                ui_x_pad(10.0f);

                UI_Theme arrow_container = {};
                arrow_container.layout_type = UI_LAYOUT_CENTER;
                arrow_container.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FILL_REMAINING };

                ui_push_widget("", arrow_container);
                {
                    UI_Theme arrow_theme = {};
                    arrow_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
                    arrow_theme.semantic_size = { 10.0f, 10.0f };
                    arrow_theme.texture_name = "texture_default";

                    ui_add_widget("", arrow_theme, UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_USE_TEXTURE);
                } ui_pop_widget();

                ui_x_pad(5.0f);
            }
            ui_pop_widget();
        }
        ui_pop_widget();
        //bool32 dropdown_button_clicked = do_text_button(button_text, theme.button_theme, button_id);

        if (dropdown_button_clicked) {
            set_is_open(state, !state->is_open);
        }

#if 0
        // the widget that moves
        UI_Widget *computed_inner_widget = ui_get_widget_prev_frame(dropdown_inner_id);
        Vec2 mouse_pos = Context::controller_state->current_mouse;
        if (state->is_open &&
            just_pressed(Context::controller_state->left_mouse) &&
            !in_bounds(mouse_pos, computed_inner_widget)) {
            set_is_open(state, false);
        }
#endif
        
        real32 transition_time = 0.9f;
        // state->t is just the linear percentage we've made it through the transition_time
        state->t = min(state->t / transition_time, 1.0f);
        // t is what we use for the transition timing, but it's based on state->t
        real32 t = 1.0f - (1.0f - state->t) * (1.0f - state->t);

        // TODO: idk if we can use t here, since the position won't be the same on both sides of the curve.
        if (state->is_open) {
            state->y_offset = mix(state->start_y_offset, 0.0f, t);
        } else {
            state->y_offset = mix(state->start_y_offset, -dropdown_height, t);
        }

        // this container is just so that the float container doesn't get put on top of the button.
        UI_Theme content_container_theme = {};
        content_container_theme.layout_type = UI_LAYOUT_VERTICAL;
        content_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
        content_container_theme.semantic_size = { 1.0f, 0.0f }; 
        
#if 1
        ui_add_and_push_widget("", content_container_theme);
        {
            UI_Theme list_container_theme = {};
            list_container_theme.position_type = UI_POSITION_FLOAT;
            list_container_theme.layout_type = UI_LAYOUT_VERTICAL;
            list_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
            list_container_theme.semantic_size = { 1.0f, 0.0f }; 
            list_container_theme.scissor_type = UI_SCISSOR_COMPUTED_SIZE;
            list_container_theme.background_color = rgb_to_vec4(255, 0, 0);

            UI_Widget *list_container = ui_add_and_push_widget(dropdown_id, list_container_theme);
            {
                UI_Theme inner_theme = {};
                inner_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
                inner_theme.layout_type = UI_LAYOUT_VERTICAL;
                inner_theme.semantic_size = { 1.0f, 0.0f };
                inner_theme.position_type = UI_POSITION_RELATIVE;
                inner_theme.semantic_position = { 0.0f, state->y_offset };
                inner_theme.scissor_type = UI_SCISSOR_INHERIT;
                ui_add_and_push_widget(dropdown_inner_id, inner_theme, 0);
                {
                    bool32 an_item_was_clicked = false;
                    for (int32 i = 0; i < num_items; i++) {
                        bool32 item_clicked = do_text_button(items[i],
                                                             (i == selected_index) ? theme.selected_item_theme : theme.item_theme,
                                                             dropdown_item_id_str, i);
                        if (item_clicked) {
                            selected_item_index = i;
                            an_item_was_clicked = true;
                        }
                    }

                    if (an_item_was_clicked) {
                        set_is_open(state, false);
                    }
                } ui_pop_widget();
            }
            ui_pop_widget();
        }
        ui_pop_widget();
#endif
    }
    ui_pop_widget();

    if (just_pressed(Context::controller_state->left_mouse) &&
        ui_manager->active.string_ptr != button_id_str &&
        ui_manager->active.string_ptr != dropdown_item_id_str &&
        ui_manager->active.string_ptr != dropdown_id_str) {
        set_is_open(state, false);
    }
    
    return selected_item_index;
}


void handle_text_field_input(UI_Text_Field_State *state) {
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

    if (controller_state->key_left.is_down && state->cursor_index > 0) {
        state->cursor_timer = 0.0f;
    }

    if (controller_state->key_right.is_down && state->cursor_index < state->buffer.current_length) {
        state->cursor_timer = 0.0f;
    }

    String_Buffer *buffer = &state->buffer;
    for (int32 i = 0; i < controller_state->num_pressed_chars; i++) {
        char c = controller_state->pressed_chars[i];
        if (c == '\b') { // backspace key
            if (state->cursor_index > 0) {
                splice(&state->buffer, state->cursor_index - 1);
                state->cursor_index--;
                state->cursor_index = max(state->cursor_index, 0);
                state->cursor_timer = 0.0f;
            }
        } else if ((buffer->current_length < buffer->size) && (state->cursor_index < buffer->size)) {
            if (c >= 32 && c <= 126) {
                splice_insert(&state->buffer, state->cursor_index, c);
                state->cursor_index++;
                state->cursor_timer = 0.0f;
            }
        }
    }
}

// note that text_widget_id needs to be the id of the widget that is specifies the bounds of the
// text. this can be confusing because your topmost parent might have padding and if you pass that
// id into this function, your text will be going outside of your expected bounds.
void handle_text_field_cursor(UI_id text_widget_id, UI_Text_Field_State *state,
                              Font *font, char *text, real32 *width_to_cursor_index) {
    *width_to_cursor_index = get_width(font, text, state->cursor_index);

    UI_Widget *computed_widget = ui_table_get(ui_manager->last_frame_widget_table, text_widget_id);

    // this is always positive. think of it as pushing the window to the right.
    // when we later pass it to text_theme, we make it negative, since we're deciding
    // where the text should go.
    real32 x_offset = 0.0f;
    if (computed_widget) {
        real32 text_width = get_width(font, state->buffer);
        real32 computed_width = computed_widget->computed_size.x;
                    
        if (*width_to_cursor_index < state->x_offset) {
            state->x_offset = *width_to_cursor_index;
        } else if (*width_to_cursor_index > state->x_offset + computed_width) {
            state->x_offset = *width_to_cursor_index - computed_width;
        }
    }
}

String do_text_field(UI_Text_Field_Theme theme,
                     String value, bool32 force_reset,
                     char *id_string, char *text_id_string,
                     int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Text_Field_State *state;
    if (!state_variant) {
        state = ui_add_text_field_state(id, value);
    } else {
        state = &state_variant->text_field;
    }

    if (force_reset) {
        _ui_delete_state(id);
        state = ui_add_text_field_state(id, value);
    }

    UI_Theme textbox_theme = {};
    textbox_theme.layout_type             = UI_LAYOUT_HORIZONTAL;
    textbox_theme.size_type               = theme.size_type;
    textbox_theme.semantic_size           = theme.size;
    textbox_theme.background_color        = theme.background_color;
    textbox_theme.hot_background_color    = theme.hot_background_color;
    textbox_theme.active_background_color = theme.active_background_color;
    textbox_theme.font                    = theme.font;
    textbox_theme.corner_radius           = theme.corner_radius;
    textbox_theme.corner_flags            = theme.corner_flags;
    textbox_theme.border_flags            = theme.border_flags;
    textbox_theme.border_color            = theme.border_color;
    textbox_theme.border_width            = theme.border_width;
    
    uint32 textbox_flags = (UI_WIDGET_IS_CLICKABLE | UI_WIDGET_IS_FOCUSABLE |
                            UI_WIDGET_DRAW_BORDER | UI_WIDGET_DRAW_BACKGROUND);
    
    UI_Widget *textbox = ui_add_and_push_widget(id_string, textbox_theme, textbox_flags);
                                                
    UI_Interact_Result interact = ui_interact(textbox);
    Font *font = get_font(textbox->font);

    state->cursor_timer += game_state->dt;
    
    real32 x_padding = 5.0f;
    if (interact.just_pressed || interact.holding) {
        int32 last_index = 0;
        
        // find where to put the cursor
        // the +3.0 is to add some tolerance around clicking between the characters instead of being
        // completely biased towards one side.
        // TODO: there might be a nice way to calculate that tolerance value
        real32 adjusted_cursor_x = interact.relative_mouse.x - x_padding + 3.0f + state->x_offset;
        for (int32 i = 1; i <= state->buffer.current_length; i++) {
            real32 width_to_i = get_width(font, &state->buffer, i);
            if (width_to_i > adjusted_cursor_x) {
                break;
            }
            last_index = i;
        }

        state->cursor_timer = 0.0f;
        state->cursor_index = last_index;
    }
    
    if (is_focus(textbox)) {
        handle_text_field_input(state);
    }

    UI_id text_widget_id = make_ui_id(text_id_string, index);
    char *str = to_char_array((Allocator *) &ui_manager->frame_arena, state->buffer);
    real32 width_to_cursor_index;
    // this should only be called after calling handle_text_field_input() because state->buffer
    // could change. i don't think it matters that we still call it even if we don't handle input.
    handle_text_field_cursor(text_widget_id, state, font, str, &width_to_cursor_index);
    
    {
        ui_x_pad(x_padding);

        UI_Theme inner_field_theme = {};
        inner_field_theme.size_type     = { UI_SIZE_FILL_REMAINING, UI_SIZE_PERCENTAGE };
        inner_field_theme.layout_type   = UI_LAYOUT_CENTER;
        inner_field_theme.semantic_size = { 1.0f, 1.0f };

        ui_add_and_push_widget("", inner_field_theme);
        {

            UI_Theme text_container_theme = {};
            text_container_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
            text_container_theme.semantic_size = { 0.0f, 1.0f };
            text_container_theme.layout_type = UI_LAYOUT_CENTER;
            text_container_theme.background_color = rgb_to_vec4(255, 0, 0);
            text_container_theme.scissor_type = UI_SCISSOR_COMPUTED_SIZE;

            UI_Widget *text_widget = make_widget(text_widget_id, text_container_theme, 0);
            ui_add_and_push_widget(text_widget);
            {
                UI_Theme text_theme = {};
                text_theme.font = default_font;
                text_theme.text_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
                text_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_TEXT };
                text_theme.semantic_size = { 1.0f, 0.0f };
                text_theme.position_type = UI_POSITION_RELATIVE;
                text_theme.semantic_position = { -state->x_offset, 0.0f };
                text_theme.scissor_type = UI_SCISSOR_INHERIT;
                do_text(str, "", text_theme);
                
                if (is_active(textbox) || is_focus(textbox)) {
                    // draw cursor
                    UI_Theme cursor_theme = {};
                    cursor_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_PERCENTAGE };
                    cursor_theme.semantic_size = { 1.0f, 1.0f };
                    cursor_theme.position_type = UI_POSITION_FLOAT;
                    cursor_theme.background_color = theme.cursor_color;
                    
                    cursor_theme.semantic_position = { floorf(width_to_cursor_index - state->x_offset), 0.0f };    
                    
                    real32 time_to_switch = 0.5f; // time spent in either state
                    bool32 show_background = ((int32) (state->cursor_timer*(1.0f / time_to_switch)) + 1) % 2;
                    
                    UI_Widget *cursor = ui_add_widget("", cursor_theme,
                                                      show_background ? UI_WIDGET_DRAW_BACKGROUND : 0);
                }
            }
            ui_pop_widget();
        }
        ui_pop_widget();

        ui_x_pad(x_padding);
    }
    ui_pop_widget();

    return make_string(state->buffer);
}

void ui_add_slider_bar(UI_Slider_Theme theme, real32 value, real32 min, real32 max) {
    UI_Theme slider_theme = {};
    slider_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_PERCENTAGE };
    slider_theme.position_type = UI_POSITION_FLOAT;
    slider_theme.background_color = theme.background_color;
    slider_theme.semantic_size = { clamp((value - min) / (max - min), 0.0f, 1.0f), 1.0f };
    
    ui_add_widget("", slider_theme, UI_WIDGET_DRAW_BACKGROUND);
}

real32 do_text_field_slider(real32 value,
                            //real32 min_value, real32 max_value,
                            UI_Text_Field_Slider_Theme theme,
                            char *id_string, char *text_id_string,
                            bool32 force_reset,
                            int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Text_Field_Slider_State *state;
    if (!state_variant) {
        state = ui_add_text_field_slider_state(id, value);
    } else {
        state = &state_variant->text_field_slider;
    }

    if (force_reset) {
        _ui_delete_state(id);
        state = ui_add_text_field_slider_state(id, value);
    }
    
    UI_Text_Field_State *text_field_state = &state->text_field_state;
    
    UI_Theme textbox_theme = {};
    textbox_theme.layout_type             = UI_LAYOUT_HORIZONTAL;
    textbox_theme.size_type               = theme.size_type;
    textbox_theme.semantic_size           = theme.size;
    textbox_theme.background_color        = theme.field_background_color;
    textbox_theme.hot_background_color    = theme.field_hot_background_color;
    textbox_theme.active_background_color = theme.field_active_background_color;
    textbox_theme.font                    = theme.font;
    textbox_theme.corner_radius           = theme.corner_radius;
    textbox_theme.corner_flags            = theme.corner_flags;
    textbox_theme.border_flags            = theme.border_flags;
    textbox_theme.border_color            = theme.border_color;
    textbox_theme.border_width            = theme.border_width;

    uint32 textbox_flags = UI_WIDGET_IS_CLICKABLE | UI_WIDGET_IS_FOCUSABLE | UI_WIDGET_DRAW_BORDER | UI_WIDGET_DRAW_BACKGROUND;
    //if (theme.show_field_background) textbox_flags |= UI_WIDGET_DRAW_BACKGROUND;

    UI_Widget *textbox = ui_add_and_push_widget(id_string, textbox_theme, textbox_flags);
    Font *font = get_font(textbox->font);

    text_field_state->cursor_timer += game_state->dt;
    
    UI_Interact_Result interact = ui_interact(textbox);
    real32 deadzone = 1.0f;
    
    if (interact.just_pressed) {
        state->mouse_press_start_relative_pos = interact.relative_mouse;
        state->mouse_press_start_t = platform_get_wall_clock_time();
    }

    if (interact.holding) {
        if (state->mode == Text_Field_Slider_Mode::NONE) {
            real32 x_delta = fabsf((interact.relative_mouse - state->mouse_press_start_relative_pos).x);
        
            if (x_delta > deadzone) {
                state->mode = Text_Field_Slider_Mode::SLIDING;
            }
            // maybe if we hold it long enough in the deadzone, we just turn into typing?
        }
    } else if (interact.released) {
        if (state->mode != Text_Field_Slider_Mode::SLIDING) {
            state->mode = Text_Field_Slider_Mode::TYPING;
            set_string_buffer_text(&text_field_state->buffer, make_string(state->current_text));
        }
    }

    real32 result = 0.0f;
    
    // we need to use is_focus and is_active for handling changing states when the mouse is not
    // on top of the textbox.
    // these blocks are for when we just exited typing or sliding state
    if (state->mode == Text_Field_Slider_Mode::TYPING && !is_focus(textbox)) {
        state->mode = Text_Field_Slider_Mode::NONE;

        // validate the text_field_state buffer and if it's fine, we set the current_text buffer to
        // its value
        real32 slider_value;
        bool32 conversion_result = string_to_real32(make_string(text_field_state->buffer), &slider_value);
        if (conversion_result) {
            Marker m = begin_region();
            char *value_text = string_format(temp_region, "%f", slider_value);
            set_string_buffer_text(&state->current_text, value_text);
            end_region(m);
        }
    } else if (state->mode == Text_Field_Slider_Mode::SLIDING && !is_active(textbox)) {
        state->mode = Text_Field_Slider_Mode::NONE;
        bool32 conversion_result = string_to_real32(make_string(state->current_text), &result);
        assert(conversion_result);
    }

    // below is based on code from do_text_field(). it is ran when we're currently focused on
    // the text field when we're in typing mode, or when the widget is active and in sliding
    // mode.
    real32 x_padding = 5.0f;
    if (state->mode == Text_Field_Slider_Mode::TYPING) {
        if (interact.just_pressed || interact.released || interact.holding) {
            int32 last_index = 0;
        
            real32 adjusted_cursor_x = interact.relative_mouse.x - x_padding + 3.0f + text_field_state->x_offset;
            for (int32 i = 1; i <= text_field_state->buffer.current_length; i++) {
                real32 width_to_i = get_width(font, &text_field_state->buffer, i);
                if (width_to_i > adjusted_cursor_x) {
                    break;
                }
                last_index = i;
            }

            text_field_state->cursor_timer = 0.0f;
            text_field_state->cursor_index = last_index;
        }
    } else if (state->mode == Text_Field_Slider_Mode::SLIDING) {
        Vec2 mouse_delta = get_mouse_delta();
        real32 x_delta = mouse_delta.x;

        real32 slider_value;
        bool32 conversion_result = string_to_real32(make_string(state->current_text), &slider_value);
        assert(conversion_result);
        
        slider_value += x_delta;

        Marker m = begin_region();
        char *value_text = string_format(temp_region, "%f", slider_value);
        set_string_buffer_text(&state->current_text, value_text);
        end_region(m);

        result = slider_value;
    }

    if (state->mode != Text_Field_Slider_Mode::SLIDING) {
        // if we just exited either typing or sliding state, we'll be in NONE mode and
        // we'll set the result value here. we only ever use current_text to set the
        // value because current_text has the validated text. i.e. we don't set
        // current_text while we're typing. we only set it once we've unfocused the text
        // field and we validate the text.

        // when in SLIDING mode, we set result above, since that doesn't need validation
        // and can be done in realtime. and it's a nicer UX.
        bool32 conversion_result = string_to_real32(make_string(state->current_text), &result);
        assert(conversion_result);
    }

    if (is_focus(textbox)) {
        handle_text_field_input(text_field_state);
    }

    UI_id text_widget_id = make_ui_id(text_id_string, index);
    char *str;

    if (state->mode == Text_Field_Slider_Mode::TYPING) {
        str = to_char_array((Allocator *) &ui_manager->frame_arena, text_field_state->buffer);
    } else {
        str = to_char_array((Allocator *) &ui_manager->frame_arena, state->current_text);
    }

    real32 width_to_cursor_index;
    // this should only be called after calling handle_text_field_input() because state->buffer
    // could change. i don't think it matters that we still call it even if we don't handle input.
    handle_text_field_cursor(text_widget_id, text_field_state, font, str, &width_to_cursor_index);

    {
        ui_x_pad(x_padding);

        UI_Theme inner_field_theme = {};
        inner_field_theme.size_type     = { UI_SIZE_FILL_REMAINING, UI_SIZE_PERCENTAGE };
        inner_field_theme.layout_type   = UI_LAYOUT_CENTER;
        inner_field_theme.semantic_size = { 1.0f, 1.0f };

        ui_add_and_push_widget("", inner_field_theme);
        {

            UI_Theme text_container_theme = {};
            text_container_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
            text_container_theme.semantic_size = { 0.0f, 1.0f };
            text_container_theme.layout_type = UI_LAYOUT_CENTER;
            text_container_theme.background_color = rgb_to_vec4(255, 0, 0);
            text_container_theme.scissor_type = UI_SCISSOR_COMPUTED_SIZE;

            UI_Widget *text_widget = make_widget(text_widget_id, text_container_theme, 0);
            ui_add_and_push_widget(text_widget);
            {
                UI_Theme text_theme = {};
                text_theme.font = theme.font;
                text_theme.text_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
                text_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_TEXT };
                text_theme.semantic_size = { 1.0f, 0.0f };
                text_theme.position_type = UI_POSITION_RELATIVE;
                if (is_focus(textbox)) {
                    text_theme.semantic_position = { -text_field_state->x_offset, 0.0f };
                } else {
                    text_theme.semantic_position = { 0.0f, 0.0f };
                }
                
                text_theme.scissor_type = UI_SCISSOR_INHERIT;
                do_text(str, "", text_theme);
                
                if (state->mode == Text_Field_Slider_Mode::TYPING) {
                    // draw cursor
                    UI_Theme cursor_theme = {};
                    cursor_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_PERCENTAGE };
                    cursor_theme.semantic_size = { 1.0f, 1.0f };
                    cursor_theme.position_type = UI_POSITION_FLOAT;
                    cursor_theme.background_color = theme.cursor_color;
                    
                    cursor_theme.semantic_position = { floorf(width_to_cursor_index - text_field_state->x_offset), 0.0f };    
                    
                    real32 time_to_switch = 0.5f; // time spent in either state
                    bool32 show_background = ((int32) (text_field_state->cursor_timer*(1.0f / time_to_switch)) + 1) % 2;
                    
                    UI_Widget *cursor = ui_add_widget("", cursor_theme,
                                                      show_background ? UI_WIDGET_DRAW_BACKGROUND : 0);
                }
            }
            ui_pop_widget();
        }
        ui_pop_widget();

        ui_x_pad(x_padding);
    }
    ui_pop_widget();

    return result;
}
