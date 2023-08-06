#include "editor.h"

namespace Asset_Library_Themes {
    UI_Container_Theme container_theme = {
        { 5.0f, 5.0f, 5.0f, 5.0f },
        DEFAULT_BOX_BACKGROUND,
        UI_POSITION_NONE,
        {},
        { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING },
        {},
        UI_LAYOUT_VERTICAL
    };
        
    UI_Theme list_container_theme = {};
    UI_Theme properties_container_theme = {};
    UI_Theme list_theme = {};
    UI_Dropdown_Theme dropdown_theme = {};
    UI_Theme row_theme = {};
    UI_Theme full_row_theme = {};
    UI_Theme space_between_row_theme = {};
};

int32 get_selected_name_index(String name, char **names, int32 num_names) {
    for (int32 i = 0; i < num_names; i++) {
        if (string_equals(name, names[i])) {
            return i;
        }
    }

    return -1;
}

void draw_mesh_library() {
    using namespace Asset_Library_Themes;

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;

    const int32 MAX_MESHES = 256;
    int32 mesh_ids[MAX_MESHES];
    int32 num_meshes_listed = 0;
    
    Mesh *selected_mesh = NULL;
        
    ui_add_and_push_widget("asset-library-mesh-list-container", list_container_theme);
    {
        ui_add_and_push_widget("", space_between_row_theme);
        {
            do_y_centered_text("Meshes");
            UI_Button_Theme add_button_theme = editor_button_theme;
            add_button_theme.size_type.x = UI_SIZE_ABSOLUTE;
            add_button_theme.size.x = 20.0f;

            bool32 add_mesh_clicked = do_text_button("+", add_button_theme, "add_mesh_button");
            if (add_mesh_clicked) {
                Allocator *temp_region = begin_region(256);

                String new_mesh_name;
                bool32 gen_result = generate_asset_name(temp_region, "Mesh", 256, get_remaining(temp_region),
                                                        &new_mesh_name, mesh_exists);
                assert(gen_result);

                Mesh *new_mesh = add_mesh(new_mesh_name, make_string("blender/cube.mesh"), Mesh_Type::LEVEL);
                assert(new_mesh);
                asset_library_state->selected_mesh_id = new_mesh->id;

                end_region(temp_region);
            }
        } ui_pop_widget();

        ui_y_pad(5.0f);

        UI_Scrollable_Region_Theme list_scroll_region_theme = {};
        list_scroll_region_theme.background_color = rgb_to_vec4(0, 0, 0);
        list_scroll_region_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

        push_scrollable_region(list_scroll_region_theme,
                               make_string("mesh-list-scroll-region"));
        ui_add_and_push_widget("asset-library-mesh-list-container2", list_theme,
                               UI_WIDGET_DRAW_BACKGROUND);
        {
            // don't use i for the button indices because that's for buckets and
            // not the actual meshes we've visited
            for (int32 i = 0; i < NUM_MESH_BUCKETS; i++) {
                Mesh *current = asset_manager->mesh_table[i];
                while (current) {
                    if (current->type == Mesh_Type::LEVEL) {
                        assert(num_meshes_listed < MAX_MESHES);
                        mesh_ids[num_meshes_listed] = current->id;
                            
                        if (num_meshes_listed == 0 && asset_library_state->selected_mesh_id < 0) {
                            // select the first one by default
                            asset_library_state->selected_mesh_id = current->id;
                        }
                            
                        char *mesh_name = to_char_array((Allocator *) &ui_manager->frame_arena,
                                                            current->name);

                        bool32 is_selected = current->id == asset_library_state->selected_mesh_id;
                        bool32 pressed = do_text_button(mesh_name,
                                                        is_selected ? selected_item_theme : item_theme,
                                                        "asset-library-mesh-list-item",
                                                        num_meshes_listed);

                        if (pressed || is_selected) {
                            asset_library_state->selected_mesh_id = current->id;
                            selected_mesh = current;
                        }

                        num_meshes_listed++;
                    }

                    current = current->table_next;
                }
            }
        }
        ui_pop_widget(); // asset-library-mesh-list-container
        ui_pop_widget(); // scrollable region
    }
    ui_pop_widget();

    ui_x_pad(5.0f);

    ui_add_and_push_widget("asset-library-mesh-info-container", properties_container_theme);
    {

        UI_Theme section_theme = {};
        section_theme.layout_type = UI_LAYOUT_VERTICAL;
        section_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

        ui_add_and_push_widget("", section_theme);
        {
            UI_Theme properties_header_theme = {};
            properties_header_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
            properties_header_theme.semantic_size = {  0.0f, editor_button_theme.size.y };
            properties_header_theme.layout_type = UI_LAYOUT_VERTICAL;
            ui_add_and_push_widget("", properties_header_theme); {
                do_y_centered_text("Properties");
            } ui_pop_widget();

            if (selected_mesh) {
                ui_y_pad(5.0f);
                {
                    do_text("Mesh Name");
                    UI_Text_Field_Theme field_theme = editor_text_field_theme;
                    field_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                    field_theme.size.x = 0.0f;
                    UI_Text_Field_Result name_result = do_text_field(field_theme, selected_mesh->name,
                                                                     "mesh_name_text_field",
                                                                     "mesh_name_text_field_text");

                    if (name_result.committed) {
                        if (!string_equals(name_result.text, selected_mesh->name) && name_result.text.length > 0) {
                            if (!mesh_exists(name_result.text)) {
                                replace_contents(&selected_mesh->name, name_result.text);
                            } else {
                                add_message(Context::message_manager, make_string("Mesh name already exists!"));
                            }
                        }
                    }
                }

                ui_y_pad(5.0f);
                
                {
                    do_text("Filepath");
                    UI_Text_Field_Theme field_theme = editor_text_field_theme;
                    field_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                    field_theme.size.x = 0.0f;

                    ui_add_and_push_widget("", full_row_theme);
                    {
                        Allocator *temp_region = begin_region();

                        UI_Text_Field_Result filepath_result = do_text_field(field_theme, selected_mesh->filename,
                                                                         "mesh_filepath_text_field",
                                                                         "mesh_filepath_text_field_text",
                                                                         PLATFORM_MAX_PATH - 1);

                        if (filepath_result.committed) {
                            if (filepath_result.text.length > 0) {
                                if (platform_file_exists(filepath_result.text)) {
                                    set_mesh_file(selected_mesh->id, filepath_result.text);
                                } else {
                                    add_message(Context::message_manager, make_string("File does not exist!"));
                                }
                            }
                        }

                        ui_x_pad(5.0f);

                        UI_Button_Theme browse_theme = editor_button_theme;
                        browse_theme.size_type.x = UI_SIZE_ABSOLUTE;
                        browse_theme.size.x = 80.0f;

                        bool32 browse_clicked = do_text_button("Browse", browse_theme, "mesh_browse_button");
                        if (browse_clicked) {
                            char absolute_path[PLATFORM_MAX_PATH];
                            if (platform_open_file_dialog(absolute_path,
                                                          MESH_FILE_FILTER_TITLE, MESH_FILE_FILTER_TYPE,
                                                          PLATFORM_MAX_PATH)) {

                                char relative_path[PLATFORM_MAX_PATH];
                                platform_get_relative_path(absolute_path, relative_path, PLATFORM_MAX_PATH);

                                String relative_path_string = make_string(temp_region, relative_path);

                                set_mesh_file(selected_mesh->id, relative_path_string);
                            }
                        }

                        end_region(temp_region);
                    } ui_pop_widget();
                }
            } // if (selected_mesh)
        } ui_pop_widget(); // section

        UI_Theme footer_theme = {};
        footer_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
        footer_theme.layout_type = UI_LAYOUT_HORIZONTAL;

        ui_add_and_push_widget("", footer_theme);
        {
            if (selected_mesh) {
                UI_Theme push_right = {};
                push_right.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                ui_add_widget("", push_right);

                UI_Button_Theme delete_theme = editor_button_danger_theme;
                delete_theme.size_type.x = UI_SIZE_ABSOLUTE;
                delete_theme.size.x = 120.0f;
                
                bool32 delete_pressed = do_text_button("Delete Mesh", delete_theme, "delete-mesh");

                if (delete_pressed) {
                    // find mesh id in list
                    int32 id_to_delete = selected_mesh->id;

                    for (int32 i = 0; i < num_meshes_listed; i++) {
                        if (mesh_ids[i] == id_to_delete) {
                            if (num_meshes_listed >= 1) {
                                int32 new_index;
                                if (i == 0) {
                                    new_index = i + 1;
                                } else {
                                    new_index = max(i - 1, 0);
                                }
                                asset_library_state->selected_mesh_id = mesh_ids[new_index];
                            } else {
                                // we're deleting the last one
                                asset_library_state->selected_mesh_id = -1;
                            }

                            break;
                        }
                    }

                    selected_mesh = get_mesh(asset_library_state->selected_mesh_id);

                    delete_mesh(id_to_delete);
                    num_meshes_listed = max(num_meshes_listed--, 0);
                }
            }
        } ui_pop_widget(); // footer
    } ui_pop_widget(); // asset-library-mesh-info-container
}

