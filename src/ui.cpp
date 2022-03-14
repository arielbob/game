#include "ui.h"

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

void ui_add_button(UI_Manager *manager, UI_Button button) {
    assert(manager->num_buttons < UI_MAX_BUTTONS);
    manager->buttons[manager->num_buttons] = button;
    manager->num_buttons++;
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
