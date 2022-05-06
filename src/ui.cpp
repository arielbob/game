#include "ui.h"
#include "string.h"

UI_Text_Style default_text_style = {
    make_vec4(1.0f, 1.0f, 1.0f, 1.0f),
    true,
    make_vec4(0.0f, 0.0f, 0.0f, 1.0f),
    0
};

UI_Text_Button_Style default_text_button_style = { TEXT_ALIGN_X | TEXT_ALIGN_Y,
                                                   rgb_to_vec4(33, 62, 69),
                                                   rgb_to_vec4(47, 84, 102),
                                                   rgb_to_vec4(19, 37, 46),
                                                   rgb_to_vec4(102, 102, 102) };

UI_Text_Button_Style default_text_button_cancel_style = { TEXT_ALIGN_X | TEXT_ALIGN_Y,
                                                          rgb_to_vec4(140, 38, 60),
                                                          rgb_to_vec4(199, 66, 103),
                                                          rgb_to_vec4(102, 22, 45),
                                                          rgb_to_vec4(102, 102, 102) };

UI_Text_Button_Style default_text_button_save_style = { TEXT_ALIGN_X | TEXT_ALIGN_Y,
                                                        rgb_to_vec4(37, 179, 80),
                                                        rgb_to_vec4(68, 201, 108),
                                                        rgb_to_vec4(31, 102, 70) };

UI_Slider_Style default_slider_style = { rgb_to_vec4(33, 62, 69),
                                         rgb_to_vec4(47, 84, 102),
                                         rgb_to_vec4(19, 37, 46),

                                         rgb_to_vec4(116, 116, 138),
                                         rgb_to_vec4(158, 158, 186),
                                         rgb_to_vec4(186, 45, 47) };

UI_Image_Button_Style default_image_button_style = { 5.0f, 5.0f, 15.0f,
                                                     CONSTRAINT_FILL_BUTTON_WIDTH | CONSTRAINT_FILL_BUTTON_HEIGHT,
                                                     rgb_to_vec4(33, 62, 69),
                                                     rgb_to_vec4(47, 84, 102),
                                                     rgb_to_vec4(19, 37, 46) };

UI_Color_Button_Style default_color_button_style = { 5.0f, 5.0f,
                                                     rgb_to_vec4(33, 62, 69),
                                                     rgb_to_vec4(47, 84, 102),
                                                     rgb_to_vec4(19, 37, 46) };

UI_Text_Box_Style default_text_box_style = { TEXT_ALIGN_Y,
                                             5.0f, 5.0f,
                                             rgb_to_vec4(33, 62, 69),
                                             rgb_to_vec4(47, 84, 102),
                                             rgb_to_vec4(33, 62, 69), };

bool32 char_is_in_array(char c, char array[], int32 num_chars) {
    for (int32 i = 0; i < num_chars; i++) {
        if (c == array[i]) {
            return true;
        } 
    }

    return false;
}

inline UI_id make_ui_id(UI_Type type, void *id, int32 index) {
    UI_id ui_id = { type, id, index };
    return ui_id;
}
          

                                   
UI_Slider make_ui_slider(real32 x, real32 y,
                         real32 width, real32 height,
                         String_Buffer buffer, char *font,
                         real32 min, real32 max, real32 value,
                         UI_Slider_Style style, UI_Text_Style text_style,
                         int32 layer, char *id, int32 index = 0) {
    UI_Slider slider;

    slider.type = UI_SLIDER;
    slider.layer = layer;

    slider.x = x;
    slider.y = y;
    slider.width = width;
    slider.height = height;
    slider.buffer = buffer;
    slider.font = font;
    slider.style = style;
    slider.text_style = text_style;
    slider.min = min;
    slider.max = max;
    slider.value = value;
    
    UI_id slider_id = make_ui_id(UI_SLIDER, id, index);
    slider.id = slider_id;

    return slider;
}

UI_Slider_State make_ui_slider_state(Allocator *string_allocator, char *text) {
    UI_Slider_State state;
    state.type = UI_Element_State_Type::SLIDER;
    state.buffer = make_string_buffer(string_allocator, make_string(text), 64);
    state.is_text_box = false;
    return state;
};

void deallocate(UI_Slider_State state) {
    delete_string_buffer(state.buffer);
}

// TODO: store UI element state in a hash table, so we can do things like fading transitions.
//       this requires some thought since we would like to remove elements from the hash table if
//       the element is no longer being displayed. we want this while retaining the nice
//       immediate mode API.

void *allocate(UI_Push_Buffer *push_buffer, uint32 size, uint32 alignment_bytes = 8) {
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0);

    uint8 *current_pointer = (uint8 *) push_buffer->base + push_buffer->used;

    uint32 align_mask = alignment_bytes - 1;
    uint32 misalignment = ((uint64) current_pointer) & align_mask;
    uint32 align_offset = 0;
    // NOTE: unlike our other allocation procedures, we do NOT check if misalignment = 0. so, the range of
    //       extra bytes added, when alignment_bytes=8, is between 1 and 8. we do it this way here because
    //       when iterating through the push_buffer, to actually get the data at some address, we need to know
    //       the alignment offset. we store the alignment offset at the beginning of some allocated memory,
    //       since when we're iterating, we usually end up at the start of some allocated block or at the end
    //       of some allocated block. so we can easily just read whatever is written in that single byte to know
    //       how much we need to move up by to get the data.
    align_offset = alignment_bytes - misalignment;
    
    size += align_offset;
    assert((push_buffer->used + size) <= push_buffer->size);

    // store the offset in the space used to offset the allocation
    // only have a single byte to store the alignment offset, so make sure the offset can fit in a single byte,
    // although, i don't see us ever passing in alignment_bytes greater than 32, which would have a max
    // align_offset of 32.
    assert(align_offset < 256); 
    *current_pointer = (uint8) align_offset;

    void *start_byte = (void *) (current_pointer + align_offset);
    push_buffer->used += size;

    return start_byte;
}

