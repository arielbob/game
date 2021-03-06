#include "editor.h"
#include "editor_history.h"

void history_deallocate(Editor_History *history, Editor_Action *editor_action) {
    Allocator *allocator = history->allocator_pointer;
    
    switch (editor_action->type) {
        case ACTION_NONE: {
            assert(!"Action has no type.");
        } break;
        case ACTION_ADD_NORMAL_ENTITY: {} break;
        case ACTION_ADD_POINT_LIGHT_ENTITY: {} break;
        case ACTION_DELETE_NORMAL_ENTITY: {} break;
        case ACTION_DELETE_POINT_LIGHT_ENTITY: {} break;
        case ACTION_TRANSFORM_ENTITY: {} break;
        case ACTION_MODIFY_MESH: {
            Modify_Mesh_Action *action = (Modify_Mesh_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_MODIFY_ENTITY: {
            Modify_Entity_Action *action = (Modify_Entity_Action *) editor_action;
            deallocate(allocator, action->original);
            deallocate(allocator, action->new_entity);
        } break;
        case ACTION_ADD_MESH: {
            Add_Mesh_Action *action = (Add_Mesh_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_DELETE_MESH: {
            Delete_Mesh_Action *action = (Delete_Mesh_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_MODIFY_MATERIAL: {
            Modify_Material_Action *action = (Modify_Material_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_DELETE_MATERIAL: {
            Delete_Material_Action *action = (Delete_Material_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_ADD_MATERIAL: {
            Add_Material_Action *action = (Add_Material_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_MODIFY_TEXTURE: {
            Modify_Texture_Action *action = (Modify_Texture_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_DELETE_TEXTURE: {
            Delete_Texture_Action *action = (Delete_Texture_Action *) editor_action;
            deallocate(*action);
        } break;
        case ACTION_ADD_TEXTURE: {
            Add_Texture_Action *action = (Add_Texture_Action *) editor_action;
            deallocate(*action);
        } break;
        default: {
            assert(!"Unhandled deallocation for action type.");
        }
    }
    
    deallocate(allocator, editor_action);
}

void _history_add_action(Editor_History *history, Editor_Action *editor_action) {
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
            history_deallocate(history, history->entries[wrapped_index]);
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
            history_deallocate(history, action);

            history->entries[start_index % MAX_EDITOR_HISTORY_ENTRIES] = NULL;
            history->start_index = (start_index + 1) % MAX_EDITOR_HISTORY_ENTRIES;
            //debug_print("deallocated history at index %d (start_index)\n", start_index);

            history->current_index = wrapped_new_entry_index;
            history->end_index = wrapped_new_entry_index;
            history->entries[wrapped_new_entry_index] = editor_action;
        } else {
            for (int32 i = new_entry_index; i <= end_index; i++) {
                int32 wrapped_index = i % MAX_EDITOR_HISTORY_ENTRIES;
                history_deallocate(history, history->entries[wrapped_index]);
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

#define history_add_action(history_pointer, type, value)      \
    type *allocated = (type *) allocate((history_pointer)->allocator_pointer, sizeof(type)); \
    *allocated = value; \
    _history_add_action(history_pointer, (Editor_Action *) allocated)

// normal entity adding
void editor_add_normal_entity(Editor_State *editor_state, Game_State *game_state, Add_Normal_Entity_Action action,
                              bool32 is_redoing) {
    Asset_Manager *asset_manager = &game_state->asset_manager;
    int32 mesh_id = get_mesh_id_by_name(asset_manager, make_string("cube"));

    AABB primitive_cube_mesh_aabb = (get_mesh(asset_manager, mesh_id)).aabb;

    Normal_Entity new_entity = make_entity(mesh_id, -1, make_transform(),
                                           primitive_cube_mesh_aabb);

    if (action.entity_id >= 0) {
        level_add_entity(&game_state->current_level, new_entity, action.entity_id);
    } else {
        action.entity_id = level_add_entity(&game_state->current_level, new_entity);
    }

    editor_state->selected_entity_type = ENTITY_NORMAL;
    editor_state->selected_entity_id = action.entity_id;

    if (!is_redoing) {
        Editor_History *history = &editor_state->history;
        history_add_action(history, Add_Normal_Entity_Action, action);
    }
}

void undo_add_normal_entity(Editor_State *editor_state, Level *level, Add_Normal_Entity_Action action) {
    deselect_entity(editor_state);
    level_delete_entity(level, ENTITY_NORMAL, action.entity_id);
}

// point light adding
void editor_add_point_light_entity(Editor_State *editor_state, Game_State *game_state,
                                   Add_Point_Light_Entity_Action action,
                                   bool32 is_redoing) {
    Point_Light_Entity new_entity = make_point_light_entity(make_vec3(1.0f, 1.0f, 1.0f),
                                                            0.0f, 5.0f, make_transform());

    if (action.entity_id >= 0) {
        level_add_point_light_entity(&game_state->current_level, new_entity, action.entity_id);
    } else {
        action.entity_id = level_add_point_light_entity(&game_state->current_level, new_entity);
    }

    editor_state->selected_entity_type = ENTITY_POINT_LIGHT;
    editor_state->selected_entity_id = action.entity_id;

    if (!is_redoing) {
        Editor_History *history = &editor_state->history;
        history_add_action(history, Add_Point_Light_Entity_Action, action);
    }
}

void undo_add_point_light_entity(Editor_State *editor_state, Level *level, Add_Point_Light_Entity_Action action) {
    deselect_entity(editor_state);
    level_delete_entity(level, ENTITY_POINT_LIGHT, action.entity_id);
}

// normal entity deleting
void editor_delete_normal_entity(Editor_State *editor_state, Level *level,
                                 Delete_Normal_Entity_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        Normal_Entity *entity = (Normal_Entity *) get_entity(level, ENTITY_NORMAL, action.entity_id);
        action.entity = *entity;
        history_add_action(&editor_state->history, Delete_Normal_Entity_Action, action);
    }

    level_delete_entity(level, ENTITY_NORMAL, action.entity_id);
    deselect_entity(editor_state);
}

void undo_delete_normal_entity(Level *level, Delete_Normal_Entity_Action action) {
    level_add_entity(level, action.entity, action.entity_id);
}

// point light entity deleting
void editor_delete_point_light_entity(Editor_State *editor_state, Level *level,
                                      Delete_Point_Light_Entity_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        Point_Light_Entity *entity = (Point_Light_Entity *) get_entity(level, ENTITY_POINT_LIGHT, action.entity_id);
        action.entity = *entity;
        history_add_action(&editor_state->history, Delete_Point_Light_Entity_Action, action);
    }

    level_delete_entity(level, ENTITY_POINT_LIGHT, action.entity_id);
    deselect_entity(editor_state);
}

// NOTE: this should only be called by the editor. this creates the action objects for us, but the objects only
//       have the entity id. the calls to editor_delete_* fill in the struct with the Entity object. we need to
//       store the entity object, since when we redo a deletion (i.e. adding the entity back), we need to the
//       entity's data.
void editor_delete_entity(Editor_State *editor_state, Level *level,
                          Entity_Type entity_type, int32 entity_id,
                          bool32 is_redoing) {
    switch (entity_type) {
        case ENTITY_NORMAL: {
            Delete_Normal_Entity_Action action = make_delete_normal_entity_action(entity_id);
            editor_delete_normal_entity(editor_state, level, action, is_redoing);
        } break;
        case ENTITY_POINT_LIGHT: {
            Delete_Point_Light_Entity_Action action = make_delete_point_light_entity_action(entity_id);
            editor_delete_point_light_entity(editor_state, level, action, is_redoing);
        } break;
        default: {
            assert(!"Unhandled entity type.");
        } break;
    }
}

void undo_delete_point_light_entity(Level *level, Delete_Point_Light_Entity_Action action) {
    level_add_point_light_entity(level, action.entity, action.entity_id);
}

// entity transforming
void editor_transform_entity(Game_State *game_state, Editor_State *editor_state,
                             Transform_Entity_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        history_add_action(&editor_state->history, Transform_Entity_Action, action);
    }
    
    Level *level = &game_state->current_level;
    Entity *entity = get_entity(level, action.entity_type, action.entity_id);
    set_entity_transform(&game_state->asset_manager, entity, action.transform);
}

void undo_transform_entity(Game_State *game_state, Transform_Entity_Action action) {
    Entity *entity = get_entity(&game_state->current_level, action.entity_type, action.entity_id);
    set_entity_transform(&game_state->asset_manager, entity, action.original_transform);
}

// copies new_entity into entity
void set_entity(Entity *entity, Entity *new_entity) {
    assert(entity->type == new_entity->type);

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) entity;
            *e = *((Normal_Entity *) new_entity);
            return;
        }
        case ENTITY_POINT_LIGHT: {
            Point_Light_Entity *e = (Point_Light_Entity *) entity;
            *e = *((Point_Light_Entity *) new_entity);
            return;
        }
        default: {
            assert(!"Unhandled entity type.");
        }
    }
}

// entity modifying
void editor_modify_entity(Editor_State *editor_state, Level *level,
                          Modify_Entity_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        history_add_action(&editor_state->history, Modify_Entity_Action, action);
    }

    Entity *entity = get_entity(level, action.original->type, action.entity_id);
    set_entity(entity, action.new_entity);
}

void undo_modify_entity(Editor_State *editor_state, Level *level,
                        Modify_Entity_Action action) {
    // yeah, this doesn't handle changing entity types, but i don't think we need to ever handle that.
    // if we stored entities in the way where we have entity type flags and all the different fields in a single
    // entity, then we could do this easily.
    Entity *entity = get_entity(level, action.original->type, action.entity_id);
    set_entity(entity, action.original);
}

// mesh modifying
void editor_modify_mesh(Game_State *game_state, Modify_Mesh_Action action, bool32 is_redoing) {
    // NOTE: we currently only handle changes to the mesh name

    Editor_History *history = &game_state->editor_state.history;
    Asset_Manager *asset_manager = &game_state->asset_manager;

    Mesh *mesh = get_mesh_pointer(asset_manager, action.mesh_id);
    action.original_name = copy(history->allocator_pointer, mesh->name);

    copy_string(&mesh->name, make_string(action.new_name));

    if (!is_redoing) {
        action.new_name = copy(history->allocator_pointer, action.new_name);
        history_add_action(history, Modify_Mesh_Action, action);
    } else {
        game_state->editor_state.editing_selected_entity_mesh = false;
    }

    // we just set this to false so we don't have to handle resetting the text box's text.
    // and we don't want to just set text box's text even when it's not active, since it's nice just in case
    // when you type something that's invalid, that it doesn't erase what you wrote, so that you don't have to
    // type it again. it would get reset if we had "reset when not active" behaviour
}

void undo_modify_mesh(Game_State *game_state,
                      Modify_Mesh_Action action) {
    Mesh *mesh = get_mesh_pointer(&game_state->asset_manager, action.mesh_id);

    game_state->editor_state.editing_selected_entity_mesh = false;

    copy_string(&mesh->name, make_string(action.original_name));
}

// mesh adding
void editor_add_mesh(Editor_State *editor_state, Asset_Manager *asset_manager,
                     Level *level,
                     Add_Mesh_Action action, bool32 is_redoing) {
    int32 mesh_id;
    Mesh new_mesh = add_level_mesh(asset_manager,
                                   action.relative_filename, action.mesh_name, action.mesh_id,
                                   &mesh_id);

    Allocator *allocator = editor_state->history.allocator_pointer;
    if (!is_redoing) {
        // copy strings if we're adding the action for the first time, i.e. the action is coming from game code
        // and not from history code
        action.relative_filename = copy(allocator, action.relative_filename);
        action.mesh_name = copy(allocator, action.mesh_name);
    }
    action.mesh_id = mesh_id;

    if (!is_redoing) {
        history_add_action(&editor_state->history, Add_Mesh_Action, action);
    }

    Entity *entity = get_entity(level, action.entity_type, action.entity_id);
    if (has_mesh_field(entity)) {
        set_entity_mesh(asset_manager, entity, action.mesh_id);
    }
}

void undo_add_mesh(Editor_State *editor_state, Asset_Manager *asset_manager,
                   Level *level,
                   Add_Mesh_Action action) {
    level_delete_mesh(asset_manager, level, action.mesh_id);
}

// mesh deleting
void editor_delete_mesh(Editor_State *editor_state, Asset_Manager *asset_manager,
                        Level *level,
                        Delete_Mesh_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        Allocator *allocator = editor_state->history.allocator_pointer;
        action.relative_filename = copy(allocator, action.relative_filename);
        action.mesh_name = copy(allocator, action.mesh_name);
    }

    bool32 result = level_delete_mesh(asset_manager, level, action.mesh_id,
                                      &action.num_entities_with_mesh,
                                      action.entity_ids_with_mesh, action.entity_types,
                                      MAX_ENTITIES_WITH_SAME_MESH);

    if (!result) {
        add_message(Context::message_manager, make_string("Too many entities have this mesh to be deleted."));
        deallocate(action.relative_filename);
        deallocate(action.mesh_name);
        return;
    }
    
    if (!is_redoing) {
        history_add_action(&editor_state->history, Delete_Mesh_Action, action);
    }
}

void undo_delete_mesh(Editor_State *editor_state, Asset_Manager *asset_manager,
                      Level *level,
                      Delete_Mesh_Action action) {
    int32 mesh_id;
    add_level_mesh(asset_manager,
                   action.relative_filename, action.mesh_name, action.mesh_id,
                   &mesh_id);

    for (int32 i = 0; i < action.num_entities_with_mesh; i++) {
        Entity *entity = get_entity(level, action.entity_types[i], action.entity_ids_with_mesh[i]);
        set_entity_mesh(asset_manager, entity, mesh_id);
    }
}

// material modifying
// NOTE: this procedure assumes that we've allocated the old and new materials onto the history allocator
void editor_modify_material(Editor_State *editor_state, Level *level,
                            Modify_Material_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        history_add_action(&editor_state->history, Modify_Material_Action, action);
    }
    
    set_material_copy(level, action.material_id, action.new_material);
}

void undo_modify_material(Editor_State *editor_state, Level *level,
                          Modify_Material_Action action) {
    set_material_copy(level, action.material_id, action.old_material);
}

// material adding
int32 editor_add_material(Editor_State *editor_state,
                          Level *level,
                          Add_Material_Action action, bool32 is_redoing) {
    action.material_id = level_add_material(level, action.material_id);
    Material *material = get_material_pointer(level, action.material_id);

    Allocator *allocator = editor_state->history.allocator_pointer;
    if (!is_redoing) {
        action.material_name = copy(allocator, material->name);
    }
    
    Entity *entity = get_entity(level, action.entity_type, action.entity_id);
    if (has_material_field(entity)) {
        set_entity_material(entity, action.material_id);
    }

    if (!is_redoing) {
        history_add_action(&editor_state->history, Add_Material_Action, action);
    }

    return action.material_id;
}

void undo_add_material(Level *level,
                       Add_Material_Action action) {
    level_delete_material(level, action.material_id);
}

// material deleting
void editor_delete_material(Editor_State *editor_state, Level *level,
                            Delete_Material_Action action, bool32 is_redoing) {
    // FIXME: this leaks if level_delete_material fails
    if (!is_redoing) {
        action.material = copy(editor_state->history.allocator_pointer, action.material);
    }

    bool32 result = level_delete_material(level, action.material_id,
                                          &action.num_entities,
                                          action.entity_ids, action.entity_types,
                                          MAX_ENTITIES_WITH_SAME_MATERIAL);
    if (!result) {
        add_message(Context::message_manager, make_string("Too many entities have this material to be deleted."));
        deallocate(action.material);
        return;
    }
    
    if (!is_redoing) {
        history_add_action(&editor_state->history, Delete_Material_Action, action);
    }
}

void undo_delete_material(Editor_State *editor_state, Level *level,
                          Delete_Material_Action action) {
    Material level_material = level_copy_material(level, action.material);
    level_add_material(level, level_material, action.material_id);

    for (int32 i = 0; i < action.num_entities; i++) {
        Entity *entity = get_entity(level, action.entity_types[i], action.entity_ids[i]);
        set_entity_material(entity, action.material_id);
    }
}










// texture modifying
// NOTE: this procedure assumes that we've allocated the old and new textures onto the history allocator
void editor_modify_texture(Editor_State *editor_state, Level *level,
                           Modify_Texture_Action action, bool32 is_redoing) {
    Texture *texture = get_texture_pointer(level, action.texture_id);
    Editor_History *history = &editor_state->history;

    action.old_name = copy(history->allocator_pointer, texture->name);
    copy_string(&texture->name, make_string(action.new_name));

    if (!is_redoing) {
        action.new_name = copy(history->allocator_pointer, action.new_name);
        history_add_action(history, Modify_Texture_Action, action);
    }
}

void undo_modify_texture(Editor_State *editor_state, Level *level,
                          Modify_Texture_Action action) {
    Texture *texture = get_texture_pointer(level, action.texture_id);
    copy_string(&texture->name, make_string(action.old_name));
}

// texture adding
int32 editor_add_texture(Editor_State *editor_state,
                         Level *level,
                         Add_Texture_Action action, bool32 is_redoing) {
    //Texture new_texture = make_texture(action.texture_name, action.texture_filename);
    action.texture_id = level_add_texture(level, action.texture_filename, action.texture_name, action.texture_id);

    Material *material = get_material_pointer(level, action.material_id);
    material->texture_id = action.texture_id;

    Allocator *allocator = editor_state->history.allocator_pointer;
    if (!is_redoing) {
        action.texture_name = copy(allocator, action.texture_name);
        action.texture_filename = copy(allocator, action.texture_filename);
        history_add_action(&editor_state->history, Add_Texture_Action, action);
    }
    
    return action.texture_id;
}

void undo_add_texture(Level *level,
                      Add_Texture_Action action) {
    level_delete_texture(level, action.texture_id);
    Material *material = get_material_pointer(level, action.material_id);
    material->texture_id = action.old_texture_id;
}

// texture deleting
void editor_delete_texture(Editor_State *editor_state, Level *level,
                           Delete_Texture_Action action, bool32 is_redoing) {
    if (!is_redoing) {
        Allocator *allocator = editor_state->history.allocator_pointer;
        action.texture_name = copy(allocator, action.texture_name);
        action.texture_filename = copy(allocator, action.texture_filename);
    }

    bool32 result = level_delete_texture(level, action.texture_id,
                                         &action.num_materials,
                                         action.material_ids,
                                         MAX_MATERIALS_WITH_SAME_TEXTURE);
    if (!result) {
        add_message(Context::message_manager, make_string("Too many materials have this texture to be deleted."));
        deallocate(action.texture_name);
        deallocate(action.texture_filename);
        return;
    }
    
    if (!is_redoing) {
        history_add_action(&editor_state->history, Delete_Texture_Action, action);
    }
}

void undo_delete_texture(Editor_State *editor_state, Level *level,
                          Delete_Texture_Action action) {
    level_add_texture(level, action.texture_filename, action.texture_name, action.texture_id);

    for (int32 i = 0; i < action.num_materials; i++) {
        Material *material = get_material_pointer(level, action.material_ids[i]);
        material->texture_id = action.texture_id;
    }
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

void history_undo(Game_State *game_state, Editor_History *history) {
    assert(history->start_index >= 0);
    assert(history->end_index >= 0);

    int32 num_entries = history_get_num_entries(history);

    if (history->num_undone == num_entries) return;

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;
    Level *level = &game_state->current_level;

    // we undo the action at current_index.
    Editor_Action *current_action = history->entries[history->current_index];
    switch (current_action->type) {
        case ACTION_NONE: {
            assert(!"Action does not have a type.");
            return;
        }
        case ACTION_ADD_NORMAL_ENTITY: {
            Add_Normal_Entity_Action *action = (Add_Normal_Entity_Action *) current_action;
            undo_add_normal_entity(editor_state, level, *action);
        } break;
        case ACTION_ADD_POINT_LIGHT_ENTITY: {
            Add_Point_Light_Entity_Action *action = (Add_Point_Light_Entity_Action *) current_action;
            undo_add_point_light_entity(editor_state, level, *action);
        } break;
        case ACTION_DELETE_NORMAL_ENTITY: {
            Delete_Normal_Entity_Action *action = (Delete_Normal_Entity_Action *) current_action;
            undo_delete_normal_entity(level, *action);
        } break;
        case ACTION_DELETE_POINT_LIGHT_ENTITY: {
            Delete_Point_Light_Entity_Action *action = (Delete_Point_Light_Entity_Action *) current_action;
            undo_delete_point_light_entity(level, *action);
        } break;
        case ACTION_TRANSFORM_ENTITY: {
            Transform_Entity_Action *action = (Transform_Entity_Action *) current_action;
            undo_transform_entity(game_state, *action);
        } break;
        case ACTION_MODIFY_ENTITY: {
            Modify_Entity_Action *action = (Modify_Entity_Action *) current_action;
            undo_modify_entity(editor_state, level, *action);
        } break;
        case ACTION_MODIFY_MESH: {
            Modify_Mesh_Action *action = (Modify_Mesh_Action *) current_action;
            undo_modify_mesh(game_state, *action);
        } break;
        case ACTION_ADD_MESH: {
            Add_Mesh_Action *action = (Add_Mesh_Action *) current_action;
            undo_add_mesh(editor_state, asset_manager, level, *action);
        } break;
        case ACTION_DELETE_MESH: {
            Delete_Mesh_Action *action = (Delete_Mesh_Action *) current_action;
            undo_delete_mesh(editor_state, asset_manager, level, *action);
        } break;
        case ACTION_MODIFY_MATERIAL: {
            Modify_Material_Action *action = (Modify_Material_Action *) current_action;
            undo_modify_material(editor_state, level, *action);
        } break;
        case ACTION_DELETE_MATERIAL: {
            Delete_Material_Action *action = (Delete_Material_Action *) current_action;
            undo_delete_material(editor_state, level, *action);
        } break;
        case ACTION_ADD_MATERIAL: {
            Add_Material_Action *action = (Add_Material_Action *) current_action;
            undo_add_material(level, *action);
        } break;

        case ACTION_MODIFY_TEXTURE: {
            Modify_Texture_Action *action = (Modify_Texture_Action *) current_action;
            undo_modify_texture(editor_state, level, *action);
        } break;
        case ACTION_DELETE_TEXTURE: {
            Delete_Texture_Action *action = (Delete_Texture_Action *) current_action;
            undo_delete_texture(editor_state, level, *action);
        } break;
        case ACTION_ADD_TEXTURE: {
            Add_Texture_Action *action = (Add_Texture_Action *) current_action;
            undo_add_texture(level, *action);
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

void history_redo(Game_State *game_state, Editor_History *history) {
    assert(history->num_undone > 0);

    // we redo the action at current_index + 1, unless current_index is -1, in which case we redo the action
    // at start_index.
    int32 redo_index;
    if (history->current_index == -1) {
        redo_index = history->start_index;
    } else {
        redo_index = (history->current_index + 1) % MAX_EDITOR_HISTORY_ENTRIES;
    }

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;
    Level *level = &game_state->current_level;

    Editor_Action *redo_action = history->entries[redo_index];
    switch (redo_action->type) {
        case ACTION_NONE: {
            assert(!"Action does not have a type.");
            return;
        }
        case ACTION_ADD_NORMAL_ENTITY: {
            Add_Normal_Entity_Action *action = (Add_Normal_Entity_Action *) redo_action;
            editor_add_normal_entity(editor_state, game_state, *action, true);
        } break;
        case ACTION_ADD_POINT_LIGHT_ENTITY: {
            Add_Point_Light_Entity_Action *action = (Add_Point_Light_Entity_Action *) redo_action;
            editor_add_point_light_entity(editor_state, game_state, *action, true);
        } break;
        case ACTION_DELETE_NORMAL_ENTITY: {
            Delete_Normal_Entity_Action *action = (Delete_Normal_Entity_Action *) redo_action;
            editor_delete_normal_entity(editor_state, level, *action, true);
        } break;
        case ACTION_DELETE_POINT_LIGHT_ENTITY: {
            Delete_Point_Light_Entity_Action *action = (Delete_Point_Light_Entity_Action *) redo_action;
            editor_delete_point_light_entity(editor_state, level, *action, true);
        } break;
        case ACTION_TRANSFORM_ENTITY: {
            Transform_Entity_Action *action = (Transform_Entity_Action *) redo_action;
            editor_transform_entity(game_state, editor_state, *action, true);
        } break;
        case ACTION_MODIFY_ENTITY: {
            Modify_Entity_Action *action = (Modify_Entity_Action *) redo_action;
            editor_modify_entity(editor_state, level, *action, true);
        } break;
        case ACTION_MODIFY_MESH: {
            Modify_Mesh_Action *action = (Modify_Mesh_Action *) redo_action;
            editor_modify_mesh(game_state, *action, true);
        } break;
        case ACTION_ADD_MESH: {
            Add_Mesh_Action *action = (Add_Mesh_Action *) redo_action;
            editor_add_mesh(editor_state, asset_manager, level, *action, true);
        } break;
        case ACTION_DELETE_MESH: {
            Delete_Mesh_Action *action = (Delete_Mesh_Action *) redo_action;
            editor_delete_mesh(editor_state, asset_manager, level, *action, true);
        } break;
        case ACTION_MODIFY_MATERIAL: {
            Modify_Material_Action *action = (Modify_Material_Action *) redo_action;
            editor_modify_material(editor_state, level, *action, true);
        } break;
        case ACTION_DELETE_MATERIAL: {
            Delete_Material_Action *action = (Delete_Material_Action *) redo_action;
            editor_delete_material(editor_state, level, *action, true);
        } break;
        case ACTION_ADD_MATERIAL: {
            Add_Material_Action *action = (Add_Material_Action *) redo_action;
            editor_add_material(editor_state, level, *action, true);
        } break;

        case ACTION_MODIFY_TEXTURE: {
            Modify_Texture_Action *action = (Modify_Texture_Action *) redo_action;
            editor_modify_texture(editor_state, level, *action, true);
        } break;
        case ACTION_DELETE_TEXTURE: {
            Delete_Texture_Action *action = (Delete_Texture_Action *) redo_action;
            editor_delete_texture(editor_state, level, *action, true);
        } break;
        case ACTION_ADD_TEXTURE: {
            Add_Texture_Action *action = (Add_Texture_Action *) redo_action;
            editor_add_texture(editor_state, level, *action, true);
        } break;
        default: {
            assert(!"Unhandled editor action type.");
            return;
        }
    }

    history->current_index = redo_index;
    history->num_undone--;
}

void history_reset(Editor_History *history) {
    clear(history->allocator_pointer);
    history->start_index = -1;
    history->end_index = -1;
    history->current_index = -1;
    history->num_undone = 0;
}
