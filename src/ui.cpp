#include "ui.h"

UI_Widget *make_widget(UI_Manager *manager, UI_id id, uint32 flags) {
    UI_Widget *widget = (UI_Widget *) allocate(&manager->frame_arena, sizeof(UI_Widget));

    *widget = {};
    widget->id = id;
    widget->flags = flags;

    // TODO: just have initial values for these, so we don't have to do these checks
    if (manager->background_color_stack) {
        widget->background_color = manager->background_color_stack->background_color;
    }

    if (manager->hot_background_color_stack) {
        widget->hot_background_color = manager->hot_background_color_stack->background_color;
    }

    if (manager->active_background_color_stack) {
        widget->active_background_color = manager->active_background_color_stack->background_color;
    }
    
    if (manager->size_stack) {
        widget->semantic_size = manager->size_stack->size;
    }

    if (manager->position_stack) {
        widget->semantic_position = manager->position_stack->position;
    }

    if (manager->layout_type_stack) {
        widget->layout_type = manager->layout_type_stack->type;
    }
    
    if (manager->size_type_stack) {
        widget->size_type = manager->size_type_stack->type;
    }

    if (manager->font_stack) {
        widget->font = manager->font_stack->font;
    }
    
    if (manager->text_color_stack) {
        widget->text_color = manager->text_color_stack->color;
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

UI_Widget *ui_add_widget(UI_Manager *manager, UI_Widget *widget) {
    assert(manager->widget_stack); // ui should be initted with a root node (call ui_frame_init)
    
    UI_Widget *parent = manager->widget_stack->widget;
    widget->id.parent_string_ptr = parent->id.string_ptr;
    widget->id.parent_index      = parent->id.index;
    widget->parent = parent;
    parent->num_children++;
    
    if (!parent->last) {
        assert(!parent->first);
        parent->first = widget;
        parent->last = widget;
    } else {
        parent->last->next = widget;
        widget->prev = parent->last;
        parent->last = widget;
    }

    ui_table_add(manager->widget_table, widget);

    return widget;
}

// NOTE: both ui_add_widget and ui_push_widget add widgets to the hierarchy. the only difference is that
//       ui_push_widget also adds the widget to the widget stack, so that next calls to ui_add_widget or
//       ui_push_widget will have their widget added as a child of the widget at the top of the widget
//       stack.
UI_Widget *ui_add_widget(UI_Manager *manager, UI_id id, uint32 flags) {
    UI_Widget *widget = make_widget(manager, id, flags);
    ui_add_widget(manager, widget);

    return widget;
}

UI_Widget *ui_add_widget(UI_Manager *manager, char *id_string_ptr, uint32 flags = 0) {
    return ui_add_widget(manager, make_ui_id(id_string_ptr), flags);
}

UI_Widget *ui_push_widget(UI_Manager *manager, UI_id id, uint32 flags) {
    UI_Widget *widget = make_widget(manager, id, flags);
    ui_add_widget(manager, widget);

    // push to the stack
    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&manager->frame_arena, sizeof(UI_Stack_Widget));

    entry->widget = widget;
    entry->next = manager->widget_stack;
    
    manager->widget_stack = entry;

    return widget;
}

UI_Widget *ui_push_widget(UI_Manager *manager, char *id_string_ptr, uint32 flags) {
    return ui_push_widget(manager, make_ui_id(id_string_ptr), flags);
}

void ui_pop_widget(UI_Manager *manager) {
    assert(manager->widget_stack);
    manager->widget_stack = manager->widget_stack->next;
}

// TODO: layout types, and calculating positions (should be done after size calculations)
// TODO: stack popping procedures

void ui_push_position(UI_Manager *manager, Vec2 position) {
    UI_Style_Position *entry = (UI_Style_Position *) allocate(&manager->frame_arena, sizeof(UI_Style_Position));

    entry->position = position;
    entry->next = manager->position_stack;
    
    manager->position_stack = entry;
}

void ui_push_size(UI_Manager *manager, Vec2 size) {
    UI_Style_Size *entry = (UI_Style_Size *) allocate(&manager->frame_arena, sizeof(UI_Style_Size));

    entry->size = size;
    entry->next = manager->size_stack;
    
    manager->size_stack = entry;
}

void ui_push_background_color(UI_Manager *manager, Vec4 color) {
    UI_Style_BG_Color *entry = (UI_Style_BG_Color *) allocate(&manager->frame_arena, sizeof(UI_Style_BG_Color));

    entry->background_color = color;
    entry->next = manager->background_color_stack;
    
    manager->background_color_stack = entry;
}

void ui_push_hot_background_color(UI_Manager *manager, Vec4 color) {
    UI_Style_BG_Color *entry = (UI_Style_BG_Color *) allocate(&manager->frame_arena, sizeof(UI_Style_BG_Color));

    entry->background_color = color;
    entry->next = manager->hot_background_color_stack;
    
    manager->hot_background_color_stack = entry;
}

void ui_push_active_background_color(UI_Manager *manager, Vec4 color) {
    UI_Style_BG_Color *entry = (UI_Style_BG_Color *) allocate(&manager->frame_arena, sizeof(UI_Style_BG_Color));

    entry->background_color = color;
    entry->next = manager->active_background_color_stack;
    
    manager->active_background_color_stack = entry;
}

void ui_push_layout_type(UI_Manager *manager, UI_Layout_Type type) {
    UI_Style_Layout_Type *entry = (UI_Style_Layout_Type *) allocate(&manager->frame_arena, sizeof(UI_Style_Layout_Type));

    entry->type = type;
    entry->next = manager->layout_type_stack;
    
    manager->layout_type_stack = entry;
}

void ui_pop_layout_type(UI_Manager *manager) {
    assert(manager->layout_type_stack);
    manager->layout_type_stack = manager->layout_type_stack->next;
}

void ui_push_size_type(UI_Manager *manager, Vec2_UI_Size_Type type) {
    UI_Style_Size_Type *entry = (UI_Style_Size_Type *) allocate(&manager->frame_arena, sizeof(UI_Style_Size_Type));

    entry->type = type;
    entry->next = manager->size_type_stack;
    
    manager->size_type_stack = entry;
}

void ui_pop_size_type(UI_Manager *manager) {
    assert(manager->size_type_stack);
    manager->size_type_stack = manager->size_type_stack->next;
}

void ui_pop_size(UI_Manager *manager) {
    assert(manager->size_stack);
    manager->size_stack = manager->size_stack->next;
}

void ui_pop_background_color(UI_Manager *manager) {
    assert(manager->background_color_stack);
    manager->background_color_stack = manager->background_color_stack->next;
}

void ui_push_text_color(UI_Manager *manager, Vec4 color) {
    UI_Style_Text_Color *entry = (UI_Style_Text_Color *) allocate(&manager->frame_arena, sizeof(UI_Style_Text_Color));

    entry->color = color;
    entry->next = manager->text_color_stack;
    
    manager->text_color_stack = entry;
}

void ui_push_font(UI_Manager *manager, char *font) {
    UI_Style_Font *entry = (UI_Style_Font *) allocate(&manager->frame_arena, sizeof(UI_Style_Font));

    entry->font = font;
    entry->next = manager->font_stack;
    
    manager->font_stack = entry;
}

// TODO: we need to use the last frame's hierarchy since the actual visual positions are not calculated until
//       the end of the update procedure. so basically just store the last frame's hierarchy and use that for..
//       well actually, we want to be able to get the widgets without having to go through the tree. so maybe
//       just store them in a hash table, keyed by the widget IDs.
UI_Interact_Result ui_interact(UI_Manager *manager, UI_Widget *semantic_widget) {
    UI_Widget *computed_widget = ui_table_get(manager->last_frame_widget_table, semantic_widget->id);
    if (!computed_widget) return {};
    //assert(computed_widget);
    
    Controller_State *controller_state = Context::controller_state;
    Vec2 mouse_pos = controller_state->current_mouse;

    UI_Interact_Result result = {};
    
    UI_id id = computed_widget->id;
    if (computed_widget->flags & UI_WIDGET_IS_CLICKABLE) {
        if (in_bounds(mouse_pos, computed_widget->computed_position, computed_widget->computed_size)) {
            manager->hot = id;

            if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
                // we check for !was_down to avoid setting a button active if we click and hold outside then
                // move into the button
                manager->active = id;
            } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
                result.clicked = true;
                if (is_active(manager, computed_widget)) {
                    manager->active = {};
                }
            }
        } else {
            if (is_hot(manager, computed_widget)) {
                manager->hot = {};
            }

            if (is_active(manager, computed_widget) && !controller_state->left_mouse.is_down) {
                manager->active = {};
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

void ui_calculate_standalone_sizes(UI_Manager *manager, Asset_Manager *asset) {
    UI_Widget *current = manager->root;
    
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
        } else {
            assert(!"UI widgets with percentage based sizing must have absolute or percentage based parents.");
        }
    }
    
    // we could add an else block here that just sets computed size to semantic size. this could be useful
    // if we wanted to be able to specify minimum sizes for widgets that use fit_children sizing.
    // when we compute children based sizes, we do max(parent->computed_size.x current->computed_size.x), so
    // if we never set it, it'll just take the max child's computed size.
    // ideally it would be clear if we're using pixels to specify the minimum size or a percentage. that's
    // why i'm not doing it now, since it might result in unexpected behaviour.
}

void ui_calculate_ancestor_dependent_sizes(UI_Manager *manager) {
    UI_Widget *current = manager->root;

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
    UI_Widget *parent = widget->parent;

    if (parent) {
        if (parent->size_type[axis] == UI_SIZE_FIT_CHILDREN) {
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

        if (axis == UI_WIDGET_X_AXIS && parent->layout_type == UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN) {
            parent->computed_child_size_sum[axis] += widget->computed_size[axis];
        }
    }
}

void ui_calculate_child_dependent_sizes(UI_Manager *manager) {
    UI_Widget *current = manager->root;

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
    }
}

// since 
void ui_calculate_ancestor_dependent_sizes_part_2(UI_Manager *manager) {
    UI_Widget *current = manager->root;

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
                real32 d = (parent->computed_size.x - parent->computed_child_size_sum.x) / (parent->num_children - 1);
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

void ui_calculate_positions(UI_Manager *manager) {
    UI_Widget *current = manager->root;

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

void ui_init(Arena_Allocator *arena, UI_Manager *manager) {
    uint32 max_padding = 8 * 3;
    uint32 persistent_heap_size = MEGABYTES(64);
    uint32 frame_arena_size = MEGABYTES(64);
    uint32 last_frame_arena_size = MEGABYTES(64) - max_padding;

    {
        uint32 size = persistent_heap_size;
        void *base = arena_push(arena, size, false);
        manager->persistent_heap = make_heap_allocator(base, size);
    }
    {
        uint32 size = frame_arena_size;
        void *base = arena_push(arena, size, false);
        manager->frame_arena = make_arena_allocator(base, size);
    }
    {
        uint32 size = last_frame_arena_size;
        void *base = arena_push(arena, size, false);
        manager->last_frame_arena = make_arena_allocator(base, size);
    }

    manager->widget_table            = (UI_Widget **) arena_push(&manager->frame_arena,
                                                                 sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
    manager->last_frame_widget_table = (UI_Widget **) arena_push(&manager->last_frame_arena,
                                                                 sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
}

void ui_frame_init(UI_Manager *manager, Display_Output *display_output) {
    ui_push_position(manager, { 0.0f, 0.0f });
    ui_push_layout_type(manager, UI_LAYOUT_NONE);
    ui_push_size_type(manager, { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE });
    ui_push_size(manager, { (real32) display_output->width, (real32) display_output->height });

    UI_Widget *widget = make_widget(manager, make_ui_id("root"), 0);

    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&manager->frame_arena, sizeof(UI_Stack_Widget));
    assert(manager->widget_stack == NULL);
    entry->widget = widget;
    entry->next = manager->widget_stack;
    manager->widget_stack = entry;

    manager->root = widget;

    manager->widget_table = (UI_Widget **) arena_push(&manager->frame_arena,
                                                      sizeof(UI_Widget *) * NUM_WIDGET_BUCKETS, true);
    
    ui_table_add(manager->widget_table, widget);
}

// TODO: don't clear.. we want to keep state actually
void ui_frame_end(UI_Manager *manager) {
    manager->last_frame_root = manager->root;
    manager->root = NULL;

    // swap allocators
    Arena_Allocator temp = manager->last_frame_arena;
    manager->last_frame_arena = manager->frame_arena;
    manager->frame_arena = temp;
    clear_arena(&manager->frame_arena);

    // swap tables
    UI_Widget **temp_widget_table = manager->last_frame_widget_table;
    manager->last_frame_widget_table = manager->widget_table;
    manager->widget_table = temp_widget_table;
    
    manager->widget_stack = NULL;
    manager->background_color_stack = NULL;
    manager->hot_background_color_stack = NULL;
    manager->active_background_color_stack = NULL;
    manager->layout_type_stack = NULL;
    manager->size_stack = NULL;
    manager->position_stack = NULL;
    manager->size_type_stack = NULL;
    manager->text_color_stack = NULL;
}

bool32 has_focus(UI_Manager *manager) {
    return false;
}

void disable_input(UI_Manager *manager) {
    manager->hot = {};
    manager->active = {};
    manager->is_disabled = true;
}

void enable_input(UI_Manager *manager) {
    manager->is_disabled = false;
}

inline bool32 is_hot(UI_Manager *manager, UI_Widget *widget) {
    return ui_id_equals(manager->hot, widget->id);
}

inline bool32 is_active(UI_Manager *manager, UI_Widget *widget) {
    return ui_id_equals(manager->active, widget->id);
}

inline bool32 ui_has_hot(UI_Manager *manager) {
    return (manager->hot.string_ptr != NULL);
}

inline bool32 ui_has_active(UI_Manager *manager) {
    return (manager->active.string_ptr != NULL);
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



// COMPOUND WIDGETS

void do_text(UI_Manager *manager, char *text, char *id, uint32 flags, int32 index = 0) {
    ui_push_size_type(manager, { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT });
    //ui_push_background_color(manager, { 1.0f, 0.0f, 0.0f, 1.0f });
    UI_Widget *text_widget = ui_add_widget(manager, make_ui_id(id, index), UI_WIDGET_DRAW_TEXT);
    text_widget->text = text;
    //ui_pop_background_color(manager);
    ui_pop_size_type(manager);
}

bool32 do_button(UI_Manager *manager, UI_id id) {
    UI_Widget *widget = ui_add_widget(manager, id, UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    UI_Interact_Result interact_result = ui_interact(manager, widget);
    
    return interact_result.clicked;
}

bool32 do_text_button(UI_Manager *manager, char *text, real32 padding, UI_id id) {
    ui_push_size(manager, {});
    ui_push_size_type(manager, { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN });
    ui_push_layout_type(manager, UI_LAYOUT_VERTICAL);
    UI_Widget *button = ui_push_widget(manager, id,
                                       UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    {
        ui_push_size(manager, { 0.0f, padding });
        ui_add_widget(manager, "");

        ui_push_layout_type(manager, UI_LAYOUT_HORIZONTAL);
        ui_push_widget(manager, "", 0);
        {
            // inner row
            ui_push_size(manager, { padding, 0.0f });
            ui_add_widget(manager, "");
            //ui_pop_size_type(manager);
        
            ui_push_size_type(manager, { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT });
            UI_Widget *text_widget = ui_add_widget(manager, make_ui_id(NULL), UI_WIDGET_DRAW_TEXT);
            text_widget->text = text;
            ui_pop_size_type(manager);

            ui_add_widget(manager, "");
            ui_pop_size(manager);
        }
        ui_pop_widget(manager);
        ui_pop_layout_type(manager);
        
        ui_add_widget(manager, "");
        ui_pop_size(manager);
    }
    ui_pop_widget(manager);
    ui_pop_layout_type(manager);
    ui_pop_size_type(manager);
    ui_pop_size(manager);
    
    
    UI_Interact_Result interact_result = ui_interact(manager, button);
#if 0
    ui_pop_size_type(manager);
    
    ui_push_size_type(manager, UI_SIZE_FIT_TEXT);

    // TODO: we should probably use a hashing method for storing UI IDs, so that we can generate IDs dynamically.
    //       right now we just use pointers, which is fine for read-only memory, but if we create new strings,
    //       then the string addresses will not be consistent.
    // TODO: scope UI IDs. have a parent UI_id be included in all UI_ids. that way, we can do stuff like this
    //       without having collisions. actually, i don't think that'll work. try and use a hash. actually,
    //       i think this is fine, since we always have the parent ID use some unique string.
    UI_Widget *text_widget = ui_add_widget(manager, make_ui_id("button-text"), UI_WIDGET_DRAW_TEXT);
    text_widget->text = text;

    ui_pop_layout_type(manager);
    ui_pop_size_type(manager);
    ui_pop_widget(manager);
#endif

    return interact_result.clicked;
}

inline bool32 do_text_button(UI_Manager *manager, char *text, real32 padding, char *id, int32 index = 0) {
    return do_text_button(manager, text, padding, make_ui_id(id, index));
}

#if 0
void do_window(UI_Manager *manager, char *text, char *id, int32 index = 0) {
    ui_push_widget(manager, 
}
        #endif
