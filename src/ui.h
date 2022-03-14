#ifndef UI_H
#define UI_H

#define UI_MAX_BUTTONS 64

#if 0
enum UI_Button_State {
    NORMAL, HOVER, PRESSED
};
#endif

struct UI_id {
    // NOTE: we use a pointer to some unique data, such as a constant string specific to a button, to
    //       identify UI elements
    void *string_ptr;
};

// TODO: this is a text button - will probably want to add different types of buttons
struct UI_Button {
    //UI_Button_State state;

    UI_id id;

    // NOTE: these are positioned in window pixel-space, with (0,0) at the bottom left, +x is right,
    //       and +y is up
    real32 x;
    real32 y;

    // NOTE: the size starts from the bottom left of the button, width goes to the right, and height
    //       goes up
    real32 width;
    real32 height;

    char *text;
    char *font;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;

    int32 num_buttons;
    UI_Button buttons[UI_MAX_BUTTONS];
};

struct Button_State {
    bool32 is_down;
    bool32 was_down;
};

#endif
