#ifndef UI_H
#define UI_H

#define UI_MAX_BUTTONS 64
#define UI_MAX_TEXT_BOXES 64

#if 0
enum UI_Button_State {
    NORMAL, HOVER, PRESSED
};
#endif

#if 0
enum UI_Type {
    TEXT_BUTTON, TEXT_BOX
};

struct UI_Header {
    UI_id id;
    UI_Type type;
};

struct UI_Element {
    UI_Header header;
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

struct UI_Text_Box {
    UI_id id;
    
    real32 x;
    real32 y;
    real32 width;
    real32 height;

    uint32 size;
    char *current_text;
    char *font;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;
    UI_id focus;

    real64 focus_timer;

    //int32 num_elements;
    //UI_Element elements[UI_MAX_ELEMENTS];
    int32 num_buttons;
    UI_Button buttons[UI_MAX_BUTTONS];
    int32 num_text_boxes;
    UI_Text_Box text_boxes[UI_MAX_TEXT_BOXES];
};

struct Button_State {
    bool32 is_down;
    bool32 was_down;
};

#if 0
struct UI_Push_Buffer {
    uint32 size;
    uint32 used;
    void *base;
};
#endif

#endif
