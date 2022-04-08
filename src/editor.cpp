#include "font.h"
#include "editor.h"
#include "game.h"

uint8 side_left   = 0x1;
uint8 side_right  = 0x2;
uint8 side_top    = 0x4;
uint8 side_bottom = 0x8;

inline real32 get_center_y_offset(real32 height, real32 box_height) {
    return (height / 2.0f) - (box_height / 2.0f);
}

void draw_row(UI_Manager *ui_manager, Controller_State *controller_state,
              real32 x, real32 y,
              real32 row_width, real32 row_height,
              Vec4 color, uint8 side_flags,
              char *row_id, int32 index) {
    UI_Box_Style box_style = { color };

    do_box(ui_manager, controller_state, x, y, row_width, row_height,
           box_style, row_id, index); 

    Vec4 line_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    
    UI_Line_Style line_style = { line_color, 1.0f };
    if (side_flags & side_left) {
        do_line(ui_manager, { x, y - 1 }, { x, y + row_height },
                line_style,
                row_id);
    }
    if (side_flags & side_bottom) {
        do_line(ui_manager, { x, y + row_height }, { x + row_width, y + row_height },
                line_style,
                row_id);
    }
    if (side_flags & side_top) {
        do_line(ui_manager, { x, y }, { x + row_width, y },
                line_style,
                row_id);
    }
    if (side_flags & side_right) {
        do_line(ui_manager, { x + row_width, y }, { x + row_width, y + row_height },
                line_style,
                row_id);
    }
}

void draw_centered_text(Game_State *game_state, UI_Manager *ui,
                        real32 box_x, real32 box_y,
                        real32 row_width, real32 row_height,
                        char *text, char *font_name, UI_Text_Style text_style) {
    Font font = get_font(game_state, font_name);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 x_offset = 0.5f * row_width - 0.5f * get_width(font, text);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui, box_x + x_offset, box_y + y_offset,
            text, font_name, text_style, "entity_properties_text");
}

void draw_v_centered_text(Game_State *game_state, UI_Manager *ui,
                          real32 box_x, real32 box_y,
                          real32 row_height,
                          real32 x_offset,
                          char *text, char *font_name, UI_Text_Style text_style) {
    Font font = get_font(game_state, font_name);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui, box_x + x_offset, box_y + y_offset,
            text, font_name, text_style, "entity_properties_text");
}

void draw_labeled_text(Game_State *game_state, UI_Manager *ui_manager,
                       real32 x, real32 y,
                       real32 x_offset,
                       real32 row_height,
                       char *label_font, char *label,
                       char *text_font, char *text,
                       UI_Text_Style text_style) {
    real32 small_spacing = 20.0f;
    draw_v_centered_text(game_state, ui_manager, x, y, row_height, x_offset,
                         label, label_font, text_style);
    draw_v_centered_text(game_state, ui_manager, x+small_spacing, y, row_height, x_offset,
                         text, text_font, text_style);
}

void draw_material_library(Memory *memory, Game_State *game_state, Controller_State *controller_state,
                           Entity *entity) {
    Render_State *render_state = &game_state->render_state;
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;

    push_layer(ui_manager);
    
    real32 padding_x = 20.0f;
    real32 padding_y = 20.0f;

    real32 x_gap = 10.0f;
    real32 y_gap = 10.0f;

    int32 num_items_per_row = 5;
    real32 item_width = 100.0f;
    real32 item_height = 100.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 1.0f);

    char *row_id = "material_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = {
        make_vec3(1.0f, 1.0f, 1.0f),
        true,
        make_vec3(0.0f, 0.0f, 0.0f)
    };

    char *title_font_name = "courier18b";
    char *font_name = "courier18";

    draw_row(ui_manager, controller_state, x, y, window_width, title_row_height, title_row_color,
             side_left | side_right | side_top | side_bottom, row_id, row_index++);
    draw_centered_text(game_state, ui_manager, x, y, window_width, title_row_height,
                       "Material Library", title_font_name, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(ui_manager, controller_state, x, y, window_width, content_height, row_color,
             side_left | side_right | side_bottom, row_id, row_index++);    

    Allocator *allocator = (Allocator *) &memory->frame_arena;

    x += padding_x;
    y += padding_y;
    Material *materials = game_state->materials;
    int32 pressed_index = -1;
    
    for (int32 i = 0; i < game_state->num_materials; i++) {
        Material m = materials[i];
        bool32 pressed = do_text_button(ui_manager, controller_state,
                                        x, y,
                                        item_width, item_height,
                                        default_text_button_style,
                                        to_char_array(allocator, m.name),
                                        font_name,
                                        "material_library_item", i);
        if (pressed) pressed_index = i;
        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }

    if (pressed_index >= 0) {
        entity->material_index = pressed_index;
        editor_state->choosing_material = false;
    }

    pop_layer(ui_manager);
};

