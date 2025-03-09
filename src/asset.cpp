#include "asset.h"
#include "entity.h"

Material_Info default_material_info = {
    make_string("texture_default"),
    make_string("texture_default"),
    make_string("texture_default"),
    Material_Type::NONE, 0, make_string(""),
    0,
    0, make_vec3(0.5f, 0.5f, 0.5f),
    0, 0.0f,
    0, 0.0f
};

// filename should be the file itself
Directory_Watcher *watch_directory_for_file(Allocator *allocator, Directory_Watcher **watchers,
                                            String filename, Directory_Change_Callback callback) {
    Allocator *temp_region = begin_region();
    String folder_to_watch = platform_get_folder_path(temp_region, filename);

    Directory_Watcher *watcher = NULL;
    Directory_Watcher *last = NULL;
    
    if (*watchers) {
        Directory_Watcher *current = *watchers;
        while (current) {
            if (path_equals(current->path, folder_to_watch)) {
                watcher = current;
                //add_message(message_manager, make_string("Adding another watcher"));
                break;
            }
            last = current;
            current = current->next;
        }
    }

    if (watcher) {
        watcher->num_watchers++;
    } else {
        int32 id = platform_watch_directory(folder_to_watch, callback);
        if (id < 0) {
            assert(!"Could not watch directory!");
            end_region(temp_region);
            return NULL;
        }
        
        watcher = (Directory_Watcher *) allocate(allocator,
                                                 sizeof(Directory_Watcher), true);
        watcher->allocator = allocator;
        watcher->path = copy(allocator, folder_to_watch);
        watcher->next = NULL;
        watcher->num_watchers = 1;
        watcher->id = id;

        if (!last) {
            // first one in list
            *watchers = watcher;
        } else {
            last->next = watcher;
        }
    }

    end_region(temp_region);

    return watcher;
}

void unwatch_directory(Directory_Watcher **watchers, int32 watcher_id) {
    assert(watchers);

    Directory_Watcher *watcher = NULL;
    Directory_Watcher *prev = NULL;
    LINKED_LIST_FOR(Directory_Watcher, *watchers) {
        if (current->id == watcher_id) {
            watcher = current;
            break;
        }
        prev = current;
    }

    assert(watcher);

    watcher->num_watchers--;
    if (watcher->num_watchers == 0) {
        if (!prev) {
            *watchers = watcher->next;
        } else {
            prev->next = watcher->next;
        }
        
        platform_unwatch_directory(watcher->id);
        deallocate(watcher->path);
        deallocate(watcher->allocator, watcher);
    }
}

Mesh *get_mesh(String name) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Mesh *current = asset_manager->mesh_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

