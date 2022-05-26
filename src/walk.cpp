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
            bool32 found_triangle = get_walkable_triangle_on_mesh(player_collider.center, player_collider.radius,
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
