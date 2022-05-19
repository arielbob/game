#ifndef EDITOR_H
#define EDITOR_H

#include "linked_list.h"
#include "level.h"

#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE  "level"
#define SAVE_SUCCESS_MESSAGE    "Level saved!"

struct Editor_State {
    Heap_Allocator entity_heap;
    Heap_Allocator history_heap;
    Heap_Allocator general_heap;

    Asset_Manager asset_manager;
    
//Linked_List<Entity *> entity_list;
    
//String current_level_name;
    Editor_Level level;
};

namespace Editor_Constants {
    char *editor_font_name = "calibri14";
    char *editor_font_name_bold = "calibri14b";
    char disallowed_chars[] = {'{', '}', '"'};
    int32 num_disallowed_chars = array_length(disallowed_chars);
};

#endif
