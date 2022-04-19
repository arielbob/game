#include "hash_table.h"
#include "entity.h"
#include "game.h"
#include "level.h"

Material get_material(Level *level, int32 material_id) {
    Material material;
    bool32 material_exists = hash_table_find(level->material_table,
                                             material_id,
                                             &material);
    assert(material_exists);
    return material;
}

Material *get_material_pointer(Level *level, int32 material_id) {
    Material *material;
    bool32 material_exists = hash_table_find_pointer(level->material_table,
                                                     material_id,
                                                     &material);
    assert(material_exists);
    return material;
}

Mesh get_mesh(Level *level, int32 mesh_id) {
    Mesh mesh;
    bool32 mesh_exists = hash_table_find(level->mesh_table,
                                         mesh_id,
                                         &mesh);
    assert(mesh_exists);
    return mesh;
}

Mesh *get_mesh_pointer(Level *level, int32 mesh_id) {
    Mesh *mesh;
    bool32 mesh_exists = hash_table_find_pointer(level->mesh_table,
                                                 mesh_id,
                                                 &mesh);
    assert(mesh_exists);
    return mesh;
}

Texture get_texture(Level *level, int32 texture_id) {
    assert(texture_id >= 0);
    Texture texture;
    bool32 texture_exists = hash_table_find(level->texture_table,
                                            texture_id,
                                            &texture);
    assert(texture_exists);
    return texture;
}

void append_string_add_quotes(String_Buffer *buffer, String_Buffer string) {
    append_string(buffer, "\"");
    append_string(buffer, string);
    append_string(buffer, "\"");
}

void append_string_add_quotes(String_Buffer *buffer, char *string) {
    append_string(buffer, "\"");
    append_string(buffer, string);
    append_string(buffer, "\"");
}

void append_default_entity_info(Level *level, String_Buffer *buffer, Entity *entity) {
    append_string(buffer, "mesh ");
    Mesh mesh = get_mesh(level, entity->mesh_id);
    append_string_add_quotes(buffer, mesh.name);
    append_string(buffer, "\n");

    append_string(buffer, "material ");
    Material material = get_material(level, entity->material_id);
    append_string_add_quotes(buffer, material.name);
    append_string(buffer, "\n");

    int32 temp_buffer_size = 128;

    Marker m = begin_region();

    Transform transform = entity->transform;

    append_string(buffer, "position ");
    char *position_string = (char *) region_push(temp_buffer_size);
    string_format(position_string, temp_buffer_size, "%f %f %f",
                  transform.position.x, transform.position.y, transform.position.z);
    append_string(buffer, position_string);
    append_string(buffer, "\n");

    append_string(buffer, "rotation ");
    char *rotation_string = (char *) region_push(temp_buffer_size);
    string_format(rotation_string, temp_buffer_size, "%f %f %f %f",
                  transform.rotation.w, transform.rotation.v.x, transform.rotation.v.y, transform.rotation.v.z);
    append_string(buffer, rotation_string);
    append_string(buffer, "\n");

    append_string(buffer, "scale ");
    char *scale_string = (char *) region_push(temp_buffer_size);
    string_format(scale_string, temp_buffer_size, "%f %f %f",
                  transform.scale.x, transform.scale.y, transform.scale.z);
    append_string(buffer, scale_string);
    append_string(buffer, "\n");

    end_region(m);
}

