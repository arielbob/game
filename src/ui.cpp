#include "ui.h"

UI_Widget *make_widget(UI_Manager *manager, UI_id id, uint32 flags) {
    UI_Widget *widget = (UI_Widget *) allocate(&manager->frame_arena, sizeof(UI_Widget));

    *widget = {};
    widget->id = id;
    widget->flags = flags;

    if (manager->background_color_stack) {
        widget->background_color = manager->background_color_stack->background_color;
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

    return widget;
}

void ui_add_widget(UI_Manager *manager, UI_Widget *widget) {
    assert(manager->widget_stack); // ui should be initted with a root node (call ui_frame_init)
    
    UI_Widget *parent = manager->widget_stack->widget;
    widget->parent = parent;
    
    if (!parent->last) {
        assert(!parent->first);
        parent->first = widget;
        parent->last = widget;
    } else {
        parent->last->next = widget;
        widget->prev = parent->last;
        parent->last = widget;
    }    
}

// NOTE: both ui_add_widget and ui_push_widget add widgets to the hierarchy. the only difference is that
//       ui_push_widget also adds the widget to the widget stack, so that next calls to ui_add_widget or
//       ui_push_widget will have their widget added as a child of the widget at the top of the widget
//       stack.
void ui_add_widget(UI_Manager *manager, UI_id id, uint32 flags) {
    UI_Widget *widget = make_widget(manager, id, flags);
    ui_add_widget(manager, widget);
}

void ui_push_widget(UI_Manager *manager, UI_id id, uint32 flags) {
    UI_Widget *widget = make_widget(manager, id, flags);
    ui_add_widget(manager, widget);

    // push to the stack
    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&manager->frame_arena, sizeof(UI_Stack_Widget));

    entry->widget = widget;
    entry->next = manager->widget_stack;
    
    manager->widget_stack = entry;
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

void ui_push_layout_type(UI_Manager *manager, UI_Layout_Type type) {
    UI_Style_Layout_Type *entry = (UI_Style_Layout_Type *) allocate(&manager->frame_arena, sizeof(UI_Style_Layout_Type));

    entry->type = type;
    entry->next = manager->layout_type_stack;
    
    manager->layout_type_stack = entry;
}

void ui_push_size_type(UI_Manager *manager, UI_Size_Type type) {
    UI_Style_Size_Type *entry = (UI_Style_Size_Type *) allocate(&manager->frame_arena, sizeof(UI_Style_Size_Type));

    entry->type = type;
    entry->next = manager->size_type_stack;
    
    manager->size_type_stack = entry;
}

void ui_frame_init(UI_Manager *manager, Display_Output *display_output) {
    ui_push_position(manager, { 0.0f, 0.0f });
    ui_push_layout_type(manager, UI_LAYOUT_HORIZONTAL);
    ui_push_size_type(manager, UI_SIZE_ABSOLUTE);
    ui_push_size(manager, { (real32) display_output->width, (real32) display_output->height });

    UI_Widget *widget = make_widget(manager, make_ui_id("root"), 0);

    UI_Stack_Widget *entry = (UI_Stack_Widget *) allocate(&manager->frame_arena, sizeof(UI_Stack_Widget));
    assert(manager->widget_stack == NULL);
    entry->widget = widget;
    entry->next = manager->widget_stack;
    manager->widget_stack = entry;

    manager->root = widget;
}

// TODO: we need to use the last frame's hierarchy since the actual visual positions are not calculated until
//       the end of the update procedure. so basically just store the last frame's hierarchy and use that for..
//       well actually, we want to be able to get the widgets without having to go through the tree. so maybe
//       just store them in a hash table, keyed by the widget IDs.
UI_Interact_Result ui_interact(UI_Manager *manager, UI_Widget *widget) {
    if (widget->flags & UI_WIDGET_IS_CLICKABLE) {
        
    }

    return {};
}

bool32 do_button(UI_Manager *manager, UI_id id) {
    ui_add_widget(manager, id, UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE);
    return false;
}

void ui_calculate_ancestor_dependent_sizes(UI_Manager *manager) {
    UI_Widget *current = manager->root;

    while (true) {
        UI_Widget *parent = current->parent;
        
        if (current->size_type == UI_SIZE_PERCENTAGE) {
            assert(current->parent); // root node cannot be percentage based
            if (parent->size_type == UI_SIZE_PERCENTAGE ||
                parent->size_type == UI_SIZE_ABSOLUTE) {
                // since this is pre-order traversal, the parent should already have a computed size
                current->computed_size = {
                    parent->computed_size.x * current->semantic_size.x,
                    parent->computed_size.y * current->semantic_size.y
                };
            } else {
                assert(!"UI widgets with percentage based sizing must have absolute or percentage based parents.");
            }
        } else if (current->size_type == UI_SIZE_ABSOLUTE) {
            current->computed_size = current->semantic_size;
        }
        // we could add an else block here that just sets computed size to semantic size. this could be useful
        // if we wanted to be able to specify minimum sizes for widgets that use fit_children sizing.
        // when we compute children based sizes, we do max(parent->computed_size.x current->computed_size.x), so
        // if we never set it, it'll just take the max child's computed size.
        // ideally it would be clear if we're using pixels to specify the minimum size or a percentage. that's
        // why i'm not doing it now, since it might result in unexpected behaviour.
        
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
                if (parent && parent->size_type == UI_SIZE_FIT_CHILDREN) {
                    if (parent->size_type == UI_LAYOUT_HORIZONTAL) {
                        parent->computed_size.x += current->computed_size.x;
                        parent->computed_size.y = max(parent->computed_size.y, current->computed_size.y);
                    } else if (parent->size_type == UI_LAYOUT_VERTICAL) {
                        parent->computed_size.y += current->computed_size.y;
                        parent->computed_size.x = max(parent->computed_size.x, current->computed_size.x);
                    } else {
                        parent->computed_size.x = max(parent->computed_size.x, current->computed_size.x);
                        parent->computed_size.y = max(parent->computed_size.y, current->computed_size.y);
                    }
                }
                
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

void ui_calculate_positions(UI_Manager *manager) {
    UI_Widget *current = manager->root;

    // pre-order traversal
    
    while (true) {
        UI_Widget *parent = current->parent;
        
        // TODO: add centered layout
        
        if (parent) {
            UI_Widget *prev = current->prev;

            if (parent->layout_type == UI_LAYOUT_HORIZONTAL) {
                current->computed_position.y = parent->computed_position.y;
                
                if (!prev) {
                    current->computed_position.x = parent->computed_position.x;
                } else {
                    current->computed_position.x = prev->computed_position.x + prev->computed_size.x;
                }
            } else if (parent->layout_type == UI_LAYOUT_VERTICAL) {
                current->computed_position.x = parent->computed_position.x;
                
                if (!prev) {
                    current->computed_position.y = parent->computed_position.y;
                } else {
                    current->computed_position.y = prev->computed_position.y + prev->computed_size.y;
                }
            } else {
                current->computed_position = current->semantic_position;
            }
        } else {
            current->computed_position = current->semantic_position;
        }
        
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
    
    manager->widget_stack = NULL;
    manager->background_color_stack = NULL;
    manager->layout_type_stack = NULL;
    manager->size_stack = NULL;
    manager->position_stack = NULL;
    manager->size_type_stack = NULL;
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

inline bool32 ui_has_hot(UI_Manager *manager) {
    return (manager->hot.string_ptr != NULL);
}

inline bool32 ui_has_active(UI_Manager *manager) {
    return (manager->active.string_ptr != NULL);
}
