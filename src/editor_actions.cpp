#include "editor_actions.h"
#include "editor.h"

void history_deallocate(Editor_State *editor_state, Editor_Action *editor_action) {
    Allocator *allocator = (Allocator *) &editor_state->history_heap;
    
    switch (editor_action->type) {
        case ACTION_NONE: {
            assert(!"Action has no type.");
        } break;
        case ACTION_ADD_NORMAL_ENTITY: {} break;
        case ACTION_ADD_POINT_LIGHT_ENTITY: {} break;
        case ACTION_DELETE_ENTITY: {
            Delete_Entity_Action *action = (Delete_Entity_Action *) editor_action;
            deallocate(allocator, action->entity);
        } break;
        case ACTION_MODIFY_ENTITY: {
            Modify_Entity_Action *action = (Modify_Entity_Action *) editor_action;
            deallocate(allocator, action->old_entity);
            deallocate(allocator, action->new_entity);
        } break;
        case ACTION_MODIFY_MESH_NAME: {
            Modify_Mesh_Name_Action *action = (Modify_Mesh_Name_Action *) editor_action;
            deallocate(action->old_name);
            deallocate(action->new_name);
        } break;
        case ACTION_ADD_MESH: {
            Add_Mesh_Action *action = (Add_Mesh_Action *) editor_action;
            deallocate(action->filename);
            deallocate(action->name);
        } break;
        case ACTION_DELETE_MESH: {
            Delete_Mesh_Action *action = (Delete_Mesh_Action *) editor_action;
            deallocate(action->filename);
            deallocate(action->mesh_name);
            deallocate(&action->entity_ids);
        } break;
        case ACTION_MODIFY_MATERIAL: {
            Modify_Material_Action *action = (Modify_Material_Action *) editor_action;
            deallocate(*action->old_material);
            deallocate(*action->new_material);
            deallocate(allocator, action->old_material);
            deallocate(allocator, action->new_material);
        } break;
        case ACTION_ADD_MATERIAL: {
            Add_Material_Action *action = (Add_Material_Action *) editor_action;
            deallocate(action->name);
        } break;
        case ACTION_DELETE_MATERIAL: {
            Delete_Material_Action *action = (Delete_Material_Action *) editor_action;
            deallocate(action->material);
            deallocate(&action->entity_ids);
        } break;
        case ACTION_MODIFY_TEXTURE: {
            Modify_Texture_Action *action = (Modify_Texture_Action *) editor_action;
            deallocate(action->old_texture);
            deallocate(action->new_texture);
        } break;
        case ACTION_ADD_TEXTURE: {
            Add_Texture_Action *action = (Add_Texture_Action *) editor_action;
            deallocate(action->filename);
            deallocate(action->name);
        } break;
        case ACTION_DELETE_TEXTURE: {
            Delete_Texture_Action *action = (Delete_Texture_Action *) editor_action;
            deallocate(action->texture);
            deallocate(&action->material_ids);
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

void set_entity(Editor_State *editor_state, Entity *uncast_entity, Entity *new_entity) {
    assert(uncast_entity->type == new_entity->type);

    Allocator *entity_allocator = (Allocator *) &editor_state->entity_heap;
    switch (uncast_entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *entity = (Normal_Entity *) uncast_entity;
            deallocate(*entity);
            *entity = copy(entity_allocator, *((Normal_Entity *) new_entity));
        } break;
        case ENTITY_POINT_LIGHT: {
            Point_Light_Entity *entity = (Point_Light_Entity *) uncast_entity;
            deallocate(*entity);
            *entity = copy(entity_allocator, *((Point_Light_Entity *) new_entity));
        } break;
        default: {
            assert(!"Unhandled entity type.");
        } break;
    }
}

void do_modify_entity(Editor_State *editor_state, int32 entity_id, Entity *old_entity, Entity *new_entity,
                      bool32 is_redoing = false) {
    if (!is_redoing) {
        Modify_Entity_Action action = { ACTION_MODIFY_ENTITY };
        action.entity_id = entity_id;
        action.old_entity = old_entity;
        action.new_entity = new_entity;
        history_add_action(editor_state, Modify_Entity_Action, action);
    }

    Entity *entity = get_entity(editor_state, entity_id);
    set_entity(editor_state, entity, new_entity);
}

void undo_modify_entity(Editor_State *editor_state, Modify_Entity_Action action) {
    Entity *entity = get_entity(editor_state, action.entity_id);
    set_entity(editor_state, entity, action.old_entity);
}

void end_entity_change(Editor_State *editor_state, Entity *entity) {
    if (!editor_state->old_entity) {
        return;
    }
    Entity *new_entity = copy_cast_entity((Allocator *) &editor_state->history_heap, entity);
    do_modify_entity(editor_state, entity->id, editor_state->old_entity, new_entity);
    editor_state->old_entity = NULL;
}

void start_entity_change(Editor_State *editor_state, Entity *entity) {
    if (editor_state->old_entity) {
        // if UI element A comes before B in the code, if B is active, then A becomes active, A will reset
        // old_entity before B gets to finish their change. so, to prevent this, we check for old_entity and
        // if it's there, we finalize the change. then later, when B calls end_entity_change since it just
        // became inactive, we just check if old_entity is NULL, meaning their change has already been
        // finalized.
        // NOTE: we assume that we're still on the same entity
        end_entity_change(editor_state, entity);
    }

    editor_state->old_entity = copy_cast_entity((Allocator *) &editor_state->history_heap, entity);
}

void do_modify_mesh_name(Editor_State *editor_state, int32 mesh_id, String new_name,
                         bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Mesh *mesh = get_mesh_pointer(asset_manager, mesh_id);
    if (!is_redoing) {
        Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
        String old_name = copy(history_allocator, mesh->name);
        new_name = copy(history_allocator, new_name);

        Modify_Mesh_Name_Action action = { ACTION_MODIFY_MESH_NAME };
        action.mesh_id = mesh_id;
        action.old_name = old_name;
        action.new_name = new_name;
        history_add_action(editor_state, Modify_Mesh_Name_Action, action);
    }

    replace_with_copy(asset_manager->allocator_pointer, &mesh->name, new_name);
}

void undo_modify_mesh_name(Editor_State *editor_state, Modify_Mesh_Name_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Mesh *mesh = get_mesh_pointer(asset_manager, action.mesh_id);
    replace_with_copy(asset_manager->allocator_pointer, &mesh->name, action.old_name);
}

void do_add_mesh(Editor_State *editor_state, String mesh_filename, String mesh_name, int32 entity_id,
                 int32 mesh_id = -1, bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    
    Allocator *allocator = asset_manager->allocator_pointer;
    String filename = copy(allocator, mesh_filename);
    String name = copy(allocator, mesh_name);

    Mesh mesh = read_and_load_mesh(allocator, filename, name, Mesh_Type::LEVEL);
    int32 id = add_mesh(asset_manager, mesh, mesh_id);

    Entity *entity = get_entity(editor_state, entity_id);
    int32 original_mesh_id;
    set_mesh(asset_manager, entity, id, &original_mesh_id);

    if (!is_redoing) {
        Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
        mesh_filename = copy(history_allocator, mesh_filename);
        mesh_name = copy(history_allocator, mesh_name);

        Add_Mesh_Action action = { ACTION_ADD_MESH };
        action.entity_id = entity_id;
        action.mesh_id = id;
        action.filename = mesh_filename;
        action.name = mesh_name;
        action.original_mesh_id = original_mesh_id;
        history_add_action(editor_state, Add_Mesh_Action, action);
    }
}

void undo_add_mesh(Editor_State *editor_state, Add_Mesh_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    delete_mesh(asset_manager, action.mesh_id);
    Entity *entity = get_entity(editor_state, action.entity_id);
    set_mesh(asset_manager, entity, action.original_mesh_id);
}

void do_delete_mesh(Editor_State *editor_state, int32 mesh_id, bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Editor_Level *level = &editor_state->level;

    if (!is_redoing) {
        Mesh *mesh = get_mesh_pointer(asset_manager, mesh_id);
        assert(mesh->type == Mesh_Type::LEVEL);

        Delete_Mesh_Action action = { ACTION_DELETE_MESH };

        Allocator *allocator = (Allocator *) &editor_state->history_heap;

        Linked_List<int32> entity_ids;
        make_and_init_linked_list(int32, &entity_ids, allocator);
    
        FOR_LIST_NODES(Entity *, level->entities) {
            Entity *entity = current_node->value;
            if (has_mesh_field(entity) && (get_mesh_id(entity) == mesh_id)) {
                add(&entity_ids, entity->id);
            }
        }

        action.mesh_id = mesh_id;
        action.filename = copy(allocator, mesh->filename);
        action.mesh_name = copy(allocator, mesh->name);
        action.entity_ids = entity_ids;

        history_add_action(editor_state, Delete_Mesh_Action, action);
    }

    int32 default_mesh_id = get_mesh_id_by_name(asset_manager, make_string("cube"));
    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        if (has_mesh_field(entity) && (get_mesh_id(entity) == mesh_id)) {
            // TODO: we may want to pass the mesh in as well, so we don't have to keep looking the mesh up to get
            //       its AABB in set_mesh()
            set_mesh(asset_manager, entity, default_mesh_id);
        }
    }
    
    delete_mesh(asset_manager, mesh_id);
}

void undo_delete_mesh(Editor_State *editor_state, Delete_Mesh_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Allocator *asset_allocator = asset_manager->allocator_pointer;
    String filename = copy(asset_allocator, action.filename);
    String name = copy(asset_allocator, action.mesh_name);

    Mesh mesh = read_and_load_mesh(asset_allocator, filename, name, Mesh_Type::LEVEL);
    add_mesh(asset_manager, mesh, action.mesh_id);

    FOR_LIST_NODES(int32, action.entity_ids) {
        Entity *entity = get_entity(editor_state, current_node->value);
        set_mesh(asset_manager, entity, action.mesh_id);
    }
}

void do_modify_material(Editor_State *editor_state, int32 material_id, Material *old_material, Material *new_material,
                        bool32 is_redoing = false) {
    if (!is_redoing) {
        Modify_Material_Action action = { ACTION_MODIFY_MATERIAL };
        action.material_id = material_id;
        action.old_material = old_material;
        action.new_material = new_material;
        history_add_action(editor_state, Modify_Material_Action, action);
    }

    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Material *material = get_material_pointer(asset_manager, material_id);

    deallocate(*material);
    *material = copy(asset_manager->allocator_pointer, *new_material);
}

void undo_modify_material(Editor_State *editor_state, Modify_Material_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Material *material = get_material_pointer(asset_manager, action.material_id);

    deallocate(*material);
    *material = copy(asset_manager->allocator_pointer, *action.old_material);
}

void start_material_change(Editor_State *editor_state, int32 material_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Material *material = get_material_pointer(asset_manager, material_id);

    Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
    Material *old_material = (Material *) allocate(history_allocator, sizeof(Material));
    *old_material = copy((Allocator *) &editor_state->history_heap, *material);
    editor_state->old_material = old_material;
}

void end_material_change(Editor_State *editor_state, int32 material_id) {
    assert(editor_state->old_material);

    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Material *material = get_material_pointer(asset_manager, material_id);

    Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
    Material *new_material = (Material *) allocate(history_allocator, sizeof(Material));
    *new_material = copy((Allocator *) &editor_state->history_heap, *material);

    do_modify_material(editor_state, material_id, editor_state->old_material, new_material);
    editor_state->old_material = NULL;
}

void do_add_material(Editor_State *editor_state, String material_name, int32 entity_id, int32 material_id = -1,
                     bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    
    Allocator *allocator = asset_manager->allocator_pointer;
    String name = copy(allocator, material_name);

    Material new_material = make_material(name);
    int32 id = add_material(asset_manager, new_material, material_id);

    Entity *entity = get_entity(editor_state, entity_id);
    int32 original_material_id;
    set_material(entity, id, &original_material_id);

    if (!is_redoing) {
        Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
        material_name = copy(history_allocator, material_name);

        Add_Material_Action action = { ACTION_ADD_MATERIAL };
        action.entity_id = entity_id;
        action.material_id = id;
        action.name = material_name;
        action.original_material_id = original_material_id;
        history_add_action(editor_state, Add_Material_Action, action);
    }
}

void undo_add_material(Editor_State *editor_state, Add_Material_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    delete_material(asset_manager, action.material_id);
    Entity *entity = get_entity(editor_state, action.entity_id);
    set_material(entity, action.original_material_id);
}

void do_delete_material(Editor_State *editor_state, int32 material_id, bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Editor_Level *level = &editor_state->level;

    if (!is_redoing) {
        Material *material = get_material_pointer(asset_manager, material_id);

        Delete_Material_Action action = { ACTION_DELETE_MATERIAL };

        Allocator *allocator = (Allocator *) &editor_state->history_heap;

        Linked_List<int32> entity_ids;
        make_and_init_linked_list(int32, &entity_ids, allocator);
    
        FOR_LIST_NODES(Entity *, level->entities) {
            Entity *entity = current_node->value;
            if (has_material_field(entity) && (get_material_id(entity) == material_id)) {
                add(&entity_ids, entity->id);
            }
        }

        action.material_id = material_id;
        action.material = copy(allocator, *material);
        action.entity_ids = entity_ids;

        history_add_action(editor_state, Delete_Material_Action, action);
    }

    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        if (has_material_field(entity) && (get_material_id(entity) == material_id)) {
            set_material(entity, -1);
        }
    }
    
    delete_material(asset_manager, material_id);
}

