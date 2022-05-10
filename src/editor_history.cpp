#include "editor.h"
#include "editor_history.h"

void editor_add_normal_entity(Game_State *game_state, Add_Normal_Entity_Action action) {
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

    game_state->editor_state.selected_entity_type = ENTITY_NORMAL;
    game_state->editor_state.selected_entity_id = action.entity_id;
}

void undo_add_normal_entity(Level *level, Add_Normal_Entity_Action action) {
    // TODO: delete the added entity
    level_delete_entity(level, action.entity_type, action.entity_id);
}