void export_level(Allocator *allocator, Level *level, char *filename) {
    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(allocator, buffer_size);

    append_string(&working_buffer, "level info {\n");
    append_string(&working_buffer, "level_name ");
    append_string_add_quotes(&working_buffer, level->name);
    append_string(&working_buffer, "\n");
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "meshes {\n");
    Hash_Table<int32, Mesh> mesh_table = level->mesh_table;
    for (int32 i = 0; i < mesh_table.max_entries; i++) {
        Hash_Table_Entry<int32, Mesh> entry = mesh_table.entries[i];
        if (entry.is_occupied) {
            append_string(&working_buffer, "mesh ");
            append_string_add_quotes(&working_buffer, entry.value.name);
            append_string(&working_buffer, " ");
            append_string_add_quotes(&working_buffer, entry.value.filename);
            append_string(&working_buffer, "\n");
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "textures {\n");
    Hash_Table<int32, Texture> texture_table = level->texture_table;
    for (int32 i = 0; i < texture_table.max_entries; i++) {
        Hash_Table_Entry<int32, Texture> entry = texture_table.entries[i];
        if (entry.is_occupied) {
            append_string(&working_buffer, "texture ");
            append_string_add_quotes(&working_buffer, entry.value.name);
            append_string(&working_buffer, " ");
            append_string_add_quotes(&working_buffer, entry.value.filename);
            append_string(&working_buffer, "\n");
        }
    }
    append_string(&working_buffer, "}\n\n");
    
    int32 temp_buffer_size = 128;

    append_string(&working_buffer, "materials {\n");
    Hash_Table<int32, Material> material_table = level->material_table;
    int32 num_materials_added = 0;
    for (int32 i = 0; i < material_table.max_entries; i++) {
        Hash_Table_Entry<int32, Material> entry = material_table.entries[i];
        if (entry.is_occupied) {
            Material material = entry.value;

            append_string(&working_buffer, "material ");
            append_string_add_quotes(&working_buffer, material.name);
            append_string(&working_buffer, "\n");
            
            if (material.texture_id >= 0) {
                append_string(&working_buffer, "texture ");
                Texture material_texture = get_texture(level, material.texture_id);
                append_string_add_quotes(&working_buffer, material_texture.name);
                append_string(&working_buffer, "\n");
            }

            Marker m = begin_region();

            append_string(&working_buffer, "gloss ");
            char *gloss_string = (char *) region_push(temp_buffer_size);
            string_format(gloss_string, temp_buffer_size, "%f", material.gloss);
            append_string(&working_buffer, gloss_string);
            append_string(&working_buffer, "\n");

            append_string(&working_buffer, "color_override ");
            char *color_override_string = (char *) region_push(temp_buffer_size);
            string_format(color_override_string, temp_buffer_size, "%f %f %f",
                          material.color_override.x, material.color_override.y, material.color_override.z);
            append_string(&working_buffer, color_override_string);
            append_string(&working_buffer, "\n");

            append_string(&working_buffer, "use_color_override ");
            append_string(&working_buffer, material.use_color_override ? "1" : "0");
            append_string(&working_buffer, "\n");

            end_region(m);

            num_materials_added++;

            if (num_materials_added < material_table.num_entries) append_string(&working_buffer, "\n");
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "entities {\n");

    // NORMAL ENTITIES
    for (int32 i = 0; i < level->num_normal_entities; i++) {
        Normal_Entity *entity = &level->normal_entities[i];

        append_string(&working_buffer, "type normal\n");
        append_default_entity_info(level, &working_buffer, (Entity *) entity);

        if (i < level->num_normal_entities - 1) append_string(&working_buffer, "\n");
    }

    if (level->num_point_lights > 0) append_string(&working_buffer, "\n");

    // POINT LIGHT ENTITIES
    for (int32 i = 0; i < level->num_point_lights; i++) {
        Point_Light_Entity *entity = &level->point_lights[i];

        append_string(&working_buffer, "type point_light\n");
        append_default_entity_info(level, &working_buffer, (Entity *) entity);

        Marker m = begin_region();

        append_string(&working_buffer, "light_color ");
        char *light_color_string = (char *) region_push(temp_buffer_size);
        string_format(light_color_string, temp_buffer_size, "%f %f %f",
                      entity->light_color.x, entity->light_color.y, entity->light_color.z);
        append_string(&working_buffer, light_color_string);
        append_string(&working_buffer, "\n");

        append_string(&working_buffer, "falloff_start ");
        char *falloff_start_string = (char *) region_push(temp_buffer_size);
        string_format(falloff_start_string, temp_buffer_size, "%f",
                      entity->falloff_start);
        append_string(&working_buffer, falloff_start_string);
        append_string(&working_buffer, "\n");

        append_string(&working_buffer, "falloff_end ");
        char *falloff_end_string = (char *) region_push(temp_buffer_size);
        string_format(falloff_end_string, temp_buffer_size, "%f",
                      entity->falloff_end);
        append_string(&working_buffer, falloff_end_string);
        append_string(&working_buffer, "\n");

        end_region(m);

        if (i < level->num_point_lights - 1) append_string(&working_buffer, "\n");
    }
    append_string(&working_buffer, "}\n");


    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);
}

