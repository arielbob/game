#include "hash_table.h"
#include "entity.h"
#include "game.h"
#include "level.h"
#include "parse.h"

#if 0
Level make_level(String_Buffer name,
                 Allocator *hash_table_allocator,
                 Arena_Allocator mesh_arena,
                 Pool_Allocator string64_pool,
                 Pool_Allocator filename_pool) {
    Level level = {};
    level.name = name;
    level.mesh_arena = mesh_arena;
    level.string64_pool = string64_pool;
    level.filename_pool = filename_pool;

    level.mesh_table = make_hash_table<int32, Mesh>(hash_table_allocator,
                                                    HASH_TABLE_SIZE,
                                                    &int32_equals);
    level.material_table = make_hash_table<int32, Material>(hash_table_allocator,
                                                            HASH_TABLE_SIZE,
                                                            &int32_equals);
    level.texture_table = make_hash_table<int32, Texture>(hash_table_allocator,
                                                          HASH_TABLE_SIZE,
                                                          &int32_equals);

    return level;
}
#endif

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

#if 0
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
#endif

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

void append_default_entity_info(Game_State *game_state, Level *level, String_Buffer *buffer, Entity *entity) {
    if (entity->mesh_type == Mesh_Type::PRIMITIVE) {
        append_string(buffer, "mesh_primitive ");
    } else {
        append_string(buffer, "mesh ");
    }
    
    Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
    append_string_add_quotes(buffer, mesh.name);
    append_string(buffer, "\n");

    if (entity->material_id >= 0) {
        append_string(buffer, "material ");
        Material material = get_material(level, entity->material_id);
        append_string_add_quotes(buffer, material.name);
        append_string(buffer, "\n");
    }

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

void export_level(Allocator *allocator, Game_State *game_state, Level *level, char *filename) {
    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(allocator, buffer_size);

    append_string(&working_buffer, "level_info {\n");
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
        append_default_entity_info(game_state, level, &working_buffer, (Entity *) entity);

        if (i < level->num_normal_entities - 1) append_string(&working_buffer, "\n");
    }

    if (level->num_point_lights > 0) append_string(&working_buffer, "\n");

    // POINT LIGHT ENTITIES
    for (int32 i = 0; i < level->num_point_lights; i++) {
        Point_Light_Entity *entity = &level->point_lights[i];

        append_string(&working_buffer, "type point_light\n");
        append_default_entity_info(game_state, level, &working_buffer, (Entity *) entity);

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

// TODO: same TODO as get_mesh_id_by_name
int32 get_material_id_by_name(Level *level, String material_name) {
    int32 num_checked = 0;
    Hash_Table<int32, Material> material_table = level->material_table;
    for (int32 i = 0; (i < material_table.max_entries) && (num_checked < material_table.num_entries); i++) {
        Hash_Table_Entry<int32, Material> entry = material_table.entries[i];
        if (entry.is_occupied) {
            if (string_equals(make_string(entry.value.name), material_name)) {
                return entry.key;
            }
            num_checked++;
        }
    }

    return -1;
}

// TODO: same as above
int32 get_texture_id_by_name(Level *level, String texture_name) {
    int32 num_checked = 0;
    Hash_Table<int32, Texture> texture_table = level->texture_table;
    for (int32 i = 0; (i < texture_table.max_entries) && (num_checked < texture_table.num_entries); i++) {
        Hash_Table_Entry<int32, Texture> entry = texture_table.entries[i];
        if (entry.is_occupied) {
            if (string_equals(make_string(entry.value.name), texture_name)) {
                return entry.key;
            }
            num_checked++;
        }
    }

    return -1;
}

int32 level_add_entity(Level *level, Normal_Entity entity) {
    assert(level->num_normal_entities < MAX_ENTITIES);
    assert(mesh_exists(level, entity.mesh_id));
    //assert(material_exists(level, entity.material_id));
    int32 entity_index = level->num_normal_entities++;
    level->normal_entities[entity_index] = entity;
    return entity_index;
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
void load_default_level(Game_State *game_state, Level *level) {
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

    Allocator *mesh_allocator = (Allocator *) level->mesh_arena;
    Allocator *filename_allocator = (Allocator *) level->filename_pool;
    Allocator *mesh_name_allocator = (Allocator *) level->string64_pool;
    Allocator *string64_allocator = (Allocator *) level->string64_pool;

    // add level meshes
    Mesh mesh;
    mesh = read_and_load_mesh(mesh_allocator,
                              make_string_buffer(filename_allocator, "blender/suzanne2.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "suzanne", MESH_NAME_MAX_SIZE));
    int32 suzanne_mesh_id = level_add_mesh(level, mesh);
    AABB suzanne_mesh_aabb = mesh.aabb;
    
    mesh = read_and_load_mesh(mesh_allocator,
                              make_string_buffer(filename_allocator, "blender/sphere.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "sphere", MESH_NAME_MAX_SIZE));
    int32 sphere_mesh_id = level_add_mesh(level, mesh);
    AABB sphere_mesh_aabb = mesh.aabb;

    // add level textures
    Texture texture;
    texture = make_texture(make_string_buffer(string64_allocator, "debug", TEXTURE_NAME_MAX_SIZE),
                           make_string_buffer(filename_allocator, "src/textures/debug_texture.png", MAX_PATH));
    int32 debug_texture_id = level_add_texture(level, texture);
    texture = make_texture(make_string_buffer(string64_allocator, "white", TEXTURE_NAME_MAX_SIZE),
                           make_string_buffer(filename_allocator, "src/textures/white.bmp", MAX_PATH));
    int32 white_texture_id = level_add_texture(level, texture);

    // add level materials
    Material shiny_monkey = make_material(make_string_buffer(string64_allocator,
                                                             "shiny_monkey", MATERIAL_NAME_MAX_SIZE),
                                          debug_texture_id,
                                          100.0f, make_vec4(0.6f, 0.6f, 0.6f, 1.0f), true);
    int32 shiny_monkey_material_id = level_add_material(level, shiny_monkey);

    Material plane_material = make_material(make_string_buffer(string64_allocator,
                                                               "diffuse_plane", MATERIAL_NAME_MAX_SIZE),
                                            -1, 1.0f, make_vec4(0.9f, 0.9f, 0.9f, 1.0f), true);
    int32 plane_material_id = level_add_material(level, plane_material);

    Material arrow_material = make_material(make_string_buffer(string64_allocator,
                                                               "arrow_material", MATERIAL_NAME_MAX_SIZE),
                                            -1, 100.0f, make_vec4(1.0f, 0.0f, 0.0f, 1.0f), true);
    int32 arrow_material_id = level_add_material(level, arrow_material);

    Material white_light_material = make_material(make_string_buffer(string64_allocator,
                                                                     "white_light", MATERIAL_NAME_MAX_SIZE),
                                                  -1, 0.0f, make_vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
    int32 white_light_material_id = level_add_material(level, white_light_material);

    Material blue_light_material = make_material(make_string_buffer(string64_allocator,
                                                                    "blue_light", MATERIAL_NAME_MAX_SIZE),
                                                 -1, 0.0f, make_vec4(0.0f, 0.0f, 1.0f, 1.0f), true);
    int32 blue_light_material_id = level_add_material(level, blue_light_material);

    Material diffuse_sphere_material = make_material(make_string_buffer(string64_allocator,
                                                                        "diffuse_sphere", MATERIAL_NAME_MAX_SIZE),
                                                     -1, 5.0f, rgb_to_vec4(176, 176, 176), true);
    int32 diffuse_sphere_material_id = level_add_material(level, diffuse_sphere_material);

    // add level entities
    Transform transform = {};
    Normal_Entity entity;
    
    
    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(2.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    //entity = make_entity(suzanne_mesh_id, shiny_monkey_material_id, transform);
    int32 primitive_cube_mesh_id = get_mesh_id_by_name(game_state, level, Mesh_Type::PRIMITIVE, make_string("cube"));
    assert(primitive_cube_mesh_id >= 0);
    AABB primitive_cube_mesh_aabb = (get_mesh(game_state, level, Mesh_Type::PRIMITIVE, primitive_cube_mesh_id)).aabb;
    entity = make_entity(Mesh_Type::PRIMITIVE, primitive_cube_mesh_id, shiny_monkey_material_id,
                         transform, primitive_cube_mesh_aabb);
    level_add_entity(level, entity);

    transform = {};
    transform.scale = make_vec3(5.0f, 0.1f, 5.0f);
    transform.position = make_vec3(0.0f, 0.0f, 0.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(Mesh_Type::PRIMITIVE, primitive_cube_mesh_id, plane_material_id,
                         transform, primitive_cube_mesh_aabb, true);
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
    entity = make_entity(Mesh_Type::LEVEL, sphere_mesh_id, diffuse_sphere_material_id,
                         transform, sphere_mesh_aabb);
    level_add_entity(level, entity);

    Vec3 light_color;
    Point_Light_Entity point_light_entity;

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(0.8f, 1.8f, -2.3f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.8f, 0.8f, 0.8f);
    point_light_entity = make_point_light_entity(Mesh_Type::PRIMITIVE, primitive_cube_mesh_id,
                                                 white_light_material_id,
                                                 light_color,
                                                 0.0f, 3.0f,
                                                 transform, primitive_cube_mesh_aabb);
    level_add_point_light_entity(level, point_light_entity);

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(-1.0f, 1.5f, 0.0f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.0f, 0.0f, 1.0f);
    point_light_entity = make_point_light_entity(Mesh_Type::PRIMITIVE, primitive_cube_mesh_id,
                                                 blue_light_material_id,
                                                 light_color,
                                                 0.0f, 5.0f,
                                                 transform, primitive_cube_mesh_aabb);
    level_add_point_light_entity(level, point_light_entity);
}

void unload_level(Level *level) {
    level->should_clear_gpu_data = true;

    level->num_normal_entities = 0;
    level->num_point_lights = 0;
    
    clear_arena(level->mesh_arena);
    clear_pool(level->string64_pool);
    clear_pool(level->filename_pool);
    hash_table_reset(&level->mesh_table);
    hash_table_reset(&level->material_table);
    hash_table_reset(&level->texture_table);
}

void new_level(Level *current_level) {
    unload_level(current_level);
    current_level->name = make_string_buffer((Allocator *) current_level->string64_pool, LEVEL_NAME_MAX_SIZE);
}

inline Level_Loader::Token Level_Loader::make_token(Token_Type type, char *contents, int32 length) {
    Token token = {
        type,
        make_string(contents, length)
    };
    return token;
}

Level_Loader::Token Level_Loader::get_token(Tokenizer *tokenizer, char *file_contents) {
    Token token = {};

    consume_leading_whitespace(tokenizer);

    if (is_end(tokenizer)) {
        token = make_token(END, NULL, 0);
        return token;
    }

    char c = *tokenizer->current;

    // it's fine to not use tokenizer_equals here since we've already checked for is_end and we're only
    // comparing against single characters
    if (is_digit(c) || (c == '-') || (c == '.')) {
        uint32 start = tokenizer->index;
        
        bool32 has_period = false;
        bool32 is_negative = false;
        if (c == '.') {
            has_period = true;
        } else if (c == '-') {
            is_negative = true;
        }

        increment_tokenizer(tokenizer);
        
        while (!is_end(tokenizer) && !is_whitespace(tokenizer)) {
            if (!is_digit(*tokenizer->current) &&
                *tokenizer->current != '.') {
                assert(!"Expected digit or period");
            }
            
            if (*tokenizer->current == '.') {
                if (!has_period) {
                    has_period = true;
                } else {
                    assert(!"More than one period in number");
                }
            }

            if (*tokenizer->current == '-') {
                assert(!"Negatives can only be at the start of a number");
            }

            increment_tokenizer(tokenizer);
        }
        
        uint32 length = tokenizer->index - start;

        Token_Type type;
        if (has_period) {
            type = REAL;
        } else {
            type = INTEGER;
        }

        token = make_token(type, &file_contents[start], length);
    } else if (tokenizer_equals(tokenizer, ";;")) {
        increment_tokenizer(tokenizer, 2);
        int32 start = tokenizer->index;
        
        // it is necessary that we check both is_end and is_line_end, since is_line_end will return false
        // if we hit the end, and so we'll be stuck in an infinite loop.
        while (!is_end(tokenizer) &&
               !is_line_end(tokenizer)) {
            increment_tokenizer(tokenizer);
        }

        int32 length = tokenizer->index - start;
        token = make_token(COMMENT, &file_contents[start], length);
    } else if (is_letter(*tokenizer->current)) {
        uint32 start = tokenizer->index;

        increment_tokenizer(tokenizer);
        
        while (!is_end(tokenizer) &&
               !is_whitespace(*tokenizer->current)) {

            char current_char = *tokenizer->current;
            if (!(is_letter(current_char) || current_char == '_')) {
                assert(!"Keywords can only contain letters and underscores.");
            }

            increment_tokenizer(tokenizer);
        }

        int32 length = tokenizer->index - start;
        token = make_token(KEYWORD, &file_contents[start], length);
    } else if (*tokenizer->current == '"') {
        increment_tokenizer(tokenizer);
        // set start after we increment tokenizer so that we don't include the quote in the token string.
        int32 start = tokenizer->index;

        while (!is_end(tokenizer) &&
               !(*tokenizer->current == '"')) {
            increment_tokenizer(tokenizer);
        }

        if (*tokenizer->current != '"') {
            assert(!"Expected a closing quote.");
        } else {
            int32 length = tokenizer->index - start;
            increment_tokenizer(tokenizer);
            token = make_token(STRING, &file_contents[start], length);
        }
    } else if (*tokenizer->current == '{') {
        int32 start = tokenizer->index;
        increment_tokenizer(tokenizer);
        int32 length = tokenizer->index - start;
        token = make_token(OPEN_BRACKET, &file_contents[start], length);
    } else if (*tokenizer->current == '}') {
        int32 start = tokenizer->index;
        increment_tokenizer(tokenizer);
        int32 length = tokenizer->index - start;
        token = make_token(CLOSE_BRACKET, &file_contents[start], length);
    } else {
        assert(!"Token type not recognized.");
    }

    return token;
}

// loads barebones level data into level
bool32 Level_Loader::load_temp_level(Allocator *temp_allocator, Game_State *game_state,
                                     File_Data file_data, Level *temp_level) {
    Tokenizer tokenizer = make_tokenizer(file_data);

    Token token;
    Parser_State state = WAIT_FOR_LEVEL_INFO_BLOCK_NAME;

    Mesh temp_mesh = {};
    Texture temp_texture = {};
    Material temp_material = {};
    bool32 should_add_new_temp_material = false;

    Entity_Type temp_entity_type = ENTITY_NONE;
    Normal_Entity temp_normal_entity = {};
    Point_Light_Entity temp_point_light_entity = {};
    Entity *temp_entity = NULL;
    bool32 should_add_new_temp_entity = false;

    int32 num_values_read = 0;
    Vec3 vec3_buffer = {};
    Quaternion quaternion_buffer = {};

    do {
        token = get_token(&tokenizer, (char *) file_data.contents);

        if (token.type == COMMENT) continue;

        switch (state) {
            case WAIT_FOR_LEVEL_INFO_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "level_info")) {
                    state = WAIT_FOR_LEVEL_INFO_BLOCK_OPEN;
                } else {
                    assert(!"Expected level_info keyword to open block.");
                }
            } break;
            case WAIT_FOR_LEVEL_INFO_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_LEVEL_NAME_KEYWORD;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_LEVEL_NAME_KEYWORD: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "level_name")) {
                    state = WAIT_FOR_LEVEL_NAME_STRING;
                } else {
                    assert(!"Expected level_name keyword.");
                }
            } break;
            case WAIT_FOR_LEVEL_NAME_STRING: {
                if (token.type == STRING) {
                    temp_level->name = make_string_buffer(temp_allocator, token.string, LEVEL_NAME_MAX_SIZE);
                    state = WAIT_FOR_LEVEL_INFO_BLOCK_CLOSE;
                } else {
                    assert (!"Expected level name string.");
                }
            } break;
            case WAIT_FOR_LEVEL_INFO_BLOCK_CLOSE: {
                if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_MESHES_BLOCK_NAME;
                } else {
                    assert(!"Expected closing bracket for level_info block.");
                }
            } break;
            case WAIT_FOR_MESHES_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "meshes")) {
                    state = WAIT_FOR_MESHES_BLOCK_OPEN;
                } else {
                    assert (!"Expected meshes keyword to open block.");
                }
            } break;
            case WAIT_FOR_MESHES_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "mesh")) {
                    state = WAIT_FOR_MESH_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_TEXTURES_BLOCK_NAME;
                } else {
                    assert(!"Expected mesh keyword or closing bracket.");
                }
            } break;
            case WAIT_FOR_MESH_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MESH_NAME_MAX_SIZE);
                    
                    temp_mesh = {};
                    temp_mesh.name = make_string_buffer(temp_allocator, token.string, token.string.length);
                    state = WAIT_FOR_MESH_FILENAME_STRING;
                } else {
                    assert(!"Expected mesh name string.");
                }
            } break;
            case WAIT_FOR_MESH_FILENAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= PLATFORM_MAX_PATH);

                    temp_mesh.filename = make_string_buffer(temp_allocator, token.string, token.string.length);
                    level_add_mesh(temp_level, temp_mesh);

                    state = WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE;
                }
            } break;
            case WAIT_FOR_TEXTURES_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "textures")) {
                    state = WAIT_FOR_TEXTURES_BLOCK_OPEN;
                } else {
                    assert (!"Expected textures keyword to open block.");
                }
            } break;
            case WAIT_FOR_TEXTURES_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "texture")) {
                    state = WAIT_FOR_TEXTURE_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_MATERIALS_BLOCK_NAME;
                } else {
                    assert(!"Expected texture keyword or closing bracket.");
                }
            } break;
            case WAIT_FOR_TEXTURE_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= TEXTURE_NAME_MAX_SIZE);
                    
                    temp_texture = {};
                    temp_texture.name = make_string_buffer(temp_allocator, token.string, token.string.length);
                    state = WAIT_FOR_TEXTURE_FILENAME_STRING;
                } else {
                    assert(!"Expected texture name string.");
                }
            } break;
            case WAIT_FOR_TEXTURE_FILENAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= PLATFORM_MAX_PATH);

                    temp_texture.filename = make_string_buffer(temp_allocator, token.string, token.string.length);
                    level_add_texture(temp_level, temp_texture);

                    state = WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE;
                }
            } break;

            // MATERIALS
            case WAIT_FOR_MATERIALS_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "materials")) {
                    state = WAIT_FOR_MATERIALS_BLOCK_OPEN;
                } else {
                    assert(!"Expected materials keyword to open block.");
                }
            } break;
            case WAIT_FOR_MATERIALS_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "material")) {
                    state = WAIT_FOR_MATERIAL_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_ENTITIES_BLOCK_NAME;
                } else {
                    assert(!"Expected material keyword or closing bracket.");
                }
            } break;
            case WAIT_FOR_MATERIAL_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MATERIAL_NAME_MAX_SIZE);
                    
                    temp_material = {};
                    temp_material.name = make_string_buffer(temp_allocator, token.string, token.string.length);
                    temp_material.texture_id = -1;
                    should_add_new_temp_material = true;

                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected material name string.");
                }
            } break;
            case WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE: {
                if (token.type == KEYWORD) {
                    if (string_equals(token.string, "material")) {
                        if (should_add_new_temp_material) {
                            level_add_material(temp_level, temp_material);
                        }
                        state = WAIT_FOR_MATERIAL_NAME_STRING;
                    } else if (string_equals(token.string, "texture")) {
                        state = WAIT_FOR_MATERIAL_TEXTURE_NAME_STRING;
                    } else if (string_equals(token.string, "gloss")) {
                        state = WAIT_FOR_MATERIAL_GLOSS_NUMBER;
                    } else if (string_equals(token.string, "color_override")) {
                        state = WAIT_FOR_MATERIAL_COLOR_OVERRIDE_VEC3;
                        num_values_read = 0;
                    } else if (string_equals(token.string, "use_color_override")) {
                        state = WAIT_FOR_MATERIAL_USE_COLOR_OVERRIDE_INTEGER;
                    } else {
                        assert(!"Unrecognized material property.");
                    }
                } else if (token.type == CLOSE_BRACKET) {
                    if (should_add_new_temp_material) {
                        level_add_material(temp_level, temp_material);
                    }
                    state = WAIT_FOR_ENTITIES_BLOCK_NAME;
                } else {
                    assert(!"Expected a material property name keyword.");
                }
            } break;
            case WAIT_FOR_MATERIAL_TEXTURE_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= TEXTURE_NAME_MAX_SIZE);

                    int32 texture_id = get_texture_id_by_name(temp_level, token.string);
                    assert(texture_id >= 0);
                    temp_material.texture_id = texture_id;

                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a string for material texture name.");
                }
            } break;
            case WAIT_FOR_MATERIAL_GLOSS_NUMBER: {
                if (token.type == REAL || token.type == INTEGER) {
                    temp_material.gloss = string_to_real32(token.string.contents, token.string.length);
                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a number for material property gloss.");
                }
            } break;
            case WAIT_FOR_MATERIAL_COLOR_OVERRIDE_VEC3: {
                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string.contents,
                                                                           token.string.length);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_material.color_override = make_vec4(vec3_buffer, 1.0f);
                        state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for material property color_override.");
                }
            } break;
            case WAIT_FOR_MATERIAL_USE_COLOR_OVERRIDE_INTEGER: {
                if (token.type == INTEGER) {
                    temp_material.use_color_override = (int32) string_to_uint32(token.string.contents,
                                                                                token.string.length);

                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected an integer for material property use_color_override.");
                }
            } break;

            // ENTITIES
            case WAIT_FOR_ENTITIES_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "entities")) {
                    state = WAIT_FOR_ENTITIES_BLOCK_OPEN;
                } else {
                    assert(!"Expected entities keyword for entities block.");
                }
            } break;
            case WAIT_FOR_ENTITIES_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket for entities block.");
                }
            } break;
            case WAIT_FOR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "type")) {
                    state = WAIT_FOR_ENTITY_TYPE_VALUE;
                } else if (token.type == CLOSE_BRACKET) {
                    state = FINISHED;
                } else {
                    assert(!"Expected type keyword or closing bracket for entities block.");
                }
            } break;
            case WAIT_FOR_ENTITY_TYPE_VALUE: {
                if (token.type == KEYWORD) {
                    if (string_equals(token.string, "normal")) {
                        temp_normal_entity = make_entity(Mesh_Type::NONE, -1, -1, make_transform(), {});
                        temp_entity_type = ENTITY_NORMAL;
                        temp_entity = (Entity *) &temp_normal_entity;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                        should_add_new_temp_entity = true;
                    } else if (string_equals(token.string, "point_light")) {
                        temp_point_light_entity = make_point_light_entity(Mesh_Type:: NONE,
                                                                          -1, -1,
                                                                          make_vec3(), 0.0f, 0.0f, make_transform(),
                                                                          {});
                        temp_entity_type = ENTITY_POINT_LIGHT;
                        temp_entity = (Entity *) &temp_point_light_entity;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                        should_add_new_temp_entity = true;
                    } else {
                        assert(!"Unrecognized entity type.");
                    }
                } else {
                    assert(!"Expected entity type value keyword.");
                }
            } break;

            case WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE: {
                if (token.type == KEYWORD) {
                    if (string_equals(token.string, "type")) {
                        if (should_add_new_temp_entity) {
                            switch (temp_entity_type) {
                                case ENTITY_NORMAL: {
                                    level_add_entity(temp_level, temp_normal_entity);
                                } break;
                                case ENTITY_POINT_LIGHT: {
                                    level_add_point_light_entity(temp_level, temp_point_light_entity);
                                } break;
                                default: {
                                    assert(!"Unhandled entity type.");
                                } break;
                            }
                        }

                        state = WAIT_FOR_ENTITY_TYPE_VALUE;
                    } else if (string_equals(token.string, "mesh")) {
                        state = WAIT_FOR_ENTITY_MESH_NAME_STRING;
                        temp_entity->mesh_type = Mesh_Type::LEVEL;
                    } else if (string_equals(token.string, "mesh_primitive")) {
                        state = WAIT_FOR_ENTITY_MESH_NAME_STRING;
                        temp_entity->mesh_type = Mesh_Type::PRIMITIVE;
                    } else if (string_equals(token.string, "material")) {
                        state = WAIT_FOR_ENTITY_MATERIAL_NAME_STRING;
                    } else if (string_equals(token.string, "position")) {
                        num_values_read = 0;
                        state = WAIT_FOR_ENTITY_POSITION_VEC3;
                    } else if (string_equals(token.string, "rotation")) {
                        num_values_read = 0;
                        state = WAIT_FOR_ENTITY_ROTATION_QUATERNION;
                    } else if (string_equals(token.string, "scale")) {
                        num_values_read = 0;
                        state = WAIT_FOR_ENTITY_SCALE_VEC3;
                    } else if (temp_entity_type == ENTITY_POINT_LIGHT) {
                        if (string_equals(token.string, "light_color")) {
                            num_values_read = 0;
                            state = WAIT_FOR_POINT_LIGHT_ENTITY_LIGHT_COLOR_VEC3;
                        } else if (string_equals(token.string, "falloff_start")) {
                            num_values_read = 0;
                            state = WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_START_NUMBER;
                        } else if (string_equals(token.string, "falloff_end")) {
                            num_values_read = 0;
                            state = WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_END_NUMBER;
                        } else {
                            assert(!"Unrecognized entity property name.");
                        }
                    } else {
                        assert(!"Unrecognized entity property name.");
                    }
                } else if (token.type == CLOSE_BRACKET) {
                    if (should_add_new_temp_entity) {
                        switch (temp_entity_type) {
                            case ENTITY_NORMAL: {
                                level_add_entity(temp_level, temp_normal_entity);
                            } break;
                            case ENTITY_POINT_LIGHT: {
                                level_add_point_light_entity(temp_level, temp_point_light_entity);
                            } break;
                            default: {
                                assert(!"Unhandled entity type.");
                            } break;
                        }
                    }
                    state = FINISHED;
                }
            } break;
            case WAIT_FOR_ENTITY_MESH_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MESH_NAME_MAX_SIZE);

                    int32 mesh_id = get_mesh_id_by_name(game_state, temp_level, temp_entity->mesh_type, token.string);
                    if (mesh_id >= 0) {
                        temp_entity->mesh_id = mesh_id;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    } else {
                        assert(!"Mesh not found.");
                    }
                } else {
                    assert(!"Expected string for entity mesh name.");
                }
            } break;
            case WAIT_FOR_ENTITY_MATERIAL_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MATERIAL_NAME_MAX_SIZE);

                    int32 material_id = get_material_id_by_name(temp_level, token.string);
                    if (material_id >= 0) {
                        temp_entity->material_id = material_id;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    } else {
                        assert(!"Material not found.");
                    }
                } else {
                    assert(!"Expected string for entity material name.");
                }                
            } break;
            case WAIT_FOR_ENTITY_POSITION_VEC3: {
                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string.contents,
                                                                           token.string.length);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_entity->transform.position = vec3_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for entity property position.");
                }
            } break;
            case WAIT_FOR_ENTITY_ROTATION_QUATERNION: {
                if (token.type == REAL || token.type == INTEGER) {
                    if (num_values_read == 0) {
                        quaternion_buffer.w = string_to_real32(token.string.contents, token.string.length);
                    } else if (num_values_read >= 1 && num_values_read <= 3) {
                        int32 index = num_values_read - 1;
                        quaternion_buffer.v[index] = string_to_real32(token.string.contents, token.string.length);
                    }

                    num_values_read++;
                    if (num_values_read == 4) {
                        temp_entity->transform.rotation = quaternion_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 4 numbers (w, x, y, z) for entity property rotation.");
                }
            } break;
            case WAIT_FOR_ENTITY_SCALE_VEC3: {
                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string.contents,
                                                                           token.string.length);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_entity->transform.scale = vec3_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for entity property scale.");
                }
            } break;

            case WAIT_FOR_POINT_LIGHT_ENTITY_LIGHT_COLOR_VEC3: {
                assert(temp_entity_type == ENTITY_POINT_LIGHT);
                Point_Light_Entity *point_light_entity = (Point_Light_Entity *) temp_entity;

                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string.contents,
                                                                           token.string.length);
                    num_values_read++;
                    if (num_values_read == 3) {
                        point_light_entity->light_color = vec3_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for point light entity property light_color.");
                }
            } break;
            case WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_START_NUMBER: {
                assert(temp_entity_type == ENTITY_POINT_LIGHT);
                Point_Light_Entity *point_light_entity = (Point_Light_Entity *) temp_entity;

                if (token.type == REAL || token.type == INTEGER) {
                    point_light_entity->falloff_start = string_to_real32(token.string.contents,
                                                                         token.string.length);
                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a number for point light entity property falloff_start.");
                }
            } break;
            case WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_END_NUMBER: {
                assert(temp_entity_type == ENTITY_POINT_LIGHT);
                Point_Light_Entity *point_light_entity = (Point_Light_Entity *) temp_entity;

                if (token.type == REAL || token.type == INTEGER) {
                    point_light_entity->falloff_end = string_to_real32(token.string.contents,
                                                                       token.string.length);
                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a number for point light entity property falloff_end.");
                }
            } break;
            case FINISHED: {
                if (token.type != END) assert(!"Unexpected extra tokens.");
            }
        }

    } while (token.type != END);

    return true;
}