inline real32 get_adjusted_font_height(Font font) {
    return font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
}

inline real32 get_center_x_offset(real32 container_width, real32 element_width) {
    return (container_width / 2.0f - element_width / 2.0f);
}

// this is different from get_center_y_offset because text is drawn from the bottom left corner and goes up
// i.e. it's drawn going up from its baseline, instead of like for quads from the top left going down
inline real32 get_center_baseline_offset(real32 container_height, real32 text_height) {
    return 0.5f * (container_height + text_height);
}

inline real32 get_center_y_offset(real32 height, real32 box_height) {
    return 0.5f * (height - box_height);
}
    
void clear_push_buffer(UI_Push_Buffer *buffer) {
    buffer->used = 0;
    buffer->first = NULL;
}

void ui_add_text_button(UI_Manager *manager, UI_Text_Button button) {
    UI_Text_Button *element = (UI_Text_Button *) allocate(&manager->push_buffer, sizeof(UI_Text_Button));
    *element = button;
}

void ui_add_image_button(UI_Manager *manager, UI_Image_Button button) {
    UI_Image_Button *element = (UI_Image_Button *) allocate(&manager->push_buffer, sizeof(UI_Image_Button));
    *element = button;
}

void ui_add_color_button(UI_Manager *manager, UI_Color_Button button) {
    UI_Color_Button *element = (UI_Color_Button *) allocate(&manager->push_buffer, sizeof(UI_Color_Button));
    *element = button;
}

void ui_add_text(UI_Manager *manager, UI_Text text) {
    UI_Text *element = (UI_Text *) allocate(&manager->push_buffer, sizeof(UI_Text));
    *element = text;
}

void ui_add_text_box(UI_Manager *manager, UI_Text_Box text_box) {
    UI_Text_Box *element = (UI_Text_Box *) allocate(&manager->push_buffer, sizeof(UI_Text_Box));
    *element = text_box;
}

void ui_add_slider(UI_Manager *manager, UI_Slider slider) {
    UI_Slider *element = (UI_Slider *) allocate(&manager->push_buffer, sizeof(UI_Slider));
    *element = slider;
}

void ui_add_box(UI_Manager *manager, UI_Box box) {
    UI_Box *element = (UI_Box *) allocate(&manager->push_buffer, sizeof(UI_Box));
    *element = box;
}

void ui_add_line(UI_Manager *manager, UI_Line line) {
    UI_Line *element = (UI_Line *) allocate(&manager->push_buffer, sizeof(UI_Line));
    *element = line;
}

void ui_add_hue_slider(UI_Manager *manager, UI_Hue_Slider hue_slider) {
    UI_Hue_Slider *element = (UI_Hue_Slider *) allocate(&manager->push_buffer, sizeof(UI_Hue_Slider));
    *element = hue_slider;
}

void ui_add_hsv_picker(UI_Manager *manager, UI_HSV_Picker hsv_picker) {
    UI_HSV_Picker *element = (UI_HSV_Picker *) allocate(&manager->push_buffer, sizeof(UI_HSV_Picker));
    *element = hsv_picker;
}

void ui_add_color_picker(UI_Manager *manager, UI_Color_Picker color_picker) {
    UI_Color_Picker *element = (UI_Color_Picker *) allocate(&manager->push_buffer, sizeof(UI_Color_Picker));
    *element = color_picker;
}

inline bool32 ui_id_is_empty(UI_id id) {
    return ((id.string_ptr == NULL) && (id.index == 0));
}

inline bool32 ui_id_equals(UI_id id1, UI_id id2) {
    return ((id1.string_ptr == id2.string_ptr) && (id1.index == id2.index));
}

uint32 get_hash(UI_id id, uint32 bucket_size) {
    String_Iterator it = make_string_iterator(make_string((char *) id.string_ptr));
    uint32 sum = 0;
    char c = get_next_char(&it);
    while (c) {
        sum += c;
        c = get_next_char(&it);
    }
    sum += id.index;

    uint32 hash = sum % bucket_size;
    
    return hash;
}

UI_Element_State *get_state(UI_Manager *manager, UI_id id) {
    UI_Element_State *result;
    bool32 state_exists = hash_table_find<UI_id, UI_Element_State *>(manager->state_table, id, &result);

    if (!state_exists) return NULL;

    return result;
};

bool32 get_state(UI_Manager *manager, UI_id id, UI_Element_State **result) {
    bool32 state_exists = hash_table_find<UI_id, UI_Element_State *>(manager->state_table, id, result);
    return state_exists;
};

void add_state(UI_Manager *manager, UI_id id, UI_Element_State *state) {
    hash_table_add(&manager->state_table, id, state);
}

bool32 in_bounds(Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max) {
    return (p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max);
}

inline bool32 in_bounds_on_layer(UI_Manager *manager, Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max) {
    return ((manager->current_layer >= manager->hot_layer) &&
            in_bounds(p, x_min, x_max, y_min, y_max));
}