void undo_delete_material(Editor_State *editor_state, Delete_Material_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Allocator *asset_allocator = asset_manager->allocator_pointer;

    Material new_material = copy(asset_allocator, action.material);
    add_material(asset_manager, new_material, action.material_id);

    FOR_LIST_NODES(int32, action.entity_ids) {
        Entity *entity = get_entity(editor_state, current_node->value);
        set_material(entity, action.material_id);
    }
}

void do_modify_texture(Editor_State *editor_state, int32 texture_id, Texture old_texture, Texture new_texture,
                       bool32 is_redoing = false) {
    if (!is_redoing) {
        Modify_Texture_Action action = { ACTION_MODIFY_TEXTURE };
        action.texture_id = texture_id;
        action.old_texture = old_texture;
        action.new_texture = new_texture;
        history_add_action(editor_state, Modify_Texture_Action, action);
    }

    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Texture *texture = get_texture_pointer(asset_manager, texture_id);

    deallocate(*texture);
    *texture = copy(asset_manager->allocator_pointer, new_texture);
}

void undo_modify_texture(Editor_State *editor_state, Modify_Texture_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Allocator *asset_allocator = asset_manager->allocator_pointer;
    Texture *texture = get_texture_pointer(asset_manager, action.texture_id);

    deallocate(*texture);
    *texture = copy(asset_allocator, action.old_texture);
}

