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
    UI_LINE,
    UI_HUE_SLIDER,
    UI_HSV_PICKER,
    UI_COLOR_PICKER
};

#define TEXT_ALIGN_X 0x1
#define TEXT_ALIGN_Y 0x2

#define TEXT_JUSTIFY_CENTER 0x1

#define SIDE_LEFT   0x1
#define SIDE_RIGHT  0x2
#define SIDE_TOP    0x4
#define SIDE_BOTTOM 0x8

#define CONSTRAINT_FILL_BUTTON_WIDTH      0x1
#define CONSTRAINT_FILL_BUTTON_HEIGHT     0x2
#define CONSTRAINT_KEEP_IMAGE_PROPORTIONS 0x4

#define CORNER_TOP_LEFT     0x1
#define CORNER_TOP_RIGHT    0x2
#define CORNER_BOTTOM_LEFT  0x4
#define CORNER_BOTTOM_RIGHT 0x8

#define UI_HEADER                               \
    UI_id id;                                   \
    UI_Type type;                               \
    int32 layer;

struct Rect {
    real32 x;
    real32 y;
    real32 width;
    real32 height;
};

inline Rect make_rect(real32 x, real32 y, real32 width, real32 height) {
    return { x, y, width, height };
}

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

UI_id copy(Allocator *allocator, UI_id id) {
    return id;
}

struct UI_Element {
    UI_HEADER
};

struct UI_Text_Style {
    Vec4 color;
    bool32 use_offset_shadow;
    Vec4 offset_shadow_color;
    uint32 text_align_flags;
};

struct UI_Text {
    UI_HEADER

    real32 x;
    real32 y;

    char *text;
    int32 font_id;

    UI_Text_Style style;
};

UI_Text make_ui_text(real32 x, real32 y, char *text, int32 font_id, UI_Text_Style style, int32 layer, char *id, int32 index = 0) {
    UI_Text ui_text = {};

    ui_text.type = UI_TEXT;
    ui_text.layer = layer;

    ui_text.x = x;
    ui_text.y = y;
    ui_text.text = text;
    ui_text.font_id = font_id;
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
    real32 corner_radius;
    uint32 corner_flags;
};

// TODO: this is a text button - will probably want to add different types of buttons
struct UI_Text_Button {
    UI_HEADER

    Rect rect;
    #if 0
    real32 x;
    real32 y;

    real32 width;
    real32 height;
    #endif

    UI_Text_Button_Style style;
    UI_Text_Style text_style;

    char *text;
    int32 font_id;

    bool32 is_disabled;
};

UI_Text_Button make_ui_text_button(//real32 x, real32 y, real32 width, real32 height,
    Rect rect,
                                   UI_Text_Button_Style style, UI_Text_Style text_style,
                                   char *text, int32 font_id, bool32 is_disabled, int32 layer, char *id, int32 index = 0) {
    UI_Text_Button button = {};

    button.type = UI_TEXT_BUTTON;
    button.layer = layer;

    button.rect = rect;
    #if 0
    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    #endif
    button.style = style;
    button.text_style = text_style;
    button.text = text;
    button.font_id = font_id;
    button.is_disabled = is_disabled;

    UI_id button_id = { UI_TEXT_BUTTON, id, index };
    button.id = button_id;

    return button;
}

struct UI_Image_Button_Style {
    // padding is NOT added to total width or height; it is assumed to be included
    real32 padding_x;
    real32 padding_y;
    real32 footer_height; // footer height is added onto the total height

    uint32 image_constraint_flags;

    Vec4 normal_color;
    Vec4 hot_color;
    Vec4 active_color;

    real32 corner_radius;
    uint32 corner_flags;
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
    int32 font_id;
};

UI_Image_Button make_ui_image_button(real32 x, real32 y, real32 width, real32 height,
                                     UI_Image_Button_Style style,
                                     int32 texture_id, int32 layer, char *id, int32 index = 0) {
    UI_Image_Button button = {};

    button.type = UI_IMAGE_BUTTON;
    button.layer = layer;

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
                                     int32 texture_id, char *text, int32 font_id,
                                     int32 layer, char *id, int32 index = 0) {
    UI_Image_Button button = {};

    button.type = UI_IMAGE_BUTTON;
    button.layer = layer;

    button.x = x;
    button.y = y;
    button.width = width;
    button.height = height;
    button.style = style;
    button.text_style = text_style;
    button.has_text = true;
    button.texture_id = texture_id;
    button.text = text;
    button.font_id = font_id;

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
                                     Vec4 color, int32 layer, char *id, int32 index = 0) {
    UI_Color_Button button = {};

    button.type = UI_COLOR_BUTTON;
    button.layer = layer;

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

    int32 font_id;
};

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

    String_Buffer buffer;
    int32 font_id;
    
    UI_Slider_Style style;
    UI_Text_Style text_style;

    bool32 is_bounded;
    real32 min;
    real32 max;
    real32 value;

    bool32 is_text_box;
};
// end slider

// start box
struct UI_Box_Style {
    Vec4 background_color;
    Vec4 border_color;
    real32 border_width;
    bool32 inside_border;
};

struct UI_Box {
    UI_HEADER
    
    real32 x;
    real32 y;

    real32 width;
    real32 height;

    UI_Box_Style style;
    uint32 border_flags;
};

UI_Box make_ui_box(real32 x, real32 y,
                   real32 width, real32 height,
                   UI_Box_Style style,
                   uint32 border_flags,
                   int32 layer, char *id, int32 index = 0) {
    UI_Box box = {};

    box.type = UI_BOX;
    box.layer = layer;

    box.x = x;
    box.y = y;
    box.width = width;
    box.height = height;
    box.style = style;
    box.border_flags = border_flags;

    UI_id box_id = { UI_BOX, id, index };
    box.id = box_id;

    return box;
}
// end box