inline bool32 ui_has_hot(UI_Manager *manager) {
    return (manager->hot.string_ptr != NULL);
}

inline bool32 ui_has_active(UI_Manager *manager) {
    return (manager->active.string_ptr != NULL);
}

void disable_input(UI_Manager *manager) {
    manager->hot = { NULL };
    manager->active = { NULL };
    manager->is_disabled = true;
}

void enable_input(UI_Manager *manager) {
    manager->is_disabled = false;
}

bool32 has_focus(UI_Manager *manager) {
    return (manager->active.type == UI_TEXT_BOX);
}

inline void push_layer(UI_Manager *manager) {
    manager->current_layer++;
}

inline void pop_layer(UI_Manager *manager) {
    manager->current_layer--;
}

inline void set_hot(UI_Manager *manager, UI_id hot) {
    manager->hot = hot;
    manager->hot_layer = manager->current_layer;
}

inline void clear_hot(UI_Manager *manager) {
    manager->hot = {};
    manager->hot_layer = 0;
}

UI_Element *next_element(UI_Element *current_element, UI_Push_Buffer *push_buffer) {
    if (!current_element) {
        if (push_buffer->used > 0) {
            uint8 byte_offset = *((uint8 *) push_buffer->base);
            uint8 *first_element_address = (uint8 *) push_buffer->base + byte_offset;
            return (UI_Element *) first_element_address;
        } else {
            return NULL;
        }
    }

    uint8 *address = (uint8 *) current_element;
    switch (current_element->type) {
        case UI_TEXT: {
            address += sizeof(UI_Text);
        } break;
        case UI_TEXT_BUTTON: {
            address += sizeof(UI_Text_Button);
        } break;
        case UI_IMAGE_BUTTON: {
            address += sizeof(UI_Image_Button);
        } break;
        case UI_COLOR_BUTTON: {
            address += sizeof(UI_Color_Button);
        } break;
        case UI_TEXT_BOX: {
            address += sizeof(UI_Text_Box);
        } break;
        case UI_SLIDER: {
            address += sizeof(UI_Slider);
        } break;
        case UI_BOX: {
            address += sizeof(UI_Box);
        } break;
        case UI_LINE: {
            address += sizeof(UI_Line);
        } break;
        case UI_HUE_SLIDER: {
            address += sizeof(UI_Hue_Slider);
        } break;
        case UI_HSV_PICKER: {
            address += sizeof(UI_HSV_Picker);
        } break;
        case UI_COLOR_PICKER: {
            address += sizeof(UI_Color_Picker);
        } break;
        default: {
            assert(!"Unhandled UI element type.");
        }
    }

    if (address < ((uint8 *) push_buffer->base + push_buffer->used)) {
        // NOTE: we don't have to check again if this address + byte_offset is within the bounds, since
        //       the allocator doesn't allocate (and thus does not increase push_buffer->used)if there's
        //       not enough space for both the offset bytes and the allocation size.
        uint8 byte_offset = *address;
        return (UI_Element *) (address + byte_offset);
    } else {
        return NULL;
    }
}

void deallocate(UI_Element_State *untyped_state) {
    switch (untyped_state->type) {
        case UI_Element_State_Type::NONE: {
            assert(!"UI_Element_State does not have a type.");
        } break;
        case UI_Element_State_Type::SLIDER: {
            UI_Slider_State *state = (UI_Slider_State *) untyped_state;
            deallocate(*state);
        } break;
        default: {
            UI_Text_Box_State *state = (UI_Text_Box_State *) untyped_state;
            deallocate(*state);
        } break;
    }

    heap_deallocate(Context::ui_manager->heap_pointer, untyped_state);
}

// NOTE: since in immediate-mode GUIs, we only update hot if we call the do_ procedure for some element,
//       if the element disappears, we lose the ability to change hot based on that element, for example,
//       setting hot to empty if we're not over a button. this procedure loops through all the elements
//       that were added during the frame and checks if the current hot element was added. if it was not,
//       that element is gone and thus if hot is that element, it should be cleared.
void clear_hot_if_gone(UI_Manager *manager) {
    UI_Push_Buffer *push_buffer = &manager->push_buffer;
    UI_Element *element = next_element(NULL, push_buffer);

    while (element) {
        if (ui_id_equals(manager->hot, element->id)) {
            // hot still exists
            return;
        }

        element = next_element(element, push_buffer);
    }

    // hot is gone
    clear_hot(manager);
}

void clear_editor_state_for_gone_color_pickers(UI_Manager *manager, Editor_State *editor_state) {
    UI_Push_Buffer *push_buffer = &manager->push_buffer;
    UI_Element *element = next_element(NULL, push_buffer);

    UI_id color_picker_parent_id = editor_state->color_picker.parent_ui_id;
    if (ui_id_is_empty(color_picker_parent_id)) return;

    while (element) {
        if (ui_id_equals(element->id, color_picker_parent_id)) {
            // the parent's still there
            return;
        }
        element = next_element(element, push_buffer);
    }

    // we didn't find the parent, so it's gone this frame, so reset the color picker parent id
    editor_state->color_picker.parent_ui_id = {};
}

