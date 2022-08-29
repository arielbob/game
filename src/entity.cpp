#include "entity.h"

bool32 has_mesh_field(Entity *entity) {
    return (entity->type == ENTITY_NORMAL);
}

bool32 has_material_field(Entity *entity) {
    return (entity->type == ENTITY_NORMAL);
}

void update_entity_aabb(Entity *entity) {
    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *normal_entity = (Normal_Entity *) entity;
            Mesh *mesh = get_mesh(normal_entity->mesh_name);
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

void set_entity_transform(Entity *entity, Transform transform) {
    entity->transform = transform;
    update_entity_aabb(entity);
}


// TODO: we probably don't always need to update the AABB in some cases; well, idk, there might be uses for AABBs
//       outside of the editor, but that's the only place we're using them right now. although, it is convenient
//       that as long as we use these procedures when transforming entities, the entities will always have an
//       up to date AABB.
void update_entity_position(Entity *entity, Vec3 new_position) {
    entity->transform.position = new_position;

    update_entity_aabb(entity);

    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;
        Collider_Variant *collider = &normal_entity->collider;
        switch (collider->type) {
            case Collider_Type::NONE: break;
            case Collider_Type::CIRCLE: {
                collider->circle.center = new_position;
            } break;
            case Collider_Type::CAPSULE: {
                Vec3 original_base = collider->capsule.capsule.base;
                collider->capsule.capsule.base = new_position;
                Vec3 diff = collider->capsule.capsule.base - original_base;
                collider->capsule.capsule.tip += diff;
            } break;
            default: {
                assert(!"Unhandled collider type.");
            } break;
        }
    }
}

void update_entity_rotation(Entity *entity, Quaternion new_rotation) {
    entity->transform.rotation = new_rotation;
    update_entity_aabb(entity);

    // TODO: modify colliders when rotating
}

void update_entity_scale(Entity *entity, Vec3 new_scale) {
    entity->transform.scale = new_scale;
    update_entity_aabb(entity);

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

void set_material(Entity *entity, String name) {
    assert(has_material_field(entity));

    Material *material = get_material(name);
    assert(material);
    
    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) entity;
            e->material_name = material->name;
        } break;
        default: {
            assert(!"Unhandled entity with material field.");
        }
    }
}

void set_mesh(Entity *entity, String name) {
    assert(has_mesh_field(entity));

    Mesh *mesh = get_mesh(name);
    assert(mesh);

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) entity;
            e->mesh_name = mesh->name;
            e->transformed_aabb = transform_aabb(mesh->aabb, e->transform);
        } break;
        default: {
            assert(!"Unhandled entity with mesh field.");
        }
    }
}