void draw_material_library() {
    using namespace Asset_Library_Themes;
    
    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;
    
    const int32 MAX_MATERIALS = 256;
    int32 material_ids[MAX_MATERIALS];
    int32 num_materials_listed = 0;
    
    Material *selected_material = NULL;
        
    ui_add_and_push_widget("asset-library-material-list-container", list_container_theme);
    {
        ui_add_and_push_widget("", space_between_row_theme);
        {
            do_y_centered_text("Materials");
            UI_Button_Theme add_button_theme = editor_button_theme;
            add_button_theme.size_type.x = UI_SIZE_ABSOLUTE;
            add_button_theme.size.x = 20.0f;

            bool32 add_material_clicked = do_text_button("+", add_button_theme, "add_material_button");
            if (add_material_clicked) {
                Material_Info material_info = default_material_info;
                Allocator *temp_region = begin_region(256);
                bool32 gen_result = generate_asset_name(temp_region, "Material", 256, get_remaining(temp_region),
                                                        &material_info.name, material_exists);
                assert(gen_result);
                    
                Material *new_material = add_material(&material_info, Material_Type::LEVEL);
                asset_library_state->selected_material_id = new_material->id;
                    
                end_region(temp_region);
            }
        } ui_pop_widget();

        ui_y_pad(5.0f);

        UI_Scrollable_Region_Theme list_scroll_region_theme = {};
        list_scroll_region_theme.background_color = rgb_to_vec4(0, 0, 0);
        list_scroll_region_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

        push_scrollable_region(list_scroll_region_theme,
                               make_string("material-list-scroll-region"));
        ui_add_and_push_widget("asset-library-material-list-container2", list_theme,
                               UI_WIDGET_DRAW_BACKGROUND);
        {
            // don't use i for the button indices because that's for buckets and
            // not the actual materials we've visited
            for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
                Material *current = asset_manager->material_table[i];
                while (current) {
                    if (current->type == Material_Type::LEVEL) {
                        assert(num_materials_listed < MAX_MATERIALS);
                        material_ids[num_materials_listed] = current->id;
                            
                        if (num_materials_listed == 0 && asset_library_state->selected_material_id < 0) {
                            // select the first one by default
                            asset_library_state->selected_material_id = current->id;
                        }
                            
                        char *material_name = to_char_array((Allocator *) &ui_manager->frame_arena,
                                                            current->name);

                        bool32 is_selected = current->id == asset_library_state->selected_material_id;
                        bool32 pressed = do_text_button(material_name,
                                                        is_selected ? selected_item_theme : item_theme,
                                                        "asset-library-material-list-item",
                                                        num_materials_listed);

                        if (pressed || is_selected) {
                            asset_library_state->selected_material_id = current->id;
                            selected_material = current;
                        }

                        num_materials_listed++;
                    }

                    current = current->table_next;
                }
            }
        }
        ui_pop_widget(); // asset-library-material-list-container
        ui_pop_widget(); // scrollable region
    }
    ui_pop_widget();

    ui_x_pad(5.0f);

    ui_add_and_push_widget("asset-library-material-info-container", properties_container_theme);
    {

        UI_Theme section_theme = {};
        section_theme.layout_type = UI_LAYOUT_VERTICAL;
        section_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

        ui_add_and_push_widget("", section_theme);
        {
            UI_Theme properties_header_theme = {};
            properties_header_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
            properties_header_theme.semantic_size = {  0.0f, editor_button_theme.size.y };
            properties_header_theme.layout_type = UI_LAYOUT_VERTICAL;
            ui_add_and_push_widget("", properties_header_theme); {
                do_y_centered_text("Properties");
            } ui_pop_widget();

            if (selected_material) {
                ui_y_pad(5.0f);
                {
                    do_text("Material Name");
                    UI_Text_Field_Theme field_theme = editor_text_field_theme;
                    field_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                    field_theme.size.x = 0.0f;
                    UI_Text_Field_Result name_result = do_text_field(field_theme, selected_material->name,
                                                                     "material_name_text_field",
                                                                     "material_name_text_field_text");

                    if (name_result.committed) {
                        if (!string_equals(name_result.text, selected_material->name) && name_result.text.length > 0) {
                            if (!material_exists(name_result.text)) {
                                replace_contents(&selected_material->name, name_result.text);
                            } else {
                                add_message(Context::message_manager, make_string("Material name already exists!"));
                            }
                        }
                    }
                }

                const int32 MAX_TEXTURE_NAMES = 256;
                char *texture_names[MAX_TEXTURE_NAMES];
                int32 num_texture_names;
                get_texture_names((Allocator *) &ui_manager->frame_arena, texture_names, MAX_TEXTURE_NAMES,
                                  &num_texture_names);
            
                ui_y_pad(10.0f);
                {
                    ui_add_and_push_widget("", row_theme);
                    {
                        do_y_centered_text("Use Albedo Texture");
                        ui_x_pad(5.0f);
                        bool32 checked = do_checkbox(selected_material->flags & MATERIAL_USE_ALBEDO_TEXTURE,
                                                     editor_checkbox_theme, "material-albedo-texture-checkbox");
                        selected_material->flags = set_bits(selected_material->flags, MATERIAL_USE_ALBEDO_TEXTURE,
                                                            checked);
                    }
                    ui_pop_widget();
                    ui_y_pad(5.0f);

                    if (selected_material->flags & MATERIAL_USE_ALBEDO_TEXTURE) {
                        do_text("Albedo Texture");
                        Texture *albedo_texture = get_texture(selected_material->albedo_texture_id);
                        int32 selected_index = get_selected_name_index(albedo_texture->name,
                                                                       texture_names, num_texture_names);
                        assert(selected_index >= 0);

                        int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                    texture_names, num_texture_names,
                                                                    selected_index,
                                                                    "albedo_texture_dropdown");
                        if (dropdown_selected_index != selected_index) {
                            Allocator *temp_region = begin_region();
                            selected_index = dropdown_selected_index;
                            Texture *selected = get_texture(make_string(temp_region, texture_names[selected_index]));
                            selected_material->albedo_texture_id = selected->id;
                            end_region(temp_region);
                        }
                    } else {
                        UI_Interact_Result interact_result;
                        bool32 open_color_picker_pressed = do_text_button("Open Color Picker",
                                                                          editor_button_theme,
                                                                          "albedo-open-color-picker", 0,
                                                                          &interact_result);
                        if (open_color_picker_pressed) {
                            asset_library_state->material_albedo_color_picker_open = !asset_library_state->material_albedo_color_picker_open;
                        }

                        if (asset_library_state->material_albedo_color_picker_open) {
                            UI_Color_Picker_Result result = do_color_picker(selected_material->albedo_color,
                                                                            "albedo-color-picker",
                                                                            "albedo-color-picker-panel",
                                                                            "albedo-color-picker-slider",
                                                                            false);

                            if (result.should_hide && !interact_result.just_pressed) {
                                asset_library_state->material_albedo_color_picker_open = false;
                            } else {
                                selected_material->albedo_color = result.color;
                            }
                        }
                    }
                }

                ui_y_pad(10.0f);
                {
                    ui_add_and_push_widget("", row_theme);
                    {
                        do_y_centered_text("Use Metalness Texture");
                        ui_x_pad(5.0f);
                        bool32 checked = do_checkbox(selected_material->flags & MATERIAL_USE_METALNESS_TEXTURE,
                                                     editor_checkbox_theme, "material-metalness-texture-checkbox");
                        selected_material->flags = set_bits(selected_material->flags,
                                                            MATERIAL_USE_METALNESS_TEXTURE,
                                                            checked);
                    }
                    ui_pop_widget();
                    ui_y_pad(5.0f);

                    if (selected_material->flags & MATERIAL_USE_METALNESS_TEXTURE) {

                        do_text("Metalness Texture");
                        Texture *metalness_texture = get_texture(selected_material->metalness_texture_id);
                        int32 selected_index = get_selected_name_index(metalness_texture->name,
                                                                       texture_names, num_texture_names);
                        assert(selected_index >= 0);

                        int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                    texture_names, num_texture_names,
                                                                    selected_index,
                                                                    "metalness_texture_dropdown");
                        if (dropdown_selected_index != selected_index) {
                            Allocator *temp_region = begin_region();
                            selected_index = dropdown_selected_index;
                            Texture *selected = get_texture(make_string(temp_region, texture_names[selected_index]));
                            selected_material->metalness_texture_id = selected->id;
                            end_region(temp_region);
                        }
                    } else {
                        do_text("Metalness");
                        UI_Text_Field_Slider_Result result = do_text_field_slider(selected_material->metalness,
                                                                                  0.0f, 1.0f,
                                                                                  editor_slider_theme,
                                                                                  "material-metalness-slider",
                                                                                  "material-metalness-slider-text");
                        selected_material->metalness = result.value;
                    }
                    ui_y_pad(5.0f);
                }

                ui_y_pad(10.0f);
                {
                    ui_add_and_push_widget("", row_theme);
                    {
                        do_y_centered_text("Use Roughness Texture");
                        ui_x_pad(5.0f);
                        bool32 checked = do_checkbox(selected_material->flags & MATERIAL_USE_ROUGHNESS_TEXTURE,
                                                     editor_checkbox_theme, "material-roughness-texture-checkbox");
                        selected_material->flags = set_bits(selected_material->flags,
                                                            MATERIAL_USE_ROUGHNESS_TEXTURE,
                                                            checked);
                    }
                    ui_pop_widget();
                    ui_y_pad(5.0f);
                    
                    if (selected_material->flags & MATERIAL_USE_ROUGHNESS_TEXTURE) {
                        do_text("Roughness Texture");
                        Texture *roughness_texture = get_texture(selected_material->roughness_texture_id);
                        int32 selected_index = get_selected_name_index(roughness_texture->name,
                                                                       texture_names, num_texture_names);
                        assert(selected_index >= 0);

                        int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                    texture_names, num_texture_names,
                                                                    selected_index,
                                                                    "roughness_texture_dropdown");
                        if (dropdown_selected_index != selected_index) {
                            Allocator *temp_region = begin_region();
                            selected_index = dropdown_selected_index;
                            Texture *selected = get_texture(make_string(temp_region, texture_names[selected_index]));
                            selected_material->roughness_texture_id = selected->id;
                            end_region(temp_region);
                        }
                    } else {
                        do_text("Roughness");
                        UI_Text_Field_Slider_Result result = do_text_field_slider(selected_material->roughness,
                                                                                  0.0f, 1.0f,
                                                                                  editor_slider_theme,
                                                                                  "material-roughness-slider",
                                                                                  "material-roughness-slider-text");
                        selected_material->roughness = result.value;
                    }
                    
                    ui_y_pad(5.0f);
                }
            }
        } ui_pop_widget(); // section

        UI_Theme footer_theme = {};
        footer_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
        footer_theme.layout_type = UI_LAYOUT_HORIZONTAL;

        ui_add_and_push_widget("", footer_theme);
        {
            if (selected_material) {
                UI_Theme push_right = {};
                push_right.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                ui_add_widget("", push_right);

                UI_Button_Theme delete_theme = editor_button_danger_theme;
                delete_theme.size_type.x = UI_SIZE_ABSOLUTE;
                delete_theme.size.x = 120.0f;
                
                bool32 delete_pressed = do_text_button("Delete Material", delete_theme, "delete-material");

                if (delete_pressed) {
                    // find material id in list
                    int32 id_to_delete = selected_material->id;

                    for (int32 i = 0; i < num_materials_listed; i++) {
                        if (material_ids[i] == id_to_delete) {
                            if (num_materials_listed >= 1) {
                                int32 new_index;
                                if (i == 0) {
                                    new_index = i + 1;
                                } else {
                                    new_index = max(i - 1, 0);
                                }
                                asset_library_state->selected_material_id = material_ids[new_index];
                            } else {
                                // we're deleting the last one
                                asset_library_state->selected_material_id = -1;
                            }

                            break;
                        }
                    }

                    selected_material = get_material(asset_library_state->selected_material_id);

                    delete_material(id_to_delete);
                    num_materials_listed = max(num_materials_listed--, 0);
                }
            }
        } ui_pop_widget(); // footer
    }
    ui_pop_widget();
}