void delete_state_if_gone(UI_Manager *manager) {
    UI_Push_Buffer *push_buffer = &manager->push_buffer;
    UI_Element *element = next_element(NULL, push_buffer);

    Hash_Table<UI_id, UI_Element_State*> *state_table = &manager->state_table;

    // loop through all the state entries and look if they're in the UI push buffer this frame.
    // if they aren't, then the element is gone and its state should be deleted from state_table.
    for (int32 i = 0; i < state_table->max_entries; i++) {
        Hash_Table_Entry<UI_id, UI_Element_State*> *entry = &state_table->entries[i];
        if (!entry->is_occupied) continue;

        bool32 exists_this_frame = false;
        while (element) {
            if (ui_id_equals(entry->key, element->id)) {
                // the corresponding element for this state entry exists
                exists_this_frame = true;
                break;
            }
            element = next_element(element, push_buffer);
        }

        if (!exists_this_frame) {
            // couldn't find the element for this state entry, so delete the state entry
            hash_table_remove(state_table, entry->key);
        }

        element = next_element(NULL, push_buffer);
    }
}

void do_text(UI_Manager *manager,
             real32 x_px, real32 y_px,
             char *text, char *font, char *id_string, int32 index = 0) {
    UI_Text ui_text = make_ui_text(x_px, y_px,
                                   text, font, default_text_style, manager->current_layer, id_string, index);

    bool32 was_clicked = false;

    ui_add_text(manager, ui_text);
}

void do_text(UI_Manager *manager,
             real32 x_px, real32 y_px,
             char *text, char *font, 
             UI_Text_Style style,
             char *id_string, int32 index = 0) {
    UI_Text ui_text = make_ui_text(x_px, y_px,
                                   text, font, style, manager->current_layer, id_string, index);

    bool32 was_clicked = false;

    ui_add_text(manager, ui_text);
}

bool32 do_text_button(real32 x_px, real32 y_px,
                      real32 width, real32 height,
                      UI_Text_Button_Style style, UI_Text_Style text_style,
                      char *text, char *font, bool32 is_disabled, char *id_string, int32 index = 0) {
    using namespace Context;
    UI_Text_Button button = make_ui_text_button(x_px, y_px, width, height,
                                                style, text_style,
                                                text, font, is_disabled,
                                                ui_manager->current_layer, id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;

    if (!ui_manager->is_disabled &&
        !is_disabled &&
        in_bounds_on_layer(ui_manager, current_mouse, x_px, x_px + width, y_px, y_px + height)) {
        // NOTE: ui state is modified in sequence that the immediate mode calls are done. this is why we have to always
        //       set hot again. if we didn't have this, if we drew a box, then drew a button on top of it, and then moved
        //       our cursor over top of the button, hot would be the box and NOT the button. which is not desired.
        set_hot(ui_manager, button.id);
        
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            // we check for !was_down to avoid setting a button active if we click and hold outside then
            // move into the button
            ui_manager->active = button.id;
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            if (ui_id_equals(ui_manager->active, button.id)) {
                was_clicked = true;
                ui_manager->active = {};
                debug_print("%s was clicked\n", button.id);
            }
        }
    } else {
        if (ui_id_equals(ui_manager->hot, button.id)) {
            clear_hot(ui_manager);
        }

        if (ui_id_equals(ui_manager->active, button.id) && !controller_state->left_mouse.is_down) {
            ui_manager->active = {};
        }
    }

    ui_add_text_button(ui_manager, button);

    return was_clicked;
}

inline bool32 do_text_button(real32 x_px, real32 y_px,
                      real32 width, real32 height,
                      UI_Text_Button_Style style, UI_Text_Style text_style,
                      char *text, char *font, char *id_string, int32 index = 0) {
    return do_text_button(x_px, y_px,
                          width, height,
                          style, text_style,
                          text, font, false, id_string, index);
}

bool32 do_image_button(real32 x_px, real32 y_px,
                       real32 width, real32 height,
                       UI_Image_Button_Style style,
                       UI_Text_Style text_style,
                       int32 texture_id, char *text, char *font,
                       char *id_string, int32 index = 0) {
    using namespace Context;
    UI_Image_Button button = make_ui_image_button(x_px, y_px, width, height,
                                                  style, text_style,
                                                  texture_id, text, font,
                                                  ui_manager->current_layer,
                                                  id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!ui_manager->is_disabled &&
        in_bounds_on_layer(ui_manager, current_mouse,
                           x_px, x_px + width, y_px, y_px + height + style.footer_height)) {
        set_hot(ui_manager, button.id);
        
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            ui_manager->active = button.id;
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            if (ui_id_equals(ui_manager->active, button.id)) {
                was_clicked = true;
                ui_manager->active = {};
                debug_print("%s was clicked\n", button.id);
            }
        }
    } else {
        if (ui_id_equals(ui_manager->hot, button.id)) {
            clear_hot(ui_manager);
        }

        if (ui_id_equals(ui_manager->active, button.id) && !controller_state->left_mouse.is_down) {
            ui_manager->active = {};
        }
    }

    ui_add_image_button(ui_manager, button);

    return was_clicked;
}

bool32 do_color_button(real32 x_px, real32 y_px,
                       real32 width, real32 height,
                       UI_Color_Button_Style style,
                       Vec4 color,
                       char *id_string, int32 index = 0,
                       UI_id *result_id = NULL) {
    using namespace Context;
    UI_Color_Button button = make_ui_color_button(x_px, y_px, width, height,
                                                  style,
                                                  color,
                                                  ui_manager->current_layer, id_string, index);

    bool32 was_clicked = false;

    if (result_id) *result_id = button.id;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!ui_manager->is_disabled &&
        in_bounds_on_layer(ui_manager, current_mouse, x_px, x_px + width, y_px, y_px + height)) {
        set_hot(ui_manager, button.id);
        
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            ui_manager->active = button.id;
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            if (ui_id_equals(ui_manager->active, button.id)) {
                was_clicked = true;
                ui_manager->active = {};
                debug_print("%s was clicked\n", button.id);
            }
        }
    } else {
        if (ui_id_equals(ui_manager->hot, button.id)) {
            clear_hot(ui_manager);
        }

        if (ui_id_equals(ui_manager->active, button.id) && !controller_state->left_mouse.is_down) {
            ui_manager->active = {};
        }
    }

    ui_add_color_button(ui_manager, button);

    return was_clicked;
}

