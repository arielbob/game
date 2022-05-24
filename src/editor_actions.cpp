#include "editor_actions.h"
#include "editor.h"

void history_deallocate(Editor_State *editor_state, Editor_Action *editor_action) {
    Allocator *allocator = (Allocator *) &editor_state->history_heap;
    
    switch (editor_action->type) {
        case ACTION_NONE: {
            assert(!"Action has no type.");
        } break;
        case ACTION_ADD_NORMAL_ENTITY: {} break;
        case ACTION_DELETE_ENTITY: {
            Delete_Entity_Action *action = (Delete_Entity_Action *) editor_action;
            deallocate(allocator, action->entity);
        } break;
        default: {
            assert(!"Unhandled deallocation for action type.");
        }
    }
    
    deallocate(allocator, editor_action);
}

void _history_add_action(Editor_State *editor_state, Editor_Action *editor_action) {
    Editor_History *history = &editor_state->history;
    if (history->start_index == -1 && history->end_index == -1) {
        int32 new_entry_index = 0;

        history->start_index = new_entry_index;
        history->entries[new_entry_index] = editor_action;
        history->end_index = new_entry_index;
        history->current_index = new_entry_index;
    } else if (history->current_index == -1) {
        int32 start_index = history->start_index;
        int32 end_index = history->end_index;

        if (start_index > end_index) {
            end_index += MAX_EDITOR_HISTORY_ENTRIES;
        }

        for (int32 i = start_index; i <= end_index; i++) {
            int32 wrapped_index = i % MAX_EDITOR_HISTORY_ENTRIES;
            history_deallocate(editor_state, history->entries[wrapped_index]);
            history->entries[wrapped_index] = NULL;
            //debug_print("deallocated history at index %d\n", i);
        }

        history->current_index = start_index;
        history->end_index = start_index;
        history->entries[start_index] = editor_action;
    } else {
        int32 start_index = history->start_index;
        int32 end_index = history->end_index;
        int32 current_index = history->current_index;

        if (start_index > end_index) {
            end_index += MAX_EDITOR_HISTORY_ENTRIES;
            if (current_index < start_index) {
                current_index += MAX_EDITOR_HISTORY_ENTRIES;
            }
        }

        int32 new_entry_index = current_index + 1;
 
        int32 wrapped_new_entry_index = new_entry_index % MAX_EDITOR_HISTORY_ENTRIES;

        if (wrapped_new_entry_index == start_index) {
            Editor_Action *action = history->entries[start_index % MAX_EDITOR_HISTORY_ENTRIES];
            history_deallocate(editor_state, action);

            history->entries[start_index % MAX_EDITOR_HISTORY_ENTRIES] = NULL;
            history->start_index = (start_index + 1) % MAX_EDITOR_HISTORY_ENTRIES;
            //debug_print("deallocated history at index %d (start_index)\n", start_index);

            history->current_index = wrapped_new_entry_index;
            history->end_index = wrapped_new_entry_index;
            history->entries[wrapped_new_entry_index] = editor_action;
        } else {
            for (int32 i = new_entry_index; i <= end_index; i++) {
                int32 wrapped_index = i % MAX_EDITOR_HISTORY_ENTRIES;
                history_deallocate(editor_state, history->entries[wrapped_index]);
                history->entries[wrapped_index] = NULL;
                //debug_print("deallocated history at index %d\n", i);
            }
            
            history->current_index = wrapped_new_entry_index;
            history->end_index = wrapped_new_entry_index;
            history->entries[wrapped_new_entry_index] = editor_action;
        }
    }

    history->num_undone = 0;
}

#define history_add_action(editor_state_pointer, type, value)                \
    type *allocated = (type *) allocate((Allocator *) &editor_state_pointer->history_heap, sizeof(type));   \
    *allocated = value;                                                 \
    _history_add_action(editor_state_pointer, (Editor_Action *) allocated)

