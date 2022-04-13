#ifndef UI_H
#define UI_H

enum UI_Type {
    UI_NONE,
    UI_TEXT,
    UI_TEXT_BUTTON,
    UI_IMAGE_BUTTON,
    UI_COLOR_BUTTON,
    UI_TEXT_BOX,
    UI_SLIDER,
    UI_BOX,
    UI_LINE
};

#define TEXT_ALIGN_X 0x1
#define TEXT_ALIGN_Y 0x2

#define CONSTRAINT_FILL_BUTTON_WIDTH      0x1
#define CONSTRAINT_FILL_BUTTON_HEIGHT     0x2
#define CONSTRAINT_KEEP_IMAGE_PROPORTIONS 0x4

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

struct UI_Text_Style {
    Vec4 color;
    bool32 use_offset_shadow;
    Vec4 offset_shadow_color;
};

struct UI_Text {
    UI_HEADER

    real32 x;
    real32 y;

    char *text;
    char *font;

    UI_Text_Style style;
};

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

struct UI_Text_Button_Style {
    uint32 text_align_flags;
    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;
    Vec4 disabled_color;
};

// TODO: this is a text button - will probably want to add different types of buttons
struct UI_Text_Button {
    UI_HEADER

    real32 x;
    real32 y;

    real32 width;
    real32 height;

    UI_Text_Button_Style style;
    UI_Text_Style text_style;

    char *text;
    char *font;

    bool32 is_disabled;
};

UI_Text_Button make_ui_text_button(real32 x, real32 y, real32 width, real32 height,
                                   UI_Text_Button_Style style, UI_Text_Style text_style,
                                   char *text, char *font, bool32 is_disabled, char *id, int32 index = 0) {
    UI_Text_Button button = {};

    button.type = UI_TEXT_BUTTON;
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.style = style;
    button.text_style = text_style;
    button.text = text;
    button.font = font;
    button.is_disabled = is_disabled;

    UI_id button_id = { UI_TEXT_BUTTON, id, index };
    button.id = button_id;

    return button;
}

struct UI_Image_Button_Style {
    real32 padding_x;
    real32 padding_y;

    uint32 image_constraint_flags;

    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;
};

struct UI_Image_Button {
    UI_HEADER

    real32 x;
    real32 y;

    real32 width;
    real32 height;

    UI_Image_Button_Style style;
    UI_Text_Style text_style;

    bool32 has_text;
    int32 texture_id;
    char *text;
    char *font;
};

UI_Image_Button make_ui_image_button(real32 x, real32 y, real32 width, real32 height,
                                     UI_Image_Button_Style style,
                                     int32 texture_id, char *id, int32 index = 0) {
    UI_Image_Button button = {};

    button.type = UI_IMAGE_BUTTON;
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.style = style;
    button.texture_id = texture_id;

    UI_id button_id = { UI_IMAGE_BUTTON, id, index };
    button.id = button_id;

    return button;
}

UI_Image_Button make_ui_image_button(real32 x, real32 y, real32 width, real32 height,
                                     UI_Image_Button_Style style,
                                     UI_Text_Style text_style,
                                     int32 texture_id, char *text, char *font,
                                     char *id, int32 index = 0) {
    UI_Image_Button button = {};

    button.type = UI_IMAGE_BUTTON;
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.style = style;
    button.text_style = text_style;
    button.has_text = true;
    button.texture_id = texture_id;
    button.text = text;
    button.font = font;

    UI_id button_id = { UI_IMAGE_BUTTON, id, index };
    button.id = button_id;

    return button;
}

struct UI_Color_Button_Style {
    real32 padding_x;
    real32 padding_y;

    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;
};

struct UI_Color_Button {
    UI_HEADER

    real32 x;
    real32 y;

    real32 width;
    real32 height;

    UI_Color_Button_Style style;

    Vec4 color;
};

