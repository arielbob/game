#ifndef UI_H
#define UI_H

enum UI_Type {
    UI_NONE,
    UI_TEXT,
    UI_TEXT_BUTTON,
    UI_TEXT_BOX,
    UI_BOX
};

#define UI_HEADER                               \
    UI_id id;                                   \
    UI_Type type;

struct UI_id {
    UI_Type type;
    // NOTE: we use a pointer to some unique data, such as a constant string specific to a button, to
    //       identify UI elements
    void *string_ptr;
};

struct UI_Element {
    UI_HEADER
};

// TODO: this is a text button - will probably want to add different types of buttons
struct UI_Text_Button {
    UI_HEADER

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

struct UI_Text {
    UI_HEADER

    real32 x;
    real32 y;

    char *text;
    char *font;
};

struct UI_Text_Box_Style {
    char *font;
    real32 width;
    real32 height;
    real32 padding_x;
    real32 padding_y;
    //real32 cursor_width;
};

struct UI_Text_Box {
    UI_HEADER
    
    real32 x;
    real32 y;

    int32 size;
    char *current_text;

    UI_Text_Box_Style style;
};

struct UI_Push_Buffer {
    void *base;
    uint32 size;
    uint32 used;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;

    UI_Push_Buffer push_buffer;

    bool32 is_disabled;
    
    real64 focus_timer;
    int32 focus_cursor_index;
};

struct Button_State {
    bool32 is_down;
    bool32 was_down;
};

#endif
