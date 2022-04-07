#ifndef UI_H
#define UI_H

enum UI_Type {
    UI_NONE,
    UI_TEXT,
    UI_TEXT_BUTTON,
    UI_TEXT_BOX,
    UI_BOX,
    UI_LINE
};

#define UI_HEADER                               \
    UI_id id;                                   \
    UI_Type type;

struct UI_id {
    UI_Type type;
    // NOTE: we use a pointer to some unique data, such as a constant string specific to a button, to
    //       identify UI elements
    void *string_ptr;
    // this can be used to differentiate between UI_ids that use the same string_ptr.
    // sometimes elements have the same string_ptr since we want to save time and we are creating a large
    // amount of UI elements such that having a unique string ID is not tenable. these ID strings should
    // not be stored in memory that can be overwritten because you could end up with undesirable behaviour.
    // this is why we use constant char arrays whose addresses point to some place in the executable.
    int32 index; 
};

struct UI_Element {
    UI_HEADER
};

struct UI_Text_Button_Style {
    real32 width;
    real32 height;

    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;
    Vec4 text_color;
};

// TODO: this is a text button - will probably want to add different types of buttons
struct UI_Text_Button {
    UI_HEADER

    real32 x;
    real32 y;

    UI_Text_Button_Style style;

    char *text;
    char *font;
};

struct UI_Text_Style {
    Vec3 color;
    bool32 use_offset_shadow;
    Vec3 offset_shadow_color;
};

struct UI_Text {
    UI_HEADER

    real32 x;
    real32 y;

    char *text;
    char *font;

    UI_Text_Style style;
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

struct UI_Box_Style {
    real32 width;
    real32 height;

    Vec4 background_color;
};

struct UI_Box {
    UI_HEADER
    
    real32 x;
    real32 y;

    UI_Box_Style style;
};

struct UI_Line_Style {
    Vec4 color;
    real32 line_width;
};

struct UI_Line {
    UI_HEADER
    
    Vec2 start;
    Vec2 end;

    UI_Line_Style style;
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