void draw_texture_library() {
    using namespace Asset_Library_Themes;

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;

    const int32 MAX_TEXTURES = 256;
    int32 texture_ids[MAX_TEXTURES];
    int32 num_textures_listed = 0;
    
    Texture *selected_texture = NULL;
        
    ui_add_and_push_widget("asset-library-texture-list-container", list_container_theme);
    {
        ui_add_and_push_widget("", space_between_row_theme);
        {
            do_y_centered_text("Textures");
            UI_Button_Theme add_button_theme = editor_button_theme;
            add_button_theme.size_type.x = UI_SIZE_ABSOLUTE;
            add_button_theme.size.x = 20.0f;

            bool32 add_texture_clicked = do_text_button("+", add_button_theme, "add_texture_button");
            if (add_texture_clicked) {
                Allocator *temp_region = begin_region(256);

                String new_texture_name;
                bool32 gen_result = generate_asset_name(temp_region, "Texture", 256, get_remaining(temp_region),
                                                        &new_texture_name, texture_exists);
                assert(gen_result);

                Texture *new_texture = add_texture(new_texture_name, make_string("blender/debug-texture.jpg"),
                                                   Texture_Type::LEVEL);
                assert(new_texture);
                asset_library_state->selected_texture_id = new_texture->id;

                end_region(temp_region);
            }
        } ui_pop_widget();

        ui_y_pad(1.0f);

        UI_Scrollable_Region_Theme list_scroll_region_theme = {};
        list_scroll_region_theme.background_color = rgb_to_vec4(0, 0, 0);
        list_scroll_region_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
        list_scroll_region_theme.hide_scrollbar_until_necessary = false;

        push_scrollable_region(list_scroll_region_theme,
                               make_string("texture-list-scroll-region"));
        ui_add_and_push_widget("asset-library-texture-list-container2", list_theme,
                               UI_WIDGET_DRAW_BACKGROUND);
        {
            UI_Theme texture_tile = {};
            texture_tile.layout_type = UI_LAYOUT_VERTICAL;
            texture_tile.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
            texture_tile.semantic_size = { 0.0f, 0.0f };
            texture_tile.background_color = DEFAULT_BUTTON_BACKGROUND;
            texture_tile.hot_background_color = DEFAULT_BUTTON_HOT_BACKGROUND;
            texture_tile.active_background_color = DEFAULT_BUTTON_ACTIVE_BACKGROUND;

            UI_Theme selected_texture_tile = texture_tile;
            selected_texture_tile.background_color = selected_item_theme.background_color;
            selected_texture_tile.hot_background_color = selected_item_theme.hot_background_color;
            selected_texture_tile.active_background_color = selected_item_theme.active_background_color;

            UI_Widget *row_widget = NULL;
            int32 tiles_in_row = 0;
            bool32 first_row = true;
            int32 tiles_per_row = 3;
            // don't use i for the button indices because that's for buckets and
            // not the actual textures we've visited

            // kind of hacky way to get the thumbnails to be squares because getting sizes to match in
            // the UI code is quite complicated
            UI_Widget *computed_thumbnail = ui_get_widget_prev_frame("texture-thumbnail", 0);
            real32 thumbnail_height = 0.0f;
            if (computed_thumbnail) {
                thumbnail_height = computed_thumbnail->computed_size.x;
            }

            for (int32 i = 0; i < NUM_TEXTURE_BUCKETS; i++) {
                Texture *current = asset_manager->texture_table[i];
                while (current) {
                    if (current->type == Texture_Type::LEVEL) {
                        assert(num_textures_listed < MAX_TEXTURES);
                        texture_ids[num_textures_listed] = current->id;

                        if (num_textures_listed == 0 && asset_library_state->selected_texture_id < 0) {
                            // select the first one by default
                            asset_library_state->selected_texture_id = current->id;
                        }

                        char *texture_name = to_char_array((Allocator *) &ui_manager->frame_arena,
                                                        current->name);

                        bool32 is_selected = current->id == asset_library_state->selected_texture_id;

                        if (tiles_in_row == 0) {
                            if (!first_row && !row_widget) {
                                ui_y_pad(1.0f);
                            }
                            row_widget = ui_push_widget("", full_row_theme);
                        }

                        UI_Container_Theme theme = {};
                        UI_Widget *tile = ui_push_widget("texture-library-tile",
                                                         is_selected ? selected_texture_tile : texture_tile,
                                                         UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_IS_CLICKABLE,
                                                         num_textures_listed);
                        {
                            ui_push_padded_area({ 5.0f, 5.0f, 5.0f, 5.0f });
                            {
                                UI_Theme texture_thumbnail = {};
                                texture_thumbnail.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                                texture_thumbnail.texture_id = current->id;
                                texture_thumbnail.semantic_size.y = thumbnail_height;
                                ui_add_widget("texture-thumbnail", texture_thumbnail,
                                              UI_WIDGET_DRAW_BACKGROUND | UI_WIDGET_USE_TEXTURE,
                                              num_textures_listed);
                                ui_y_pad(5.0f);
                                do_text(texture_name);
                            }
                            ui_pop_widget();
                        } ui_pop_widget();

                        UI_Interact_Result interact = ui_interact(tile);
                        bool32 pressed = interact.released;

                        if (pressed || is_selected) {
                            asset_library_state->selected_texture_id = current->id;
                            selected_texture = current;
                        }

                        ui_x_pad(1.0f);

                        tiles_in_row++;

                        if (tiles_in_row == tiles_per_row) {
                            tiles_in_row = 0;
                            row_widget = NULL;
                            ui_pop_widget();
                            if (first_row) {
                                first_row = false;
                            }
                        }
                        
                        num_textures_listed++;
                    }

                    current = current->table_next;
                }
            }

            if (row_widget) {
                int32 num_fillers_needed = tiles_per_row - tiles_in_row;
                UI_Theme filler = {};
                filler.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                for (int32 i = 0; i < num_fillers_needed; i++) {
                    ui_add_widget("", filler);
                }
                ui_pop_widget();
            }
        }
        ui_pop_widget(); // asset-library-texture-list-container
        ui_pop_widget(); // scrollable region
    }
    ui_pop_widget();

    ui_x_pad(5.0f);

    ui_add_and_push_widget("asset-library-texture-info-container", properties_container_theme);
    {

        UI_Theme section_theme = {};
        section_theme.layout_type = UI_LAYOUT_VERTICAL;
        section_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

        ui_add_and_push_widget("", section_theme);
        {
            UI_Theme properties_header_theme = {};
            properties_header_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
            properties_header_theme.semantic_size = {  0.0f, editor_button_theme.size.y };
            properties_header_theme.layout_type = UI_LAYOUT_VERTICAL;
            ui_add_and_push_widget("", properties_header_theme); {
                do_y_centered_text("Properties");
            } ui_pop_widget();

            if (selected_texture) {
                ui_y_pad(5.0f);
                {
                    do_text("Texture Name");
                    UI_Text_Field_Theme field_theme = editor_text_field_theme;
                    field_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                    field_theme.size.x = 0.0f;
                    UI_Text_Field_Result name_result = do_text_field(field_theme, selected_texture->name,
                                                                     "texture_name_text_field",
                                                                     "texture_name_text_field_text");

                    if (name_result.committed) {
                        if (!string_equals(name_result.text, selected_texture->name) && name_result.text.length > 0) {
                            if (!texture_exists(name_result.text)) {
                                replace_contents(&selected_texture->name, name_result.text);
                            } else {
                                add_message(Context::message_manager, make_string("Texture name already exists!"));
                            }
                        }
                    }
                }

                ui_y_pad(5.0f);
                
                {
                    do_text("Filepath");
                    UI_Text_Field_Theme field_theme = editor_text_field_theme;
                    field_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                    field_theme.size.x = 0.0f;

                    ui_add_and_push_widget("", full_row_theme);
                    {
                        Allocator *temp_region = begin_region();

                        UI_Text_Field_Result filepath_result = do_text_field(field_theme, selected_texture->filename,
                                                                             "texture_filepath_text_field",
                                                                             "texture_filepath_text_field_text",
                                                                             PLATFORM_MAX_PATH - 1);

                        if (filepath_result.committed) {
                            if (filepath_result.text.length > 0) {
                                if (platform_file_exists(filepath_result.text)) {
                                    set_texture_file(selected_texture->id, filepath_result.text);
                                } else {
                                    add_message(Context::message_manager, make_string("File does not exist!"));
                                }
                            }
                        }

                        ui_x_pad(5.0f);

                        UI_Button_Theme browse_theme = editor_button_theme;
                        browse_theme.size_type.x = UI_SIZE_ABSOLUTE;
                        browse_theme.size.x = 80.0f;

                        bool32 browse_clicked = do_text_button("Browse", browse_theme, "texture_browse_button");
                        if (browse_clicked) {
                            char absolute_path[PLATFORM_MAX_PATH];
                            if (platform_open_file_dialog(absolute_path,
                                                          TEXTURE_FILE_FILTER_TITLE, TEXTURE_FILE_FILTER_TYPE,
                                                          PLATFORM_MAX_PATH)) {

                                char relative_path[PLATFORM_MAX_PATH];
                                platform_get_relative_path(absolute_path, relative_path, PLATFORM_MAX_PATH);

                                String relative_path_string = make_string(temp_region, relative_path);

                                set_texture_file(selected_texture->id, relative_path_string);
                            }
                        }

                        end_region(temp_region);
                    } ui_pop_widget();
                }
            } // if (selected_texture)
        } ui_pop_widget(); // section

        UI_Theme footer_theme = {};
        footer_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
        footer_theme.layout_type = UI_LAYOUT_HORIZONTAL;

        ui_add_and_push_widget("", footer_theme);
        {
            if (selected_texture) {
                UI_Theme push_right = {};
                push_right.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                ui_add_widget("", push_right);

                UI_Button_Theme delete_theme = editor_button_danger_theme;
                delete_theme.size_type.x = UI_SIZE_ABSOLUTE;
                delete_theme.size.x = 120.0f;
                
                bool32 delete_pressed = do_text_button("Delete Texture", delete_theme, "delete-texture");

                if (delete_pressed) {
                    // find texture id in list
                    int32 id_to_delete = selected_texture->id;

                    for (int32 i = 0; i < num_textures_listed; i++) {
                        if (texture_ids[i] == id_to_delete) {
                            // reset the thumbnail texture so the widget isn't using a deleted texture
                            // NOTE: if we ever delete textures from outside of the texture library, we
                            //       may need to rethink this..
                            UI_Widget *thumbnail = ui_get_widget("texture-thumbnail", i);
                            assert(thumbnail);
                            thumbnail->texture_id = 0;
                            
                            if (num_textures_listed >= 1) {
                                int32 new_index;
                                if (i == 0) {
                                    new_index = i + 1;
                                } else {
                                    new_index = max(i - 1, 0);
                                }
                                asset_library_state->selected_texture_id = texture_ids[new_index];
                            } else {
                                // we're deleting the last one
                                asset_library_state->selected_texture_id = -1;
                            }

                            break;
                        }
                    }

                    selected_texture = get_texture(asset_library_state->selected_texture_id);

                    delete_texture(id_to_delete);
                    num_textures_listed = max(num_textures_listed--, 0);
                }
            }
        } ui_pop_widget(); // footer
    } ui_pop_widget(); // asset-library-texture-info-container
}