// TODO: this should return a bool32, since we want to be able to submit fields by pressing the enter key
// TODO: this may be messed up since we changed width/height to include padding
#if 0
// TODO: this should return a bool32, since we want to be able to submit fields by pressing the enter key
// TODO: this may be messed up since we changed width/height to include padding
void do_text_box(UI_Manager *manager, Controller_State *controller_state,
                 real32 x, real32 y,
                 real32 width, real32 height,
                 char *current_text, int32 text_buffer_size,
                 UI_Text_Box_Style style,
                 char *id_string, int32 index = 0) {
    UI_Text_Box text_box =  make_ui_text_box(x, y, width, height,
                                             current_text, text_buffer_size,
                                             style,
                                             id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x, x + width, y, y + height)) {
        if (!controller_state->left_mouse.is_down) {
            set_hot(manager, text_box.id);
        }
        

        if (ui_id_equals(manager->hot, text_box.id) &&
            controller_state->left_mouse.is_down) {
            manager->active = text_box.id;
            manager->focus_timer = platform_get_wall_clock_time();
            manager->focus_cursor_index = string_length(text_box.current_text);
        }
    } else {
        // FIXME: this isn't really a bug (other programs seem to behave this way), but i'm not sure
        //        why it's happening: if you hold down a key in the textbox and you click outside of the textbox,
        //        the textbox should lose focus and text should no longer be inputted. but, for some reason it
        //        keeps focus after clicking outside of the textbox and characters keep getting inputted.
        //        actually, i think it's because we're doing while(PeekMessage...).
        if (ui_id_equals(manager->hot, text_box.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, text_box.id) &&
            controller_state->left_mouse.is_down &&
            !controller_state->left_mouse.was_down) {
            manager->active = {};
            manager->focus_cursor_index = 0;
        }
    }

    if (ui_id_equals(manager->active, text_box.id)) {
        for (int32 i = 0; i < controller_state->num_pressed_chars; i++) {
            char c = controller_state->pressed_chars[i];
            if (c == '\b') {
                manager->focus_cursor_index--;
                if (manager->focus_cursor_index < 0) {
                    manager->focus_cursor_index = 0;
                }
                current_text[manager->focus_cursor_index] = '\0';
            } else if (manager->focus_cursor_index < (text_box.size - 1) &&
                       c >= 32 &&
                       c <= 126) {
                current_text[manager->focus_cursor_index] = c;
                current_text[manager->focus_cursor_index + 1] = '\0';
                manager->focus_cursor_index++;
            }
        }
    }

    ui_add_text_box(manager, text_box);
}
#endif

#if 0
void do_text_box_with_state(real32 x, real32 y,
                            real32 width, real32 height,
                            String_Buffer initial_text,
                            char *font,
                            UI_Text_Box_Style style, UI_Text_Style text_style,
                            char *id_string, int32 index = 0) {
    using namespace Context;

    
}
#endif

UI_Slider_State *add_ui_slider_state(UI_Manager *manager, UI_id id) {
    UI_Slider_State *state = (UI_Slider_State *) heap_allocate(manager->heap_pointer, sizeof(UI_Slider_State));
    hash_table_add(&manager->state_table, id, (UI_Element_State *) state);
    return state;
}

UI_Text_Box_State *add_ui_text_box_state(UI_Manager *manager, UI_id id) {
    UI_Text_Box_State *state = (UI_Text_Box_State *) heap_allocate(manager->heap_pointer, sizeof(UI_Text_Box_State));
    hash_table_add(&manager->state_table, id, (UI_Element_State *) state);
    return state;
}

bool32 do_text_box_logic(UI_Manager *ui_manager, Controller_State *controller_state, UI_id id,
                         real32 x, real32 y, real32 width, real32 height,
                         String_Buffer *buffer) {
    Vec2 current_mouse = controller_state->current_mouse;
    bool32 submitted = false;

    if (!ui_manager->is_disabled && in_bounds_on_layer(ui_manager, current_mouse, x, x + width, y, y + height)) {
        set_hot(ui_manager, id);

        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            ui_manager->active = id;
            ui_manager->focus_timer = platform_get_wall_clock_time();
            ui_manager->focus_cursor_index = buffer->current_length;
        }
    } else {
        // FIXME: this isn't really a bug (other programs seem to behave this way), but i'm not sure
        //        why it's happening: if you hold down a key in the textbox and you click outside of the textbox,
        //        the textbox should lose focus and text should no longer be inputted. but, for some reason it
        //        keeps focus after clicking outside of the textbox and characters keep getting inputted.
        //        actually, i think it's because we're doing while(PeekMessage...).
        if (ui_id_equals(ui_manager->hot, id)) {
            clear_hot(ui_manager);
        }

        if (ui_id_equals(ui_manager->active, id) &&
            controller_state->left_mouse.is_down &&
            !controller_state->left_mouse.was_down) {
            ui_manager->active = {};
            ui_manager->focus_cursor_index = 0;
            submitted = true;
        }
    }

    if (ui_id_equals(ui_manager->active, id)) {
        for (int32 i = 0; i < controller_state->num_pressed_chars; i++) {
            char c = controller_state->pressed_chars[i];
            if (c == '\b') { // backspace key
                ui_manager->focus_cursor_index--;
                if (ui_manager->focus_cursor_index < 0) {
                    ui_manager->focus_cursor_index = 0;
                }
                buffer->current_length = ui_manager->focus_cursor_index;
            } else if (ui_manager->focus_cursor_index < buffer->size &&
                       c >= 32 &&
                       c <= 126) {
                buffer->contents[ui_manager->focus_cursor_index] = c;
                ui_manager->focus_cursor_index++;
                buffer->current_length++;
            } else if (c == '\r') { // enter key
                submitted = true;
                if (ui_id_equals(ui_manager->active, id)) {
                    ui_manager->active = {};
                }
            }
        }
    }

    return submitted;
}

