#include "ui.h"

UI_Button make_ui_button(real32 x, real32 y, real32 width, real32 height, char *text, char *font) {
    UI_Button button = {};
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.text = text;
    button.font = font;
    return button;
}

void ui_add_button(UI_Manager *manager, UI_Button button) {
    assert(manager->num_buttons < UI_MAX_BUTTONS);
    manager->buttons[manager->num_buttons] = button;
    manager->num_buttons++;
}

bool32 in_bounds (Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max) {
    return (p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max);
}

UI_Button_State do_button(UI_Manager *manager, Controller_State *controller_state,
                          real32 x_px, real32 y_px, real32 width_px, real32 height_px,
                          char *text, char *font) {
    UI_Button button = make_ui_button(x_px, y_px,
                                      width_px, height_px,
                                      text, font);

    UI_Button_State button_state;

    Vec2 current_mouse = controller_state->current_mouse;
    if (in_bounds(current_mouse, x_px, x_px + width_px, y_px, y_px + height_px)) {
        if (controller_state->left_mouse.is_down) {
            button_state = PRESSED;
        } else {
            button_state = HOVER;
        }
    } else {
        button_state = NORMAL;
    }

    button.state = button_state;
    ui_add_button(manager, button);

    return button_state;
}