UI_Button_Theme get_tab_theme(Asset_Library_Tab tab) {
    using namespace Asset_Library_Themes;
    
    UI_Button_Theme tab_theme = item_theme;
    tab_theme.size_type.x = UI_SIZE_FIT_CHILDREN;
    tab_theme.padding.x = 5.0f;

    UI_Button_Theme selected_tab_theme = selected_item_theme;
    selected_tab_theme.size_type.x = UI_SIZE_FIT_CHILDREN;
    selected_tab_theme.padding.x = 5.0f;
    
    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;
    UI_Button_Theme result = asset_library_state->selected_tab == tab ? selected_tab_theme : tab_theme;
    return result;
}

void draw_asset_library() {
    using namespace Asset_Library_Themes;
    
    UI_Window_Theme window_theme = DEFAULT_WINDOW_THEME;

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;
    
    // since DEFAULT_WINDOW_THEME is FIT_CHILDREN, this is a minimum size
    window_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
    window_theme.semantic_size = { 500.0f, 500.0f };

    list_container_theme.layout_type = UI_LAYOUT_VERTICAL;
    list_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_PERCENTAGE };
    list_container_theme.semantic_size = { 0.4f, 1.0f };

    properties_container_theme = list_container_theme;
    properties_container_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_PERCENTAGE };
    properties_container_theme.semantic_size = { 0.0f, 1.0f };
    properties_container_theme.background_color = rgb_to_vec4(255, 0, 0);
    
    list_theme.layout_type = UI_LAYOUT_VERTICAL;
    list_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
    list_theme.background_color = { 0.0f, 0.0f, 0.0f, 1.0f };
    list_theme.scissor_type = UI_SCISSOR_INHERIT;

    dropdown_theme.button_theme = editor_button_theme;
    dropdown_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
    dropdown_theme.size = { 1.0f, 0.0f };
    dropdown_theme.item_theme = editor_dropdown_item_theme;
    dropdown_theme.selected_item_theme = editor_selected_dropdown_item_theme;

    row_theme.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };
    row_theme.semantic_size = { 0.0f, 0.0f };
    row_theme.layout_type = UI_LAYOUT_HORIZONTAL;

    full_row_theme = row_theme;
    full_row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };

    space_between_row_theme = full_row_theme;
    space_between_row_theme.layout_type = UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN;