// NOTE: this returns the state's buffer if use_state = true, otherwise, it returns the passed in buffer.
//       
UI_Text_Box_Result do_text_box(real32 x, real32 y,
                               real32 width, real32 height,
                               String_Buffer *buffer,
                               char *font,
                               UI_Text_Box_Style style, UI_Text_Style text_style,
                               bool32 use_state, bool32 reset_state,
                               char *id_string, int32 index = 0) {
    using namespace Context;

    UI_id text_box_id = make_ui_id(UI_TEXT_BOX, id_string, index);
    UI_Text_Box_State *state = (UI_Text_Box_State *) get_state(ui_manager, text_box_id);
    if (state) {
        // use_state should not change over the lifetime of the text box
        assert(use_state);
    }

    if (use_state) {
        if (!state) {
            UI_Text_Box_State *new_state = add_ui_text_box_state(ui_manager, text_box_id);
            *new_state = make_ui_text_box_state((Allocator *) &memory.string64_pool,
                                                make_string(*buffer), buffer->size);
            state = new_state;
        }

        if (reset_state) {
            copy_string(&state->buffer, buffer);
        }

        buffer = &state->buffer;
    }

    UI_Text_Box text_box =  make_ui_text_box(x, y, width, height,
                                             *buffer,
                                             font,
                                             style, text_style,
                                             ui_manager->current_layer, id_string, index);

    bool32 submitted = do_text_box_logic(ui_manager, controller_state, text_box.id, x, y, width, height, buffer);

    ui_add_text_box(ui_manager, text_box);

    UI_Text_Box_Result result = {};
    result.submitted = submitted;
    result.buffer = *buffer;

    return result;
}

UI_Text_Box_Result do_text_box(real32 x, real32 y,
                               real32 width, real32 height,
                               String_Buffer *buffer,
                               char *font,
                               UI_Text_Box_Style style, UI_Text_Style text_style,
                               bool32 use_state,
                               char *id_string, int32 index = 0) {
    return do_text_box(x, y,
                       width, height,
                       buffer,
                       font,
                       style, text_style,
                       use_state, false,
                       id_string, index);
}

UI_Text_Box_Result do_text_box(real32 x, real32 y,
                               real32 width, real32 height,
                               String_Buffer *buffer,
                               char *font,
                               UI_Text_Box_Style style, UI_Text_Style text_style,
                               char *id_string, int32 index = 0) {
    return do_text_box(x, y,
                       width, height,
                       buffer,
                       font,
                       style, text_style,
                       false, false,
                       id_string, index);
}

real32 do_slider(real32 x, real32 y,
                 real32 width, real32 height,
                 char *font,
                 real32 min, real32 max, real32 value,
                 UI_Slider_Style style, UI_Text_Style text_style,
                 char *id_string, int32 index = 0) {
    using namespace Context;

    UI_id slider_id = make_ui_id(UI_SLIDER, id_string, index);

    UI_Slider_State *state = (UI_Slider_State *) get_state(ui_manager, slider_id);
    if (!state) {
        UI_Slider_State *new_state = add_ui_slider_state(ui_manager, slider_id);
        Marker m = begin_region();
        char *buf = string_format((Allocator *) &memory.global_stack, 64, "%f", value);
        *new_state = make_ui_slider_state((Allocator *) &memory.string64_pool, buf);
        end_region(m);
        state = new_state;
    }

    UI_Slider slider = make_ui_slider(x, y, width, height,
                                      state->buffer, font,
                                      min, max, value,
                                      style, text_style,
                                      ui_manager->current_layer, id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;

    if (!state->is_text_box) {
        if (!ui_manager->is_disabled && in_bounds_on_layer(ui_manager, current_mouse, x, x + width, y, y + height)) {
            set_hot(ui_manager, slider.id);

            if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
                ui_manager->active = slider.id;
                ui_manager->active_initial_position = controller_state->current_mouse;
                ui_manager->active_initial_time = platform_get_wall_clock_time();
            }

            if (!controller_state->left_mouse.is_down) {
                if (ui_id_equals(ui_manager->active, slider.id)) {
                    ui_manager->active = {};
                }

                real32 deadzone_time = 0.5;
                real32 time_since_first_active = (real32) (platform_get_wall_clock_time() - ui_manager->active_initial_time);

                if (time_since_first_active < deadzone_time) {
                    state->is_text_box = true;
                    ui_manager->active = slider.id;
                } else {
                    ui_manager->active = {};
                }
            }
        } else {
            if (ui_id_equals(ui_manager->hot, slider.id)) {
                clear_hot(ui_manager);
            }

            if (ui_id_equals(ui_manager->active, slider.id) && !controller_state->left_mouse.is_down) {
                ui_manager->active = {};
            }
        }

        if (ui_id_equals(ui_manager->active, slider.id) && being_held(controller_state->left_mouse)) {
#if 0
            real32 pixel_deadzone_radius = 5.0f;
            real64 deadzone_time = 0.5;
            real32 time_since_first_active = platform_get_wall_clock_time() - ui_manager->active_initial_time;
            if (time_since_first_active >= deadzone_time || delta_pixels > pixel_deadzone_radius) {
                value += delta_pixels * rate;
                value = min(max, value);
                value = max(min, value);
            }
#endif
            real32 delta_pixels = (controller_state->current_mouse - controller_state->last_mouse).x;
            real32 rate = (max - min) / width;

            value += delta_pixels * rate;
            value = min(max, value);
            value = max(min, value);
        }
    } else {

    }
    
    Marker m = begin_region();
    char *buf = string_format((Allocator *) &memory.global_stack, 64, "%f", value);
    set_string_buffer_text(&state->buffer, buf);
    end_region(m);

    ui_add_slider(ui_manager, slider);

    return value;
}

