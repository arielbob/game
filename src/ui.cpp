#include "ui.h"
#include "string.h"

#if 0
void push_element(UI_Push_Buffer *buffer, UI_Element element) {
    switch (element.header.type) {
        case TEXT_BUTTON: {
            assert((buffer->used + sizeof(UI_Button)) <= buffer->size);
            *(base + buffer->used) = (UI_Button) element;
            buffer->used += sizeof(UI_Button);
        } break;
        case TEXT_BOX: {
            assert((buffer->used + sizeof(UI_Text_Box)) <= buffer->size);
            *(base + buffer->used) = (UI_Text_Box) element;
            buffer->used += sizeof(UI_Text_Box);
        } break;
    }
}
#endif

UI_Button make_ui_button(real32 x, real32 y, real32 width, real32 height, char *text, char *font, char *id) {
    UI_Button button = {};
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.text = text;
    button.font = font;

    UI_id button_id = { id };
    button.id = button_id;

    return button;
}

UI_Text_Box make_ui_text_box(real32 x, real32 y,
                             char *current_text, uint32 size,
                             UI_Text_Box_Style style,
                             char *id) {
    UI_Text_Box text_box = {};

    text_box.x = x;
    text_box.y = y;
    text_box.size = size;
    text_box.current_text = current_text;
    text_box.style = style;

    UI_id text_box_id = { id };
    text_box.id = text_box_id;

    return text_box;
}

void ui_add_button(UI_Manager *manager, UI_Button button) {
    assert(manager->num_buttons < UI_MAX_BUTTONS);
    manager->buttons[manager->num_buttons] = button;
    manager->num_buttons++;
}

void ui_add_text_box(UI_Manager *manager, UI_Text_Box text_box) {
    assert(manager->num_text_boxes < UI_MAX_TEXT_BOXES);
    manager->text_boxes[manager->num_text_boxes++] = text_box;
}

inline bool32 ui_id_equals(UI_id id1, UI_id id2) {
    return id1.string_ptr == id2.string_ptr;
}

bool32 in_bounds(Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max) {
    return (p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max);
}

bool32 do_button(UI_Manager *manager, Controller_State *controller_state,
                 real32 x_px, real32 y_px, real32 width_px, real32 height_px,
                 char *text, char *font, char *id_string) {
    UI_Button button = make_ui_button(x_px, y_px,
                                      width_px, height_px,
                                      text, font, id_string);

    bool32 was_clicked = false;

    Vec2 current_mouse = controller_state->current_mouse;
    if (in_bounds(current_mouse, x_px, x_px + width_px, y_px, y_px + height_px)) {
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

    ui_add_button(manager, button);

    return was_clicked;
}

// TODO: this should return a bool32, since we want to be able to submit fields by pressing the enter key
void do_text_box(UI_Manager *manager, Controller_State *controller_state,
                 real32 x, real32 y,
                 char *current_text, int32 text_buffer_size,
                 UI_Text_Box_Style style,
                 char *id_string) {
    UI_Text_Box text_box =  make_ui_text_box(x, y,
                                             current_text, text_buffer_size,
                                             style,
                                             id_string);

    real32 width = style.width + style.padding_x * 2;
    real32 height = style.height + style.padding_y * 2;

    Vec2 current_mouse = controller_state->current_mouse;
    if (in_bounds(current_mouse, x, x + width, y, y + height)) {
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

