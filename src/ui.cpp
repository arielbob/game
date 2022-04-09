#include "ui.h"
#include "string.h"

UI_Text_Style default_text_style = {
    make_vec4(1.0f, 1.0f, 1.0f, 1.0f),
    true,
    make_vec4(0.0f, 0.0f, 0.0f, 1.0f)
};

UI_Text_Button_Style default_text_button_style = { TEXT_ALIGN_X | TEXT_ALIGN_Y,
                                                   rgb_to_vec4(33, 62, 69),
                                                   rgb_to_vec4(47, 84, 102),
                                                   rgb_to_vec4(19, 37, 46),
                                                   rgb_to_vec4(102, 102, 102) };

UI_Text_Button_Style default_text_button_cancel_style = { TEXT_ALIGN_X | TEXT_ALIGN_Y,
                                                          rgb_to_vec4(140, 38, 60),
                                                          rgb_to_vec4(199, 66, 103),
                                                          rgb_to_vec4(102, 22, 45) };

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

UI_Image_Button_Style default_image_button_style = { 5.0f, 5.0f,
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
                                             rgb_to_vec4(19, 37, 46) };

// TODO: store UI element state in a hash table, so we can do things like fading transitions.
//       this requires some thought since we would like to remove elements from the hash table if
//       the element is no longer being displayed. we want this while retaining the nice
//       immediate mode API.

// TODO: memory alignment
#define _push_element(push_buffer, type, element)                       \
    assert(((push_buffer)->used + sizeof(type)) <= (push_buffer)->size);    \
    *((type *) ((uint8 *) (push_buffer)->base + (push_buffer)->used)) = element; \
    (push_buffer)->used += sizeof(type)
    
void clear_push_buffer(UI_Push_Buffer *buffer) {
    buffer->used = 0;
}

void ui_add_text_button(UI_Manager *manager, UI_Text_Button button) {
    _push_element(&manager->push_buffer, UI_Text_Button, button);
}

void ui_add_image_button(UI_Manager *manager, UI_Image_Button button) {
    _push_element(&manager->push_buffer, UI_Image_Button, button);
}

void ui_add_color_button(UI_Manager *manager, UI_Color_Button button) {
    _push_element(&manager->push_buffer, UI_Color_Button, button);
}

void ui_add_text(UI_Manager *manager, UI_Text text) {
    _push_element(&manager->push_buffer, UI_Text, text);
}

void ui_add_text_box(UI_Manager *manager, UI_Text_Box text_box) {
    _push_element(&manager->push_buffer, UI_Text_Box, text_box);
}

void ui_add_slider(UI_Manager *manager, UI_Slider slider) {
    _push_element(&manager->push_buffer, UI_Slider, slider);
}

void ui_add_box(UI_Manager *manager, UI_Box box) {
    _push_element(&manager->push_buffer, UI_Box, box);
}

void ui_add_line(UI_Manager *manager, UI_Line line) {
    _push_element(&manager->push_buffer, UI_Line, line);
}