void do_add_normal_entity(Editor_State *editor_state, int32 entity_id = -1, bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    int32 mesh_id;
    Mesh default_mesh = get_mesh_by_name(asset_manager, make_string("cube"), &mesh_id);
    AABB aabb = default_mesh.aabb;

    Normal_Entity new_entity = make_normal_entity(make_transform(), mesh_id, -1, aabb);
    Normal_Entity *entity = (Normal_Entity *) allocate((Allocator *) &editor_state->entity_heap,
                                                       sizeof(Normal_Entity));
    *entity = new_entity;

    int32 id = add_entity(&editor_state->level, (Entity *) entity, entity_id);

    editor_state->selected_entity_id = id;

    if (!is_redoing) {
        Editor_History *history = &editor_state->history;
        Add_Normal_Entity_Action action = { ACTION_ADD_NORMAL_ENTITY, id };
        history_add_action(editor_state, Add_Normal_Entity_Action, action);
    }
}

void undo_add_normal_entity(Editor_State *editor_state, Add_Normal_Entity_Action action) {
    delete_entity(editor_state, action.entity_id);
    editor_state->selected_entity_id = -1;
}

// TODO: the entity already has an id field, so maybe we just use that instead of having it as an argument?
void do_add_point_light_entity(Editor_State *editor_state, int32 entity_id = -1, bool32 is_redoing = false) {
    Point_Light_Entity new_entity = make_point_light_entity(make_vec3(0.5f, 0.5f, 0.5f), 0.0f, 5.0f);
    Point_Light_Entity *entity = (Point_Light_Entity *) allocate((Allocator *) &editor_state->entity_heap,
                                                                 sizeof(Point_Light_Entity));
    *entity = new_entity;

    int32 id = add_entity(&editor_state->level, (Entity *) entity, entity_id);

    editor_state->selected_entity_id = id;

    if (!is_redoing) {
        Editor_History *history = &editor_state->history;
        Add_Point_Light_Entity_Action action = { ACTION_ADD_POINT_LIGHT_ENTITY, id };
        history_add_action(editor_state, Add_Point_Light_Entity_Action, action);
    }
}

void undo_add_point_light_entity(Editor_State *editor_state, Add_Point_Light_Entity_Action action) {
    delete_entity(editor_state, action.entity_id);
    editor_state->selected_entity_id = -1;
}

void do_delete_entity(Editor_State *editor_state, int32 entity_id, bool32 is_redoing = false) {
    if (!is_redoing) {
        Editor_History *history = &editor_state->history;
        Allocator *history_allocator = (Allocator *) &editor_state->history_heap;

        Delete_Entity_Action action;
        action.type = ACTION_DELETE_ENTITY;

        Entity *uncast_entity = get_entity(editor_state, entity_id);
        switch (uncast_entity->type) {
            case ENTITY_NORMAL: {
                Normal_Entity *entity = (Normal_Entity *) uncast_entity;
                Normal_Entity *allocated = (Normal_Entity *) allocate(history_allocator, sizeof(Normal_Entity));

                *allocated = copy(history_allocator, *entity);
                action.entity = (Entity *) allocated;
            } break;
            case ENTITY_POINT_LIGHT: {
                Point_Light_Entity *entity = (Point_Light_Entity *) uncast_entity;
                Point_Light_Entity *allocated = (Point_Light_Entity *) allocate(history_allocator,
                                                                                sizeof(Point_Light_Entity));
                *allocated = copy(history_allocator, *entity);
                action.entity = (Entity *) allocated;
            } break;
            default: {
                assert(!"Unhandled entity type.");
            }
        }

        history_add_action(editor_state, Delete_Entity_Action, action);
    }

    delete_entity(editor_state, entity_id);
    editor_state->selected_entity_id = -1;
}

void undo_delete_entity(Editor_State *editor_state, Delete_Entity_Action action) {
    Allocator *entity_allocator = (Allocator *) &editor_state->entity_heap;
    Entity *to_add = NULL;
    switch (action.entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *deleted_entity = (Normal_Entity *) action.entity;
            Normal_Entity *allocated = (Normal_Entity *) allocate(entity_allocator, sizeof(Normal_Entity));
            *allocated = copy(entity_allocator, *deleted_entity);
            to_add = (Entity *) allocated;
        } break;
        case ENTITY_POINT_LIGHT: {
            Point_Light_Entity *deleted_entity = (Point_Light_Entity *) action.entity;
            Point_Light_Entity *allocated = (Point_Light_Entity *) allocate(entity_allocator,
                                                                            sizeof(Point_Light_Entity));
            *allocated = copy(entity_allocator, *deleted_entity);
            to_add = (Entity *) allocated;
        } break;
        default: {
            assert(!"Unhandled entity type.");
        } break;
    }

    assert(to_add);
    add_entity(&editor_state->level, to_add, to_add->id);
}

