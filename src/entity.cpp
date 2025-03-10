#include "entity.h"
#include "level.h"

Entity make_entity_from_info(Allocator *allocator, Entity_Info *info) {
    Entity result = {};

    result.flags            = info->flags;
    result.transform        = info->transform;

    if (info->flags & ENTITY_MESH) {
        Mesh *mesh = get_mesh(info->mesh_name);
        result.mesh_id = mesh->id;
    }

    if (info->flags & ENTITY_MATERIAL) {
        Material *material = get_material(info->material_name);
        result.material_id = material->id;
    }

    if (info->has_animation) {
        assert(info->flags & ENTITY_MESH);
        // TODO: we may want to check if the animation is compatible..
        Skeletal_Animation *animation = get_animation(info->animation_name);
        assert(animation);
        result.animation_id = animation->id;
    }
    
    result.transformed_aabb = info->transformed_aabb;

    result.collider         = info->collider;

    result.light_type            = info->light_type;
    result.light_color           = info->light_color;
    result.point_light_intensity = info->point_light_intensity;
    result.falloff_start         = info->falloff_start;
    result.falloff_end           = info->falloff_end;
    result.sun_color             = info->sun_color;
    result.sun_intensity         = info->sun_intensity;

    return result;
}

void update_entity_aabb(Entity *entity) {
    // TODO: not sure if all entities with AABBs necessarily need a mesh
    if (entity->flags & ENTITY_MESH) {
        Mesh *mesh = get_mesh(entity->mesh_id);
        assert(mesh);
        entity->transformed_aabb = transform_aabb(mesh->aabb, get_model_matrix(entity->transform));
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

    if (entity->flags & ENTITY_COLLIDER) {
        Collider_Variant *collider = &entity->collider;
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

void set_material(Entity *entity, int32 material_id) {
    assert(entity->flags & ENTITY_MATERIAL);

    // i think it's fine to not assert finding the material so that we don't need to
    // look for it. we'll assert later on anyways since we eventually call get_material(id)
    entity->material_id = material_id;
}

void set_material(Entity *entity, String name) {
    assert(entity->flags & ENTITY_MATERIAL);

    Material *material = get_material(name);
    assert(material);

    entity->material_id = material->id;
}

void set_material(Entity *entity, char *name) {
    Allocator *temp_region = begin_region();

    String material_name = make_string(temp_region, name);
    set_material(entity, material_name);
    
    end_region(temp_region);
}

void set_mesh(Entity *entity, int32 id) {
    assert(entity->flags & ENTITY_MESH);

    Mesh *mesh = get_mesh(id);
    assert(mesh);

    entity->mesh_id = mesh->id;
    entity->transformed_aabb = transform_aabb(mesh->aabb, entity->transform);
}

void set_mesh(Entity *entity, String name) {
    assert(entity->flags & ENTITY_MESH);

    Mesh *mesh = get_mesh(name);
    assert(mesh);

    entity->mesh_id = mesh->id;
}

void set_mesh(Entity *entity, char *name) {
    Allocator *temp_region = begin_region();

    String mesh_name = make_string(temp_region, name);
    set_mesh(entity, mesh_name);
    
    end_region(temp_region);
}

void set_animation(Entity *entity, String name) {
    assert(entity->flags & ENTITY_MESH);

    Skeletal_Animation *animation = get_animation(name);
    assert(animation);

    entity->animation_id = animation->id;
}

void set_animation(Entity *entity, char *name) {
    if (!name) {
        entity->animation_id = -1; // no animation
        return;
    }

    Allocator *temp_region = begin_region();

    set_animation(entity, make_string(temp_region, name));
    
    end_region(temp_region);
}
