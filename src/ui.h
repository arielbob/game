#ifndef UI_2_H
#define UI_2_H

#define UI_WIDGET_IS_CLICKABLE       (1 << 0)
#define UI_WIDGET_DRAW_BACKGROUND    (1 << 1)
#define UI_WIDGET_DRAW_TEXT          (1 << 2)
#define UI_WIDGET_IS_FOCUSABLE       (1 << 3)
#define UI_WIDGET_DRAW_BORDER        (1 << 4)

#define CORNER_TOP_LEFT     (1 << 0)
#define CORNER_TOP_RIGHT    (1 << 1)
#define CORNER_BOTTOM_LEFT  (1 << 2)
#define CORNER_BOTTOM_RIGHT (1 << 3)
#define CORNER_ALL          (0b1111)

#define BORDER_LEFT   (1 << 0)
#define BORDER_RIGHT  (1 << 1)
#define BORDER_BOTTOM (1 << 2)
#define BORDER_TOP    (1 << 3)
#define BORDER_ALL    (0b1111)

#define NUM_WIDGET_BUCKETS 128

struct UI_Widget;

enum UI_Widget_Axis {
    UI_WIDGET_X_AXIS = 0,
    UI_WIDGET_Y_AXIS = 1
};

enum UI_Size_Type {
    UI_SIZE_NONE,
    UI_SIZE_PERCENTAGE,
    UI_SIZE_ABSOLUTE,
    UI_SIZE_FIT_CHILDREN,
    UI_SIZE_FIT_TEXT,
    UI_SIZE_FILL_REMAINING
};

enum UI_Layout_Type {
    UI_LAYOUT_NONE,
    UI_LAYOUT_HORIZONTAL,
    UI_LAYOUT_VERTICAL,
    UI_LAYOUT_CENTER,
    UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN
};

enum UI_Position_Type {
    UI_POSITION_NONE,
    UI_POSITION_FLOAT
};

enum UI_Widget_State_Type {
    UI_STATE_WINDOW,
    UI_STATE_TEXT_FIELD_SLIDER
};

struct Vec2_UI_Size_Type {
    union {
        UI_Size_Type values[2];
        struct {
            UI_Size_Type x;
            UI_Size_Type y;
        };
    };
    inline UI_Size_Type &operator[](int32 index);
};

inline UI_Size_Type &Vec2_UI_Size_Type::operator[](int32 index) {
    return (*this).values[index];
}

struct Rect {
    real32 x;
    real32 y;
    real32 width;
    real32 height;
};

struct UI_Theme {
    Vec4 background_color;
    Vec4 hot_background_color;
    Vec4 active_background_color;

    Vec4 text_color;
    char *font;
    char *text;

    Vec4 border_color;
    uint32 border_flags;
    real32 border_width;
    real32 corner_radius;
    uint32 corner_flags;
    
    UI_Layout_Type layout_type;
    Vec2_UI_Size_Type size_type;
    UI_Position_Type position_type;
    
    Vec2 semantic_size;
    Vec2 semantic_position;
};

struct UI_Style_BG_Color {
    Vec4 background_color;
    UI_Style_BG_Color *next;
};

struct UI_Style_Border_Color {
    Vec4 border_color;
    UI_Style_Border_Color *next;
};

struct UI_Style_Border_Flags {
    uint32 border_flags;
    UI_Style_Border_Flags *next;
};

struct UI_Style_Border_Width {
    real32 border_width;
    UI_Style_Border_Width *next;
};

struct UI_Style_Corner_Flags {
    uint32 corner_flags;
    UI_Style_Corner_Flags *next;
};

struct UI_Style_Corner_Radius {
    real32 radius;
    UI_Style_Corner_Radius *next;
};

struct UI_Style_Position {
    Vec2 position;
    UI_Style_Position *next;
};

struct UI_Style_Position_Type {
    UI_Position_Type type;
    UI_Style_Position_Type *next;
};

struct UI_Style_Size {
    Vec2 size;
    UI_Style_Size *next;
};

struct UI_Style_Layout_Type {
    UI_Layout_Type type;
    UI_Style_Layout_Type *next;
};

struct UI_Style_Size_Type {
    Vec2_UI_Size_Type type;
    UI_Style_Size_Type *next;
};

struct UI_Style_Text_Color {
    Vec4 color;
    UI_Style_Text_Color *next;
};

struct UI_Style_Font {
    char *font;
    UI_Style_Font *next;
};

struct UI_id {
    // NOTE: we use a pointer to some unique data, such as a constant string specific to a button, to
    //       identify UI elements
    char *string_ptr;
    // this can be used to differentiate between UI_ids that use the same string_ptr.
    // sometimes elements have the same string_ptr since we want to save time and we are creating a large
    // amount of UI elements such that having a unique string ID is not tenable. these ID strings should
    // not be stored in memory that can be overwritten because you could end up with undesirable behaviour.
    // this is why we use constant char arrays whose addresses point to some place in the executable.
    int32 index;

