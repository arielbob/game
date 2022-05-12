#ifndef EDITOR_HISTORY_H
#define EDITOR_HISTORY_H

#include "level.h"

#define MAX_EDITOR_HISTORY_ENTRIES 128

struct Editor_State;
struct Game_State;

// TODO: idk if we have a use for ACTION_TRANSFORM_ENTITY anymore (also the procedure that uses it doesn't
//       reset old_entity, so starting a new entity modification will assert)
enum Action_Type {
    ACTION_NONE,
    ACTION_ADD_NORMAL_ENTITY,
    ACTION_ADD_POINT_LIGHT_ENTITY,
    ACTION_DELETE_NORMAL_ENTITY,
    ACTION_DELETE_POINT_LIGHT_ENTITY,
    ACTION_TRANSFORM_ENTITY, 
    ACTION_MODIFY_ENTITY,
    ACTION_MODIFY_MESH
};

#define ACTION_HEADER                           \
    Action_Type type;

struct Editor_Action {
    ACTION_HEADER
};


struct Add_Normal_Entity_Action {
    ACTION_HEADER

    int32 entity_id;
};

Add_Normal_Entity_Action make_add_normal_entity_action() {
    Add_Normal_Entity_Action action = {};
    action.type = ACTION_ADD_NORMAL_ENTITY;
    action.entity_id = -1;
    return action;
}


struct Add_Point_Light_Entity_Action {
    ACTION_HEADER

    int32 entity_id;
};

Add_Point_Light_Entity_Action make_add_point_light_entity_action() {
    Add_Point_Light_Entity_Action action = {};
    action.type = ACTION_ADD_POINT_LIGHT_ENTITY;
    action.entity_id = -1;
    return action;
}


struct Delete_Normal_Entity_Action {
    ACTION_HEADER
    
    int32 entity_id;
    Normal_Entity entity;
};

Delete_Normal_Entity_Action make_delete_normal_entity_action(int32 entity_id) {
    Delete_Normal_Entity_Action action = {};
    action.type = ACTION_DELETE_NORMAL_ENTITY;
    action.entity_id = entity_id;
    return action;
}


struct Delete_Point_Light_Entity_Action {
    ACTION_HEADER
    
    int32 entity_id;
    Point_Light_Entity entity;
};

Delete_Point_Light_Entity_Action make_delete_point_light_entity_action(int32 entity_id) {
    Delete_Point_Light_Entity_Action action = {};
    action.type = ACTION_DELETE_POINT_LIGHT_ENTITY;
    action.entity_id = entity_id;
    return action;
}


struct Transform_Entity_Action {
    ACTION_HEADER
    
    int32 entity_id;
    Entity_Type entity_type;
    Transform original_transform;
    Transform transform;
};

Transform_Entity_Action make_transform_entity_action(Entity_Type entity_type, int32 entity_id,
                                                     Transform original_transform, Transform new_transform) {
    Transform_Entity_Action action = {};
    action.type = ACTION_TRANSFORM_ENTITY;
    action.entity_type = entity_type;
    action.entity_id = entity_id;
    action.original_transform = original_transform;
    action.transform = new_transform;
    return action;
}


struct Modify_Entity_Action {
    ACTION_HEADER
    
    int32 entity_id;
    Entity *original;
    Entity *new_entity;
};

Modify_Entity_Action make_modify_entity_action(int32 entity_id,
                                               Entity *original,  Entity *new_entity) {
    Modify_Entity_Action action = {};
    action.type = ACTION_MODIFY_ENTITY;
    action.entity_id = entity_id;
    action.original = original;
    action.new_entity = new_entity;
    return action;
}

struct Modify_Mesh_Action {
    ACTION_HEADER
    
    Mesh_Type mesh_type;
    int32 mesh_id;
    String_Buffer original_name;
    String_Buffer new_name;
};

Modify_Mesh_Action make_modify_mesh_action(Mesh_Type mesh_type, int32 mesh_id, String_Buffer new_name) {
    Modify_Mesh_Action action = {};
    action.type = ACTION_MODIFY_MESH;
    action.mesh_type = mesh_type;
    action.mesh_id = mesh_id;
    action.new_name = new_name;
    return action;
}

struct Editor_History {
    Allocator *allocator_pointer;
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

/*
- you press the button to add an entity
- this adds the entity
- undo just deletes the entity
- redo adds the entity back in by calling the same procedure
- the procedure would need to be modified to take in the ID
- will the IDs get messed up?
- i guess not, since IDs are always incrementing
- we never re-use IDs even when an entity is deleted

- { add entity, id=0 }

- does it matter that the IDs are the same? yeah, i think it does.
- if for example, when we have entity parenting or entity instancing, well, i guess if you were to delete a parent
  entity, all the children would go away with it
- then the undo would add the parent and all the children back

- memento pattern is alright, but it precludes you from having pointers in your state
- the command pattern seems very straightforward and allows you to have pointers. you just have to write the code
  for allocating and deallocating them yourself
- i guess then, the memento pattern can also allow you to do that. you just have to, well actually, it may be more
  complicated, yeah, you would have to hold extra stuff for stuff like pointers. you would need to make a copy,
  then, actually, honestly, i don't know if it would be more complicated. but, the command pattern seems more
  straightforward, so i'm just going to do that.

 */

void editor_add_normal_entity(Editor_State *editor_state, Game_State *game_state, Add_Normal_Entity_Action action,
                              bool32 is_redoing = false);
void editor_add_point_light_entity(Editor_State *editor_state, Game_State *game_state,
                                   Add_Point_Light_Entity_Action action,
                                   bool32 is_redoing = false);
void editor_delete_entity(Editor_State *editor_state, Level *level,
                          Entity_Type entity_type, int32 entity_id,
                          bool32 is_redoing = false);
void editor_transform_entity(Game_State *game_state, Editor_State *editor_state,
                             Transform_Entity_Action action, bool32 is_redoing = false);
void editor_modify_entity(Editor_State *editor_state, Level *level,
                          Modify_Entity_Action action, bool32 is_redoing = false);
void editor_modify_mesh(Game_State *game_state, Modify_Mesh_Action action, bool32 is_redoing = false);
void history_undo(Game_State *game_state, Editor_History *history);
void history_redo(Game_State *game_state, Editor_History *history);
int32 history_get_num_entries(Editor_History *history);
void history_reset(Editor_History *history);

#endif