UI_Color_Button make_ui_color_button(real32 x, real32 y, real32 width, real32 height,
                                     UI_Color_Button_Style style,
                                     Vec4 color, char *id, int32 index = 0) {
    UI_Color_Button button = {};

    button.type = UI_COLOR_BUTTON;
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.style = style;
    button.color = color;

    UI_id button_id = { UI_COLOR_BUTTON, id, index };
    button.id = button_id;

    return button;
}

struct UI_Text_Box_Style {
    uint32 text_align_flags;

    real32 padding_x;
    real32 padding_y;

    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;
};

struct UI_Text_Box {
    UI_HEADER
    
    real32 x;
    real32 y;

    real32 width;
    real32 height;

    String_Buffer buffer;

    UI_Text_Box_Style style;
    UI_Text_Style text_style;

    char *font;
};

UI_Text_Box make_ui_text_box(real32 x, real32 y,
                             real32 width, real32 height,
                             String_Buffer buffer,
                             char *font,
                             UI_Text_Box_Style style, UI_Text_Style text_style,
                             char *id, int32 index = 0) {
    UI_Text_Box text_box = {};

    text_box.type = UI_TEXT_BOX;
    text_box.x = x;
    text_box.y = y;
    text_box.width = width;
    text_box.height = height;
    text_box.buffer = buffer;
    text_box.font = font;
    text_box.style = style;
    text_box.text_style = text_style;

    UI_id text_box_id = { UI_TEXT_BOX, id, index };
    text_box.id = text_box_id;

    return text_box;
}

// start slider
struct UI_Slider_Style {
    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;

    Vec4 slider_normal_color;
    Vec4 slider_hot_color;
    Vec4 slider_active_color;
};

struct UI_Slider {
    UI_HEADER
    
    real32 x;
    real32 y;

    real32 width;
    real32 height;

    char *text;
    char *font;
    
    UI_Slider_Style style;
    UI_Text_Style text_style;

    real32 min;
    real32 max;
    real32 value;
};

UI_Slider make_ui_slider(real32 x, real32 y,
                         real32 width, real32 height,
                         char *text, char *font,
                         real32 min, real32 max, real32 value,
                         UI_Slider_Style style, UI_Text_Style text_style,
                         char *id, int32 index = 0) {
    UI_Slider slider;

    slider.type = UI_SLIDER;
    slider.x = x;
    slider.y = y;
    slider.width = width;
    slider.height = height;
    slider.text = text;
    slider.font = font;
    slider.style = style;
    slider.text_style = text_style;
    slider.min = min;
    slider.max = max;
    slider.value = value;
    
    UI_id slider_id = { UI_SLIDER, id, index };
    slider.id = slider_id;

    return slider;
}
// end slider


struct UI_Box_Style {
    Vec4 background_color;
};

struct UI_Box {
    UI_HEADER
    
    real32 x;
    real32 y;

    real32 width;
    real32 height;

    UI_Box_Style style;
};

UI_Box make_ui_box(real32 x, real32 y,
                   real32 width, real32 height,
                   UI_Box_Style style,
                   char *id, int32 index = 0) {
    UI_Box box = {};

    box.type = UI_BOX;
    box.x = x;
    box.y = y;
    box.width = width;
    box.height = height;
    box.style = style;

    UI_id box_id = { UI_BOX, id, index };
    box.id = box_id;

    return box;
}

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

struct UI_Push_Buffer {
    void *base;
    uint32 size;
    uint32 used;
};

// UI element states
enum UI_Element_State_Type {
    UI_STATE_NONE,
    UI_STATE_SLIDER
};

struct UI_Slider_State {
    String_Buffer buffer;
};

struct UI_State_Variant {
    UI_Element_State_Type type;
    union {
        UI_Slider_State slider_state;
    };
};

struct UI_Manager {
    UI_id hot;
    UI_id active;

    int32 hot_layer;
    int32 current_layer;
    
    UI_Push_Buffer push_buffer;
    Hash_Table<UI_id, UI_State_Variant> state_table;

    bool32 is_disabled;
    
    real64 focus_timer;
    int32 focus_cursor_index;

    Vec2 active_initial_position;
    real64 active_initial_time;
};

struct Button_State {
    bool32 is_down;
    bool32 was_down;
};

#endif