void start_texture_change(Editor_State *editor_state, int32 texture_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Texture texture = get_texture(asset_manager, texture_id);
    editor_state->old_texture = copy((Allocator *) &editor_state->history_heap, texture);
}

void end_texture_change(Editor_State *editor_state, int32 texture_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Texture texture = get_texture(asset_manager, texture_id);
    Texture new_texture = copy((Allocator *) &editor_state->history_heap, texture);
    do_modify_texture(editor_state, texture_id, editor_state->old_texture, new_texture);
}

void do_add_texture(Editor_State *editor_state, String texture_filename, String texture_name,
                    bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    
    Allocator *allocator = asset_manager->allocator_pointer;
    String filename = copy(allocator, texture_filename);
    String name = copy(allocator, texture_name);

    Texture texture = make_texture(filename);
    add_texture(asset_manager, texture, name);

    if (!is_redoing) {
        Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
        texture_filename = copy(history_allocator, texture_filename);
        texture_name = copy(history_allocator, texture_name);

        Add_Texture_Action action = { ACTION_ADD_TEXTURE };
        action.filename = texture_filename;
        action.name = texture_name;
        history_add_action(editor_state, Add_Texture_Action, action);
    }
}

void undo_add_texture(Editor_State *editor_state, Add_Texture_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    delete_texture(asset_manager, action.texture_id);
    Material *material = get_material_pointer(asset_manager, action.material_id);
    set_texture(material, action.original_texture_id);
}

