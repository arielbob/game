#include "linked_list.h"
#include "level.h"

void unload_level(Level *level) {
    level->entities = NULL;
    unload_level_assets();

    clear_heap(&level->heap);

    level->name     = make_string("");
    level->filename = make_string("");
    
    level->entities = NULL;
    level->total_entities_added_ever = 0;
    
    level->is_loaded = false;
}

void new_level(Level *level) {
    unload_level(level);
}

bool32 read_and_load_level(Level *level, char *filename) {
    Marker m = begin_region();

    Level_Info *level_info = (Level_Info *) allocate(temp_region, sizeof(Level_Info), true);
    level_info->filename = make_string(filename);
    File_Data level_file = platform_open_and_read_file(temp_region, filename);

    // TODO: add error string and output error if this fails
    char *error;
    bool32 parse_result = Level_Loader::parse_level(temp_region, level_file, level_info, &error);

    if (!parse_result) {
        // TODO: don't assert here and handle the error instead somehow
        debug_print(error);
        debug_print("\n");
        assert(!"Level parsing failed.");
        return false;
    }
    
    load_level(level, level_info);
    
    end_region(m);

    return true;
}

int32 add_entity(Level *level, Entity *entity) {
    int32 id = level->total_entities_added_ever++;

    entity->id = id;

    update_entity_aabb(entity);

    if (level->entities) {
        level->entities->prev = entity;
    }
    entity->next = level->entities;
    level->entities = entity;
    
    return id;
}

void make_and_add_entity(Level *level, Entity_Info info) {
    Allocator *allocator = (Allocator *) &level->heap;

    Entity *entity = (Entity *) allocate(allocator, sizeof(Entity));
    *entity = make_entity_from_info(allocator, &info);

    add_entity(level, entity);
}

void delete_entity(Level *level, int32 id) {
    Entity *current = level->entities;
    while (current) {
        if (current->id == id) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                level->entities = current->next;
            }
            
            if (current->next) {
                current->next->prev = current->prev;
            }

            // TODO: we might want to store allocators with entities if they end up being different, but for now,
            //       all editor entities are stored in the editor heap
            deallocate((Allocator *) &level->heap, current);
            break;
        }
    }
}

Entity *get_entity(Level *level, int32 id) {
    Entity *current = level->entities;
    while (current) {
        if (current->id == id) {
            return current;
        }

        current = current->next;
    }
    
    assert(!"Entity not found.");
    return NULL;
}

void load_level_entities(Level *level, Level_Info *level_info) {
    Allocator *allocator = (Allocator *) &level->heap;
    
    for (int32 i = 0; i < level_info->num_entities; i++) {
        Entity *entity = (Entity *) allocate(allocator, sizeof(Entity));
        *entity = make_entity_from_info(allocator, &level_info->entities[i]);
        
        add_entity(level, entity);
    }
}

void load_level(Level *level, Level_Info *level_info) {
    if (level->is_loaded) {
        assert(!"Current level must be unloaded before loading a level.");
        return;
    }

    level->name         = copy((Allocator *) &level->heap, level_info->name);
    level->filename     = copy((Allocator *) &level->heap, level_info->filename);
    
    load_level_assets(level_info);
    load_level_entities(&game_state->level, level_info);

    level->is_loaded = true;

#if 0
    Vec3 capsule_position = make_vec3(0.0f, 1.0f, 0.0f);
    Vec3 entity_scale = make_vec3(0.1f, 0.1f, 0.1f);
    Normal_Entity *capsule_entity = (Normal_Entity *) allocate(entity_allocator, sizeof(Normal_Entity));
    int32 mesh_id;
    Mesh basic_cube = get_mesh_by_name(asset_manager, make_string("cube"), &mesh_id);
    Transform transform = make_transform(capsule_position, make_quaternion(), entity_scale);
    *capsule_entity = make_normal_entity(transform,
                                         mesh_id,
                                         -1, transform_aabb(basic_cube.aabb, transform));
    capsule_entity->collider.type = Collider_Type::CAPSULE;
    capsule_entity->collider.capsule = make_capsule_collider(capsule_position, make_vec3(0.0f, Player_Constants::player_height, 0.0f), Player_Constants::capsule_radius);
    
    add_entity(level, (Entity *) capsule_entity);
#endif
}

// TODO: don't think we need this?
#if 0
// gather entities by type
void gather_entities_by_type(Allocator *allocator,
                             Editor_Level *level,
                             Linked_List<Normal_Entity *> *normal_entities,
                             Linked_List<Point_Light_Entity *> *point_light_entities) {
    make_and_init_linked_list(Point_Light_Entity *, point_light_entities, allocator);
    make_and_init_linked_list(Normal_Entity *, normal_entities, allocator);

    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        switch (entity->type) {
            case ENTITY_NORMAL: {
                add(normal_entities, (Normal_Entity *) entity);
            } break;
            case ENTITY_POINT_LIGHT: {
                add(point_light_entities, (Point_Light_Entity *) entity);
            } break;
            default: {
                assert(!"Unhandled entity type.");
            }
        }
    }

}
#endif