void draw_entity_box(Memory *memory, Game_State *game_state, Controller_State *controller_state, Entity *entity) {
    int32 row_index = 0;

    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_State = &game_state->editor_state;

    real32 box_x = 5.0f;
    real32 box_y = 50.0f;

    real32 box_padding_x = 10.0f;
    real32 box_padding_y = 10.0f;

    Allocator *allocator = (Allocator *) &memory->frame_arena;

    char *mesh_name = to_char_array(allocator, game_state->meshes[entity->mesh_index].name);
    char *material_name = to_char_array(allocator, game_state->materials[entity->material_index].name);
    Transform transform = entity->transform;

    UI_Text_Button_Style button_style = default_text_button_style;
    
    UI_Text_Style text_style = {
        make_vec3(1.0f, 1.0f, 1.0f),
        true,
        make_vec3(0.0f, 0.0f, 0.0f)
    };
    char *title_font_name = "courier18b";
    char *font_name = "courier18";
    char *font_name_bold = "courier18b";
    
    real32 row_height = 25.0f;
    real32 small_row_height = 20.0f;
    real32 row_width = 500.0f;

    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 0.9f);

    real32 x = box_x;
    real32 y = box_y;

    real32 title_row_height = 30.0f;
    uint8 side_flags = side_left | side_right;

    char *row_id = "mesh_properties_line";

    draw_row(ui_manager, controller_state, x, y, row_width, title_row_height, title_row_color,
             side_flags | side_top | side_bottom, row_id, row_index++);
    draw_centered_text(game_state, ui_manager, x, y, row_width, title_row_height,
                       "Entity Properties", title_font_name, text_style);
    y += title_row_height;
    
    real32 padding_left = 5.0f;
    real32 padding_right = padding_left;
    real32 right_column_offset = padding_left + 200.0f;
    real32 small_spacing = 20.0f;
    
    draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags | side_bottom, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x, y, row_height, padding_left,
                         "Mesh Name", font_name_bold, text_style);
    draw_v_centered_text(game_state, ui_manager, x, y, row_height, padding_left + right_column_offset,
                         mesh_name, font_name, text_style);
    y += row_height;

    char *buf;
    int32 buffer_size = 16;
    
    // position
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x, y, small_row_height, padding_left,
                         "Position", font_name_bold, text_style);
    // x
    buf = string_format(allocator, buffer_size, "%f", transform.position.x);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "x", font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.position.y);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "y", font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags | side_bottom,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.position.z);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "z", font_name, buf, text_style);
    y += small_row_height;    

    // rotation
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x, y, small_row_height, padding_left,
                         "Rotation", font_name_bold, text_style);
    // w
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.w);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "w", font_name, buf, text_style);
    y += small_row_height;
    // x
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.x);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "x", font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.y);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "y", font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags | side_bottom,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.z);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "z", font_name, buf, text_style);
    y += small_row_height;

    // scale
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x, y, small_row_height, padding_left,
                         "Scale", font_name_bold, text_style);
    // x
    buf = string_format(allocator, buffer_size, "%f", transform.scale.x);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "x", font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.scale.y);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "y", font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags | side_bottom,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.scale.z);
    draw_labeled_text(game_state, ui_manager, x, y, x + right_column_offset, small_row_height,
                      font_name_bold, "z", font_name, buf, text_style);
    y += small_row_height;

    // material info
    draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags | side_bottom,
             row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x, y, small_row_height, padding_left,
                         "Material", font_name_bold, text_style);
    draw_v_centered_text(game_state, ui_manager, x, y, small_row_height, x + right_column_offset,
                         material_name, font_name, text_style);
    
    bool32 edit_material_pressed = do_text_button(ui_manager, controller_state,
                                                  x + row_width - 50.0f - padding_right,
                                                  y + get_center_y_offset(row_height, small_row_height),
                                                  50.0f, small_row_height, button_style,
                                                  "Edit", font_name_bold, "edit_material");

    if (edit_material_pressed) {
        game_state->editor_state.choosing_material = true;
    }

    y += small_row_height;
}