void do_delete_texture(Editor_State *editor_state, int32 texture_id, bool32 is_redoing = false) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Editor_Level *level = &editor_state->level;    

    if (!is_redoing) {
        Allocator *history_allocator = (Allocator *) &editor_state->history_heap;
        Texture texture = get_texture(asset_manager, texture_id);

        Delete_Texture_Action action = { ACTION_DELETE_TEXTURE };

        Linked_List<int32> material_ids;
        make_and_init_linked_list(int32, &material_ids, history_allocator);

        FOR_ENTRY_POINTERS(int32, Material, asset_manager->material_table) {
            Material *material = &entry->value;
            if (material->texture_id == texture_id) {
                add(&material_ids, entry->key);
            }
        }

        action.material_ids = material_ids;

        texture.is_loaded = false;
        texture.should_unload = false;
        action.texture = copy(history_allocator, texture);
        action.texture_id = texture_id;
        history_add_action(editor_state, Delete_Texture_Action, action);
    }

    FOR_ENTRY_POINTERS(int32, Material, asset_manager->material_table) {
        Material *material = &entry->value;
        if (material->texture_id == texture_id) {
            material->texture_id = -1;
        }
    }

    delete_texture(asset_manager, texture_id);
}

void undo_delete_texture(Editor_State *editor_state, Delete_Texture_Action action) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Texture new_texture = copy(asset_manager->allocator_pointer, action.texture);
    add_texture(asset_manager, new_texture, action.texture_id);

    FOR_LIST_NODES(int32, action.material_ids) {
        Material *material = get_material_pointer(asset_manager, current_node->value);
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
        case ACTION_MODIFY_ENTITY: {
            Modify_Entity_Action *action = (Modify_Entity_Action *) current_action;
            undo_modify_entity(editor_state, *action);
        } break;
        case ACTION_MODIFY_MESH_NAME: {
            Modify_Mesh_Name_Action *action = (Modify_Mesh_Name_Action *) current_action;
            undo_modify_mesh_name(editor_state, *action);
        } break;
        case ACTION_ADD_MESH: {
            Add_Mesh_Action *action = (Add_Mesh_Action *) current_action;
            undo_add_mesh(editor_state, *action);
        } break;
        case ACTION_DELETE_MESH: {
            Delete_Mesh_Action *action = (Delete_Mesh_Action *) current_action;
            undo_delete_mesh(editor_state, *action);
        } break;
        case ACTION_MODIFY_MATERIAL: {
            Modify_Material_Action *action = (Modify_Material_Action *) current_action;
            undo_modify_material(editor_state, *action);
        } break;
        case ACTION_ADD_MATERIAL: {
            Add_Material_Action *action = (Add_Material_Action *) current_action;
            undo_add_material(editor_state, *action);
        } break;
        case ACTION_DELETE_MATERIAL: {
            Delete_Material_Action *action = (Delete_Material_Action *) current_action;
            undo_delete_material(editor_state, *action);
        } break;
        case ACTION_MODIFY_TEXTURE: {
            Modify_Texture_Action *action = (Modify_Texture_Action *) current_action;
            undo_modify_texture(editor_state, *action);
        } break;
        case ACTION_ADD_TEXTURE: {
            Add_Texture_Action *action = (Add_Texture_Action *) current_action;
            undo_add_texture(editor_state, *action);
        } break;
        case ACTION_DELETE_TEXTURE: {
            Delete_Texture_Action *action = (Delete_Texture_Action *) current_action;
            undo_delete_texture(editor_state, *action);
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
        case ACTION_MODIFY_ENTITY: {
            Modify_Entity_Action *action = (Modify_Entity_Action *) redo_action;
            do_modify_entity(editor_state, action->entity_id, action->old_entity, action->new_entity, true);
        } break;
        case ACTION_MODIFY_MESH_NAME: {
            Modify_Mesh_Name_Action *action = (Modify_Mesh_Name_Action *) redo_action;
            do_modify_mesh_name(editor_state, action->mesh_id, action->new_name, true);
        } break;
        case ACTION_ADD_MESH: {
            Add_Mesh_Action *action = (Add_Mesh_Action *) redo_action;
            do_add_mesh(editor_state, action->filename, action->name, action->entity_id, action->mesh_id, true);
        } break;
        case ACTION_DELETE_MESH: {
            Delete_Mesh_Action *action = (Delete_Mesh_Action *) redo_action;
            do_delete_mesh(editor_state, action->mesh_id, true);
        } break;
        case ACTION_MODIFY_MATERIAL: {
            Modify_Material_Action *action = (Modify_Material_Action *) redo_action;
            do_modify_material(editor_state, action->material_id, action->old_material, action->new_material, true);
        } break;
        case ACTION_ADD_MATERIAL: {
            Add_Material_Action *action = (Add_Material_Action *) redo_action;
            do_add_material(editor_state, action->name, action->entity_id, action->material_id, true);
        } break;
        case ACTION_DELETE_MATERIAL: {
            Delete_Material_Action *action = (Delete_Material_Action *) redo_action;
            do_delete_material(editor_state, action->material_id, true);
        } break;
        case ACTION_MODIFY_TEXTURE: {
            Modify_Texture_Action *action = (Modify_Texture_Action *) redo_action;
            do_modify_texture(editor_state, action->texture_id, action->old_texture, action->new_texture, true);
        } break;
        case ACTION_ADD_TEXTURE: {
            Add_Texture_Action *action = (Add_Texture_Action *) redo_action;
            do_add_texture(editor_state, action->filename, action->name, true);
        } break;
        case ACTION_DELETE_TEXTURE: {
            Delete_Texture_Action *action = (Delete_Texture_Action *) redo_action;
            do_delete_texture(editor_state, action->texture_id, true);
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
