#ifndef UI_2_H
#define UI_2_H

#define UI_WIDGET_IS_CLICKABLE          (1 << 0)
#define UI_WIDGET_DRAW_BACKGROUND       (1 << 1)
#define UI_WIDGET_DRAW_TEXT             (1 << 2)
#define UI_WIDGET_USE_TEXTURE           (1 << 3)
#define UI_WIDGET_IS_FOCUSABLE          (1 << 4)
#define UI_WIDGET_DRAW_BORDER           (1 << 5)
#define UI_WIDGET_IS_WINDOW             (1 << 6) // should only be active on the window widget in push_window()
#define UI_WIDGET_FORCE_TO_TOP_OF_LAYER (1 << 7)
#define UI_WIDGET_USE_CUSTOM_SHADER     (1 << 8)
#define UI_WIDGET_USE_CUSTOM_SHAPE      (1 << 9)

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

#define UI_MAX_VERTICES  16384
#define UI_MAX_TRIANGLES 8192
#define UI_MAX_INDICES   UI_MAX_TRIANGLES*3
#define UI_MAX_RENDER_COMMANDS 1024
#define UI_MAX_WINDOWS 128
#define NUM_WIDGET_BUCKETS 128

#define UI_MAX_STATES_TO_DELETE 256

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
    UI_LAYOUT_NONE, // uhh, what even is this? i'm not sure
    UI_LAYOUT_HORIZONTAL,
    UI_LAYOUT_VERTICAL,
    UI_LAYOUT_CENTER,
    UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN,
    //UI_LAYOUT_VERTICAL_SPACE_BETWEEN
};

/*
  UI_POSITION_FLOAT:    widget is taken out of the flow of widgets
  UI_POSITION_RELATIVE: widget is placed as if UI_POSITION_NONE, but then can be moved relative to that 
                        using semantic_position
*/
enum UI_Position_Type {
    UI_POSITION_NONE,
    UI_POSITION_FLOAT,
    UI_POSITION_RELATIVE 
};

/*
  UI_SCISSOR_COMPUTED_SIZE: scissor based on computed size (and position) of the widget
  UI_SCISSOR_INHERIT: scissor based on parent; if parent is also inherit, keeps going until we hit a
                      UI_SCISSOR_COMPUTED_SIZE widget
 */
enum UI_Scissor_Type {
    UI_SCISSOR_NONE,
    UI_SCISSOR_COMPUTED_SIZE,
    UI_SCISSOR_INHERIT
};

enum UI_Widget_State_Type {
    UI_STATE_WINDOW,
    UI_STATE_TEXT_FIELD,
    UI_STATE_TEXT_FIELD_SLIDER,
    UI_STATE_DROPDOWN,
    UI_STATE_COLOR_PICKER
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

struct Vec4_UI_Padding {
    union {
        real32 values[4];
        struct {
            real32 top;
            real32 right;
            real32 bottom;
            real32 left;
        };
    };
};

struct Rect {
    real32 x;
    real32 y;
    real32 width;
    real32 height;
};

enum class UI_Shape_Type {
    NONE,
    CIRCLE
};

enum class UI_Shader_Type {
    NONE,
    HSV,
    HSV_SLIDER
};

struct UI_Shader_HSV_Uniforms {
    float degrees;
};

struct UI_Shader_HSV_Slider_Uniforms {
    float hue;
};

union UI_Shader_Uniforms {
    UI_Shader_HSV_Uniforms hsv;
    UI_Shader_HSV_Slider_Uniforms hsv_slider;
};

struct UI_Theme {
    Vec4 background_color;
    Vec4 hot_background_color;
    Vec4 active_background_color;
    char *texture_name;
    
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

    UI_Scissor_Type scissor_type;
    bool32 force_to_top_of_layer;

    UI_Shape_Type shape_type;
    
    UI_Shader_Type shader_type;
    UI_Shader_Uniforms shader_uniforms;
};

struct UI_Window_Theme {
    Vec2 initial_position;
    Vec4 background_color;
    Vec4 title_text_color;
    Vec4 title_bgc;
    Vec4 title_hot_bgc;
    Vec4 title_active_bgc;
    uint32 corner_flags;
    real32 corner_radius;
    uint32 border_flags;
    Vec4 border_color;
    real32 border_width;
    Vec2 semantic_size;
    Vec2_UI_Size_Type size_type;
};

struct UI_Container_Theme {
    Vec4_UI_Padding padding;
    Vec4 background_color;
    