int32 history_get_num_entries(Editor_History *history) {
    if (history->start_index == -1 && history->end_index == -1) return 0;

    int32 count;
    if (history->end_index < history->start_index) {
        // [e|end|_|_|start|e|e|e]
        // end = 1, start = 4, max = 8
        // unwrapped_end = 8 + 1 = 9
        // count = 9 - 4 + 1 = 6
        int32 unwrapped_end = MAX_EDITOR_HISTORY_ENTRIES + history->end_index;
        count = unwrapped_end - history->start_index + 1;
    } else {
        // [oldest|e|e|e|e|e|end|_]
        count = history->end_index - history->start_index + 1;
    }
    
    return count;
}

void history_undo(Editor_State *editor_state) {
    Editor_History *history = &editor_state->history;

    assert(history->start_index >= 0);
    assert(history->end_index >= 0);

    int32 num_entries = history_get_num_entries(history);

    if (history->num_undone == num_entries) return;

    //Editor_State *editor_state = &game_state->editor_state;
    //Asset_Manager *asset_manager = &game_state->asset_manager;
    //Level *level = &game_state->current_level;

    // we undo the action at current_index.
    Editor_Action *current_action = history->entries[history->current_index];
    switch (current_action->type) {
        case ACTION_NONE: {
            assert(!"Action does not have a type.");
            return;
        }
        case ACTION_ADD_NORMAL_ENTITY: {
            Add_Normal_Entity_Action *action = (Add_Normal_Entity_Action *) current_action;
            undo_add_normal_entity(editor_state, *action);
        } break;
        case ACTION_ADD_POINT_LIGHT_ENTITY: {
            Add_Point_Light_Entity_Action *action = (Add_Point_Light_Entity_Action *) current_action;
            undo_add_point_light_entity(editor_state, *action);
        } break;
        case ACTION_DELETE_ENTITY: {
            Delete_Entity_Action *action = (Delete_Entity_Action *) current_action;
            undo_delete_entity(editor_state, *action);
        } break;
        default: {
            assert(!"Unhandled editor action type.");
            return;
        }
    }

    if (history->current_index == history->start_index) {
        history->current_index = -1;
    } else {
        history->current_index = ((MAX_EDITOR_HISTORY_ENTRIES + (history->current_index - 1))
                                  % MAX_EDITOR_HISTORY_ENTRIES);    
    }
    
    history->num_undone++;
}

void history_redo(Editor_State *editor_state) {
    Editor_History *history = &editor_state->history;
    assert(history->num_undone > 0);

    // we redo the action at current_index + 1, unless current_index is -1, in which case we redo the action
    // at start_index.
    int32 redo_index;
    if (history->current_index == -1) {
        redo_index = history->start_index;
    } else {
        redo_index = (history->current_index + 1) % MAX_EDITOR_HISTORY_ENTRIES;
    }

    Editor_Action *redo_action = history->entries[redo_index];
    switch (redo_action->type) {
        case ACTION_NONE: {
            assert(!"Action does not have a type.");
            return;
        }
        case ACTION_ADD_NORMAL_ENTITY: {
            Add_Normal_Entity_Action *action = (Add_Normal_Entity_Action *) redo_action;
            do_add_normal_entity(editor_state, action->entity_id, true);
        } break;
        case ACTION_ADD_POINT_LIGHT_ENTITY: {
            Add_Point_Light_Entity_Action *action = (Add_Point_Light_Entity_Action *) redo_action;
            do_add_point_light_entity(editor_state, action->entity_id, true);
        } break;
        case ACTION_DELETE_ENTITY: {
            Delete_Entity_Action *action = (Delete_Entity_Action *) redo_action;
            do_delete_entity(editor_state, action->entity->id, true);
        } break;
        default: {
            assert(!"Unhandled editor action type.");
            return;
        }
    }

    history->current_index = redo_index;
    history->num_undone--;
}

void history_reset(Editor_State *editor_state) {
    Editor_History *history = &editor_state->history;
    clear_heap(&editor_state->history_heap);
    history->start_index = -1;
    history->end_index = -1;
    history->current_index = -1;
    history->num_undone = 0;
}