void draw_editor_ui(Memory *memory, Game_State *game_state, Controller_State *controller_state) {
    Editor_State *editor_state = &game_state->editor_state;

    if (editor_state->selected_entity_index >= 0) {
        Entity *selected_entity = get_selected_entity(game_state);
        draw_entity_box(memory, game_state, controller_state, selected_entity);

        if (editor_state->choosing_material) {
            draw_material_library(memory, game_state, controller_state, selected_entity);
        }
    } else {
        editor_state->choosing_material = false;
    }
}

int32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, real32 *t_result) {
    Mat4 object_to_world = get_model_matrix(transform);
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
        // convert the t_min on the object space ray to a t_min on the world space ray
        Vec3 object_space_hit_point = object_space_ray_origin + object_space_ray_direction * t_min;
        Vec3 world_space_hit_point = truncate_v4_to_v3(object_to_world * make_vec4(object_space_hit_point, 1.0f));
        real32 world_space_t_min = dot(world_space_hit_point - ray.origin, ray.direction);
        *t_result = world_space_t_min;
    }

    return hit;
}

// TODO: optimize this by checking against AABB before checking against triangles
int32 pick_entity(Game_State *game_state, Ray cursor_ray, Entity *entity_result, int32 *index_result) {
    Editor_State *editor_state = &game_state->editor_state;

    Mesh *meshes = game_state->meshes;
    Normal_Entity *entities = game_state->entities;
    Point_Light_Entity *point_lights = game_state->point_lights;

    Entity *picked_entity = NULL;
    int32 entity_index = -1;

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < game_state->num_entities; i++) {
        real32 t;
        Normal_Entity *entity = &entities[i];
        Mesh mesh = meshes[entity->mesh_index];
        if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
            picked_entity = (Entity *) entity;
        }
    }

    for (int32 i = 0; i < game_state->num_point_lights; i++) {
        real32 t;
        Point_Light_Entity *entity = &point_lights[i];
        Mesh mesh = meshes[entity->mesh_index];
        if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
            picked_entity = (Entity *) entity;
        }
    }

    if (entity_index >= 0) {
        *entity_result = *picked_entity;
        *index_result = entity_index;
        return true;
    }

    return false;
}

bool32 is_translation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_TRANSLATE_X ||
            gizmo_axis == GIZMO_TRANSLATE_Y ||
            gizmo_axis == GIZMO_TRANSLATE_Z);
}

bool32 is_rotation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_ROTATE_X ||
            gizmo_axis == GIZMO_ROTATE_Y ||
            gizmo_axis == GIZMO_ROTATE_Z);
}

