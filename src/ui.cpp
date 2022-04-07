#include "ui.h"
#include "string.h"

// TODO: store UI element state in a hash table, so we can do things like fading transitions.
//       this requires some thought since we would like to remove elements from the hash table if
//       the element is no longer being displayed. we want this while retaining the nice
//       immediate mode API.

// TODO: memory alignment
#define _push_element(push_buffer, type, element)                       \
    assert(((push_buffer)->used + sizeof(type)) <= (push_buffer)->size);    \
    *((type *) ((uint8 *) (push_buffer)->base + (push_buffer)->used)) = element; \
    (push_buffer)->used += sizeof(type)
    

#if 0
void push_element(UI_Push_Buffer *buffer, UI_Text element) {
    _push_element(buffer, UI_Text, element);
}

void push_element(UI_Push_Buffer *buffer, UI_Text_Button element) {
    _push_element(buffer, UI_Text_Button, element);
}

void push_element(UI_Push_Buffer *buffer, UI_Text_Box element) {
    _push_element(buffer, UI_Text_Box, element);
}
#endif

void clear_push_buffer(UI_Push_Buffer *buffer) {
    buffer->used = 0;
}

UI_Text_Button make_ui_text_button(real32 x, real32 y,
                                   UI_Text_Button_Style style,
                                   char *text, char *font, char *id, int32 index = 0) {
    UI_Text_Button button = {};

    button.type = UI_TEXT_BUTTON;
    button.x = x;
    button.y = y;
    button.style = style;
    button.text = text;
    button.font = font;

    UI_id button_id = { UI_TEXT_BUTTON, id, index };
    button.id = button_id;

    return button;
}

UI_Image_Button make_ui_image_button(real32 x, real32 y,
                                     UI_Image_Button_Style style,
                                     char *texture_name, char *id, int32 index = 0) {
    UI_Image_Button button = {};

    button.type = UI_IMAGE_BUTTON;
    button.x = x;
    button.y = y;
    button.style = style;
    button.texture_name = texture_name;

    UI_id button_id = { UI_IMAGE_BUTTON, id, index };
    button.id = button_id;

    return button;
}

UI_Text make_ui_text(real32 x, real32 y, char *text, char *font, UI_Text_Style style, char *id, int32 index = 0) {
    UI_Text ui_text = {};

    ui_text.type = UI_TEXT;
    ui_text.x = x;
    ui_text.y = y;
    ui_text.text = text;
    ui_text.font = font;
    ui_text.style = style;

    UI_id ui_text_id = { UI_TEXT, id, index };
    ui_text.id = ui_text_id;

    return ui_text;
}

UI_Text_Box make_ui_text_box(real32 x, real32 y,
                             char *current_text, uint32 size,
                             UI_Text_Box_Style style,
                             char *id, int32 index = 0) {
    UI_Text_Box text_box = {};

    text_box.type = UI_TEXT_BOX;
    text_box.x = x;
    text_box.y = y;
    text_box.size = size;
    text_box.current_text = current_text;
    text_box.style = style;

    UI_id text_box_id = { UI_TEXT_BOX, id, index };
    text_box.id = text_box_id;

    return text_box;
}

UI_Box make_ui_box(real32 x, real32 y,
                   UI_Box_Style style,
                   char *id, int32 index = 0) {
    UI_Box box = {};

    box.type = UI_BOX;
    box.x = x;
    box.y = y;
    box.style = style;

    UI_id box_id = { UI_BOX, id, index };
    box.id = box_id;

    return box;
}

UI_Line make_ui_line(Vec2 start_pixels, Vec2 end_pixels,
                     UI_Line_Style style,
                     char *id, int32 index = 0) {
    UI_Line line = {};

    line.type = UI_LINE;
    line.start = start_pixels;
    line.end = end_pixels;
    line.style = style;

    UI_id line_id = { UI_LINE, id, index };
    line.id = line_id;

    return line;
}

void ui_add_text_button(UI_Manager *manager, UI_Text_Button button) {
    _push_element(&manager->push_buffer, UI_Text_Button, button);
}

void ui_add_image_button(UI_Manager *manager, UI_Image_Button button) {
    _push_element(&manager->push_buffer, UI_Image_Button, button);
}

void ui_add_text(UI_Manager *manager, UI_Text text) {
    _push_element(&manager->push_buffer, UI_Text, text);
}