#if 0
    item_theme = editor_button_theme;
    item_theme.scissor_type = UI_SCISSOR_INHERIT;

    selected_item_theme = item_theme;
    selected_item_theme.background_color = rgb_to_vec4(61, 96, 252);
    selected_item_theme.hot_background_color = selected_item_theme.background_color;
    selected_item_theme.active_background_color = selected_item_theme.background_color;
#endif
    
    push_window("Asset Library", window_theme,
                "asset-library-window", "asset-library-window-title-bar");

    UI_Theme inner_column = {};
    inner_column.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
    inner_column.layout_type = UI_LAYOUT_VERTICAL;

    UI_Theme inner_row = {};
    inner_row.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };
    inner_row.layout_type = UI_LAYOUT_HORIZONTAL;
    
    ui_push_container(container_theme, "asset-library-content");
    {
        ui_add_and_push_widget("asset-library-inner", inner_column);
        {
            ui_add_and_push_widget("asset-library-tab-row", row_theme);
            {
                if (do_text_button("Materials", get_tab_theme(Asset_Library_Tab::MATERIALS),
                                   "asset-library-materials-tab")) {
                    asset_library_state->selected_tab = Asset_Library_Tab::MATERIALS;
                }
                ui_x_pad(1.0f);
                
                if (do_text_button("Meshes", get_tab_theme(Asset_Library_Tab::MESHES),
                                   "asset-library-meshes-tab")) {
                    asset_library_state->selected_tab = Asset_Library_Tab::MESHES;
                }
                ui_x_pad(1.0f);
                
                if (do_text_button("Textures", get_tab_theme(Asset_Library_Tab::TEXTURES),
                                   "asset-library-textures-tab")) {
                    asset_library_state->selected_tab = Asset_Library_Tab::TEXTURES;
                }
            } ui_pop_widget(); // asset-library-tab-row

            ui_y_pad(5.0f);
            
            ui_add_and_push_widget("asset-library-inner-inner", inner_row);
            {
                switch (asset_library_state->selected_tab) {
                    case Asset_Library_Tab::MATERIALS: {
                        draw_material_library();
                    } break;
                    case Asset_Library_Tab::MESHES: {
                        draw_mesh_library();
                    } break;
                    case Asset_Library_Tab::TEXTURES: {
                        draw_texture_library();
                    } break;
                    default: {
                        assert(!"Unhandled Asset_Library_State type!");
                    } break;
                }
            } ui_pop_widget(); // asset-library-inner-inner
        } ui_pop_widget(); // asset-library-inner
        //draw_material_library();
    } ui_pop_widget(); // asset-library-content-container

    ui_pop_widget(); // window

    // all the consumers of this have ran
    asset_library_state->material_modified = false;
}
