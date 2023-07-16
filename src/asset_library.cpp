#include "editor.h"

int32 get_selected_name_index(String name, char **names, int32 num_names) {
    for (int32 i = 0; i < num_names; i++) {
        if (string_equals(name, names[i])) {
            return i;
        }
    }

    return -1;
}

void draw_asset_library() {
    UI_Window_Theme window_theme = DEFAULT_WINDOW_THEME;

    // since DEFAULT_WINDOW_THEME is FIT_CHILDREN, this is a minimum size
    window_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
    window_theme.semantic_size = { 500.0f, 500.0f };
    
    push_window("Asset Library", window_theme,
                "asset-library-window", "asset-library-window-title-bar");

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;
    
    UI_Container_Theme content_theme = {
        { 5.0f, 5.0f, 5.0f, 5.0f },
        DEFAULT_BOX_BACKGROUND,
        UI_POSITION_NONE,
        {},
        { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING },
        {},
        UI_LAYOUT_HORIZONTAL
    };

    UI_Theme list_container_theme = NULL_THEME;
    list_container_theme.layout_type = UI_LAYOUT_VERTICAL;
    list_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_PERCENTAGE };
    list_container_theme.semantic_size = { 0.4f, 1.0f };

    UI_Theme properties_container_theme = list_container_theme;
    properties_container_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_PERCENTAGE };
    properties_container_theme.semantic_size = { 0.0f, 1.0f };
    properties_container_theme.background_color = rgb_to_vec4(255, 0, 0);
    //properties_container_theme.layout_type = UI_LAYOUT_VERTICAL_SPACE_BETWEEN;
    
    UI_Theme list_theme = NULL_THEME;
    list_theme.layout_type = UI_LAYOUT_VERTICAL;
    list_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
    list_theme.background_color = { 0.0f, 0.0f, 0.0f, 1.0f };
    list_theme.scissor_type = UI_SCISSOR_INHERIT;

    UI_Dropdown_Theme dropdown_theme = {};
    dropdown_theme.button_theme = editor_button_theme;
    dropdown_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
    dropdown_theme.size = { 1.0f, 0.0f };
    dropdown_theme.item_theme = editor_dropdown_item_theme;
    dropdown_theme.selected_item_theme = editor_selected_dropdown_item_theme;

    UI_Theme row_theme = {};
    row_theme.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };
    row_theme.semantic_size = { 0.0f, 0.0f };
    row_theme.layout_type = UI_LAYOUT_HORIZONTAL;

    UI_Theme full_row_theme = row_theme;
    full_row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
    full_row_theme.layout_type = UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN;

    const int32 MAX_MATERIALS = 256;
    int32 material_ids[MAX_MATERIALS];
    int32 num_materials_listed = 0;
    
    ui_push_container(content_theme, "asset-library-content");
    {
        Material *selected_material = NULL;
        
        ui_add_and_push_widget("asset-library-material-list-container", list_container_theme);
        {
            ui_add_and_push_widget("", full_row_theme);
            {
                do_y_centered_text("Materials");
                UI_Button_Theme add_button_theme = editor_button_theme;
                add_button_theme.size_type.x = UI_SIZE_ABSOLUTE;
                add_button_theme.size.x = 20.0f;

                bool32 add_material_clicked = do_text_button("+", add_button_theme, "add_material_button");
                if (add_material_clicked) {
                    Material_Info material_info = default_material_info;
                    Marker m = begin_region();
                    bool32 gen_result = generate_asset_name(temp_region, "Material", 256, &material_info.name);
                    assert(gen_result);
                    
                    Material *new_material = add_material(&material_info, Material_Type::LEVEL);
                    asset_library_state->selected_material_id = new_material->id;
                    
                    end_region(m);
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
                UI_Button_Theme item_theme = editor_button_theme;
                item_theme.scissor_type = UI_SCISSOR_INHERIT;

                UI_Button_Theme selected_item_theme = item_theme;
                selected_item_theme.background_color = rgb_to_vec4(61, 96, 252);
                selected_item_theme.hot_background_color = selected_item_theme.background_color;
                selected_item_theme.active_background_color = selected_item_theme.background_color;
                
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

        ui_x_pad(10.0f);

        ui_add_and_push_widget("asset-library-material-info-container", properties_container_theme);
        {

            UI_Theme section_theme = {};
            section_theme.layout_type = UI_LAYOUT_VERTICAL;
            section_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

            ui_add_and_push_widget("", section_theme);
            {
                do_text("Properties");

                if (selected_material) {
                    ui_y_pad(10.0f);
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
                            int32 selected_index = get_selected_name_index(selected_material->albedo_texture_name,
                                                                           texture_names, num_texture_names);
                            assert(selected_index >= 0);

                            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                        texture_names, num_texture_names,
                                                                        selected_index,
                                                                        "albedo_texture_dropdown");
                            if (dropdown_selected_index != selected_index) {
                                selected_index = dropdown_selected_index;
                                replace_contents(&selected_material->albedo_texture_name, texture_names[selected_index]);
                            }
                        } else {
                            UI_Interact_Result interact_result;
                            bool32 open_color_picker_pressed = do_text_button("Open Color Picker",
                                                                              editor_button_theme,
                                                                              "open-color-picker", 0,
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
                            int32 selected_index = get_selected_name_index(selected_material->metalness_texture_name,
                                                                           texture_names, num_texture_names);
                            assert(selected_index >= 0);
                            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                        texture_names, num_texture_names,
                                                                        selected_index,
                                                                        "metalness_texture_dropdown");
                            if (dropdown_selected_index != selected_index) {
                                selected_index = dropdown_selected_index;
                                replace_contents(&selected_material->metalness_texture_name, texture_names[selected_index]);
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
                            int32 selected_index = get_selected_name_index(selected_material->roughness_texture_name,
                                                                           texture_names, num_texture_names);
                            assert(selected_index >= 0);

                            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                        texture_names, num_texture_names,
                                                                        selected_index,
                                                                        "roughness_texture_dropdown");
                            if (dropdown_selected_index != selected_index) {
                                selected_index = dropdown_selected_index;
                                replace_contents(&selected_material->roughness_texture_name, texture_names[selected_index]);
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
    ui_pop_widget(); // container

    ui_pop_widget(); // window

    // all the consumers of this have ran
    asset_library_state->material_modified = false;
}