inline bool32 ui_id_equals(UI_id id1, UI_id id2) {
    return ((id1.string_ptr == id2.string_ptr) && (id1.index == id2.index));
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

// NOTE: since in immediate-mode GUIs, we only update hot if we call the do_ procedure for some element,
//       if the element disappears, we lose the ability to change hot based on that element, for example,
//       setting hot to empty if we're not over a button. this procedure loops through all the elements
//       that were added during the frame and checks if the current hot element was added. if it was not,
//       that element is gone and thus if hot is that element, it should be cleared.
void clear_hot_if_gone(UI_Manager *manager) {
    UI_Push_Buffer *push_buffer = &manager->push_buffer;
    uint8 *address = (uint8 *) push_buffer->base;

    while (address < ((uint8 *) push_buffer->base + push_buffer->used)) {
        UI_Element *element = (UI_Element *) address;

        // hot is still visible
        if (ui_id_equals(manager->hot, element->id)) return;

        switch (element->type) {
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
            default: {
                assert(!"Unhandled UI element type.");
            }
        }
    }

    // hot is gone
    clear_hot(manager);
}

void do_text(UI_Manager *manager,
             real32 x_px, real32 y_px,
             char *text, char *font, char *id_string, int32 index = 0) {
    UI_Text ui_text = make_ui_text(x_px, y_px,
                                   text, font, default_text_style, id_string, index);

    bool32 was_clicked = false;

    ui_add_text(manager, ui_text);
}

void do_text(UI_Manager *manager,
             real32 x_px, real32 y_px,
             char *text, char *font, 
             UI_Text_Style style,
             char *id_string, int32 index = 0) {
    UI_Text ui_text = make_ui_text(x_px, y_px,
                                   text, font, style, id_string, index);

    bool32 was_clicked = false;

    ui_add_text(manager, ui_text);
}

bool32 do_text_button(UI_Manager *manager, Controller_State *controller_state,
                      real32 x_px, real32 y_px,
                      real32 width, real32 height,
                      UI_Text_Button_Style style, UI_Text_Style text_style,
                      char *text, char *font, bool32 is_disabled, char *id_string, int32 index = 0) {
    UI_Text_Button button = make_ui_text_button(x_px, y_px, width, height,
                                                style, text_style,
                                                text, font, is_disabled, id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;

    if (!manager->is_disabled &&
        !is_disabled &&
        in_bounds_on_layer(manager, current_mouse, x_px, x_px + width, y_px, y_px + height)) {
        // NOTE: ui state is modified in sequence that the immediate mode calls are done. this is why we have to always
        //       set hot again. if we didn't have this, if we drew a box, then drew a button on top of it, and then moved
        //       our cursor over top of the button, hot would be the box and NOT the button. which is not desired.
        set_hot(manager, button.id);
        
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            // we check for !was_down to avoid setting a button active if we click and hold outside then
            // move into the button
            manager->active = button.id;
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            if (ui_id_equals(manager->active, button.id)) {
                was_clicked = true;
                manager->active = {};
                debug_print("%s was clicked\n", button.id);
            }
        }
    } else {
        if (ui_id_equals(manager->hot, button.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, button.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    ui_add_text_button(manager, button);

    return was_clicked;
}

inline bool32 do_text_button(UI_Manager *manager, Controller_State *controller_state,
                      real32 x_px, real32 y_px,
                      real32 width, real32 height,
                      UI_Text_Button_Style style, UI_Text_Style text_style,
                      char *text, char *font, char *id_string, int32 index = 0) {
    return do_text_button(manager, controller_state,
                          x_px, y_px,
                          width, height,
                          style, text_style,
                          text, font, false, id_string, index);
}

bool32 do_image_button(UI_Manager *manager, Controller_State *controller_state,
                       real32 x_px, real32 y_px,
                       real32 width, real32 height,
                       UI_Image_Button_Style style,
                       char *texture_name,
                       char *id_string, int32 index = 0) {
    UI_Image_Button button = make_ui_image_button(x_px, y_px, width, height,
                                                  style,
                                                  texture_name, id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x_px, x_px + width, y_px, y_px + height)) {
        set_hot(manager, button.id);
        
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            manager->active = button.id;
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            if (ui_id_equals(manager->active, button.id)) {
                was_clicked = true;
                manager->active = {};
                debug_print("%s was clicked\n", button.id);
            }
        }
    } else {
        if (ui_id_equals(manager->hot, button.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, button.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    ui_add_image_button(manager, button);

    return was_clicked;
}

bool32 do_color_button(UI_Manager *manager, Controller_State *controller_state,
                       real32 x_px, real32 y_px,
                       real32 width, real32 height,
                       UI_Color_Button_Style style,
                       Vec4 color,
                       char *id_string, int32 index = 0) {
    UI_Color_Button button = make_ui_color_button(x_px, y_px, width, height,
                                                  style,
                                                  color, id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x_px, x_px + width, y_px, y_px + height)) {
        set_hot(manager, button.id);
        
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            manager->active = button.id;
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            if (ui_id_equals(manager->active, button.id)) {
                was_clicked = true;
                manager->active = {};
                debug_print("%s was clicked\n", button.id);
            }
        }
    } else {
        if (ui_id_equals(manager->hot, button.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, button.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    ui_add_color_button(manager, button);

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

void do_text_box(UI_Manager *manager, Controller_State *controller_state,
                 real32 x, real32 y,
                 real32 width, real32 height,
                 String_Buffer *buffer,
                 char *font,
                 UI_Text_Box_Style style, UI_Text_Style text_style,
                 char *id_string, int32 index = 0) {
    UI_Text_Box text_box =  make_ui_text_box(x, y, width, height,
                                             *buffer,
                                             font,
                                             style, text_style,
                                             id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x, x + width, y, y + height)) {
        set_hot(manager, text_box.id);

        // TODO: this is useless
        if (!controller_state->left_mouse.is_down) {
            set_hot(manager, text_box.id);
        }

        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
        } else if (!controller_state->left_mouse.is_down && controller_state->left_mouse.was_down) {
            manager->active = text_box.id;
            manager->focus_timer = platform_get_wall_clock_time();
            manager->focus_cursor_index = buffer->current_length;
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
                buffer->current_length = manager->focus_cursor_index;
            } else if (manager->focus_cursor_index < buffer->size &&
                       c >= 32 &&
                       c <= 126) {
                buffer->contents[manager->focus_cursor_index] = c;
                manager->focus_cursor_index++;
                buffer->current_length++;
            }
        }
    }

    ui_add_text_box(manager, text_box);
}

real32 do_slider(UI_Manager *manager, Controller_State *controller_state,
                 real32 x, real32 y,
                 real32 width, real32 height,
                 char *text, char *font,
                 real32 min, real32 max, real32 value,
                 UI_Slider_Style style, UI_Text_Style text_style,
                 char *id_string, int32 index = 0) {
    UI_Slider slider =  make_ui_slider(x, y, width, height,
                                       text, font,
                                       min, max, value,
                                       style, text_style,
                                       id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse, x, x + width, y, y + height)) {
        set_hot(manager, slider.id);

        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            manager->active = slider.id;
        }

        if (!controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    } else {
        // FIXME: this isn't really a bug (other programs seem to behave this way), but i'm not sure
        //        why it's happening: if you hold down a key in the textbox and you click outside of the textbox,
        //        the textbox should lose focus and text should no longer be inputted. but, for some reason it
        //        keeps focus after clicking outside of the textbox and characters keep getting inputted.
        //        actually, i think it's because we're doing while(PeekMessage...).
        if (ui_id_equals(manager->hot, slider.id)) {
            clear_hot(manager);
        }

        if (ui_id_equals(manager->active, slider.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    if (ui_id_equals(manager->active, slider.id) && being_held(controller_state->left_mouse)) {
        real32 rate = (max - min) / width;
        real32 delta_pixels = (controller_state->current_mouse - controller_state->last_mouse).x;
        value += delta_pixels * rate;
        value = min(max, value);
        value = max(min, value);
    }

    ui_add_slider(manager, slider);

    return value;
}

void do_box(UI_Manager *manager, Controller_State *controller_state,
            real32 x, real32 y,
            real32 width, real32 height,
            UI_Box_Style style,
            char *id_string, int32 index = 0) {
    UI_Box box =  make_ui_box(x, y, width, height,
                              style,
                              id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds_on_layer(manager, current_mouse,
                                                    x, x + width,
                                                    y, y + height)) {
        set_hot(manager, box.id);
    } else {
        if (ui_id_equals(manager->hot, box.id)) {
            clear_hot(manager);
        }
    }

    ui_add_box(manager, box);
}

void do_line(UI_Manager *manager,
             Vec2 start_pixels, Vec2 end_pixels,
             UI_Line_Style style,
             char *id_string, int32 index = 0) {
    UI_Line line =  make_ui_line(start_pixels, end_pixels,
                                 style,
                                 id_string, index);

    // right now, we only draw lines on top of other UI elements, so we don't check if the mouse is over

    ui_add_line(manager, line);
}
