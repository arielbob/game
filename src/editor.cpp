#include "editor.h"

int32 ray_intersects_entity(Ray ray, Mesh *meshes, Entity entity, real32 *t_result) {
    Mesh mesh = meshes[entity.mesh_index];
    Mat4 object_to_world = get_model_matrix(entity.transform);
    Mat4 world_to_object = inverse(object_to_world);

    // instead of transforming every vertex, we just transform the world-space ray to object-space by
    // multiplying the origin and the direction of the ray by the inverse of the entity's model matrix.
    // note that we must zero-out the w component of the ray direction when doing this multiplication so that
    // we ignore the translation of the model matrix.
    Vec3 object_space_ray_origin = truncate_v4_to_v3(world_to_object * make_vec4(ray.origin, 1.0f));
    Vec3 object_space_ray_direction = normalize(truncate_v4_to_v3(world_to_object *
                                                                  make_vec4(ray.direction, 0.0f)));
    Ray object_space_ray = { object_space_ray_origin, object_space_ray_direction };

    uint32 *indices = mesh.indices;

    // this might be very slow
    real32 t_min = FLT_MAX;
    bool32 hit = false;
    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        triangle[0] = get_vertex_from_index(&mesh, indices[i * 3]);
        triangle[1] = get_vertex_from_index(&mesh, indices[i * 3 + 1]);
        triangle[2] = get_vertex_from_index(&mesh, indices[i * 3 + 2]);
        real32 t;

        // TODO: we might be able to pass t_min into this test to early-out before we check if a hit point
        //       is within the triangle, but after we've hit the plane
        if (ray_intersects_triangle(object_space_ray, triangle, &t)) {
            t_min = min(t, t_min);
            hit = true;
        }
    }

    if (hit) {
        *t_result = t_min;
    }

    return hit;
}

// TODO: optimize this by checking against AABB before checking against triangles
int32 pick_entity(Game_State *game_state, Ray cursor_ray) {
    Editor_State *editor_state = &game_state->editor_state;

    Mesh *meshes = game_state->meshes;
    Entity *entities = game_state->entities;

    int32 entity_index = -1;

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < game_state->num_entities; i++) {
        real32 t;
        Entity entity = entities[i];
        if (ray_intersects_entity(cursor_ray, meshes, entity, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
        }
    }

    return entity_index;
}