// we keep border_flags out of UI_Box_Style, since boxes can often have the same style, but have different
// borders showing. this is the same reasoning behind keeping width and height out of UI_Box_Style.
void do_box(real32 x, real32 y,
            real32 width, real32 height,
            UI_Box_Style style,
            uint32 border_flags,
            char *id_string, int32 index = 0) {
    using namespace Context;
    UI_Box box = make_ui_box(x, y, width, height,
                             style, border_flags,
                             ui_manager->current_layer, id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!ui_manager->is_disabled && in_bounds_on_layer(ui_manager, current_mouse,
                                                       x, x + width,
                                                       y, y + height)) {
        set_hot(ui_manager, box.id);
    } else {
        if (ui_id_equals(ui_manager->hot, box.id)) {
            clear_hot(ui_manager);
        }
    }

    ui_add_box(ui_manager, box);
}

inline void do_box(real32 x, real32 y,
                   real32 width, real32 height,
                   UI_Box_Style style,
                   char *id_string, int32 index = 0) {
    return do_box(x, y,
                  width, height,
                  style, 0,
                  id_string, index);
}

void do_line(UI_Manager *manager,
             Vec2 start_pixels, Vec2 end_pixels,
             UI_Line_Style style,
             char *id_string, int32 index = 0) {
    UI_Line line = make_ui_line(start_pixels, end_pixels,
                                style,
                                manager->current_layer, id_string, index);

    // right now, we only draw lines on top of other UI elements, so we don't check if the mouse is over
    // we don't even draw lines in the UI anymore; they're too finnicky

    ui_add_line(manager, line);
}