/*
  call reset_level
  all the meshes and textures get marked as should_unload
  openGL unloads all the meshes that are marked as should_unload
  set should_clean_and_load_level to true

  next frame
  if should_clean_and_load_level:
  clear the mesh and level arenas
  reset the hash tables
 */

inline bool32 mesh_exists(Level *level, int32 mesh_id) {
    return hash_table_exists(level->mesh_table, mesh_id);
}

inline bool32 material_exists(Level *level, int32 material_id) {
    return hash_table_exists(level->material_table, material_id);
}

inline bool32 texture_exists(Level *level, int32 texture_id) {
    return hash_table_exists(level->texture_table, texture_id);
}

int32 level_add_mesh(Level *level, Mesh mesh) {
    int32 mesh_id = level->mesh_table.total_added_ever;
    hash_table_add(&level->mesh_table, mesh_id, mesh);
    return mesh_id;
}

void level_add_entity(Level *level, Normal_Entity entity) {
    assert(level->num_normal_entities < MAX_ENTITIES);
    assert(mesh_exists(level, entity.mesh_id));
    assert(material_exists(level, entity.material_id));
    level->normal_entities[level->num_normal_entities++] = entity;
}

void level_add_point_light_entity(Level *level, Point_Light_Entity entity) {
    assert(mesh_exists(level, entity.mesh_id));
    assert(material_exists(level, entity.material_id));
    assert(level->num_point_lights < MAX_POINT_LIGHTS);
    level->point_lights[level->num_point_lights++] = entity;
}

int32 level_add_material(Level *level, Material material) {
    if (material.texture_id >= 0) {
        assert(texture_exists(level, material.texture_id));
    }
    int32 material_id = level->material_table.total_added_ever;
    hash_table_add(&level->material_table, material_id, material);
    return material_id;
}

int32 level_add_texture(Level *level, Texture texture) {
    int32 texture_id = level->texture_table.total_added_ever;
    hash_table_add(&level->texture_table, texture_id, texture);
    return texture_id;
}

