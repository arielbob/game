#include "entity.h"

bool32 has_mesh_field(Entity *entity) {
    return (entity->type == ENTITY_NORMAL);
}

bool32 has_material_field(Entity *entity) {
    return (entity->type == ENTITY_NORMAL);
}

void update_entity_aabb(Asset_Manager *asset_manager, Entity *entity) {
    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *normal_entity = (Normal_Entity *) entity;
            Mesh *mesh = get_mesh_pointer(asset_manager, normal_entity->mesh_id);
            normal_entity->transformed_aabb = transform_aabb(mesh->aabb, get_model_matrix(entity->transform));
        } break;
        case ENTITY_POINT_LIGHT: {
            // no aabb
        } break;
        default: {
            assert(!"Unhandled entity type with mesh and AABB");
        }
    }
}

void set_entity_transform(Asset_Manager *asset_manager, Entity *entity, Transform transform) {
    entity->transform = transform;
    update_entity_aabb(asset_manager, entity);
}


// TODO: we probably don't always need to update the AABB in some cases; well, idk, there might be uses for AABBs
//       outside of the editor, but that's the only place we're using them right now. although, it is convenient
//       that as long as we use these procedures when transforming entities, the entities will always have an
//       up to date AABB.
void update_entity_position(Asset_Manager *asset_manager, Entity *entity, Vec3 new_position) {
    entity->transform.position = new_position;

    update_entity_aabb(asset_manager, entity);

    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;
        Collider_Variant *collider = &normal_entity->collider;
        switch (collider->type) {
            case Collider_Type::NONE: break;
            case Collider_Type::CIRCLE: {
                collider->circle.center = new_position;
            } break;
            default: {
                assert(!"Unhandled collider type.");
            } break;
        }
    }
}

void update_entity_rotation(Asset_Manager *asset_manager, Entity *entity, Quaternion new_rotation) {
    entity->transform.rotation = new_rotation;
    update_entity_aabb(asset_manager, entity);

    // TODO: modify colliders when rotating
}

void update_entity_scale(Asset_Manager *asset_manager, Entity *entity, Vec3 new_scale) {
    entity->transform.scale = new_scale;
    update_entity_aabb(asset_manager, entity);

    // TODO: modify colliders when scaling
#if 0
    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;
        Collider_Variant *collider = &normal_entity->collider;
        switch (collider->type) {
            case Collider_Type::NONE: break;
            case Collider_Type::CIRCLE: {
                collider->circle.center = new_position;
            } break;
            default: {
                assert(!"Unhandled collider type.");
            } break;
        }
    }
#endif
}

void set_material(Entity *entity, int32 material_id) {
    assert(has_material_field(entity));

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) entity;
            e->material_id = material_id;
        } break;
        default: {
            assert(!"Unhandled entity with material field.");
        }
    }
}

void set_mesh(Asset_Manager *asset_manager, Entity *entity, int32 mesh_id, int32 *original_mesh_id = NULL) {
    assert(has_mesh_field(entity));

    Mesh *mesh = get_mesh_pointer(asset_manager, mesh_id);

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) entity;
            if (original_mesh_id) {
                *original_mesh_id = e->mesh_id;
            }
            e->mesh_id = mesh_id;
            e->transformed_aabb = transform_aabb(mesh->aabb, e->transform);
        } break;
        default: {
            assert(!"Unhandled entity with mesh field.");
        }
    }
}

int32 get_mesh_id(Entity *entity) {
    assert(has_mesh_field(entity));

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) entity;
            return e->mesh_id;
        } break;
        default: {
            assert(!"Unhandled entity with mesh field.");
        }
    }

    return -1;
}
