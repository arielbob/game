#ifndef UI_2_H
#define UI_2_H

#define UI_WIDGET_IS_CLICKABLE    (1 << 0)
#define UI_WIDGET_DRAW_BACKGROUND (1 << 1)
#define UI_WIDGET_DRAW_TEXT       (1 << 2)

enum UI_Size_Type {
    UI_SIZE_NONE,
    UI_SIZE_PERCENTAGE,
    UI_SIZE_ABSOLUTE,
    UI_SIZE_FIT_CHILDREN
};

enum UI_Layout_Type {
    UI_LAYOUT_NONE,
    UI_LAYOUT_HORIZONTAL,
    UI_LAYOUT_VERTICAL
};

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
    UI_Size_Type type;
    UI_Style_Size_Type *next;
};

struct UI_id {
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

inline UI_id make_ui_id(void *id) {
    UI_id ui_id = { id, 0 };
    return ui_id;
}

inline UI_id make_ui_id(void *id, int32 index) {
    UI_id ui_id = { id, index };
    return ui_id;
}

struct UI_Widget {
    UI_id id;
    uint32 flags;

    // this is a linked-list of the children, right???
    // first is the first child and next is a pointer to the next sibling, i.e. the next node on the same level
    // of the tree
    UI_Widget *first;  // first child
    UI_Widget *last;   // last child
    UI_Widget *prev;   // prev sibling
    UI_Widget *next;   // next sibling
    UI_Widget *parent;

    Vec4 background_color;

    UI_Layout_Type layout_type;
    UI_Size_Type size_type;
    
    Vec2 semantic_size;
    Vec2 semantic_position;

    Vec2 computed_size;
    Vec2 computed_position;
};

struct UI_Stack_Widget {
    UI_Widget *widget;
    UI_Stack_Widget *next;
};

struct UI_Interact_Result {
    bool32 clicked;
};

struct UI_Manager {
    UI_id hot;
    UI_id active;
    UI_id last_frame_active;

    UI_Widget *root;
    
    Allocator *allocator;

    UI_Stack_Widget *widget_stack;
    
    UI_Style_BG_Color *background_color_stack;
    UI_Style_Layout_Type *layout_type_stack;
    UI_Style_Size *size_stack;
    UI_Style_Position *position_stack;
    UI_Style_Size_Type *size_type_stack;
    
    bool32 is_disabled;
};

#endif
