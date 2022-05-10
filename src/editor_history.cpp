#include "editor.h"
#include "editor_history.h"

void _history_add_action(Editor_History *history, Editor_Action *editor_action) {
    // [oldest|e|e|e|x|newest| | ] oldest < x < newest
    // [oldest|e|e|e|e|newest|x| ] oldest < newest < x
    // [newest| | | | |oldest|x|e] newest < oldest < x
    // [e|e|x|e|newest|oldest|e|e] x < newest < oldest

    int32 num_between_oldest_and_current; // does not include current, since current needs to be deallocated
    if (history->current_index < history->oldest_index) {
        // for example, [e|current|e|e|_|_|_|oldest]
        // num_entries = 5, 2 between oldest and current, 3 need to be deleted after and including current
        // unwrapped_current = 8 + 1 = 9
        // num_between_oldest_and_current = 9 - 7 = 2
        // num_to_be_deleted = num_entries - num_between_oldest_and_current = 3
        // num_entries -= num_to_be_deleted

        int32 unwrapped_current = MAX_EDITOR_HISTORY_ENTRIES + history->current_index;
        num_between_oldest_and_current = unwrapped_current - history->oldest_index;
    } else {
        // [oldest|e|e|current|e|e|e|_]
        // num_between_oldest_and_current = 3 - 0 = 3
        // num_to_be_deleted = num_entries - num_between_oldest_and_current = 7 - 3 = 4
        // num_entries -= num_to_be_deleted
        num_between_oldest_and_current = history->current_index - history->oldest_index;
    }

    // [current, oldest|e|e|e|e|e|e|e]

    int32 num_to_be_deleted = history->num_entries - num_between_oldest_and_current;
    assert(num_to_be_deleted >= 0);

    int32 num_deleted = 0;

    int32 i = history->current_index;
    while (num_deleted < num_to_be_deleted) {
        deallocate(history->allocator_pointer, history->entries[i]);
        debug_print("deallocated history at index %d\n", i);

        i = (i + 1) % MAX_EDITOR_HISTORY_ENTRIES;
        num_deleted++;
    }
    history->num_entries -= num_deleted;

    history->entries[history->current_index] = editor_action;
    history->num_entries++;
    history->num_entries = min(history->num_entries, MAX_EDITOR_HISTORY_ENTRIES);
    history->current_index = (history->current_index + 1) % MAX_EDITOR_HISTORY_ENTRIES;

    if (history->current_index == history->oldest_index) {
        // there are two cases when this condition is true: (1) when you've undone enough to reach the oldest and
        // (2) when you've added enough entries such that you loop around the circular buffer and end up at the
        // oldest index.

        // this handles case 2. we don't want to handle this case at the beginning of this procedure, since it's
        // possible that this code might run during case 1, which would be incorrect. by doing this at the end of
        // this procedure, we guarantee that it's case 2, since we just added an entry.

        history->oldest_index = (history->oldest_index + 1) % MAX_EDITOR_HISTORY_ENTRIES;

        // example:
        // [oldest|e|e|e|e|e|e|current]
        // [current,oldest|e|e|e|e|e|e|e]
        // [current|oldest|e|e|e|e|e|e]
        // current will be deallocated next time we add.
    }

    history->num_undone = 0;
}

#define history_add_action(history_pointer, type, value)      \
    type *allocated = (type *) allocate(history_pointer->allocator_pointer, sizeof(type)); \
    *allocated = value; \
    _history_add_action(history_pointer, (Editor_Action *) allocated)

void editor_add_normal_entity(Editor_State *editor_state, Game_State *game_state, Add_Normal_Entity_Action action) {
    int32 mesh_id = get_mesh_id_by_name(game_state,
                                        &game_state->current_level,
                                        Mesh_Type::PRIMITIVE,
                                        make_string("cube"));

    AABB primitive_cube_mesh_aabb = (get_mesh(game_state, &game_state->current_level,
                                              Mesh_Type::PRIMITIVE, mesh_id)).aabb;

    Normal_Entity new_entity = make_entity(Mesh_Type::PRIMITIVE, mesh_id, -1, make_transform(),
                                           primitive_cube_mesh_aabb);

    if (action.entity_id >= 0) {
        level_add_entity(game_state, &game_state->current_level, new_entity, action.entity_id);
    } else {
        action.entity_id = level_add_entity(game_state, &game_state->current_level, new_entity);
    }

    editor_state->selected_entity_type = ENTITY_NORMAL;
    editor_state->selected_entity_id = action.entity_id;

    Editor_History *history = &editor_state->history;

    history_add_action(history, Add_Normal_Entity_Action, action);
}

void undo_add_normal_entity(Editor_State *editor_state, Level *level, Add_Normal_Entity_Action action) {
    deselect_entity(editor_state);
    level_delete_entity(level, action.entity_type, action.entity_id);
}

void history_undo(Game_State *game_state, Editor_History *history) {
    if (history->num_undone == history->num_entries) return;

    // we undo the entry one before current_index. current_index is where the new entry will go when we're
    // adding an action.

    int32 last_action_index = ((MAX_EDITOR_HISTORY_ENTRIES + (history->current_index - 1))
                               % MAX_EDITOR_HISTORY_ENTRIES);
    Editor_Action *last_action = history->entries[last_action_index];
    switch (last_action->type) {
        case ACTION_NONE: {
            assert(!"Action does not have a type.");
            return;
        }
        case ACTION_ADD_NORMAL_ENTITY: {
            Add_Normal_Entity_Action *action = (Add_Normal_Entity_Action *) last_action;
            undo_add_normal_entity(&game_state->editor_state, &game_state->current_level, *action);
        } break;
        default: {
            assert(!"Unhandled editor action type.");
            return;
        }
    }

    history->current_index = ((MAX_EDITOR_HISTORY_ENTRIES + (history->current_index - 1))
                              % MAX_EDITOR_HISTORY_ENTRIES);
    history->num_undone++;
}
