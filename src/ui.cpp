#include "ui.h"

char *default_font = "calibri14";
UI_Theme NULL_THEME = {
    {}, {}, {},
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

void ui_pop_widget() {
    assert(ui_manager->widget_stack);
    ui_manager->widget_stack = ui_manager->widget_stack->next;
}

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
        {}, {}, {},
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

UI_Text_Field_State *ui_add_text_field_state(UI_id id, String value) {
    UI_Widget_State *state = ui_make_widget_state();
    state->id = id;
    state->type = UI_STATE_TEXT_FIELD;
    String_Buffer buffer = make_string_buffer((Allocator *) &ui_manager->persistent_heap, value, 64);
    state->text_field_slider = { buffer, false, 0 };
    
    _ui_add_state(state);

    return &state->text_field;
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
    UI_Theme text_theme = NULL_THEME;
    text_theme.size_type = { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT };
    text_theme.text_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    //text_theme.background_color = { 1.0f, 0.0f, 0.0f, 1.0f };
    //UI_Widget *text_widget = ui_add_widget(make_ui_id(id, index), UI_WIDGET_DRAW_TEXT | UI_WIDGET_DRAW_BACKGROUND);
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

void do_text(char *text) {
    return do_text(text, "");
}

void do_text(char *text, UI_Theme theme) {
    return do_text(text, "", theme);
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

    UI_Widget *button = ui_push_widget(id, theme,
                                       UI_WIDGET_DRAW_BORDER | UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);

    {
        UI_Theme row_theme = NULL_THEME;
        row_theme.layout_type = UI_LAYOUT_CENTER;
        row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
        ui_push_widget("", row_theme);
        {
            UI_Theme text_theme = NULL_THEME;
            text_theme.text_color = button_theme.text_color;
            text_theme.font = button_theme.font;
            text_theme.size_type = { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT };
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

void ui_push_container(UI_Container_Theme theme) {
    UI_Theme column_theme = {};
    column_theme.size_type = theme.size_type;
    column_theme.semantic_size = theme.size;
    column_theme.layout_type = UI_LAYOUT_VERTICAL;
    column_theme.position_type = theme.position_type;
    column_theme.semantic_position = theme.position;
    column_theme.background_color = theme.background_color;
    
    UI_Widget *inner;

    // vertical
    UI_Widget *column = ui_add_and_push_widget("", column_theme, UI_WIDGET_DRAW_BACKGROUND);
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

    ui_push_existing_widget(inner);
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
                            real32 min_value, real32 max_value,
                            UI_Text_Field_Slider_Theme theme,
                            char *id_string, int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Text_Field_Slider_State *state;
    if (!state_variant) {
        state = ui_add_text_field_slider_state(id, value);
    } else {
        state = &state_variant->text_field_slider;
    }

    UI_Theme textbox_theme = {};
    textbox_theme.layout_type             = UI_LAYOUT_HORIZONTAL;
    textbox_theme.size_type               = theme.size_type;
    textbox_theme.semantic_size           = theme.size;
    textbox_theme.background_color        = theme.field_background_color;
    textbox_theme.hot_background_color    = theme.field_background_color;
    textbox_theme.active_background_color = theme.field_background_color;
    textbox_theme.font                    = theme.font;
    textbox_theme.corner_radius           = theme.corner_radius;
    textbox_theme.corner_flags            = theme.corner_flags;
    textbox_theme.border_flags            = theme.border_flags;
    textbox_theme.border_color            = theme.border_color;
    textbox_theme.border_width            = theme.border_width;

    uint32 textbox_flags = UI_WIDGET_IS_CLICKABLE | UI_WIDGET_IS_FOCUSABLE | UI_WIDGET_DRAW_BORDER;
    if (theme.show_field_background) textbox_flags |= UI_WIDGET_DRAW_BACKGROUND;
    
    UI_Widget *textbox = ui_add_and_push_widget(id_string, textbox_theme, textbox_flags);
                                                
    UI_Interact_Result interact = ui_interact(textbox);

    real32 x_delta = fabsf((get_mouse_delta()).x);
    real32 deadzone = 1.0f;

    if (!state->is_using) {
        state->is_sliding = is_active(textbox) && (x_delta > deadzone);

        if (state->is_sliding) {
            state->is_using = true;
        } else if (interact.released) {
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
            if (!is_focus(textbox)) {
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

    #if 1
    UI_Slider_Theme slider_theme = {};
    slider_theme.background_color = theme.slider_background_color;
    
    if (theme.show_slider) {
        if (!state->is_using || state->is_sliding) {
            ui_add_slider_bar(slider_theme, value, min_value, max_value);
        }
    }
    
    {
        ui_x_pad(5.0f);
        UI_Theme inner_field_theme = {};
        inner_field_theme.size_type     = { UI_SIZE_FILL_REMAINING, UI_SIZE_PERCENTAGE };
        inner_field_theme.layout_type   = UI_LAYOUT_CENTER;
        inner_field_theme.semantic_size = { 1.0f, 1.0f };

        ui_add_and_push_widget("", inner_field_theme);
        {
            ui_x_pad(5.0f);

            UI_Theme text_theme = {};
            text_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
            text_theme.layout_type = UI_LAYOUT_HORIZONTAL;

            ui_add_and_push_widget("", text_theme);
            {
                if (state->is_using && !state->is_sliding) {
                    char *str = to_char_array((Allocator *) &ui_manager->frame_arena, state->buffer);
                    do_text(str, "");

                    // draw cursor

                    UI_Theme cursor_theme = {};
                    cursor_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_PERCENTAGE };
                    cursor_theme.semantic_size = { 1.0f, 1.0f };
                    cursor_theme.position_type = UI_POSITION_FLOAT;
                    cursor_theme.background_color = theme.cursor_color;
                    
                    Font *font = get_font(textbox->font);
                    real32 width_to_index = get_width(font, str, state->cursor_index);
                    cursor_theme.semantic_position = { floorf(width_to_index), 0.0f };
                    
                    real32 time_to_switch = 0.5f; // time spent in either state
                    bool32 show_background = ((int32) (ui_manager->focus_t*(1.0f / time_to_switch)) + 1) % 2;
                    
                    UI_Widget *cursor = ui_add_widget("", cursor_theme,
                                                      show_background ? UI_WIDGET_DRAW_BACKGROUND : 0);
                } else {
                    char *buf = string_format((Allocator *) &ui_manager->frame_arena, "%f", value);
                    do_text(buf, "");
                }
                //do_text("hello");
            }
            ui_pop_widget();

            ui_x_pad(5.0f);
        }
        ui_pop_widget();
    }
    #endif
    ui_pop_widget();

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
    window_theme.layout_type             = UI_LAYOUT_VERTICAL;
    window_theme.size_type               = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };
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

String do_text_field(UI_Text_Field_Theme theme,
                     String value,
                     char *id_string, int32 index = 0) {
    UI_id id = make_ui_id(id_string, index);
    UI_Widget_State *state_variant = ui_get_state(id);
    UI_Text_Field_State *state;
    if (!state_variant) {
        state = ui_add_text_field_state(id, value);
    } else {
        state = &state_variant->text_field;
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
        
        // find where to put it
        for (int32 i = 1; i <= state->buffer.current_length; i++) {
            real32 width_to_i = get_width(font, &state->buffer, i);
            if (width_to_i > interact.relative_mouse.x - x_padding + 3.0f) {
                break;
            }
            last_index = i;
        }

        state->cursor_timer = 0.0f;
        state->cursor_index = last_index;
    }
    
    if (is_focus(textbox)) {
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

        if (controller_state->key_left.is_down || controller_state->key_right.is_down) {
            state->cursor_timer = 0.0f;
        }

        String_Buffer *buffer = &state->buffer;
        for (int32 i = 0; i < controller_state->num_pressed_chars; i++) {
            char c = controller_state->pressed_chars[i];
            if (c == '\b') { // backspace key
                splice(&state->buffer, state->cursor_index - 1);
                state->cursor_index--;
                state->cursor_index = max(state->cursor_index, 0);
                state->cursor_timer = 0.0f;
            } else if ((buffer->current_length < buffer->size) && (state->cursor_index < buffer->size)) {
                if (c >= 32 && c <= 126) {
                    splice_insert(&state->buffer, state->cursor_index, c);
                    state->cursor_index++;
                    state->cursor_timer = 0.0f;
                }
            }
        }

        if (state->cursor_index != original_cursor_index) {
            ui_manager->focus_t = 0.0f;
        }
    }

#if 1
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

            ui_add_and_push_widget("", text_container_theme, UI_WIDGET_DRAW_BACKGROUND);
            {
                UI_Theme text_theme = {};
                text_theme.font = default_font;
                text_theme.text_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
                text_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_TEXT };
                text_theme.semantic_size = { 1.0f, 0.0f };
                text_theme.position_type = UI_POSITION_RELATIVE;
                //text_theme.semantic_position = { -10.0f, 0.0f };
                text_theme.semantic_position = { 0.0f, 0.0f };
                text_theme.scissor_type = UI_SCISSOR_INHERIT;
                char *str = to_char_array((Allocator *) &ui_manager->frame_arena, state->buffer);
                do_text(str, "", text_theme);

                if (is_active(textbox) || is_focus(textbox)) {
                    // draw cursor
                    UI_Theme cursor_theme = {};
                    cursor_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_PERCENTAGE };
                    cursor_theme.semantic_size = { 1.0f, 1.0f };
                    cursor_theme.position_type = UI_POSITION_FLOAT;
                    cursor_theme.background_color = theme.cursor_color;
                    
                    real32 width_to_index = get_width(font, str, state->cursor_index);
                    cursor_theme.semantic_position = { floorf(width_to_index), 0.0f };
                    
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
#endif
    ui_pop_widget();

    // TODO: should we only return a value if we lost focus?
    
    return value;
}