Mesh *get_mesh_by_path(String path) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Mesh *current = asset_manager->mesh_table[i];
        while (current) {
            if (path_equals(current->filename, path)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

Mesh *get_mesh(int32 id) {
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    while (current) {
        if (current->id == id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void set_mesh_id(Mesh *mesh, int32 new_id) {
    assert(mesh);
    
    TABLE_DELETE(asset_manager->mesh_table, id, mesh->id);

    // note that TABLE_ADD sets mesh->id to new_id
    TABLE_ADD(asset_manager->mesh_table, new_id, mesh);
    
    r_set_mesh_id(mesh->id, new_id);
}

void init_asset_update_queue(Arena_Allocator *game_arena, Asset_Update_Queue *queue) {
    *queue = {};

    int32 queue_arena_size = MEGABYTES(8);
    Arena_Allocator queue_arena = make_arena_allocator(arena_push(game_arena, queue_arena_size),
                                                       queue_arena_size);
    queue->arena = queue_arena;
    queue->critical_section = platform_make_critical_section();
}

void deinit_asset_update_queue(Asset_Update_Queue *queue) {
    // this should be done after stopping the directory watching thread
    platform_delete_critical_section(&queue->critical_section);
}

void push_update(Asset_Update_Queue *queue, Asset_Update update) {
    platform_enter_critical_section(&queue->critical_section);

    assert(queue->num_updates < MAX_ASSET_UPDATES);
    
    // copy the update
    switch (update.type) {
        case ASSET_UPDATE_FILENAME_RENAMED: {
            update.filename_change.old_filename = copy((Allocator *) &queue->arena,
                                                       update.filename_change.old_filename);
            update.filename_change.new_filename = copy((Allocator *) &queue->arena,
                                                       update.filename_change.new_filename);
        } break;
        case ASSET_UPDATE_MODIFIED: {
            update.filename = copy((Allocator *) &queue->arena,
                                   update.filename);
        } break;
        default: {
            assert(!"Unhandled asset file update type!");
        } break;
    }

    queue->updates[queue->num_updates++] = update;

    platform_leave_critical_section(&queue->critical_section);
}

void handle_mesh_update(String filename) {
    Mesh *mesh = get_mesh_by_path(filename);

    if (!mesh) {
        return;
    }

    // ignore_file_in_use here, since sometimes when we save in another program,
    // ex: blender, we get an update, but the file is still in use. we do still
    // get another update after that one where the file is not still in use.
    // see: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
    // - "When writing to a file, the last write time is not fully updated until all handles
    //    used for writing have been closed."
    bool32 result = refresh_mesh(mesh, true);

    if (!result) {
        // this is expected when another process has access to the file.
        // hopefully when they let go of the handle, we get an update? idk
        debug_print("refresh_mesh() failed in update from file.");
    }
}

void handle_texture_update(String filename) {
    Texture *texture = get_texture_by_path(filename);

    if (!texture) {
        // just in case it got deleted previously during the frame
        return;
    }

    bool32 result = refresh_texture(texture);
    if (!result) {
        debug_print("refresh_texture() failed in update from file.");
    }
}

void handle_animation_update(String filename) {
    Skeletal_Animation *animation = get_animation_by_path(filename);

    if (!animation) {
        return;
    }

    bool32 result = refresh_animation(animation, true);

    if (!result) {
        debug_print("refresh_animation() failed in update from file.");
    }
}

void handle_asset_updates_from_queue(Asset_Update_Queue *queue, Handle_Asset_Update_Callback handle_update) {
    platform_enter_critical_section(&queue->critical_section);

    for (int32 i = 0; i < queue->num_updates; i++) {
        Asset_Update *update = &queue->updates[i];
        if (update->type == ASSET_UPDATE_FILENAME_RENAMED) {
            // TODO: i'm actually not even sure if we should handle this case..
            //       because we would also end up having to update the level file
            //       with the new filepaths
            //       - i guess if we haven't saved already then it's fine
            //       - also we shouldn't rename non-user added meshes, but i guess
            //         it's fine if it happens. we'll just crash.
#if 0
            Mesh *mesh = get_mesh_by_path(update->old_filename);
            assert(mesh);
            replace_contents(&mesh->filename, update->new_filename);
#endif
        } else if (update->type == ASSET_UPDATE_MODIFIED) {
            handle_update(update->filename);
        }
    }

    // this stuff should be done in the same critical section so that
    // we don't miss any updates.
    clear_arena(&queue->arena);
    queue->num_updates = 0;

    platform_leave_critical_section(&queue->critical_section);
}

void update_assets_from_queues() {
    handle_asset_updates_from_queue(&asset_manager->mesh_update_queue, &handle_mesh_update);
    handle_asset_updates_from_queue(&asset_manager->texture_update_queue, &handle_texture_update);
    handle_asset_updates_from_queue(&asset_manager->animation_update_queue, &handle_animation_update);
}

// called by the directory watcher
void asset_file_update_callback(Asset_Type type,
                                Asset_Update_Queue *asset_update_queue,
                                Allocator *temp_stack,
                                Directory_Change_Type change_type, WString path,
                                WString old_path = {}, WString new_path = {}) {
    // note that this callback runs on the file watcher thread

    Allocator *temp_region = begin_region(temp_stack);

#if 0
    char filepath_c_str[MAX_PATH];
    platform_wide_char_to_multi_byte(path, filepath_c_str, MAX_PATH);
    String filepath = make_string(filepath_c_str);
#endif

    //OutputDebugStringA("mesh_changed\n");

    Asset_Update update = {};
    
    switch (change_type) {
        case DIR_CHANGE_FILE_MODIFIED: {
            String filepath = platform_wide_char_to_multi_byte(temp_region, path);

            // mark it for update
            update.type = ASSET_UPDATE_MODIFIED;
            update.filename = filepath;
        } break;
        case DIR_CHANGE_FILE_RENAMED: {
            // TODO: we need to handle the rename event, because we also get a FILE_MODIFIED
            //       event for the new named file, but it doesn't exist, so we assert.
            // NOTE: we aren't doing the update right now.. so the mesh won't be renamed right away
            // - this is annoying.. the updates should be by id.
            // - when we get the old_name event, save the id
            // - when the new_name event comes in, look for the old_name, make a special event for this
            // - TODO: we probably want to store some more information in the update queue
            
            // - we get the rename_old event
            //   - we save it in the update queue with the old name
            //   - we need to assume that the previous one will always be old_name...

            // - just save a single old_name thing in the asset_manager, then when you get the new_name,
            //   then add something to the queue
            // - what if we get multiple old/new name events in the same loop???
            //   - fuck, maybe it is just easier to try and update it...
            //   - wait, actually, all you need to do is when you get old_name, store old_name->id
            //     - then, any old_name/new_name pairs, just search for it in the map, if it exists,
            //       replace old_name, if it doesn't create a new entry. that's pretty simple, i think.

            // - just store all the old name, new names
            // - why don't we just store all the updates?
            //   - then just loop through and do the updates
            //   - that makes this callback super simple..
            //   - and it's better that it's simple because it's happening on a different thread
            // - it's possible that the user deletes the mesh or whatever asset before we run these updates
            //   - i don't think that's an issue; we just ignore the change because the assets gone
            //   - that seems like completely fine and expected behaviour

            // push_update will copy the strings to the queue's arena
            assert(!is_empty(old_path));
            assert(!is_empty(new_path));
            String old_filepath = platform_wide_char_to_multi_byte(temp_region, old_path);
            String new_filepath = platform_wide_char_to_multi_byte(temp_region, new_path);

            update.type = ASSET_UPDATE_FILENAME_RENAMED;
            update.filename_change.old_filename = old_filepath;
            update.filename_change.new_filename = new_filepath;
                
            // TODO: handle other change types
        } break;
        default: {
            assert(!"Unhandled directory update type!");
        } break;
    }

    assert(update.type != ASSET_UPDATE_NONE);
    push_update(asset_update_queue, update);

    end_region(temp_region);
}

void mesh_file_update_callback(Allocator *temp_stack,
                               Directory_Change_Type change_type, WString path,
                               WString old_path = {}, WString new_path = {}) {
    asset_file_update_callback(Asset_Type::MESH,
                               &asset_manager->mesh_update_queue,
                               temp_stack,
                               change_type, path,
                               old_path, new_path);
}

void texture_file_update_callback(Allocator *temp_stack,
                               Directory_Change_Type change_type, WString path,
                               WString old_path = {}, WString new_path = {}) {
    asset_file_update_callback(Asset_Type::TEXTURE,
                               &asset_manager->texture_update_queue,
                               temp_stack,
                               change_type, path,
                               old_path, new_path);
}

void animation_file_update_callback(Allocator *temp_stack,
                               Directory_Change_Type change_type, WString path,
                               WString old_path = {}, WString new_path = {}) {
    asset_file_update_callback(Asset_Type::ANIMATION,
                               &asset_manager->animation_update_queue,
                               temp_stack,
                               change_type, path,
                               old_path, new_path);
}

Skeletal_Animation *get_animation(int32 id) {
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);

    Skeletal_Animation *current = asset_manager->animation_table[hash];
    while (current) {
        if (current->id == id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

Skeletal_Animation *get_animation(String name) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Skeletal_Animation *current = asset_manager->animation_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

Skeletal_Animation *get_animation_by_path(String path) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Skeletal_Animation *current = asset_manager->animation_table[i];
        while (current) {
            if (path_equals(current->filename, path)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

// replace any entities with mesh_id with default mesh id
void replace_entity_meshes(int32 mesh_id_to_replace, int32 new_mesh_id) {
    Mesh *default_mesh = get_mesh(make_string("cube"));
    
    Entity *current = game_state->level.entities;
    while (current) {
        // no need to check for ENTITY_MESH here since all entities still have
        // mesh_ids, even if they don't have that flag set. adding that check
        // would make it so their mesh_id doesn't get deleted, so enabling that
        // flag after would result in an invalid mesh id.
        if (current->mesh_id == mesh_id_to_replace) {
            set_mesh(current, new_mesh_id);
        }

        current = current->next;
    }
}

// delete a mesh without replacing entities with that mesh
void delete_mesh_no_replace(int32 id) {
    Mesh *mesh = get_mesh(id);

    if (!mesh) {
        assert(!"Mesh does not exist.");
        return;
    }
    
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);
    
    if (mesh->table_prev) {
        mesh->table_prev->table_next = mesh->table_next;
    } else {
        // if we're first in list, we need to update bucket array when we delete
        asset_manager->mesh_table[hash] = mesh->table_next;
    }

    if (mesh->table_next) {
        mesh->table_next->table_prev = mesh->table_prev;
    }

    unwatch_directory(&asset_manager->mesh_dir_watchers, mesh->watcher_id);
    
    deallocate(mesh);
    deallocate(asset_manager->allocator, mesh);

    r_unload_mesh(id);
}

void delete_mesh(int32 id) {
    delete_mesh_no_replace(id);

    // set entity meshes to default if they had the deleted mesh
    Mesh *default_mesh = get_mesh(make_string("cube"));
    assert(default_mesh);
    replace_entity_meshes(id, default_mesh->id);
}

bool32 load_mesh(Allocator *allocator, Mesh **mesh_result, Mesh_Type type, String name, String filename,
                 bool32 *is_in_use = NULL) {
    Allocator *temp_region = begin_region();

    char *c_filename = to_char_array(temp_region, filename);
    File_Data file_data = platform_open_and_read_file(temp_region, filename, is_in_use);

    if (!file_data.contents) {
        char *error = "Could not open mesh file";
        debug_print(error);
        //add_message(Context::message_manager, make_string(error));
        end_region(temp_region);
        return false;
    }

    char *error;

    Mesh *mesh;
    bool32 result = Mesh_Loader::load_mesh(allocator, file_data, &mesh, &error);
    if (result) {
        mesh->type     = type;
        mesh->name     = copy(allocator, name);
        mesh->filename = copy(allocator, filename);
        *mesh_result = mesh;
    } else {
        // we can call load_mesh before we're rendering, so.. just print the error too
        debug_print(error);
        add_message(message_manager, make_string(error));
    }
    
    end_region(temp_region);

    return result;
}

bool32 mesh_exists(String name) {
    Mesh *mesh = get_mesh(name);
    return mesh != NULL;
}

bool32 refresh_mesh(Mesh *mesh, bool32 ignore_file_in_use) {
    int32 id = mesh->id;

    Mesh *new_mesh;
    bool32 is_in_use;
    bool32 result = load_mesh(asset_manager->allocator, &new_mesh, mesh->type, mesh->name, mesh->filename,
                              &is_in_use);
    if (!result) {
        if (is_in_use && ignore_file_in_use) {
            debug_print("Mesh loading failed. File was in use, but ignoring.");
        } else {
            assert(!"Mesh loading failed.");
        }
        
        return false;
    }

    // if we succeed in loading, delete the old one
    delete_mesh_no_replace(id);

    // note that we don't just call add_mesh because this is a bit more special case.
    // we may want to extract out load_mesh so that we can do a similar thing we did
    // for textures.. which is a bit cleaner than this.
    new_mesh->id = id;
    TABLE_ADD(asset_manager->mesh_table, id, new_mesh);

    // delete_mesh_no_replace calls r_unload_mesh, so we need to load it again, except with
    // the new data.
    r_load_mesh(id);

    // watch the directory again, since we didn't call add_mesh()
    String_Buffer buf = make_string_buffer(asset_manager->allocator, make_string(""), 128);
    append_string(&buf, "adding mesh watcher for: ");
    append_string(&buf, new_mesh->filename);
    add_message(message_manager, make_string(buf));
    Directory_Watcher *watcher = watch_directory_for_file(asset_manager->allocator,
                                                          &asset_manager->mesh_dir_watchers,
                                                          new_mesh->filename, mesh_file_update_callback);
    new_mesh->watcher_id = watcher->id;

    return true;
}

Mesh *add_mesh(String name, String filename, Mesh_Type type, int32 id) {
    if (mesh_exists(name)) {
        assert(!"Mesh with name already exists.");
        return NULL;
    }

    Mesh *mesh;
    // note that this should be called before we set mesh->id, or else we would overwrite
    // the mesh->id value with 0
    bool32 result = load_mesh(asset_manager->allocator, &mesh, type, name, filename);
    if (!result) {
        assert(!"Mesh loading failed.");
        return NULL;
    }

    if (type == Mesh_Type::LEVEL) {
        if (id < 0) {
            id = asset_manager->total_meshes_added_ever++;
        }
    } else {
        assert(id < 0);
    }
    
    Mesh *found_mesh = get_mesh(id);
    if (found_mesh) {
        assert(!"Mesh with ID already exists!");
    } else {
        mesh->id = id;
    }
    
    uint32 hash = get_hash(mesh->id, NUM_TABLE_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    mesh->table_next = current;
    mesh->table_prev = NULL;
    if (current) {
        current->table_prev = mesh;
    }
    asset_manager->mesh_table[hash] = mesh;

    r_load_mesh(mesh->id);
    
    // watch the directory
    String_Buffer buf = make_string_buffer(asset_manager->allocator, make_string(""), 128);
    append_string(&buf, "adding mesh watcher for: ");
    append_string(&buf, filename);
    add_message(message_manager, make_string(buf));
    Directory_Watcher *watcher = watch_directory_for_file(asset_manager->allocator,
                                                          &asset_manager->mesh_dir_watchers,
                                                          filename, mesh_file_update_callback);
    mesh->watcher_id = watcher->id;
    
    return mesh;
}

inline Mesh *add_mesh(char *name, char *filename, Mesh_Type type, int32 id) {
    return add_mesh(make_string(name), make_string(filename), type, id);
}

void set_mesh_file(int32 id, String new_filename) {
    Mesh *mesh = get_mesh(id);
    assert(mesh);

    Allocator *temp_region = begin_region();
    String name = copy(temp_region, mesh->name);
    Mesh_Type type = mesh->type;

    // delete it, then add it back. we keep the mesh id the same because
    // we don't want any lists to change order, i.e., it should basically
    // appear like we're really modifying the mesh. also this allows us
    // not to have to replace any entity meshes with a new ID
    delete_mesh_no_replace(id);
    add_mesh(name, new_filename, type, id);

    end_region(temp_region);
}

bool32 animation_exists(String name) {
    Skeletal_Animation *animation = get_animation(name);
    return animation != NULL;
}

bool32 refresh_animation(Skeletal_Animation *animation, bool32 ignore_file_in_use) {
    int32 id = animation->id;

    char *error;
    Skeletal_Animation *new_animation;
    bool32 is_in_use;
    bool32 result = Animation_Loader::load_animation(asset_manager->allocator,
                                                     animation->name, animation->filename,
                                                     &new_animation, &error, &is_in_use);
    if (!result) {
        if (is_in_use && ignore_file_in_use) {
            debug_print("Animation loading failed. File was in use, but ignoring.");
        } else {
            assert(!"Mesh loading failed.");
        }
        
        return false;
    }

    delete_animation_no_replace(id);
    add_animation(new_animation, id);

    return true;
}

Skeletal_Animation *add_animation(Skeletal_Animation *loaded_animation, int32 id) {
    assert(loaded_animation);
    assert(loaded_animation->id == 0);

    if (id < 0) {
        id = asset_manager->total_animations_added_ever++;
    }
    
    Skeletal_Animation *found_animation = get_animation(id);
    if (found_animation) {
        assert(!"Animation with ID already exists!");
    } else {
        loaded_animation->id = id;
    }
    
    uint32 hash = get_hash(loaded_animation->id, NUM_TABLE_BUCKETS);

    Skeletal_Animation *current = asset_manager->animation_table[hash];
    loaded_animation->table_next = current;
    loaded_animation->table_prev = NULL;
    if (current) {
        current->table_prev = loaded_animation;
    }
    asset_manager->animation_table[hash] = loaded_animation;

    // watch the directory
    String_Buffer buf = make_string_buffer(asset_manager->allocator, make_string(""), 128);
    append_string(&buf, "adding animation watcher for: ");
    append_string(&buf, loaded_animation->filename);
    add_message(message_manager, make_string(buf));
    Directory_Watcher *watcher = watch_directory_for_file(asset_manager->allocator,
                                                          &asset_manager->animation_dir_watchers,
                                                          loaded_animation->filename,
                                                          animation_file_update_callback);
    loaded_animation->watcher_id = watcher->id;
    
    return loaded_animation;
}

Skeletal_Animation *add_animation(String name, String filename, int32 id = -1) {
    if (animation_exists(name)) {
        assert(!"Animation with name already exists.");
        return NULL;
    }

    Allocator *allocator = asset_manager->allocator;
    Skeletal_Animation *animation;
    char *error;

    // note that this should be called before we set animation->id, or else we would overwrite
    // the animation->id value with 0
    bool32 result = Animation_Loader::load_animation(allocator, name, filename, &animation, &error);
    if (!result) {
        debug_print(error);
        add_message(message_manager, make_string(error));

        return NULL;
    }

    return add_animation(animation, id);
}

// remove animations from entities that have animation_id_to_remove
void remove_entity_animations(int32 animation_id_to_remove) {
    Entity *current = game_state->level.entities;
    while (current) {
        if (current->animation_id == animation_id_to_remove) {
            set_animation(current, NULL);
        }

        current = current->next;
    }
}

// delete a animation without replacing entities with that animation
void delete_animation_no_replace(int32 id) {
    Skeletal_Animation *animation = get_animation(id);

    if (!animation) {
        assert(!"Animation does not exist.");
        return;
    }
    
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);
    
    if (animation->table_prev) {
        animation->table_prev->table_next = animation->table_next;
    } else {
        // if we're first in list, we need to update bucket array when we delete
        asset_manager->animation_table[hash] = animation->table_next;
    }

    if (animation->table_next) {
        animation->table_next->table_prev = animation->table_prev;
    }
    
    deallocate(animation);
    deallocate(asset_manager->allocator, animation);

    unwatch_directory(&asset_manager->animation_dir_watchers, animation->watcher_id);
}

void delete_animation(int32 id) {
    delete_animation_no_replace(id);
    remove_entity_animations(id);
}

void set_animation_file(int32 id, String new_filename) {
    Skeletal_Animation *animation = get_animation(id);
    assert(animation);

    Allocator *temp_region = begin_region();
    String name = copy(temp_region, animation->name);

    // delete it, then add it back. we keep the animation id the same because
    // we don't want any lists to change order, i.e., it should basically
    // appear like we're really modifying the animation. also this allows us
    // not to have to replace any entity animations with a new ID
    delete_animation_no_replace(id);
    add_animation(name, new_filename, id);

    end_region(temp_region);
}

Texture *get_texture(int32 id) {
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    while (current) {
        if (current->id == id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

Texture *get_texture(String name) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Texture *current = asset_manager->texture_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

Texture *get_texture_by_path(String path) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Texture *current = asset_manager->texture_table[i];
        while (current) {
            if (path_equals(current->filename, path)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

void delete_texture_no_replace(int32 id) {
    Texture *texture = get_texture(id);

    if (!texture) {
        assert(!"Texture does not exist.");
        return;
    }
    
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);
    
    if (texture->table_prev) {
        texture->table_prev->table_next = texture->table_next;
    } else {
        // if we're first in list, we need to update bucket array when we delete
        asset_manager->texture_table[hash] = texture->table_next;
    }

    if (texture->table_next) {
        texture->table_next->table_prev = texture->table_prev;
    }
    
    deallocate(texture);
    deallocate(asset_manager->allocator, texture);

    unwatch_directory(&asset_manager->texture_dir_watchers, texture->watcher_id);
    
    r_unload_texture(id);
}

void get_texture_names(Allocator *allocator, char **names, int max_names, int *num_names) {
    int32 num_texture_names = 0;
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Texture *current = asset_manager->texture_table[i];
        bool32 should_exit = false;
        while (current) {
            if (num_texture_names >= max_names) {
                assert(num_texture_names < max_names);
                should_exit = true;
                break;
            }
            names[num_texture_names++] = to_char_array(allocator, current->name);
            current = current->table_next;
        }

        if (should_exit) {
            break;
        }
    }

    *num_names = num_texture_names;
}

void replace_texture_if_equal(int32 *texture_id_to_replace, int32 id) {
    if (*texture_id_to_replace == id) {
        *texture_id_to_replace = 0;
    }
}

void delete_texture(int32 id) {
    delete_texture_no_replace(id);
    
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        while (current) {
            replace_texture_if_equal(&current->albedo_texture_id, id);
            replace_texture_if_equal(&current->metalness_texture_id, id);
            replace_texture_if_equal(&current->roughness_texture_id, id);

            current = current->table_next;
        }
    }
}

#if 0
void delete_texture(String name) {
    uint32 hash = get_hash(name, NUM_TABLE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            if (current->table_prev) {
                current->table_prev->table_next = current->table_next;
            } else {
                // if we're first in list, we need to update bucket array when we delete
                asset_manager->texture_table[hash] = current->table_next;
            }
            
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            deallocate(current->name);
            deallocate(current->filename);
            deallocate(asset_manager->allocator, current);

            r_unload_texture(name);
            
            return;
        }

        current = current->table_next;
    }

    assert(!"Texture does not exist.");
}
#endif

bool32 texture_exists(String name) {
    Texture *texture = get_texture(name);
    return texture != NULL;
}

// no need for ignore_file_in_use bool here because that's handled in GL code
bool32 refresh_texture(Texture *texture) {
    // we might create a new render command for reloading textures
    // - because i mean, we don't actually have things on the CPU side we need to update
    // - we just need to refresh the image data, which is in game_gl
    // - we don't save the data on the CPU for textures, unlike meshes. we just load it
    //   temporarily and send it to the GPU.
    // - in other words, there's nothing that we need to refresh on the game data (CPU)
    //   side when textures refresh. for meshes, since we store that data on the CPU,
    //   we need to delete and reload the mesh on the CPU side when mesh files update.
    
    assert(texture);
    r_reload_texture(texture->id);

    return true;
}

Texture *add_texture(String name, String filename, Texture_Type type, int32 id = 0) {
    if (texture_exists(name)) {
        assert(!"Texture with name already exists.");
        return NULL;
    }

    Texture *texture  = (Texture *) allocate(asset_manager->allocator, sizeof(Texture), true);

    if (type == Texture_Type::LEVEL) {
        if (id <= 0) {
            id = ++asset_manager->total_textures_added_ever;
        }
    } else {
        // non-level assets have non-positive IDs
        // we make the default debug texture 0 just for easy initialization
        assert(id <= 0);
    }

    Texture *found_texture = get_texture(id);
    if (found_texture) {
        assert(!"Texture with ID already exists!");
    } else {
        texture->id = id;
    }

    texture->name     = copy(asset_manager->allocator, name);
    texture->filename = copy(asset_manager->allocator, filename);
    texture->type     = type;
    
    uint32 hash = get_hash(texture->id, NUM_TABLE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    texture->table_next = current;
    texture->table_prev = NULL;
    if (current) {
        current->table_prev = texture;
    }
    asset_manager->texture_table[hash] = texture;

    r_load_texture(texture->id);

    String_Buffer buf = make_string_buffer(asset_manager->allocator, make_string(""), 128);
    append_string(&buf, "adding texture watcher for: ");
    append_string(&buf, filename);
    append_string(&buf, "\n");
    add_message(message_manager, make_string(buf));
    debug_print(to_char_array((Allocator*)frame_arena, make_string(buf)));
    Directory_Watcher *watcher = watch_directory_for_file(asset_manager->allocator,
                                                          &asset_manager->texture_dir_watchers,
                                                          filename, texture_file_update_callback);
    texture->watcher_id = watcher->id;

    return texture;
}

inline Texture *add_texture(char *name, char *filename, Texture_Type type, int32 id = -1) {
    return add_texture(make_string(name), make_string(filename), type, id);
}

void set_texture_file(int32 id, String new_filename) {
    // this is based on set_texture_file()
    Texture *texture = get_texture(id);
    assert(texture);

    Allocator *temp_region = begin_region();
    String name = copy(temp_region, texture->name);
    Texture_Type type = texture->type;

    delete_texture_no_replace(id);
    add_texture(name, new_filename, type, id);

    end_region(temp_region);
}

Cube_Map *get_cube_map(int32 id) {
    Cube_Map *result;
    TABLE_FIND(asset_manager->cube_map_table, id, id, result);
    return result;
}

Cube_Map *get_cube_map(String name) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Cube_Map *current = asset_manager->cube_map_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

bool32 cube_map_exists(String name) {
    Cube_Map *cube_map = get_cube_map(name);
    return cube_map != NULL;
}

Cube_Map *add_cube_map(String name, String filenames[6], Cube_Map_Type type, int32 id = -1) {
    if (cube_map_exists(name)) {
        assert(!"Cube map with name already exists.");
        return NULL;
    }

    Cube_Map *cube_map = (Cube_Map *) allocate(asset_manager->allocator, sizeof(Cube_Map), true);

    if (type == Cube_Map_Type::LEVEL) {
        if (id <= 0) {
            id = ++asset_manager->total_cube_maps_added_ever;
        }
    } else {
        // non-level assets have non-positive IDs
        // we make the default debug cube_map 0 just for easy initialization
        assert(id <= 0);
    }

    Cube_Map *found_with_id = get_cube_map(id);
    if (found_with_id) {
        assert(!"Texture with ID already exists!");
    } else {
        cube_map->id = id;
    }

    cube_map->name      = copy(asset_manager->allocator, name);
    // array elements are already allocated in the struct; just need to copy the data
    copy_array(cube_map->filenames, asset_manager->allocator, filenames, 6);
    cube_map->type      = type;

    TABLE_ADD(asset_manager->cube_map_table, cube_map->id, cube_map);
    
    r_load_cube_map(cube_map->id);

    // no directory watching for now?
#if 0
    Directory_Watcher *watcher = watch_directory_for_file(asset_manager->allocator,
                                                          &asset_manager->texture_dir_watchers,
                                                          filename, texture_file_update_callback);
    texture->watcher_id = watcher->id;
#endif

    return cube_map;
}

bool32 material_exists(String name) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return true;
            }
            current = current->table_next;
        }
    }

    return false;
}

Material *add_material(Material_Info *material_info, Material_Type type) {
    if (material_exists(material_info->name)) {
        assert(!"Material with name already exists.");
        return NULL;
    }

    Allocator *allocator = asset_manager->allocator;
    Material *material = (Material *) allocate(allocator, sizeof(Material), true);

    material->type                 = type;
    material->id                   = asset_manager->total_materials_added_ever++;
    material->name                 = copy(allocator, material_info->name);
    material->flags                = material_info->flags;

    Texture *albedo_texture        = get_texture(material_info->albedo_texture_name);
    assert(albedo_texture);
    material->albedo_texture_id    = albedo_texture->id;
    material->albedo_color         = material_info->albedo_color;

    Texture *metalness_texture     = get_texture(material_info->metalness_texture_name);
    assert(metalness_texture);
    material->metalness_texture_id = metalness_texture->id;
    material->metalness            = material_info->metalness;

    Texture *roughness_texture     = get_texture(material_info->roughness_texture_name);
    assert(roughness_texture);
    material->roughness_texture_id = roughness_texture->id;
    material->roughness            = material_info->roughness;

    uint32 hash = get_hash(material->id, NUM_TABLE_BUCKETS);

    Material *current = asset_manager->material_table[hash];
    material->table_next = current;
    material->table_prev = NULL;
    if (current) {
        current->table_prev = material;
    }
    asset_manager->material_table[hash] = material;

    return material;
}

Material *get_material(String name) {
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

Material *get_material(int32 id) {
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);

    Material *current = asset_manager->material_table[hash];
    while (current) {
        if (current->id == id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void delete_material(int32 id) {
    Material *material = get_material(id);

    if (!material) {
        assert(!"Material does not exist.");
        return;
    }
    
    uint32 hash = get_hash(id, NUM_TABLE_BUCKETS);
    
    if (material->table_prev) {
        material->table_prev->table_next = material->table_next;
    } else {
        // if we're first in list, we need to update bucket array when we delete
        asset_manager->material_table[hash] = material->table_next;
    }

    if (material->table_next) {
        material->table_next->table_prev = material->table_prev;
    }
    
    deallocate(material);
    deallocate(asset_manager->allocator, material);

    // set entity materials to default if they had the deleted material
    Material *default_material = get_material(make_string("default_material"));
    
    Entity *current = game_state->level.entities;
    while (current) {
        if (current->flags & ENTITY_MATERIAL) {
            if (current->material_id == id) {
                set_material(current, default_material->id);
            }
        }

        current = current->next;
    }
}

void get_material_names(Allocator *allocator, char **names, int max_names, int *num_names) {
    int32 num_material_names = 0;
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        bool32 should_exit = false;
        while (current) {
            if (num_material_names >= max_names) {
                assert(num_material_names < max_names);
                should_exit = true;
                break;
            }
            names[num_material_names++] = to_char_array(allocator, current->name);
            current = current->table_next;
        }

        if (should_exit) {
            break;
        }
    }

    *num_names = num_material_names;
}

void get_mesh_names(Allocator *allocator, Mesh_Type types, char **names, int max_names, int *num_names) {
    int32 num_mesh_names = 0;
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Mesh *current = asset_manager->mesh_table[i];
        bool32 should_exit = false;
        while (current) {
            if (num_mesh_names >= max_names) {
                assert(num_mesh_names < max_names);
                should_exit = true;
                break;
            }

            if (current->type & types) {
                names[num_mesh_names++] = to_char_array(allocator, current->name);
            }

            current = current->table_next;
        }

        if (should_exit) {
            break;
        }
    }

    *num_names = num_mesh_names;
}

void get_animation_names(Allocator *allocator, char **names, int max_names, int *num_names) {
    int32 num_animation_names = 0;
    
    // make first one "None", for animation_id = -1
    assert(max_names >= 1);
    names[0] = "None";
    num_animation_names++;
    
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Skeletal_Animation *current = asset_manager->animation_table[i];
        bool32 should_exit = false;
        while (current) {
            if (num_animation_names >= max_names) {
                assert(num_animation_names < max_names);
                should_exit = true;
                break;
            }

            names[num_animation_names++] = to_char_array(allocator, current->name);
            current = current->table_next;
        }

        if (should_exit) {
            break;
        }
    }

    *num_names = num_animation_names;
}

Font_File *get_font_file(char *filename) {
    uint32 hash = get_hash(make_string(filename), NUM_TABLE_BUCKETS);

    Font_File *current = asset_manager->font_file_table[hash];
    while (current) {
        if (string_equals(current->filename, filename)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

Font_File *add_font_file(char *filename, File_Data font_file_data) {
    if (get_font_file(filename)) {
        assert(!"Font file already exists.");
        return NULL;
    }

    Font_File *font_file = (Font_File *) allocate(asset_manager->allocator, sizeof(Font_File));
    uint32 hash = get_hash(filename, NUM_TABLE_BUCKETS);

    Font_File *current = asset_manager->font_file_table[hash];
    font_file->table_next = current;
    font_file->table_prev = NULL;
    font_file->filename = filename;
    font_file->file_data = font_file_data;
    if (current) {
        current->table_prev = font_file;
    }

    asset_manager->font_file_table[hash] = font_file;

    return font_file;
}

bool32 font_exists(char *font_name) {
    uint32 hash = get_hash(font_name, NUM_TABLE_BUCKETS);

    Font *current = asset_manager->font_table[hash];
    while (current) {
        if (string_equals(current->name, font_name)) {
            return true;
        }

        current = current->table_next;
    }

    return false;
}

Font *add_font(char *font_name, char *font_filename,
               real32 font_height_pixels,
               int32 font_texture_width = 512, int32 font_texture_height = 512) {
    if (font_exists(font_name)) {
        assert(!"Font with name already exists.");
        return NULL;
    }

    Font *font = (Font *) allocate(asset_manager->allocator, sizeof(Font), true);
    
    font->height_pixels = font_height_pixels;
    font->texture_width = font_texture_width;
    font->texture_height = font_texture_height;
    
    File_Data font_file_data;
    Font_File *font_file = get_font_file(font_filename);
    if (!font_file) {
        font_file_data = platform_open_and_read_file(asset_manager->allocator, font_filename);
        add_font_file(font_filename, font_file_data);
    } else {
        font_file_data = font_file->file_data;
    }

    // get font info
    stbtt_fontinfo font_info;

    // NOTE: this assumes that the TTF file only has a single font and is at index 0, or else
    //       stbtt_GetFontOffsetForIndex will return a negative value.
    // NOTE: font_info uses the raw data from the file contents, so the file data allocation should NOT
    //       be temporary.
    stbtt_InitFont(&font_info, (uint8 *) font_file_data.contents,
                   stbtt_GetFontOffsetForIndex((uint8 *) font_file_data.contents, 0));
    font->scale_for_pixel_height = stbtt_ScaleForPixelHeight(&font_info, font_height_pixels);
    stbtt_GetFontVMetrics(&font_info, &font->ascent, &font->descent, &font->line_gap);
    font->font_info = font_info;
    font->file_data = font_file_data;

    // allocate memory for the baked characters
    int32 first_char = 32;
    int32 num_chars = 96;
    font->cdata = (stbtt_bakedchar *) allocate(asset_manager->allocator, num_chars * sizeof(stbtt_bakedchar), false);
    font->first_char = first_char;
    font->num_chars = num_chars;

    // NOTE: these font names are expected to be char array constants
    font->name = make_string(font_name);

    // bake it
    font->bitmap = (uint8 *) allocate(asset_manager->allocator, font->texture_width*font->texture_height);
    // NOTE: no guarantee that the bitmap will fit the font, so choose temp_bitmap dimensions carefully
    // TODO: we may want to maybe render this out to an image so that we can verify that the font fits
    int32 result = stbtt_BakeFontBitmap((uint8 *) font->file_data.contents, 0,
                                        font->height_pixels,
                                        font->bitmap, font->texture_width, font->texture_height,
                                        font->first_char, font->num_chars,
                                        font->cdata);
    assert(result > 0);
    font->is_baked = true;
    
    // add to table
    uint32 hash = get_hash(font_name, NUM_TABLE_BUCKETS);
    Font *current = asset_manager->font_table[hash];
    font->table_next = current;
    font->table_prev = NULL;
    
    if (current) {
        current->table_prev = font;
    }
    asset_manager->font_table[hash] = font;

    r_load_font(font->name);
    
    return font;
}

Font *get_font(String name) {
    uint32 hash = get_hash(name, NUM_TABLE_BUCKETS);

    Font *current = asset_manager->font_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

Font *get_font(char *name) {
    uint32 hash = get_hash(name, NUM_TABLE_BUCKETS);

    Font *current = asset_manager->font_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void unload_level_meshes() {
    Mesh **mesh_table = asset_manager->mesh_table;
    
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Mesh *current = mesh_table[i];
        while (current) {
            Mesh *next = current->table_next;

            if (current->type == Mesh_Type::LEVEL) {
                if (current->table_prev) {
                    current->table_prev->table_next = next;
                } else {
                    // if we're first in list, we need to update bucket array when we delete
                    mesh_table[i] = current->table_next;
                }
                
                if (current->table_next) {
                    current->table_next->table_prev = current->table_prev;
                }

                r_unload_mesh(current->id);
                deallocate(current);
                deallocate(asset_manager->allocator, current);

                if (current == mesh_table[i]) {
                    // if it's first in the list, then we need to update mesh table when we delete it
                    mesh_table[i] = next;
                }
            }
            
            current = next;
        }
    }
}

void unload_level_materials() {
    Material **material_table = asset_manager->material_table;

    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Material *current = material_table[i];
        while (current) {
            Material *next = current->table_next;

            if (current->type == Material_Type::LEVEL) {
                if (current->table_prev) {
                    current->table_prev->table_next = next;
                } else {
                    material_table[i] = current->table_next;
                }
            
                if (current->table_next) {
                    current->table_next->table_prev = current->table_prev;
                }
            
                deallocate(current);
                deallocate(asset_manager->allocator, current);

                if (current == material_table[i]) {
                    // if it's first in the list, then we need to update mesh table when we delete it
                    material_table[i] = next;
                }
            }

            current = next;
        }
    }
}

void unload_level_textures() {
    Texture **texture_table = asset_manager->texture_table;
    
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Texture *current = texture_table[i];
        while (current) {
            Texture *next = current->table_next;

            if (current->type == Texture_Type::LEVEL) {
                if (current->table_prev) {
                    current->table_prev->table_next = next;
                } else {
                    // if we're first in list, we need to update bucket array when we delete
                    texture_table[i] = current->table_next;
                }
                
                if (current->table_next) {
                    current->table_next->table_prev = current->table_prev;
                }

                r_unload_texture(current->id);
                deallocate(current);
                deallocate(asset_manager->allocator, current);

                if (current == texture_table[i]) {
                    // if it's first in the list, then we need to update texture table when we delete it
                    texture_table[i] = next;
                }
            }
            
            current = next;
        }
    }
}

void unload_level_animations() {
    Skeletal_Animation **animation_table = asset_manager->animation_table;
    
    for (int32 i = 0; i < NUM_TABLE_BUCKETS; i++) {
        Skeletal_Animation *current = animation_table[i];
        while (current) {
            Skeletal_Animation *next = current->table_next;

            if (current->table_prev) {
                current->table_prev->table_next = next;
            } else {
                // if we're first in list, we need to update bucket array when we delete
                animation_table[i] = current->table_next;
            }
                
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            deallocate(current);
            deallocate(asset_manager->allocator, current);

            if (current == animation_table[i]) {
                // if it's first in the list, then we need to update animation table when we delete it
                animation_table[i] = next;
            }
            
            current = next;
        }
    }
}

void load_level_assets(Level_Info *level_info) {
    // we always copy strings in here; i don't think functions that take strings should copy them.
    // i think it's better to assume that functions never copy strings and the caller of those
    // functions is responsible to copy them.
    for (int32 i = 0; i < level_info->num_meshes; i++) {
        Mesh_Info *mesh_info = &level_info->meshes[i];
        add_mesh(mesh_info->name,
                 mesh_info->filename,
                 Mesh_Type::LEVEL);
    }

    for (int32 i = 0; i < level_info->num_textures; i++) {
        Texture_Info *texture_info = &level_info->textures[i];
        add_texture(texture_info->name,
                    texture_info->filename,
                    Texture_Type::LEVEL);
    }

    for (int32 i = 0; i < level_info->num_materials; i++) {
        Material_Info *material_info = &level_info->materials[i];
        add_material(material_info, Material_Type::LEVEL);
    }

    for (int32 i = 0; i < level_info->num_animations; i++) {
        Animation_Info *animation_info = &level_info->animations[i];
        add_animation(animation_info->name,
                      animation_info->filename);
    }
}

// maybe a better word is delete, since we're just deleting the level assets from their tables
// but then we also unload them on the GPU
void unload_level_assets() {
    // we deallocate the assets
    // then in opengl code, we just unload all the level assets from the GPU, we don't need
    // the asset data at that point since we know which resources are for levels and which are not

    unload_level_meshes();
    unload_level_materials();
    unload_level_textures();
    unload_level_animations();
    
    asset_manager->gpu_should_unload_level_assets = true;
}

void load_default_assets() {
    // we need default assets for assets that are based on some file, for example meshes and textures.
    // this is because if we create a new mesh or a new texture, we need some default asset to use/display.
    // i guess we could just make a default texture just be no texture, but that's not ideal, since we would
    // ideally want some type of conspicuous texture that makes it obvious that an entity does not have a
    // texture.
    //
    // we don't need default materials. materials reference meshes and textures but materials themselves
    // don't directly require any files.

    add_mesh("gizmo_arrow",      "assets/meshes/engine/gizmo_arrow.mesh",  Mesh_Type::ENGINE, ENGINE_GIZMO_ARROW_MESH_ID);
    add_mesh("gizmo_ring",       "assets/meshes/engine/gizmo_ring.mesh",   Mesh_Type::ENGINE, ENGINE_GIZMO_RING_MESH_ID);
    add_mesh("gizmo_sphere",     "assets/meshes/engine/gizmo_sphere.mesh", Mesh_Type::ENGINE, ENGINE_GIZMO_SPHERE_MESH_ID);
    add_mesh("gizmo_cube",       "assets/meshes/engine/gizmo_cube.mesh",   Mesh_Type::ENGINE, ENGINE_GIZMO_CUBE_MESH_ID);
    add_mesh("capsule_cylinder", "assets/meshes/engine/capsule_cylinder.mesh",
             Mesh_Type::ENGINE, ENGINE_CAPSULE_CYLINDER_MESH_ID);
    add_mesh("capsule_cap",      "assets/meshes/engine/capsule_cap.mesh",
             Mesh_Type::ENGINE, ENGINE_CAPSULE_CAP_MESH_ID);
    add_mesh("cube",             "assets/meshes/engine/cube.mesh",  Mesh_Type::PRIMITIVE, ENGINE_DEFAULT_CUBE_MESH_ID);
    add_mesh("skinned_cube",     "assets/meshes/skinned_mesh_test.mesh", Mesh_Type::PRIMITIVE,
             ENGINE_DEFAULT_SKINNED_CUBE_MESH_ID);
    add_mesh("player_capsule",     "assets/meshes/capsule.mesh", Mesh_Type::PRIMITIVE,
             ENGINE_DEFAULT_PLAYER_CAPSULE_MESH_ID);

    // if you're seeing white borders around semi-transparent parts of exported PNGs, make sure the
    // fully transparent parts of your image have an RGB value of (0, 0, 0) (use eyedropper tool in
    // your photo editor).
    // though, this is probably only for when the semi transparent parts are also black, like with the
    // outlines of the light icons. when the fully transparent background wasn't RGB(0, 0, 0), opengl
    // was blending between the black outline and the white fully transparent background when scaling
    // the texture, which resulted in faint white artifacts on the edges of the outlines.
    add_texture("texture_default",   "assets/textures/debug-texture.jpg",          Texture_Type::DEFAULT,
                ENGINE_DEBUG_TEXTURE_ID);
    add_texture("lightbulb",         "assets/textures/lightbulb.png",         Texture_Type::ENGINE,
                ENGINE_LIGHTBULB_TEXTURE_ID);
    add_texture("editor_down_arrow", "assets/textures/editor_down_arrow.png", Texture_Type::ENGINE,
                ENGINE_EDITOR_DOWN_ARROW_TEXTURE_ID);
    add_texture("editor_check",      "assets/textures/editor_check.png",      Texture_Type::ENGINE,
                ENGINE_EDITOR_CHECK_TEXTURE_ID);
    add_texture("sun_icon",          "assets/textures/sun-icon.png",          Texture_Type::ENGINE,
                ENGINE_SUN_TEXTURE_ID);

    String skybox_files[6] = {
        make_string("assets/textures/skyboxes/night/right.png"),
        make_string("assets/textures/skyboxes/night/left.png"),
        make_string("assets/textures/skyboxes/night/top.png"),
        make_string("assets/textures/skyboxes/night/bottom.png"),
        make_string("assets/textures/skyboxes/night/front.png"),
        make_string("assets/textures/skyboxes/night/back.png")
    };

    add_cube_map(make_string("sky"), skybox_files, Cube_Map_Type::DEFAULT,
                 ENGINE_DEFAULT_SKYBOX_CUBE_MAP_ID);
    
    add_font("times32",         "c:/windows/fonts/times.ttf",    32.0f);
    add_font("times24",         "c:/windows/fonts/times.ttf",    24.0f);
    add_font("courier24b",      "c:/windows/fonts/courbd.ttf",   24.0f);
    add_font("calibri14",       "c:/windows/fonts/calibri.ttf",  14.0f);
    add_font("calibri14b",      "c:/windows/fonts/calibrib.ttf", 14.0f);
    add_font("calibri24b",      "c:/windows/fonts/calibrib.ttf", 24.0f);
    add_font("lucidaconsole18", "c:/windows/fonts/lucon.ttf",    18.0f);

    Material_Info material_info = default_material_info;
    
    material_info.name = make_string("default_material");
    add_material(&material_info, Material_Type::DEFAULT);
}

bool32 generate_asset_name(Allocator *allocator, char *asset_type, int32 max_attempts, int32 buffer_size,
                           String *result, bool32 (*exists_fn)(String)) {
    assert(exists_fn);
    
    int32 num_attempts = 0;
    Allocator *temp_region = begin_region();

    String_Buffer buffer = make_string_buffer(temp_region, buffer_size);
    bool success = false;

    char *zero_format = string_format(temp_region, "New %s", asset_type);
    char *n_format = string_format(temp_region, "New %s %%d", asset_type);
    
    while (num_attempts < max_attempts) {
        char *format = (num_attempts == 0) ? zero_format : n_format;
        string_format(&buffer, format, num_attempts + 1);
        if (!exists_fn(make_string(buffer))) {
            success = true;
            break;
        }

        num_attempts++;
    }

    if (success) {
        *result = make_string(allocator, buffer);
    } else {
        assert(!"Could not generate new asset name.");
    }

    end_region(temp_region);

    return success;
}