void ui_add_text_box(UI_Manager *manager, UI_Text_Box text_box) {
    _push_element(&manager->push_buffer, UI_Text_Box, text_box);
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

void do_text(UI_Manager *manager,
             real32 x_px, real32 y_px,
             char *text, char *font, char *id_string, int32 index = 0) {
    Vec3 default_color = make_vec3(1.0f, 1.0f, 1.0f);
    UI_Text_Style style = {
        default_color,
        false,
        make_vec3(0.0f, 0.0f, 0.0f)
    };

    UI_Text ui_text = make_ui_text(x_px, y_px,
                                   text, font, style, id_string, index);

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
                      UI_Text_Button_Style style,
                      char *text, char *font, char *id_string, int32 index = 0) {
    UI_Text_Button button = make_ui_text_button(x_px, y_px,
                                                style,
                                                text, font, id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds(current_mouse, x_px, x_px + style.width, y_px, y_px + style.height)) {
        if (controller_state->left_mouse.is_down) {
            if (ui_id_equals(manager->hot, button.id) || !controller_state->left_mouse.was_down) {
                manager->active = button.id;
            } else if (ui_id_equals(manager->active, button.id)) {
                manager->hot = button.id;
            }
        } else if (controller_state->left_mouse.was_down) {
            if (ui_id_equals(manager->active, button.id)) {
                was_clicked = true;
                debug_print("%s was clicked\n", button.id);
            }
        } else {
            manager->hot = button.id;
            if (ui_id_equals(manager->active, button.id)) {
                manager->active = {};
            }   
        }
    } else {
        if (ui_id_equals(manager->hot, button.id)) {
            manager->hot = {};
        }

        if (ui_id_equals(manager->active, button.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    ui_add_text_button(manager, button);

    return was_clicked;
}

bool32 do_image_button(UI_Manager *manager, Controller_State *controller_state,
                       real32 x_px, real32 y_px,
                       UI_Image_Button_Style style,
                       char *texture_name,
                       char *id_string, int32 index = 0) {
    UI_Image_Button button = make_ui_image_button(x_px, y_px,
                                                  style,
                                                  texture_name, id_string, index);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds(current_mouse, x_px, x_px + style.width, y_px, y_px + style.height)) {
        if (controller_state->left_mouse.is_down) {
            if (ui_id_equals(manager->hot, button.id) || !controller_state->left_mouse.was_down) {
                manager->active = button.id;
            } else if (ui_id_equals(manager->active, button.id)) {
                manager->hot = button.id;
            }
        } else if (controller_state->left_mouse.was_down) {
            if (ui_id_equals(manager->active, button.id)) {
                was_clicked = true;
                debug_print("%s was clicked\n", button.id);
            }
        } else {
            manager->hot = button.id;
            if (ui_id_equals(manager->active, button.id)) {
                manager->active = {};
            }   
        }
    } else {
        if (ui_id_equals(manager->hot, button.id)) {
            manager->hot = {};
        }

        if (ui_id_equals(manager->active, button.id) && !controller_state->left_mouse.is_down) {
            manager->active = {};
        }
    }

    ui_add_image_button(manager, button);

    return was_clicked;
}

// TODO: this should return a bool32, since we want to be able to submit fields by pressing the enter key
void do_text_box(UI_Manager *manager, Controller_State *controller_state,
                 real32 x, real32 y,
                 char *current_text, int32 text_buffer_size,
                 UI_Text_Box_Style style,
                 char *id_string, int32 index = 0) {
    UI_Text_Box text_box =  make_ui_text_box(x, y,
                                             current_text, text_buffer_size,
                                             style,
                                             id_string, index);

    real32 width = style.width + style.padding_x * 2;
    real32 height = style.height + style.padding_y * 2;

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds(current_mouse, x, x + width, y, y + height)) {
        if (!controller_state->left_mouse.is_down) {
            manager->hot = text_box.id;
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
            manager->hot = {};
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

    //return was_clicked;
}

void do_box(UI_Manager *manager, Controller_State *controller_state,
            real32 x, real32 y,
            UI_Box_Style style,
            char *id_string, int32 index = 0) {
    UI_Box box =  make_ui_box(x, y,
                              style,
                              id_string, index);

    Vec2 current_mouse = controller_state->current_mouse;
    if (!manager->is_disabled && in_bounds(current_mouse,
                                           x, x + style.width,
                                           y, y + style.height)) {
        //DebugBreak();
        manager->hot = box.id;
    } else {
        if (ui_id_equals(manager->hot, box.id)) {
            manager->hot = {};
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