// NOTE: should only be called once, or at least make sure you deallocate everything before
void load_default_level(Level *level) {
    // init tables
    level->mesh_table = make_hash_table<int32, Mesh>((Allocator *) &memory.hash_table_stack,
                                                     HASH_TABLE_SIZE,
                                                     &int32_equals);
    level->material_table = make_hash_table<int32, Material>((Allocator *) &memory.hash_table_stack,
                                                             HASH_TABLE_SIZE,
                                                             &int32_equals);
    level->texture_table = make_hash_table<int32, Texture>((Allocator *) &memory.hash_table_stack,
                                                           HASH_TABLE_SIZE,
                                                           &int32_equals);

    // add level meshes
    Allocator *mesh_allocator = (Allocator *) &level->mesh_arena;
    Allocator *filename_allocator = (Allocator *) &level->filename_pool;
    Allocator *mesh_name_allocator = (Allocator *) &level->string64_pool;
    Mesh mesh;
    mesh = read_and_load_mesh(mesh_allocator,
                              make_string_buffer(filename_allocator, "blender/cube.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "cube", MESH_NAME_MAX_SIZE));
    int32 cube_mesh_id = level_add_mesh(level, mesh);

    mesh = read_and_load_mesh(mesh_allocator,
                              make_string_buffer(filename_allocator, "blender/suzanne2.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "suzanne", MESH_NAME_MAX_SIZE));
    int32 suzanne_mesh_id = level_add_mesh(level, mesh);

    mesh = read_and_load_mesh(mesh_allocator,
                              make_string_buffer(filename_allocator, "blender/sphere.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "sphere", MESH_NAME_MAX_SIZE));
    int32 sphere_mesh_id = level_add_mesh(level, mesh);

    // add level textures
    Allocator *string64_allocator = (Allocator *) &level->string64_pool;
    Texture texture;
    texture = make_texture(make_string_buffer(string64_allocator, "debug", 64),
                           make_string_buffer(filename_allocator, "src/textures/debug_texture.png", MAX_PATH));
    int32 debug_texture_id = level_add_texture(level, texture);
    texture = make_texture(make_string_buffer(string64_allocator, "white", 64),
                           make_string_buffer(filename_allocator, "src/textures/white.bmp", MAX_PATH));
    int32 white_texture_id = level_add_texture(level, texture);

    // add level materials
    Material shiny_monkey = make_material(make_string_buffer(string64_allocator,
                                                             "shiny_monkey", MATERIAL_STRING_MAX_SIZE),
                                          debug_texture_id,
                                          100.0f, make_vec4(0.6f, 0.6f, 0.6f, 1.0f), true);
    int32 shiny_monkey_material_id = level_add_material(level, shiny_monkey);

    Material plane_material = make_material(make_string_buffer(string64_allocator,
                                                               "diffuse_plane", MATERIAL_STRING_MAX_SIZE),
                                            -1, 1.0f, make_vec4(0.9f, 0.9f, 0.9f, 1.0f), true);
    int32 plane_material_id = level_add_material(level, plane_material);

    Material arrow_material = make_material(make_string_buffer(string64_allocator,
                                                               "arrow_material", MATERIAL_STRING_MAX_SIZE),
                                            -1, 100.0f, make_vec4(1.0f, 0.0f, 0.0f, 1.0f), true);
    int32 arrow_material_id = level_add_material(level, arrow_material);

    Material white_light_material = make_material(make_string_buffer(string64_allocator,
                                                                     "white_light", MATERIAL_STRING_MAX_SIZE),
                                                  -1, 0.0f, make_vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
    int32 white_light_material_id = level_add_material(level, white_light_material);

    Material blue_light_material = make_material(make_string_buffer(string64_allocator,
                                                                    "blue_light", MATERIAL_STRING_MAX_SIZE),
                                                 -1, 0.0f, make_vec4(0.0f, 0.0f, 1.0f, 1.0f), true);
    int32 blue_light_material_id = level_add_material(level, blue_light_material);

    Material diffuse_sphere_material = make_material(make_string_buffer(string64_allocator,
                                                                        "diffuse_sphere", MATERIAL_STRING_MAX_SIZE),
                                                     -1, 5.0f, rgb_to_vec4(176, 176, 176), true);
    int32 diffuse_sphere_material_id = level_add_material(level, diffuse_sphere_material);

    // add level entities
    Transform transform = {};
    Normal_Entity entity;

    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(2.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    entity = make_entity(suzanne_mesh_id, shiny_monkey_material_id, transform);
    level_add_entity(level, entity);

    transform = {};
    transform.scale = make_vec3(5.0f, 0.1f, 5.0f);
    transform.position = make_vec3(0.0f, 0.0f, 0.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(cube_mesh_id, plane_material_id, transform);
    level_add_entity(level, entity);

#if 0
    transform = {};
    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(0.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    entity = make_entity(gizmo_arrow_mesh_id, arrow_material_id, transform);
    level_add_entity(level, entity);
#endif

    transform = {};
    transform.scale = make_vec3(0.5f, 0.5f, 0.5f);
    transform.position = make_vec3(-1.5f, 1.5f, -1.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(sphere_mesh_id, diffuse_sphere_material_id, transform);
    level_add_entity(level, entity);

    Vec3 light_color;
    Point_Light_Entity point_light_entity;

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(0.8f, 1.8f, -2.3f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.8f, 0.8f, 0.8f);
    point_light_entity = make_point_light_entity(cube_mesh_id, white_light_material_id,
                                                 light_color,
                                                 0.0f, 3.0f,
                                                 transform);
    level_add_point_light_entity(level, point_light_entity);

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(-1.0f, 1.5f, 0.0f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.0f, 0.0f, 1.0f);
    point_light_entity = make_point_light_entity(cube_mesh_id, blue_light_material_id,
                                                 light_color,
                                                 0.0f, 5.0f,
                                                 transform);
    level_add_point_light_entity(level, point_light_entity);
}

void unload_level(Game_State *game_state) {
    Level *level = &game_state->current_level;
    game_state->should_clear_level_gpu_data = true;

    level->num_normal_entities = 0;
    level->num_point_lights = 0;
    
    clear_arena(&level->mesh_arena);
    clear_pool(&level->string64_pool);
    clear_pool(&level->filename_pool);
    hash_table_reset(&level->mesh_table);
    hash_table_reset(&level->material_table);
    hash_table_reset(&level->texture_table);
}

void load_level(Level *level) {

}

void read_and_load_level(Level *level, char *filename) {
    Marker m = begin_region();

    Allocator *global_stack = (Allocator *) &memory.global_stack;
    File_Data level_file = platform_open_and_read_file(global_stack, filename);

    load_level(level);

    end_region(m);
}
