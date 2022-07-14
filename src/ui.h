#ifndef UI_2_H
#define UI_2_H

#define UI_WIDGET_IS_CLICKABLE    (1 << 0)
#define UI_WIDGET_DRAW_BACKGROUND (1 << 1)
#define UI_WIDGET_DRAW_TEXT       (1 << 2)

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
    UI_SIZE_FIT_TEXT
};

enum UI_Widget_State_Type {
    UI_STATE_WINDOW
};

enum UI_Layout_Type {
    UI_LAYOUT_NONE,
    UI_LAYOUT_HORIZONTAL,
    UI_LAYOUT_VERTICAL,
    UI_LAYOUT_CENTER,
    UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN
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

struct UI_Style_BG_Color {
    Vec4 background_color;
    UI_Style_BG_Color *next;
};

struct UI_Style_Rect {
    Rect rect;
    UI_Style_Rect *next;
};

struct UI_Style_Position {
    Vec2 position;
    UI_Style_Position *next;
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

struct UI_Widget_State {
    UI_id id;
    UI_Widget_State_Type type;

    union {
        UI_Window_State window;
    };

    UI_Widget_State *next;
};

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
    
    UI_Layout_Type layout_type;
    Vec2_UI_Size_Type size_type;
    
    Vec2 semantic_size;
    Vec2 semantic_position;

    int32 num_children;
    
    Vec2 computed_size;
    Vec2 computed_position;
    Vec2 computed_child_size_sum;
};

struct UI_Stack_Widget {
    UI_Widget *widget;
    UI_Stack_Widget *next;
};

struct UI_Interact_Result {
    bool32 clicked;
    bool32 holding;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;
    UI_id last_frame_active;

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
    UI_Style_Layout_Type *layout_type_stack;
    UI_Style_Size *size_stack;
    UI_Style_Position *position_stack;
    UI_Style_Size_Type *size_type_stack;
    UI_Style_Font *font_stack;
    UI_Style_Text_Color *text_color_stack;
    
    bool32 is_disabled;
};

bool32 is_hot(UI_Widget *widget);
bool32 is_active(UI_Widget *widget);
bool32 in_bounds(Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max);
bool32 in_bounds(Vec2 p, Vec2 widget_position, Vec2 widget_size);
bool32 ui_id_equals(UI_id id1, UI_id id2);
real32 get_adjusted_font_height(Font font);
real32 get_center_x_offset(real32 container_width, real32 element_width);
real32 get_center_baseline_offset(real32 container_height, real32 text_height);
real32 get_center_y_offset(real32 height, real32 box_height);

#endif