// start line
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
                     int32 layer, char *id, int32 index = 0) {
    UI_Line line = {};

    line.type = UI_LINE;
    line.layer = layer;

    line.start = start_pixels;
    line.end = end_pixels;
    line.style = style;

    UI_id line_id = { UI_LINE, id, index };
    line.id = line_id;

    return line;
}
// end line

// start hue slider
struct UI_Hue_Slider {
    UI_HEADER

    real32 x;
    real32 y;

    real32 width;
    real32 height;

    real32 hue_degrees; // between 0 and 360
};

struct UI_Hue_Slider_Result {
    bool32 submitted;
    real32 hue_degrees;
};

UI_Hue_Slider make_ui_hue_slider(real32 x, real32 y,
                                 real32 width, real32 height,
                                 real32 hue_degrees,
                                 int32 layer,
                                 char *id, int32 index = 0) {
    UI_Hue_Slider hue_slider;

    hue_slider.type = UI_HUE_SLIDER;
    hue_slider.layer = layer;

    hue_slider.x = x;
    hue_slider.y = y;
    hue_slider.width = width;
    hue_slider.height = height;
    hue_slider.hue_degrees = hue_degrees;

    UI_id hue_slider_id = { UI_HUE_SLIDER, id, index };
    hue_slider.id = hue_slider_id;

    return hue_slider;
}
// end hue slider

// start HSV picker
struct UI_HSV_Picker {
    UI_HEADER

    real32 x;
    real32 y;

    real32 width;
    real32 height;

    HSV_Color hsv_color;
    real32 relative_cursor_x;
    real32 relative_cursor_y;
};

struct UI_HSV_Picker_Result {
    bool32 submitted;
    HSV_Color hsv_color;
    real32 relative_cursor_x;
    real32 relative_cursor_y;
};

UI_HSV_Picker make_ui_hsv_picker(real32 x, real32 y,
                                 real32 width, real32 height,
                                 HSV_Color hsv_color,
                                 real32 relative_cursor_x, real32 relative_cursor_y,
                                 int32 layer,
                                 char *id, int32 index) {
    UI_HSV_Picker hsv_picker;

    hsv_picker.type = UI_HSV_PICKER;
    hsv_picker.layer = layer;

    hsv_picker.x = x;
    hsv_picker.y = y;
    hsv_picker.width = width;
    hsv_picker.height = height;
    hsv_picker.hsv_color = hsv_color;

    hsv_picker.relative_cursor_x = relative_cursor_x;
    hsv_picker.relative_cursor_y = relative_cursor_y;

    UI_id hsv_picker_id = { UI_HSV_PICKER, id, index }; 
    hsv_picker.id = hsv_picker_id;

    return hsv_picker;
}
// end HSV picker

// start color picker
struct UI_Color_Picker_Style {
    real32 width;
    real32 height;
    real32 hsv_picker_width;
    real32 hsv_picker_height;
    real32 hue_slider_width;
    real32 padding_x;
    real32 padding_y;
    Vec4 background_color;
};

struct UI_Color_Picker_Result {
    bool32 started;
    bool32 submitted;
    bool32 should_hide;
    Vec4 color;
};

struct UI_Color_Picker {
    UI_HEADER
    
    real32 x;
    real32 y;
    
    UI_Color_Picker_Style style;
    Vec4 color;
};

UI_Color_Picker make_ui_color_picker(real32 x, real32 y,
                                     UI_Color_Picker_Style style,
                                     Vec4 color,
                                     int32 layer, char *id, int32 index = 0) {
    UI_Color_Picker color_picker;

    color_picker.type = UI_COLOR_PICKER;
    color_picker.layer = layer;

    color_picker.x = x;
    color_picker.y = y;
    color_picker.style = style;

    UI_id color_picker_id = { UI_COLOR_PICKER, id, index };
    color_picker.id = color_picker_id;

    return color_picker;
}
// end color picker

// UI element states
enum class UI_Element_State_Type {
    NONE, SLIDER, TEXT_BOX, COLOR_PICKER
};

#define UI_ELEMENT_STATE_HEADER \
    UI_Element_State_Type type;

struct UI_Element_State {
    UI_ELEMENT_STATE_HEADER
};

struct UI_Text_Box_State {
    UI_ELEMENT_STATE_HEADER
    String_Buffer buffer;
};

struct UI_Text_Box_Result {
    bool32 submitted;
    String_Buffer buffer;
};

UI_Text_Box_State make_ui_text_box_state(Allocator *string_allocator, String initial_string, int32 size) {
    UI_Text_Box_State state;
    state.type = UI_Element_State_Type::TEXT_BOX;
    state.buffer = make_string_buffer(string_allocator, initial_string, size);
    return state;
};

struct UI_Slider_State {
    UI_ELEMENT_STATE_HEADER

    String_Buffer buffer;
    bool32 is_text_box;
};

struct UI_Color_Picker_State {
    UI_ELEMENT_STATE_HEADER

    RGB_Color rgb_color;
    HSV_Color hsv_color;
    real32 relative_cursor_x;
    real32 relative_cursor_y;
};

struct UI_Push_Buffer {
    void *base;
    uint32 size;
    uint32 used;
    // we store first here, since it's possible that the first element is not at base due to alignment
    UI_Element *first;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;
    UI_id last_frame_active;

    int32 hot_layer;
    int32 current_layer;

    UI_Push_Buffer push_buffer;

    Heap_Allocator *heap_pointer;
    Hash_Table<UI_id, UI_Element_State *> state_table;

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