real32 do_hue_slider(real32 x, real32 y,
                    real32 width, real32 height,
                    real32 hue_degrees,
                    char *id_string) {
    using namespace Context;
    UI_Manager *manager = ui_manager;
    UI_Hue_Slider slider = make_ui_hue_slider(x, y, width, height, hue_degrees,
                                              manager->current_layer, id_string);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x, x + width, y, y + height)) {
        set_hot(manager, slider.id);

        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            manager->active = slider.id;
            hue_degrees = 360.0f - 360.0f * ((current_mouse.y - y) / height);
            hue_degrees = min(hue_degrees, 360.0f);
            hue_degrees = max(hue_degrees, 0.0f);
            manager->active_initial_position = controller_state->current_mouse;
            //manager->active_initial_time = platform_get_wall_clock_time();
        }

        if (!controller_state->left_mouse.is_down) {
            if (ui_id_equals(manager->active, slider.id)) {
                manager->active = {};
            }
        }
    } else {
        if (ui_id_equals(manager->hot, slider.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, slider.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    if (ui_id_equals(manager->active, slider.id) && being_held(controller_state->left_mouse)) {
        hue_degrees = 360.0f - 360.0f * ((current_mouse.y - slider.y) / slider.height);
        hue_degrees = min(hue_degrees, 360.0f);
        hue_degrees = max(hue_degrees, 0.0f);
    }

    ui_add_hue_slider(manager, slider);

    return hue_degrees;
}

// NOTE: we only use this procedure for initializing the picker's cursor position since HSV_Color uses ints
//       and as a result the returned cursor position is not very fluid when moving the cursor.
Vec2 hsv_to_cursor_position_inside_quad(HSV_Color hsv_color,
                                        real32 width, real32 height) {
    real32 x_percentage = (real32) hsv_color.s / 100.0f;
    real32 y_percentage = 1.0f - ((real32) hsv_color.v / 100.0f);

    real32 cursor_x = x_percentage*width;
    real32 cursor_y = y_percentage*height;

    return make_vec2(cursor_x, cursor_y);
}

HSV_Color get_hsv_inside_quad(Vec2 current_mouse, real32 hue,
                              real32 x, real32 y,
                              real32 width, real32 height) {
    assert(hue >= 0.0f && hue <= 360.0f);
    real32 x_percentage = (current_mouse.x - x) / width;
    real32 y_percentage = 1.0f - (current_mouse.y - y) / height; // bottom = 0.0, top = 1.0
    real32 saturation = x_percentage * 100.0f;
    real32 value = y_percentage * 100.0f;

    hue = clamp(hue, 0.0f, 360.0f);
    saturation = clamp(saturation, 0.0f, 100.0f);
    value = clamp(value, 0.0f, 100.0f);
            
    HSV_Color result = { hue, saturation, value };
    return result;
}

UI_HSV_Picker_State do_hsv_picker(real32 x, real32 y,
                                  real32 width, real32 height,
                                  UI_HSV_Picker_State state,
                                  char *id_string) {
    using namespace Context;
    Vec2 current_mouse = controller_state->current_mouse;
    UI_Manager *manager = ui_manager;

    // TODO: is it possible to move this to the end so that we don't have a frame of lag?
    UI_HSV_Picker picker = make_ui_hsv_picker(x, y,
                                              width, height,
                                              state,
                                              manager->current_layer, id_string);

    real32 relative_x = current_mouse.x - x;
    real32 relative_y = current_mouse.y - y;

    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x, x + width, y, y + height)) {
        set_hot(manager, picker.id);

        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            manager->active = picker.id;
            state.hsv_color = get_hsv_inside_quad(current_mouse, state.hsv_color.h, x, y, width, height);
            state.relative_cursor_x = clamp(relative_x, 0.0f, width);
            state.relative_cursor_y = clamp(relative_y, 0.0f, height);
        }

        if (!controller_state->left_mouse.is_down) {
            if (ui_id_equals(manager->active, picker.id)) {
                manager->active = {};
            }
        }
    } else {
        if (ui_id_equals(manager->hot, picker.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, picker.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    if (ui_id_equals(manager->active, picker.id) && being_held(controller_state->left_mouse)) {
        state.hsv_color = get_hsv_inside_quad(current_mouse, state.hsv_color.h, x, y, width, height);
        state.relative_cursor_x = clamp(relative_x, 0.0f, width);
        state.relative_cursor_y = clamp(relative_y, 0.0f, height);
    }

    ui_add_hsv_picker(manager, picker);

    return state;
}

UI_Color_Picker_State make_color_picker_state(Vec4 rgb_color_v4,
                                              real32 hsv_picker_width, real32 hsv_picker_height) {
    RGB_Color rgb_color = vec3_to_rgb(truncate_v4_to_v3(rgb_color_v4));
    HSV_Color hsv_color = rgb_to_hsv(rgb_color);
    Vec2 relative_position = hsv_to_cursor_position_inside_quad(hsv_color,
                                                                hsv_picker_width, hsv_picker_height);

    UI_HSV_Picker_State hsv_picker_state = {
        hsv_color,
        relative_position.x, relative_position.y
    };
    UI_Color_Picker_State color_picker_state = {
        false, hsv_picker_state, rgb_color
    };

    return color_picker_state;
}

UI_Color_Picker_State do_color_picker(real32 x, real32 y,
                                      UI_Color_Picker_Style style,
                                      UI_Color_Picker_State state,
                                      char *id_string) {
    using namespace Context;
    UI_Color_Picker color_picker = make_ui_color_picker(x, y, style, state, ui_manager->current_layer, id_string);

    Vec2 current_mouse = controller_state->current_mouse;

    char *box_id = string_format((Allocator *) &memory.frame_arena, 64, "%s_box", id_string);
    UI_Box_Style box_style;
    box_style.background_color = style.background_color;
    box_style.border_color = Editor_Constants::border_color;
    box_style.border_width = 1.0f;
    box_style.inside_border = false;

    uint32 border_flags = SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM | SIDE_TOP;
    do_box(x, y, style.width, style.height, box_style, border_flags, box_id);
    ui_add_color_picker(ui_manager, color_picker);

    real32 initial_x = x;
    real32 initial_y = y;

    x += style.padding_x;
    y += style.padding_y;

    // we save last_active since if we're controlling one of the sliders/pickers and we let go outside of
    // the color picker's (and thus, all of the elements') bounds, they elements will no longer be active,
    // and then when we check if we should hide the color picker, we will be outside of the bounds, and
    // none of them will be equal, and we will have just released the left mouse, so the color picker
    // will hide, which is undesirable, since we were just using one of the color picker's elements.
    UI_id last_active = ui_manager->active;

    char *hue_slider_id = string_format((Allocator *) &memory.frame_arena, 64, "%s_hue_slider", id_string);
    state.hsv_picker_state.hsv_color.h = do_hue_slider(x + style.hsv_picker_width + style.padding_x, y,
                                                       style.hue_slider_width, style.hsv_picker_height,
                                                       state.hsv_picker_state.hsv_color.h,
                                                       hue_slider_id);

    char *hsv_picker_id = string_format((Allocator *) &memory.frame_arena, 64, "%s_hsv_picker", id_string);
    state.hsv_picker_state = do_hsv_picker(x, y,
                                           style.hsv_picker_width, style.hsv_picker_height,
                                           state.hsv_picker_state,
                                           hsv_picker_id);
    state.rgb_color = hsv_to_rgb(state.hsv_picker_state.hsv_color);

    // TODO: i think this could be better.. but it's confusing
    if (!in_bounds_on_layer(ui_manager, current_mouse,
                            initial_x, initial_x+style.width,
                            initial_y, initial_y+style.height) &&
        !ui_id_equals(last_active, { UI_HUE_SLIDER, hue_slider_id, 0 }) &&
        !ui_id_equals(last_active, { UI_HSV_PICKER, hsv_picker_id, 0 })) {
        if (was_clicked(controller_state->left_mouse)) {
            state.should_hide = true;
        } else {
            state.should_hide = false;
        }
    }

    return state;
}