Gizmo_Handle pick_gizmo(Game_State *game_state, Ray cursor_ray,
                        Vec3 *gizmo_initial_hit, Vec3 *gizmo_transform_axis) {
    Editor_State *editor_state = &game_state->editor_state;
    Entity *entity = get_selected_entity(game_state);
    Gizmo gizmo = editor_state->gizmo;

    Transform_Mode transform_mode = editor_state->transform_mode;

    Mat4 entity_model_matrix = get_model_matrix(entity->transform);

    Transform x_transform, y_transform, z_transform;
    Vec3 transform_x_axis, transform_y_axis, transform_z_axis;
    // TODO: maybe add some scale here? for a bit of extra space to click on the gizmo.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo.transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo.transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);

        transform_x_axis = x_axis;
        transform_y_axis = y_axis;
        transform_z_axis = z_axis;
    } else {
        // local transform
        x_transform = gizmo.transform;
        y_transform = gizmo.transform;
        y_transform.rotation = gizmo.transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = gizmo.transform.rotation*make_quaternion(-90.0f, y_axis);

        transform_x_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col1));
        transform_y_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col2));
        transform_z_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col3));
    }

    Gizmo_Handle picked_handle = GIZMO_HANDLE_NONE;

    Transform gizmo_handle_transforms[3] = { x_transform, y_transform, z_transform };
    Vec3 transform_axes[3] = { transform_x_axis, transform_y_axis, transform_z_axis };
    Vec3 selected_transform_axis = transform_x_axis;

    // check ray against translation arrows
    Gizmo_Handle gizmo_translation_handles[3] = { GIZMO_TRANSLATE_X, GIZMO_TRANSLATE_Y, GIZMO_TRANSLATE_Z };
    int32 gizmo_arrow_mesh_index = get_mesh_index(game_state, gizmo.arrow_mesh_name);
    assert(gizmo_arrow_mesh_index >= 0);
    Mesh arrow_mesh = game_state->meshes[gizmo_arrow_mesh_index];

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, arrow_mesh, gizmo_handle_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_handle = gizmo_translation_handles[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    // check ray against rotation rings
    Gizmo_Handle gizmo_rotation_handles[3] = { GIZMO_ROTATE_X, GIZMO_ROTATE_Y, GIZMO_ROTATE_Z };
    int32 gizmo_ring_mesh_index = get_mesh_index(game_state, gizmo.ring_mesh_name);
    assert(gizmo_ring_mesh_index >= 0);
    Mesh ring_mesh = game_state->meshes[gizmo_ring_mesh_index];

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, ring_mesh, gizmo_handle_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_handle = gizmo_rotation_handles[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    if (picked_handle) {
        if (is_rotation(picked_handle)) {
            real32 t;
            Plane plane = { dot(gizmo.transform.position, selected_transform_axis),
                            selected_transform_axis };
            bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);
            if (intersects_plane) {
                *gizmo_initial_hit = cursor_ray.origin + t*cursor_ray.direction;
            } else {
                *gizmo_initial_hit = cursor_ray.origin + t_min*cursor_ray.direction;;
            }
        } else {
            *gizmo_initial_hit = cursor_ray.origin + t_min*cursor_ray.direction;
        }

        *gizmo_transform_axis = selected_transform_axis;
    }

    return picked_handle;
}

Vec3 do_gizmo_translation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Ray transform_ray = make_ray(editor_state->gizmo_initial_hit,
                                 editor_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 delta_result = make_vec3();

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - editor_state->last_gizmo_transform_point,
                                  editor_state->gizmo_transform_axis);
        Vec3 delta = editor_state->gizmo_transform_axis * delta_length;
        editor_state->last_gizmo_transform_point += delta;
        delta_result = delta;
    }

    return delta_result;
}

Quaternion do_gizmo_rotation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Vec3 center = editor_state->gizmo.transform.position;
    Plane plane = { dot(center, editor_state->gizmo_transform_axis),
                    editor_state->gizmo_transform_axis };
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Quaternion delta_result = make_quaternion();

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;

        Vec3 center_to_intersect_point = intersect_point - center;
        Vec3 center_to_last_intersect_point = editor_state->last_gizmo_transform_point - center;

        if (are_collinear(center_to_last_intersect_point, center_to_intersect_point)) {
            return delta_result;
        }

        Vec3 out_vector = cross(editor_state->gizmo_transform_axis, center_to_last_intersect_point);
        
        real32 sign = 1.0f;
        if (dot(center_to_intersect_point, out_vector) < 0.0f) sign = -1.0f;

        real32 a = distance(center_to_intersect_point);
        real32 b = distance(center_to_last_intersect_point);
        real32 c = distance(intersect_point - editor_state->last_gizmo_transform_point);

        real32 angle_delta_degrees = sign * cosine_law_degrees(a, b, c);
        Quaternion delta = make_quaternion(angle_delta_degrees, editor_state->gizmo_transform_axis);

        editor_state->last_gizmo_transform_point = intersect_point;
        delta_result = delta;
    }

    return delta_result;
}