bool32 read_and_load_level(Game_State *game_state,
                           Level *level, char *filename,
                           Arena_Allocator *mesh_arena,
                           Pool_Allocator *string64_pool,
                           Pool_Allocator *filename_pool) {
    Marker m = begin_region();

    Allocator *temp_allocator = (Allocator *) &memory.global_stack;
    File_Data level_file = platform_open_and_read_file(temp_allocator, filename);

    Level *temp_level = (Level *) allocate(temp_allocator, sizeof(Level), true);
    temp_level->mesh_table = make_hash_table<int32, Mesh>(temp_allocator,
                                                          level->mesh_table.max_entries,
                                                          level->mesh_table.key_equals);
    temp_level->material_table = make_hash_table<int32, Material>(temp_allocator,
                                                                  level->material_table.max_entries,
                                                                  level->material_table.key_equals);
    temp_level->texture_table = make_hash_table<int32, Texture>(temp_allocator,
                                                                level->texture_table.max_entries,
                                                                level->texture_table.key_equals);

    bool32 load_temp_level_result = Level_Loader::load_temp_level(temp_allocator, game_state, level_file, temp_level);

    if (load_temp_level_result) {
        unload_level(level);

        Allocator *level_mesh_allocator = (Allocator *) mesh_arena;
        Allocator *level_string64_allocator = (Allocator *) string64_pool;
        Allocator *level_filename_allocator = (Allocator *) filename_pool;

        // copy all the values to start; we'll overwrite some of them
        // TODO: honestly, doing this is kind of annoying, since unload_level sets should_clear_gpu_data to
        //       to true, but temp_level has it set to false. so we have to re-set should_clear_gpu_data to
        //       true..
        *level = *temp_level;
        level->should_clear_gpu_data = true;

        // copy strings
        level->name = make_string_buffer(level_string64_allocator,
                                         make_string(temp_level->name),
                                         LEVEL_NAME_MAX_SIZE);
        // set allocators
        level->mesh_arena = mesh_arena;
        level->string64_pool = string64_pool;
        level->filename_pool = filename_pool;

        // NOTE: copy_hash_table
        level->mesh_table = copy_hash_table((Allocator *) &memory.hash_table_stack, temp_level->mesh_table);
        // copy mesh strings and load all the meshes
        int32 num_checked = 0;
        Hash_Table<int32, Mesh> temp_mesh_table = temp_level->mesh_table;
        Hash_Table<int32, Mesh> mesh_table = level->mesh_table;
        for (int32 i = 0; (i < temp_mesh_table.max_entries) && (num_checked < temp_mesh_table.num_entries); i++) {
            Hash_Table_Entry<int32, Mesh> entry = temp_mesh_table.entries[i];
            Mesh temp_mesh = entry.value;
            if (entry.is_occupied) {
                String_Buffer mesh_name = make_string_buffer(level_string64_allocator,
                                                             make_string(temp_mesh.name),
                                                             MESH_NAME_MAX_SIZE);
                String_Buffer mesh_filename = make_string_buffer(level_filename_allocator,
                                                                 make_string(temp_mesh.filename),
                                                                 PLATFORM_MAX_PATH);
                Mesh mesh = read_and_load_mesh(level_mesh_allocator, mesh_filename, mesh_name);
                mesh_table.entries[i].value = mesh;

                num_checked++;
            }
        }

        level->texture_table = copy_hash_table((Allocator *) &memory.hash_table_stack, temp_level->texture_table);
        // copy all the textures
        num_checked = 0;
        Hash_Table<int32, Texture> temp_texture_table = temp_level->texture_table;
        Hash_Table<int32, Texture> texture_table = level->texture_table;
        for (int32 i = 0;
             (i < temp_texture_table.max_entries) && (num_checked < temp_texture_table.num_entries);
             i++) {
            Hash_Table_Entry<int32, Texture> entry = temp_texture_table.entries[i];
            Texture temp_texture = entry.value;
            if (entry.is_occupied) {
                String_Buffer texture_name = make_string_buffer(level_string64_allocator,
                                                                make_string(temp_texture.name),
                                                                TEXTURE_NAME_MAX_SIZE);
                String_Buffer texture_filename = make_string_buffer(level_filename_allocator,
                                                                    make_string(temp_texture.filename),
                                                                    PLATFORM_MAX_PATH);

                Texture *dest_texture = &texture_table.entries[i].value;
                dest_texture->name = texture_name;
                dest_texture->filename = texture_filename;

                num_checked++;
            }
        }

        level->material_table = copy_hash_table((Allocator *) &memory.hash_table_stack, temp_level->material_table);
        // copy all the material strings
        num_checked = 0;
        Hash_Table<int32, Material> temp_material_table = temp_level->material_table;
        Hash_Table<int32, Material> material_table = level->material_table;
        for (int32 i = 0;
             (i < temp_material_table.max_entries) && (num_checked < temp_material_table.num_entries);
             i++) {
            Hash_Table_Entry<int32, Material> entry = temp_material_table.entries[i];
            Material temp_material = entry.value;
            if (entry.is_occupied) {
                String_Buffer material_name = make_string_buffer(level_string64_allocator,
                                                                 make_string(temp_material.name),
                                                                 MATERIAL_NAME_MAX_SIZE);

                Material *dest_material = &material_table.entries[i].value;
                dest_material->name = material_name;
                num_checked++;
            }
        }

        // set the AABBs
        for (int32 i = 0; i < level->num_normal_entities; i++) {
            Normal_Entity *entity = &level->normal_entities[i];
            Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
            entity->transformed_aabb = transform_aabb(mesh.aabb, entity->transform);
        }

        for (int32 i = 0; i < level->num_point_lights; i++) {
            Point_Light_Entity *entity = &level->point_lights[i];
            Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
            entity->transformed_aabb = transform_aabb(mesh.aabb, entity->transform);
        }
        
        end_region(m);
        return true;
    }

    end_region(m);
    return false;
}