    UI_Position_Type position_type;
    Vec2 position;
    Vec2_UI_Size_Type size_type;
    Vec2 size;
    UI_Layout_Type layout_type;
};

struct UI_Button_Theme {
    Vec2_UI_Size_Type size_type;
    Vec2 size;
    Vec2 position;

    Vec4 background_color;
    Vec4 hot_background_color;
    Vec4 active_background_color;

    Vec4 text_color;
    char *font;

    UI_Scissor_Type scissor_type;
};

struct UI_Slider_Theme {
    Vec4 background_color;
};

struct UI_Text_Field_Slider_Theme {
    Vec4 field_background_color;
    Vec4 field_hot_background_color;
    Vec4 field_active_background_color;

    Vec4 slider_background_color;
    Vec4 cursor_color;
    bool32 show_slider;
    
    real32 corner_radius;
    uint32 corner_flags;
    uint32 border_flags;
    Vec4 border_color;
    real32 border_width;
    
    char *font;
    
    Vec2_UI_Size_Type size_type;
    Vec2 size;
};

struct UI_Text_Field_Theme {
    Vec4 background_color;
    Vec4 hot_background_color;
    Vec4 active_background_color;

    Vec4 cursor_color;
    
    real32 corner_radius;
    uint32 corner_flags;
    uint32 border_flags;
    Vec4 border_color;
    real32 border_width;
    
    char *font;
    
    Vec2_UI_Size_Type size_type;
    Vec2 size;
};

struct UI_Checkbox_Theme {
    Vec4 background_color;
    Vec4 hot_background_color;
    Vec4 active_background_color;

    Vec4 check_color;
    
    Vec2 size;
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
};

inline UI_id make_ui_id(char *id, int32 index) {
    if (!id || string_equals(id, "")) id = NULL;
    UI_id ui_id = { id, index };
    return ui_id;
}

inline UI_id make_ui_id(char *id) {
    return make_ui_id(id, 0);
}

struct UI_Window_State {
    Vec2 position;
    Vec2 relative_start;
};

struct UI_Text_Field_State {
    String_Buffer buffer;
    int32 cursor_index; // 0 is before the first character, 1 is before second character, etc.
    real32 cursor_timer;
    real32 x_offset;
};

struct UI_Text_Field_Result {
    String text;
    bool32 committed;
};

struct UI_Color_Picker_State {
    Vec2 relative_cursor_pos;
    real32 relative_slider_cursor_y; // 0 at top, increases downwards
    real32 hue;
};

struct UI_Color_Picker_Result {
    Vec3 color;
    bool32 committed;
    bool32 should_hide;
};

enum class Text_Field_Slider_Mode {
    NONE,
    SLIDING,
    TYPING
};

struct UI_Text_Field_Slider_State {
    UI_Text_Field_State text_field_state;
    Text_Field_Slider_Mode mode;
    Vec2 mouse_press_start_relative_pos;
    real64 mouse_press_start_t;

    // we show this when not in typing mode and only set it to what's in the
    // text_field_state buffer once text_field_state's buffer is validated by us.
    String_Buffer current_text; 
};

struct UI_Dropdown_State {
    bool32 is_open;
    real32 y_offset;
    real32 start_y_offset;
    real32 t;
};

struct UI_Dropdown_Theme {
    UI_Button_Theme button_theme; // for the button that opens and closes the dropdown
    UI_Button_Theme item_theme;
    UI_Button_Theme selected_item_theme;
    
    UI_Position_Type position_type;
    Vec2 position;
    Vec2_UI_Size_Type size_type;
    Vec2 size;
};

struct UI_Widget_State {
    UI_id id;
    UI_Widget_State_Type type;

    union {
        UI_Window_State window;
        UI_Text_Field_State text_field;
        UI_Text_Field_Slider_State text_field_slider;
        UI_Dropdown_State dropdown;
        UI_Color_Picker_State color_picker;
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
    char *texture_name;
    
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

    Vec2_int32 num_fill_children; // only on x-axis
    Vec2_int32 num_sized_children;
    int32 num_total_children;
    
