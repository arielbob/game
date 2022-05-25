#ifndef EDITOR_ACTIONS_H
#define EDITOR_ACTIONS_H

#include "linked_list.h"

#define MAX_EDITOR_HISTORY_ENTRIES 1024

enum Action_Type {
    ACTION_NONE,
    ACTION_ADD_NORMAL_ENTITY,
    ACTION_ADD_POINT_LIGHT_ENTITY,
    ACTION_DELETE_ENTITY,
    ACTION_MODIFY_ENTITY,
    ACTION_MODIFY_MESH_NAME,
    ACTION_ADD_MESH,

    // TODO: do this
    ACTION_DELETE_MESH,
#if 0
    ACTION_MODIFY_MATERIAL,
    ACTION_ADD_MATERIAL,
    ACTION_DELETE_MATERIAL,
    ACTION_MODIFY_TEXTURE,
    ACTION_ADD_TEXTURE,
    ACTION_DELETE_TEXTURE
#endif
};

#define ACTION_HEADER                           \
    Action_Type type;

struct Editor_Action {
    ACTION_HEADER
};

struct Editor_History {
    Editor_Action *entries[MAX_EDITOR_HISTORY_ENTRIES];
    int32 start_index = -1;
    int32 end_index = -1; // end index is the index of the last entry
    int32 current_index = -1; // the entry we just added/the entry that will be undone
    int32 num_undone = 0;

    // use a push buffer?
    // idk, we kind of want to have a set length
    // could just use an array of pointers then maybe?

    // you could use a push buffer, but let's just use an array and store entries using our heap allocator.
    // why not use a stack allocator? we want to be able to redo, so we don't actually want to deallocate memory
    // when we pop an action off of the stack (i.e. undo).

    // we can use a fixed-size circular array of pointers to actions that are stored in our heap.

    // actually, we can store the actions on a stack allocator
    // we just have to store the Marker in the action
    // this is actually nice since if every action would allocate a new region on the stack, but if you, say, were
    // to undo, then do an action, we could just take the current index in the history entry array and end
    // that action's region, which would deallocate every action that was ahead of it as well, since that's just how
    // the stack allocator works.

    // although, using a stack allocator might get confusing when we loop around the circular buffer. how do we
    // deallocate the memory used by a previous entry without deallocating ourselves? i'm not sure. i don't think
    // this'll work out actually. it may just be simpler to use the heap allocator. and then when we overwrite
    // history (i.e. undo, then do some action), we would just loop until the end and deallocate.
};

struct Add_Normal_Entity_Action {
    ACTION_HEADER

    int32 entity_id;
};

struct Add_Point_Light_Entity_Action {
    ACTION_HEADER

    int32 entity_id;
};

struct Delete_Entity_Action {
    ACTION_HEADER

    Entity *entity;
};

struct Modify_Entity_Action {
    ACTION_HEADER

    int32 entity_id;
    Entity *old_entity;
    Entity *new_entity;
};

struct Modify_Mesh_Name_Action {
    ACTION_HEADER

    int32 mesh_id;
    String old_name;
    String new_name;
};

struct Add_Mesh_Action {
    ACTION_HEADER

    int32 mesh_id;
    int32 original_mesh_id;
    int32 entity_id;
    String filename;
    String name;
};

struct Delete_Mesh_Action {
    ACTION_HEADER

    int32 mesh_id;
    String filename;
    String mesh_name;
    Linked_List<int32> entity_ids;
};

#endif
