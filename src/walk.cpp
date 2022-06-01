// gets the highest point within the cylinder of some radius around some center from min_y to max_y
bool32 get_walkable_triangle_on_mesh(Vec3 center, real32 radius,
                                     Mesh *mesh, Transform transform,
                                     real32 min_y, real32 max_y,
                                     Get_Walkable_Triangle_On_Mesh_Result *result) {
    Mat4 object_to_world = get_model_matrix(transform);

    uint32 *indices = mesh->indices;

    bool32 found = false;
    Vec3 highest_point_in_range = make_vec3();
    real32 found_point_height = min_y;
    int32 triangle_index = -1;
    Vec3 found_triangle_normal = make_vec3();
    for (int32 i = 0; i < (int32) mesh->num_triangles; i++) {
        Vec3 triangle[3];
        get_triangle(mesh, i, triangle);
        transform_triangle(triangle, &object_to_world);
        Vec3 triangle_normal = get_triangle_normal(triangle);
        if (fabs(triangle_normal.y) < EPSILON) continue;

        // TODO: we could do a check for slope angle here instead of just checking for < 0.0f.
        // NOTE: we check for < 0, since we only want to be standing on triangles if they're pointing up and if
        //       y is negative, then it's pointing downwards and we only want to step on it if the mesh is
        //       double sided.
        if (!mesh->is_double_sided && triangle_normal.y < 0.0f) continue;

        if (circle_intersects_triangle_on_xz_plane(center, radius, triangle, triangle_normal)) {
            Vec3 point_on_triangle_plane = get_point_on_plane_from_xz(center.x, center.z,
                                                                      triangle_normal, triangle[0]);

            real32 point_y = point_on_triangle_plane.y;
            if ((point_y > found_point_height) && (point_y > min_y) && (point_y < max_y)) {
                found_point_height = point_y;
                highest_point_in_range = point_on_triangle_plane;
                triangle_index = i;
                found = true;
                found_triangle_normal = triangle_normal;
            }
        }
    }

    if (found) {
        result->point = highest_point_in_range;
        result->triangle_index = triangle_index;
        result->triangle_normal = found_triangle_normal;
    }

    return found;
}

bool32 get_new_walk_state(Game_Level *level, Asset_Manager *asset_manager,
                          Walk_State current_walk_state, Vec3 player_position,
                          Walk_State *walk_state_result, Vec3 *grounded_position) {
    Circle_Collider player_collider = make_circle_collider(player_position, Player_Constants::walk_radius);
    real32 max_lower_offset = Player_Constants::max_lower_ground_offset;
    real32 max_upper_offset = Player_Constants::max_upper_ground_offset;

    Vec3 highest_point = make_vec3(0.0f, player_collider.center.y - max_lower_offset, 0.0f);
    int32 triangle_index = -1;
    Vec3 triangle_normal = make_vec3();
    bool32 found_walkable_point = false;
    Entity_Type ground_entity_type = ENTITY_NONE;
    int32 ground_entity_id = -1;

    FOR_LIST_NODES(Entity *, level->entities) {
        if (current_node->value->type != ENTITY_NORMAL) continue;

        Normal_Entity *entity = (Normal_Entity *) current_node->value;
        if (entity->is_walkable) {
            Mesh *mesh = get_mesh_pointer(asset_manager, entity->mesh_id);
            Get_Walkable_Triangle_On_Mesh_Result result;

            real32 radius = ((entity->id == current_walk_state.ground_entity_id) ?
                             Player_Constants::small_walk_radius : Player_Constants::walk_radius);
            bool32 found_triangle = get_walkable_triangle_on_mesh(player_collider.center, radius,
                                                                  mesh,
                                                                  entity->transform,
                                                                  player_collider.center.y - max_lower_offset,
                                                                  player_collider.center.y + max_upper_offset,
                                                                  &result);
            if (found_triangle && (result.point.y > highest_point.y)) {
                highest_point = result.point;
                triangle_index = result.triangle_index;
                triangle_normal = result.triangle_normal;

                ground_entity_id = entity->id;
                
                found_walkable_point = true;
            }
        }
    }

    if (found_walkable_point) {
        walk_state_result->triangle_normal = triangle_normal;
        walk_state_result->triangle_index = triangle_index;
        walk_state_result->ground_entity_id = ground_entity_id;
        *grounded_position = highest_point;

#if 0
        add_debug_line(&game_state->debug_state, player_position, highest_point,
                       make_vec4(1.0f, 1.0f, 0.0f, 1.0f));
#endif
    }

    return found_walkable_point;
}