    Vec2 computed_size;
    Vec2 computed_position;
    Vec2 computed_child_size_sum; // only on x-axis

    // rendering is done in pre-order; we set this in ui_calculate_standalone_sizes since that is
    // also in pre-order
    int32 rendering_index;

    UI_Scissor_Type scissor_type;
    Vec2_int32 computed_scissor_position;
    Vec2_int32 computed_scissor_dimensions;

    UI_Shape_Type shape_type;
    
    UI_Shader_Type shader_type;
    UI_Shader_Uniforms shader_uniforms;
};

struct UI_Stack_Widget {
    UI_Widget *widget;
    UI_Stack_Widget *next;
};

struct UI_Interact_Result {
    bool32 just_pressed;
    bool32 released;
    bool32 holding;
    bool32 just_focused;
    bool32 lost_focus;

    real32 click_t;

    Vec2 relative_mouse; // relative mouse position from top left of the computed position
    Vec2 relative_mouse_percentage;
};

struct UI_Vertex {
    Vec2 position;
    Vec2 uv;
    Vec4 color;
};

enum class UI_Texture_Type {
    UI_TEXTURE_NONE,
    UI_TEXTURE_IMAGE,
    UI_TEXTURE_FONT
};

enum class UI_Rendering_Mode {
    TRIANGLES,
    TRIANGLE_FAN
};

// - vertices and indices need to be pointers and we need to store max vertices/indices per command
// - also store the indices start and num indices (these get set after we've gone through the UI tree; they're
//   for when we actually invoke the draw call)
// - store an array of UI_Render_Commands
// - each group starts at a direct child of the root
// - every time we go to a new element within the group, we check if we can coalesce within the current
//   range of [base + group_offset, base + group_offset + group_size]
// - if so, we append the vertices and indices
// - otherwise, we just add a new command to the group
// - texture commands will always just have a single quad for now, since we aren't doing texture batching
// - font commands will be grouped by font
// - the commands actually need to be sorted though... like it needs to go quads/textured quads, then text quads
// 
// - store array of UI_Render_Groups
// - go through widget tree
// - for each widget, go through the current group and see if we can coalese the widget's draw command with a
//   command that's already there
// - if not, we just add a new command
//
// - actually, we shouldn't coalesce commands. the order is important and should stay as the traversal order.
// - all the quads (basic quads and textured quads) can be in the same array
//   - [basic, textured, basic] - in this, we don't want to group basics since we need to keep the order correct
// - when we're adding a new command, we can just check the last entry and merge into that. we don't want to
//   merge into a non-last one.
//   - imagine a tree like this:
//            root
//            /   \
//           b     t 
//          / \   /
//         b   b b
//   - we wouldn't want to merge the final basic into the group before the textured quad, since when we would
//     draw the first group, the textured quad, would end up on top of the first group, which includes its child.
//     this would be incorrect since the traversal order means that the last basic should be on top of the
//     textured quad.
//
// TODO: do this
// - store array of UI_Render_Groups
// - traverse tree and check if we can append to last entry for basic/textured quads
//   (although, for textured quads, we just add a new entry)
// - for text, we can coalesce with any entry, since the text will always be on top
//   - i don't think this is true?
// - go through all groups and commands and create a vertex array and index array and send those to gpu
//   - while doing this, each command should be updated with an index_start and num_indices
// - go through all the commands and draw them

// render groups and draw commands only draw triangles, so num_indices should always be
// a multiple of 3.
struct UI_Render_Command {
    UI_Texture_Type texture_type;
    union {
        // for per vertex colors, type will be UI_TEXTURE_NONE. we just ignore this union.
        // however, for text quads, we use the per vertex color data for the text color.
        String texture_name;
        String font_name;
    };

#if 0
    int32 num_vertices;
    int32 max_vertices;
    UI_Vertex *vertices;
    
    int32 num_indices;
    int32 max_indices;
    uint32 *indices;
#endif

    bool32 use_scissor;
    Vec2_int32 scissor_position; // on opengl, this is the lower left corner of the scissor box
    Vec2_int32 scissor_dimensions;
    
    int32 indices_start;
    int32 num_indices;

    UI_Shader_Type shader_type;
    UI_Shader_Uniforms shader_uniforms;