    char *parent_string_ptr;
    int32 parent_index;
};

inline UI_id make_ui_id(char *id, int32 index) {
    if (!id || string_equals(id, "")) id = NULL;
    UI_id ui_id = { id, index, NULL, 0 };
    return ui_id;
}

inline UI_id make_ui_id(char *id) {
    return make_ui_id(id, 0);
}

struct UI_Window_State {
    Vec2 position;
};

struct UI_Text_Field_Slider_State {
    String_Buffer buffer;
    bool32 is_using;
    bool32 is_sliding;
    int32 cursor_index;
};

struct UI_Widget_State {
    UI_id id;
    UI_Widget_State_Type type;

    union {
        UI_Window_State window;
        UI_Text_Field_Slider_State text_field_slider;
    };

    UI_Widget_State *next;
};

// NOTE: fields should not be set directly, since there is logic when adding widgets based on some fields
//       (for example, whether we add to num_sized_children or num_fill_children based on size type)
struct UI_Widget {
    UI_id id;
    uint32 flags;

    UI_Widget *first;  // first child
    UI_Widget *last;   // last child
    UI_Widget *prev;   // prev sibling
    UI_Widget *next;   // next sibling
    UI_Widget *parent;

    //UI_Widget *table_prev;
    UI_Widget *table_next;
    
    Vec4 background_color;
    Vec4 hot_background_color;
    Vec4 active_background_color;

    Vec4 text_color;
    char *font;
    char *text;

    Vec4 border_color;
    uint32 border_flags;
    real32 border_width;
    real32 corner_radius;
    uint32 corner_flags;
    
    UI_Layout_Type layout_type;
    Vec2_UI_Size_Type size_type;
    UI_Position_Type position_type;
    
    Vec2 semantic_size;
    Vec2 semantic_position;

    int32 num_fill_children; // only on x-axis
    int32 num_sized_children;
    int32 num_total_children;
    
    Vec2 computed_size;
    Vec2 computed_position;
    Vec2 computed_child_size_sum; // only on x-axis
    
    // rendering is done in pre-order; we set this in ui_calculate_standalone_sizes since that is
    // also in pre-order
    int32 rendering_index; 
};

struct UI_Stack_Widget {
    UI_Widget *widget;
    UI_Stack_Widget *next;
};

struct UI_Interact_Result {
    bool32 clicked;
    bool32 holding;
    bool32 focused;
    bool32 lost_focus;

    real32 click_t;

    Vec2 relative_mouse; // relative mouse position from top left of the computed position
    Vec2 relative_mouse_percentage;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;
    UI_id last_frame_active;
    UI_id focus;

    real32 active_t;
    real32 focus_t;

    UI_Widget *last_frame_root;
    UI_Widget *root;

    // these tables are just so that we can easily get widgets without traversing the tree.
    // these do NOT hold state; state is handled by the layer that creates stateful widgets out of the primitive
    // widgets.
    UI_Widget **last_frame_widget_table;
    UI_Widget **widget_table;

    UI_Widget_State **state_table;
    
    Heap_Allocator persistent_heap;
    Arena_Allocator last_frame_arena;
    Arena_Allocator frame_arena;

    UI_Stack_Widget *widget_stack;
    
    UI_Style_BG_Color *background_color_stack;
    UI_Style_BG_Color *hot_background_color_stack;
    UI_Style_BG_Color *active_background_color_stack;

    UI_Style_Corner_Radius *corner_radius_stack;
    UI_Style_Border_Color *border_color_stack;
    UI_Style_Border_Flags *border_flags_stack;
    UI_Style_Border_Width *border_width_stack;
    UI_Style_Corner_Flags *corner_flags_stack;
    
    UI_Style_Layout_Type *layout_type_stack;
    UI_Style_Size *size_stack;
    UI_Style_Position *position_stack;
    UI_Style_Position_Type *position_type_stack;
    UI_Style_Size_Type *size_type_stack;
    UI_Style_Font *font_stack;
    UI_Style_Text_Color *text_color_stack;
    
    bool32 is_disabled;
};

bool32 is_hot(UI_Widget *widget);
bool32 is_active(UI_Widget *widget);
bool32 is_focus(UI_Widget *widget);
bool32 in_bounds(Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max);
bool32 in_bounds(Vec2 p, Vec2 widget_position, Vec2 widget_size);
bool32 ui_id_equals(UI_id id1, UI_id id2);
real32 get_adjusted_font_height(Font font);
real32 get_center_x_offset(real32 container_width, real32 element_width);
real32 get_center_baseline_offset(real32 container_height, real32 text_height);
real32 get_center_y_offset(real32 height, real32 box_height);

#endif