    UI_Rendering_Mode rendering_mode;
};

#if 0
struct UI_Render_Command_List {
    UI_Render_Command *first;
    UI_Render_Command *last;
};
#endif

struct UI_Render_Data {
    int32 num_vertices;
    UI_Vertex vertices[UI_MAX_VERTICES];
    
    int32 num_indices;
    uint32 indices[UI_MAX_INDICES];
};

/*
TODO: may 23, 2023
what's the minimum that we need to render?

- we need vertices and indices
- we need fonts and textures
- do we actually need "groups"?
- i don't think so
- we just have two big arrays or vertices and indices
- then another struct that has the spans
  - i guess this can be considered groups

- how do we decide where a group begins and when one ends?
- we need to split on font changes, definitely
- if it's just a basic quad, we can coalesce no matter what, i think
  - we don't care about windows because we assume that the window order will be set
    by the caller
  - i.e. we can already expect the windows to be in the correct render order in the tree
- we can actually coalesce text as well
  - if it's in the same window, we can coalesce, but that's kind of more annoying
  - because if we have commands after the first one, we would have to move those vertices so that
    the new command's vertices can be adjacent to the first one's
  - i think it's fine if we don't coalesce text right now; it's way simpler than how we have it
    right now

 */

#if 0
struct UI_Render_Group {
    // we make two command lists because text_quads are always rendered after the triangles, since text is
    // always on top of a group.
    // we could have a single command list, and i guess that would be nice since then we wouldn't have to do anything
    // if for some reason we wanted quads inside a group to overlap text.
    // but this makes drawing less efficient since we would have more draw calls since we'd be switching from
    // basic quads to text quads a lot (for example with text buttons). i guess you could just reorganize it
    // later, and it would be easy, since we're using linked lists. actually, you would have to reorganize, but then
    // also have to coalesce again, which is annoying.
    // it's nice that we don't have to reorganize stuff the way we have it now.
    UI_Render_Command_List triangles;
    UI_Render_Command_List text_quads;
};
#endif

struct UI_Manager {
    UI_id hot;
    UI_id active;
    UI_id last_frame_active;
    UI_id focus;

    real32 hot_t;
    real32 active_t;
    real32 focus_t;

    UI_id window_order[UI_MAX_WINDOWS];
    int32 num_windows;
    
    UI_Widget *last_frame_root;
    UI_Widget *root;
    
    UI_Widget *main_layer;
    UI_Widget *window_layer;
    UI_Widget *always_on_top_layer;

    // these tables are just so that we can easily get widgets without traversing the tree.
    // these do NOT hold state; state is handled by the layer that creates stateful widgets out of the primitive
    // widgets.
    // these are arrays. that's why they're double pointers.
    // UI_Widgets are their own nodes in this table via their table_next members.
    UI_Widget **last_frame_widget_table;
    UI_Widget **widget_table;

    UI_Widget_State **state_table;
    
    Heap_Allocator persistent_heap;
    Arena_Allocator last_frame_arena;
    Arena_Allocator frame_arena;

    UI_Stack_Widget *widget_stack;

    bool32 is_disabled;

    // rendering
#if 0
    int32 num_render_groups;
    UI_Render_Group *render_groups;
#endif
    UI_Render_Data render_data;
    int32 num_render_commands;
    UI_Render_Command render_commands[UI_MAX_RENDER_COMMANDS];
};

bool32 is_hot(UI_Widget *widget);
bool32 is_active(UI_Widget *widget);
bool32 is_focus(UI_Widget *widget);
bool32 in_bounds(Vec2 p, real32 x_min, real32 x_max, real32 y_min, real32 y_max);
bool32 in_bounds(Vec2 p, Vec2 widget_position, Vec2 widget_size);
bool32 in_bounds(Vec2 p, Vec2_int32 widget_position, Vec2_int32 widget_size);
bool32 in_bounds(Vec2 p, UI_Widget *widget);
bool32 ui_id_equals(UI_id id1, UI_id id2);
bool32 ui_id_is_empty(UI_id id);
real32 get_adjusted_font_height(Font font);
real32 get_center_x_offset(real32 container_width, real32 element_width);
real32 get_center_baseline_offset(real32 container_height, real32 text_height);
real32 get_center_y_offset(real32 height, real32 box_height);
void ui_create_render_commands();

#endif
