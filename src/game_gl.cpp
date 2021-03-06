#include "platform.h"
#include "math.h"
#include "hash_table.h"
#include "game_gl.h"
#include "mesh.h"

// TODO (done): draw other 2D primitives: boxes, lines, etc
// TODO (done): 2D drawing functions using pixel position instead of percentages
// TODO (done): draw text
// TODO (done): mesh loading
// TODO (done): mesh rendering
// TODO (done): basic ui buttons
// TODO (done): mesh picking (ray vs triangle, convert cursor to ray)
// TODO (done): blender randomly crashes sometimes when running the export script, which is also causing issues
//              with the exported normals.
//              update: see the comment above the vertices_list assignment in game_export_2.py for more info.
// TODO (done): fix ray vs triangle test not working sometimes
// TODO (done): basic quaternions
// TODO (done): draw translation gizmo
// TODO (done): move entities using translation gizmo
// TODO (done): draw rotation gizmo
// TODO (done): rotate entities using rotation gizmo
// TODO (done): scale gizmo based on camera distance from gizmo, so that the gizmo stays big and clickable on screen
// TODO (done): textures
// TODO (done): add the Entity struct with the shared Entity info for easier entity access.
// TODO (done): actually send point_light_entity data to shaders (read multiple lights article on learnopengl.com)
// TODO (done): point lights
//              we don't need to do PBR right now - we can just do basic blinn-phong shading
// TODO (done): point light attenuation
// TODO (done): fix entity picking not working when entities are overlapping
//              (try light overlapping plane - the plane gets selected when you click on the light.)
//              this actually had to do with scaling and us not converting t_min back to world_space in
//              ray_intersects_mesh().
// TODO (done): use push buffer for UI elements
// TODO (done): switch to 0,0 in top left coordinate system for screen-space drawing
// TODO (done): first person camera movement
// TODO (done): disable hovering buttons when in freecam mode
// TODO (done): add struct for storing material information
// TODO (done): be able to get font metrics from game code (will have to init fonts in game.cpp, with game_gl.cpp
//              just holding the texture_ids for that font)
// TODO (done): nicer button rendering (center the text)
// TODO (done): move ortho clip matrix into render_state
// TODO (done): modify quad vbos to draw quads
// TODO (done): fix buttons being able to be set to hot/active behind layered UI
//              we could push layers and pop layers and just assert at end of the frame that current_layer == 0
// TODO (done): add remove key procedure for hash tables.
//              we should use open addressing, which is just storing all the values in an array instead of using
//              linked lists.
//       this is more cache-friendly. this also makes it easier to remove elements without fragmenting memory.
//       we may need to think about ways to have the tables dynamically resize. this would require the allocator
//       to be able to free arbitrary chunks of memory, so we could use something like a free list. or we can just
//       always store enough.
// TODO (done): make it so you can specify the max number of hash table slots.
//              (do this after replacing the hash table linked lists with arrays)
// TODO (done): memory alignment in allocate procedures and in ui push buffer
// TODO (done): make memory a global variable
/*
  we call do_slider()
  in Slider_State, we allocate a String_Buffer,
  at the end of the game update procedure, we do the same thing we do for removing hot if the element no longer
  exists. except this time, if it no longer exists, we delete the state object. to delete the state object,
  in the hash map in which the UI element states are stored, we make that slot free. and we also delete the
  string buffer.

  this requires:
  - TODO (done): hash map implementation that uses open addressing
  - TODO (done): figure out memory alignment
  - TODO (done): pool allocator for strings (fixed size array with ability to allocate and deallocate)
  - TODO (done): some type of hash map implementation that can store base classes (without having to store pointers)
  - just use the new hash map implementation with a variant struct, which is just a union of all the
  - derived structs
  - TODO (done): allocation of string buffers when a slider is first shown
  - TODO (done): deallocation of string buffers when a slider is no longer showing
*/
// TODO (done): fix issue when letting go of slider UI outside of any UI element causes a mesh pick to happen,
//              which can cause the editor UI to go away, which is annoying. i think we can just check if there's
//              an active UI element and if so, don't mesh pick.
// TODO (done): closing material library
// TODO (done): basic UI slider
// TODO (done): material editing in editor
// TODO (done): list existing materials and be able to change an entity's active material

// TODO (done): basic texture library
//       - TODO (done): show a box when the texture button is clicked
//       - TODO (done): use hash table to store textures in game state
//       - TODO (done): move texture adding to game_state
//       - TODO (done): modify image_button to also be able to have text
//       - TODO (done): draw image boxes for all the textures
//       - TODO (done): change material texture using texture library
// TODO (nevermind): standalone material library 
//                   - actually, i don't think there's really a point to a standalone material library. you
//                     usually want to look at materials on an entity, so it makes sense that material adding is
//                     also in the entity window.
// TODO (done): material creation
//       - TODO (done): add an add button next to material on the entity window
//       - TODO (done): use hash tables for storing materials
//       - TODO (done): create new material when add button is pressed
// TODO (done): switch to material and texture IDs instead material and texture names as hash table keys
//       - this is because names can change and it's annoying to constantly have to delete a hash table entry
//         and add it again just because the name changed.

// TODO (done): mesh library
//       - TODO (done): use a hash table to store meshes
//       - TODO (done): change the mesh name text into a button
//       - TODO (done): display mesh library box
// TODO (done): fix annoying UI padding and border stuff
// TODO (done): light properties in editor
// TODO (done): basic mesh adding
//       - TODO (done): put add button next to mesh button
//       - TODO (done): put edit button next to mesh button for editing mesh name
//       - TODO (done): copy-paste file dialog code from game.cpp
//       - TODO (done): load and add mesh when a file is picked
// TODO (done): don't use BMPs for textures; use PNG or something

// TODO (done): level saving
//       - TODO (done): figure out file format
//       - TODO (done): add save level button
//       - TODO (done): add windows code for opening save file dialog
//       - TODO (done): add basic windows code for file writing
//       - TODO (done): save a test file using save file dialog
//       - TODO (done): use temp files for writing files
//       - TODO (done): add level name textbox
//       - TODO (done): write some game data into the file (just write the level name)
//       - TODO (done): add filename to mesh struct
//       - TODO (done): write meshes to level file
//       - TODO (done): add win32 procedure to convert full paths to relative paths and use this for meshes
//       - TODO (done): write textures to level file
//       - TODO (done): write materials to level file
//       - TODO (done): write entities to level file

// TODO (done): level loading
//       in level loading, we should ensure that duplicates of mesh, texture, and material names do not exist.
//       - TODO (done): add hash table resetting procedure
//       - TODO (done): add common mesh table to game_state (to hold things like the gizmo meshes)
//       - TODO (done): switch to using a level struct to hold level data
//       - TODO (done): add procedure to clear the current level
//       - TODO (done): unload meshes and textures in opengl when a level is unloaded
//       - TODO (done): add open level button
//       - TODO (done): add procedure to parse and load level file
//       - TODO (done): tokenize the level file
//       - TODO (done): add level parser states
//       - TODO (done): level parsing
//       - TODO (done): write code for copying level data into another level
//       - TODO (done): load meshes into current_level's mesh_arena
//       - TODO (done): copy temp_level from parsing into game_state's current_level
//       - TODO (done): try loading a test level
//       - TODO (done): add filters to platform_open_file_dialog
//       - TODO (done): add new level button
//       - TODO (done): save without dialog if a saved level is opened and save with file dialog if new level
//       - TODO (done): save as button for saving a duplicate of a level
//       - TODO (done): clean up level box

// TODO (done): normal entity adding
//       - TODO (done): add 'add entity' button
//       - TODO (done): add primitive mesh table to Game_State
//       - TODO (done): add a cube mesh primitive to primitive_mesh_table
//       - TODO (done): try using mesh primitive in default level
//       - TODO (done): add mesh primitives to level writing
//                      - just add an entity property like 'mesh_primitive "cube"'
//       - TODO (done): mesh library filtering (all, level, primitives)
//       - TODO (done): add an entity with a primitive cube mesh by default on button click
//       - TODO (done): allow empty materials when saving and loading levels

// TODO (done): hash table iterators

// TODO (done): basic color selector
//       - TODO (done): draw rainbow quad
//       - TODO (done): add a hue slider UI element
//       - TODO (done): draw a quad who's color is based on a hue degree value
//       - TODO (done): draw a quad with the hsv gradient
//       - TODO (done): write code to convert mouse position to HSV value
//       - TODO (done): add an hsv picker UI element that takes in a hue in degrees and returns the HSV value for
//                      whatever the cursor selected.
//       - TODO (done): draw a circle at the HSV picker's cursor
//       - TODO (done): write procedure to generate vertices for a filled circle
//       - TODO (done): add procedure to draw filled circle
//       - TODO (done): draw filled circle with the actual selected color at the HSV picker's cursor
//       - TODO (done): write procedure to convert RGB to HSV
//       - TODO (done): add procedure for drawing a basic color picker
//       - TODO (done): hide color picker on click outside
//       - TODO (done): add Context namespace that has things like Game_State and Controller_State in it, so we don't
//                      have to constantly be passing them around
//       - TODO (done): try moving do_color_picker to UI code instead of having it in editor.cpp
//                      - we can still have the editor handle the color picker's state
//       - TODO (done): fix bug where if you let go of either the slider or the hsv picker outside of the color
//                      picker bounds, you hide the picker
//       - TODO (done): fix bug where if you have the color picker open and click another entity, that entity gets
//                      the same color applied
//       - TODO (done): store RGB values and HSL values with floats to prevent rounding error
//       - TODO (done): draw border around color picker
//       - TODO (done): add color picker to light color property
//       - TODO (done): layer support in rendering GUI
//               - basically allow us to push a new layer to guarantee that elements on that layer will be rendered
//                 after things on previous layers.
//       - TODO (done): remove all the old editor color picker code
//       - TODO (done): fix bug where color picker shows for a frame when opening edit panel again after closing it
//                      with color picker open
//       - TODO (done): write procedure for converting HSV to RGB

// TODO (done): optimize pick_entity() by checking against AABB before checking against triangles
// TODO (done): replace ray_intersects_triangle with faster algorithm

// TODO (done): basic walking on meshes
//       - TODO (done): add update_editor() procedure and move all editor updating code there
//       - TODO (done): add keyboard shortcut to toggle between edit mode and play mode
//       - TODO (done): don't render editor UI when in play mode
//       - TODO (done): add dt variable and FPS/dt display
//       - TODO (done): use dt in update_editor_camera()
//       - TODO (done): add is_walkable member for normal entities
//       - TODO (done): simulate gravity with player
//       - TODO (done): basic collision with triangles of walk mesh to stop player from falling through ground
//       - TODO (done): take into account the height of the player when setting the camera position
//       - TODO (done): reset cursor position when in first person mode
//       - TODO (done): don't show gizmo or wireframe if in PLAYING mode. don't reset the editor state, since it's
//                      convenient to be able to quickly resume what you were doing.
//       - TODO (done): add some type of is_grounded member to player
//       - TODO (done): write procedure to check if a point is inside a triangle (only need to do it in one plane)
//       - TODO (done): write procedure to convert some x, z coordinate to a point on some triangle on a mesh
//       - TODO (done): test x,z to triangle point procedure
//       - TODO (done): fix bug where closest_point_below_on_mesh doesn't work with certain transforms
//       - TODO (done): write ray vs. ray intersection test
//       - TODO (done): move player with WASD
//       - TODO (done): get and save triangle that the player lands on
//       - TODO (done): add GL code for drawing line in 3D
//       - TODO (done): move player on single triangle adjusted using normal; don't care about leaving triangle
//                      right now
//       - TODO (done): detect when player has left its triangle; maybe just is_grounded to true when that happens
//                      for now
//       - TODO (done): get the closest point on another triangle when the player has left a triangle
//       - TODO (done): set player's walk_state when moving to another triangle
//       - TODO (done): ignore current ground entity
//       - TODO (done): implement capsule vs triangle test
//                      (https://wickedengine.net/2020/04/26/capsule-collision-detection/)

//       - TODO (actually, don't think we need this): implement circle intersects triangle
//               - check if circle center is inside triangle, if not, then get closest distance between the
//                 circle's center and each of the triangle's edges. if either of the distances are less than
//                 the circle's radius, then it intersects.
//       - TODO (done): implement circle vs triangle intersection test
//       - TODO (done): if we leave the triangle, do another circle triangle intersection test on the walk mesh to
//                      find the next triangle
//       - TODO (done): fix weird collision when walkable entity's position is negative

// TODO (done): add collider information to entities
// TODO (done): draw colliders
// TODO (done): add is_walkable to entity boxes
// TODO (done): add is_walkable to levels
// TODO (done): fix bug where you can walk on the inside face of a mesh

// TODO (done): entity deleting (probably have to replace entity arrays with free lists or tables)
//       - TODO (done): replace normal_entities with table
//       - TODO (done): replace point_light_entities with table
//       - TODO (done): add delete entity button
//       - TODO (done): add deallocate procedures for entities, nothing to deallocate really, but hash_table
//                      needs one for deletion

// TODO (done): be able to add point light entities
// TODO (done): show lights as a light icon
// TODO (done): make clickable region for point light entities match the icon size
// TODO (done): remove meshes and materials from point light entities

// TODO (done): texture adding
//       - TODO (done): add an add texture button
//       - TODO (done): add texture image with file dialog
//       - TODO (done): texture name editing
// TODO (done): texture deletion
//       - TODO (done): add delete texture button
//       - TODO (done): delete texture when clicked
//       - TODO (done): fix assert fail when deleting a texture that multiple materials use
//       - TODO (done): delete texture in OpenGL state if the texture no longer exists in the level

// TODO (done): mesh deleting
// TODO (done): add basic free list for meshes
//       - TODO (done): just allocate a big block, when you deallocate, go through
//       all the free blocks and check if any of them end where you started, and if they do, then join the
//       free blocks and if the one after starts where the deallocated block ends, then join the end to that start
//       - we can use this to store mesh.data and mesh.indices, and just store a pointer to the allocator in Mesh
//       - TODO (done): fix bug where deallocating b overwrites c's header
//       - TODO (done): store level meshes in heap
//       - TODO (done): clear heap on level unload

// TODO (done): material deletion

// TODO (done): use heap for UI states
// TODO (done): use state for sliders
// TODO (done): add text box element that uses state
//       - i think we can just create a do_text_box_with_state procedure that just calls do_text_box, but passes
//         in its state's buffer, actually idk, if we should do that. we may want to update state depending on
//         whether or not the text box is active/hot and so that logic has to be hoisted out of do_text_box.
//       - although, i think we can use make_ui_text_box, since that's just for rendering
//       - we could actually just modify do_text_box to accept an argument called use_state
// TODO (done): modify material name box to use text box with state

// TODO (done): some type of messaging system that isn't in the game console, like toasts (messages that appear
//       then disappear after a few seconds). this would be nice for some type of feedback like for file saving.
//       - i don't think this needs to be part of the immediate mode UI, since we won't interact with it; they'll
//         just appear and disappear
//       - TODO (done): make message that disappears after a certain time
//       - TODO (done): have multiple messages disappear
//       - TODO (done): add a message manager to game_state and context, so other things can access it
//       - TODO (done): init the message manager
//       - TODO (done): add a button that creates messages
//       - TODO (done): show message in reverse chronological order, i.e. newest messages are at the top
//       - TODO (done): deallocate strings when gone
//       - TODO (done): styling - maybe center justify the text? and maybe use smaller font..

// TODO (done): material/texture/mesh name strings validation
//       check for duplicates and empties. it matters that we don't have duplicates since texture names are used
//       as keys in the opengl code. we don't store material structs in the opengl code, but it's better to bea
//       consistent. show a message using the messaging system.
//       - TODO (done): add material name exists procedure
//       - TODO (done): show message if material name is invalid
//       - TODO (done): give new materials default names
//       - TODO (done): add texture name exists procedure
//       - TODO (done): use textbox with state with texture name box
//       - TODO (done): texture name validation
//       - TODO (done): mesh name validation
// TODO (done): don't allow quotes or brackets in any level strings, i.e. in texture name, material name, etc.
// TODO (done): level name validation
//       - TODO (done): reset the state of the text box when a new level loads

// TODO (done): click slider for manual value entry
//       - TODO (done): make slider set state.is_text_box to true when clicked
//       - TODO (done): move text box UI logic to separate procedure
//       - TODO (done): use text box UI logic when slider is a text box
//       - TODO (done): render text box instead of slider when slider is a text box
//       - TODO (done): fix typing in text box
//       - TODO (done): fix value not getting set to text box value
//       - TODO (done): fix heap allocation bug when deallocating slider state from the UI heap
//               - happens randomly, just open slider and do manual entry on a bunch of entities and try to
//                 get it
//       - TODO (done): figure out how we want to handle slider min/max changing due to text box entry leaving bounds
//               - just clamp the text box result for now, although i think we will want to sometimes just be able to
//                 manually enter any value, even if it's outside of the initial bounds of the sliders

// TODO (done): fix rotation gizmos hitbox not being aligned
// TODO (done): scale gizmo
//       - TODO (done): draw cubes for scaling handles
//       - TODO (done): better switching between global and local transform gizmo
//       - TODO (done): scale gizmo picking
//       - TODO (done): local scaling with scale gizmo
//       - TODO (done): global scaling with scale gizmo

// TODO: editor history (undoing and redoing)
//       - TODO (done): doing an add entity
//       - TODO (done): add action to history
//       - TODO (done): undoing history
//       - TODO (done): add undo button
//       - TODO (done): test undoing
//               - TODO (done): make sure things are being deallocated when we loop around
//       - TODO (done): add redo button
//       - TODO (done): use start, end, and current indices instead
//       - TODO (done): undoing
//       - TODO (done): redoing history
//       - TODO (done): add actions for adding point light entities
//       - TODO (done): add actions for deleting entities
//       - TODO (done): reset history on level load
//       - TODO (done): add actions for transforms with gizmo
//       - TODO (nevermind): add actions for text box changes?
//               - i think it might be better to just have the actions tied to the state that is changed by the
//                 text boxes, and not tied to text box states. this is because text boxes and other UI stuff
//                 can go away between frames, so if for example you tried to undo or redo something in a text box,
//                 there's no guarantee that that text box is even there, unless we also had UI stuff in the history,
//                 which would be annoying, i think.
//       - TODO (done): add is_active procedure
//       - TODO (done): add last_frame_active field to UI
//       - TODO (done): add was_active procedure
//       - TODO (done): add transform action to history when changing slideable text boxes in entity box
//       - TODO (done): we may just want to save the entire entity in the action and just reset it. it's fine since
//               we don't allocate anything in entities. even if we did allocate things, we would just copy them
//               into the editor history heap then restore them. this would only be an issue if our entities were
//               huge. but it makes adding history actions for new entity fields a lot easier. if we want finer
//               control, we can still always just add actions for those specific fields.
//       - TODO (done): add procedure for copying old and new entity
//       - TODO (done): add editor procedure for entity modifications
//       - TODO (done): add editor undo procedure for entity modifications
//       - TODO (done): write a history_deallocate() procedure that handles specific deallocation needs
//       - TODO (done): change gizmos to use the new modify action
//       - TODO (done): add history to changing meshes
//       - TODO (done): add history to changing mesh names
//       - TODO (done): hide text boxes when undoing/redoing mesh name changes, so that we don't have to reset
//                      text box state.
//       - TODO (done): figure out how to save meshes in history
//               - save the file data?
//               - save the mesh struct?
//               - probably just save the mesh struct. that way we don't have to parse the file again and it'll
//                 actually be less memory too.
//       - TODO (done): write copy function for meshes
//       - TODO (done): add history to adding meshes
//       - TODO (done): add history to deleting meshes
//       - FIXME (done): we're writing over memory somewhere...
//                - you can see it often when you set a bunch of meshes to suzanne then delete then try and overwrite the history by undoing all the way and doing another action
//               - we were using the wrong size when zeroing memory in the heap_allocate() procedure (we were
//                 using the aligned data size, which includes the alignment and not the regular aligned size.)
//       - TODO: change other entity fields to use new modify action
//       - TODO (done): add history for material modification
//       - TODO (done): add history for material texture change
//       - TODO (done): add history for color changes
//       - TODO (done): add history for deleting materials
//       - TODO (done): add history for adding materials
//       - TODO (done): add history for material choosing
//       - TODO (done): add history for adding, deleting, and modifying textures
//       - TODO: add history for light falloff changes
//       - TODO: add history for light color changes
//       - TODO: hide use_color_override button and don't change it, since we want to keep its state when we undo
//               and redo. if there's no texture, then just disable the button, but don't change the actual value
//       - TODO: add history for use_color_override
//       - TODO: add shortcuts for undoing and redoing


// TODO (done): (refactor) store all the meshes in the same table so we don't need to constantly differentiate
//              between primitive and level meshes
//       - TODO (done): create Asset_Manager struct
//       - TODO (done): add procedures for adding level, primitive, and engine meshes
//       - TODO (done): remove old mesh tables from game_state
//       - TODO (done): init asset manager
//       - TODO (done): remove mesh_types, since all the meshes are in the same table, we can just use mesh_id
//       - TODO (done): update level loader to use asset_manager for meshes
//       - TODO (done): remove mesh_table from level struct
//       - TODO (done): make sure level loading and exporting works
//       - TODO (done): make sure asset_manager memory and tables for levels is being cleared on level load
//       - TODO (done): make sure opengl mesh table is being cleared when new level loads

// TODO (done): color picker with state, instead of storing state in editor_state
//       - TODO (done): add ui state struct for color picker
//       - TODO (done): remove color picker state from editor_state
//       - TODO (done): make do_color_picker() return some type of struct like the textbox

// TODO: (refactor)
//       - TODO (done): strip out most things
//       - TODO (done): create editor state with memory
//       - TODO (done): create intermediate level struct
//       - TODO (done): change level stuff to use new intermediate level struct
//       - TODO (done): separate asset_managers for editor and for game
//       - TODO (done): load level data into asset_manager (editor asset_manager right now, since we're just doing
//               the editor)
//       - TODO (done): load meshes
//       - TODO (done): load textures
//       - TODO (done): load materials
//       - TODO (done): load entities
//       - TODO (done): resolve entity asset IDs
//       - TODO (done): in level_info loading, ensure that duplicate names don't exist. do this BEFORE you load
//               the level since everything in level_info is temporary
//       - TODO (done): load new assets into opengl
//       - TODO (done): render entities

//       - TODO (done): unload opengl assets when changing levels
//       - TODO (done): gizmo
//       - TODO (done): work through errors for entity box stuff
//       - TODO (done): level exporting using new format
//       - TODO: editor UI stuff
//       - TODO: editor history
//       - TODO: other editor stuff
//       - TODO: load fonts into game as well
//       - TODO: unload opengl assets when switching between play and edit mode

// TODO (done): editor actions
//       - TODO (done): adding normal entities
//       - TODO (done): adding point light entities
//       - TODO (done): deleting entities
//       - TODO (done): entity changes
//       - TODO (done): mesh name changes
//       - TODO (done): mesh adding
//       - TODO (done): mesh deleting
//       - TODO (done): material changes
//       - TODO (done): material adding
//       - TODO (done): material deleting
//       - TODO (done): texture changing
//       - TODO (done): texture adding
//       - TODO (done): texture deleting
//       - TODO (done): point light changes
//       - TODO (done): load default level using new format

// TODO (done): switch to play mode
//       - for assets, we could just use the same asset_manager as the editor. for entities, well.. idk.
//       - maybe just have an editor_level to game_level procedure that just like copies all the assets and all the
//         entities to some struct in game_state.
//       - we also need a load_level procedure for loading a game level from a level file, but that can wait, i
//         think. (this would actually just be a conversion from level info struct to game level struct)
//       - we could export to a level info struct, but then that makes it more complicated when trying to share
//         data easily.
//       - TODO (done): write update_game() procedure
//       - TODO (done): write render_game() procedure in gl code
//       - TODO (done): step through code, make sure arena is being cleared when going from edit mode to game mode

// TODO (done): capsule vs triangle collision
//       - TODO (done): add debug entity with capsule collider and draw useful things from capsule_intersects_triangle
//       - TODO (done): fix penetration depth being larger than expected. it seems like it's the center of the
//               sphere to the penetration point instead of the point on the triangle to the penetration point.
//               - it was just a misunderstanding: penetration depth is from the closest point on the triangle to
//                 the center of the sphere you're testing against. so to get the vector to push it out, you need
//                 to push along that vector by an amount of radius - penetration_depth.
//       - TODO (done): fix clipping through cube that's in the ground
//       - TODO (done): debug the case where the penetration depth is larger for the other triangle, but that
//               triangle is not being set to the triangle we use to push it out.
//       - TODO (done): fix sliding through corner created by two meshes; i think we just need to compare the player
//               against all the meshes. yeah, right now we just break on the first intersection.
//       - TODO (done): collision response
//       - TODO (done): fix weird collision case in startup level
//              - TODO (done): add new ray_intersects_plane procedure that passes when t is < 0
//       - TODO (done): calculate the intersection point
//       - can ensuring that all walk meshes are convex help? yes: it means that you will never collide with
//         the walk mesh if you're walking on it.
//       - if an entity's collider mesh is convex, then if you collide with it, if you take the one with the largest
//         push-out vector, then that is guaranteed to push you out of the entity.
//       - we could always still do it the walk mesh / collider mesh way, where we separate the meshes.
//       - TODO (done): just use capsule with regular collision response, i.e. use the bottom cap to push up slopes
//               - TODO (done): remove the walk code
//               - TODO (done): do penetration collision response using displacement vector
//               - TODO (done): set walk state from do_colisions() code
//               - TODO (done): fix do_collisions making player's transform into NaNs
//               - TODO (done): do collisions with displacement vector, then with gravity vector
//                       OR just do collisions recursively?
//                       i think just do it recursively for now. well, we don't actually need to recurse; just
//                       loop, but also have a max loops assert in case we end up in a weird case. i think then
//                       we need to push the capsule out a bit more so we aren't always penetrating.
//               - TODO (done): just use regular basis based on camera rotation and not on triangle normal.
//               - TODO (done): update player capsule every time it gets moved due to an intersection
//               - TODO (done): set is_grounded based on the intersection point being on the bottom sphere of the
//                              player capsule
//               - TODO (done): smooth out walking down slopes by shooting ray down to see if there is a triangle close
//                       enough and just setting the player position to the intersection point if it exists
//                       - we can't just do was_grounded since when we walk on a single triangle, we are constantly
//                         alternating between is_grounded=false and is_grounded=true. so was_grounded is basically
//                         always true while we're walking, and thus we always end up calling the move to closest
//                         ground procedure, which is incorrect, since this makes us phase through walls and
//                         prevents us from walking up slopes.
//                       - ideally we would not keep alternating is_grounded and instead just keep is_grounded=true
//                         as long as we're on a surface. i think then we should actually call do_collisions() twice.
//                       - TODO (done): just use the current triangle's normal to calculate the walk basis only if it's a
//                               downward slope. otherwise, just use the regular basis. this way, we don't break
//                               stepping up small walls. also, with the getting the closest ground with a ray
//                               method, we actually end up going faster, just because of pythogorean's theorem. we
//                               could try correcting it? we would have to check if it's a slope though, since we don't
//                               want to end up in the air. we ideally want to verify that the faces are connected too,
//                               but i think it's fine if we assume that the displacements will be small and the ground
//                               mesh isn't filled with tiny triangles that we step over in a single frame. we can use
//                               both methods. the ray method for getting onto the slope, so we smoothly walk onto a
//                               downwards slope instead of moving then falling down, and the slope walk basis method
//                               for moving on the slope once we're on it.
//               - TODO (done): reomve unnecessary walk_state properties

// TODO (done): use face data in blender mesh export script
// TODO (done): fix crash with undoing/redoing add material
// TODO (done): string_format procedure that calculates necessary allocation size

// TODO: add collider controls to entities, like have collider be the mesh, or an OBB, or a sphere, etc.
// TODO: add control for specular strength, basically just be able to multiply intensity of spec
//       - also probably add control for spec color for both light and entity

// TODO: fix extreme lag when walking into house and when pushing against one of the walls
//                   - seems to have to do with when we don't solidify a mesh, or when the mesh's normals are backwards

// TODO: capsule vs AABB for optimization
//       - TODO: do this check first in capsule_intersects_mesh before checking triangles

// TODO: collision with OBBs
//       - TODO: implement OBBs
//       - TODO: implement capsule vs OBB test
//               - get the shortest distance between a line and an OBB, compare it to radius
//       - TODO: should maybe do sweeped capsule vs OBB test? or maybe just do more iterations if we pass some
//               maximum?

// TODO: the game data will store the level data in its own way that makes it more efficient. it can do this
//       since we don't need to do things like make it so entities can be deleted. we may just be able to
//       put it all into an arena.
// TODO: redo heap allocator, and just always align on 8 byte boundaries. having the boundary be 8 bytes
//       makes things a lot easier. you still can't easily just add a struct header.. but you can if you
//       have a member that's 8 bytes, i.e. if you have a pointer in your header, then alignment will be
//       handled for you and to get the header you can just subtract sizeof(header) from a pointer to some
//       allocation.

// TODO: error handling in level loading

// TODO: make_string() with formatting built in with arguments

// TODO: i'm pretty sure whenever we make string buffers using PLATFORM_MAX_PATH, we should be adding 1 to it,
//       since PLATFORM_MAX_PATH includes the null terminator.

// TODO: use sRGB framebuffers (this will also allow us to do gamma correct alpha blending, since everything will
//       be in linear space before converting)

// TODO: we may want to just use arrays again for level entities, since it's more cache-friendly. i'm not sure why
//       we even switched to using tables for them.
//       - TODO: figure out why we even started using hash tables for entities in the first place
//       - TODO: create an Array struct
//       - TODO: ideally, we just store entities in hash maps when editing, but when the game is playing, we just
//               put them in arrays. but we use different data structures depending on the access and modification
//               patterns of groups of entities
// TODO: when using MSAA, there are sometimes white artifacts in areas with sharp corners and specular highlights.
//       this can be fixed by just limiting gloss, but there are othere ways as well. see:
//       https://gamedev.stackexchange.com/questions/84186/fighting-aliasing-on-specular-highlights

// TODO: material duplicating
// TODO: entity duplicating

// TODO: instead of passing game_state to procedures like get_mesh_pointer (because we need the primitive mesh table),
//       create a new struct called Asset_Tables that holds pointers to the current level's mesh table and the
//       primitive mesh table

// TODO: (refactor) instead of having a bunch of different structs for different entities, well we would still have
//       different entities, but we would instead have a single struct called Entity with a union with the different
//       entity structs. each entity struct would have a flags member of the entity type.
//       - actually, we could kind of do a version of this now. we should replace Entity_Type with a uint32 of
//         entity type flags. then, we don't have to constantly make switch statements in places where we have to
//         do something like get an entity's mesh. and instead we could just create a has_mesh bitmask that just
//         ORs together all the entity types that have meshes. then we just do (entity_type & has_mesh_mask)
//         whenever we need to check if it has a mesh. but you would still need to have a switch statement to cast
//         it so that you can actually get the mesh_id... so i guess instead of having a union, whenever we need to
//         add new members to some entity type, we would just add those to the same entity struct. so we can just
//         do entity.material_id if it has a material.
//       - let's just stick with what we have for now.
// TODO: could we use something like an Entity_Variant struct for things like selected_entity?

// TODO: replace the transform values in the entity box with slideable text boxes
//       - TODO (done): create do_slider with no limits, would have to hide the slider when rendering
//       - TODO (done): replace position and scale texts
//       - TODO: euler angle entry in entity box
//       - TODO: replace rotation with slideable text boxes that use euler angles (we just convert euler angles to
//               quaternions to store the value - euler angles are just more user friendly)
// TODO: show an image button for the texture instead of a regular button under material properties
// TODO: better string format that uses win32 function for getting formatted string length
//       - allows you to not pass in size of the buffer and instead automatically allocates the needed amount

// TODO: better gizmo controls
//       - TODO: translation on a plane (little squares on the planes of the gizmo)
//       - TODO: bigger hitboxes on the controls or just scale the meshes
//       - TODO: maybe just draw lines for the rotation handles. this makes it kind of hard to select a handle
//               when the normal of the rotation plane is orthogonal to the view vector, but it's hard to control
//               the rotation at that angle anyways. the way you would do the picking is you do a ray vs plane and
//               then check if the intersection point is within some bounds of the circle, using the circle's
//               radius. in the orthogonal case, we may just want to hide the handle, like how blender does it.

// TODO: change do_text to use Strings?
//       - idk, i think it's fine that we use the frame arena, since it's guaranteed that the char array will still
//         exist when we go to render it
// TODO: convert text colors to linear space in draw_messages before fading, then convert back to gamma-space

// TODO: the UX of deleting materials and textures is kind of weird when an entity can have no material or texture
//       - the minus button can seem like you're removing the material/texture from the entity, but it's actually
//         deleting the material/texture. maybe we can make the minus button remove it from the entity, but if we
//         want to remove the material/texture, we do that from the material/texture library that opens when you
//         click the choose button
// TODO: use the MAX_* constants when we generate level tables for meshes/textures/materials

// TODO: for checking for memory leaks, it's actually kind of unreliable to use task manager, since we just take
//       a bunch of memory at the beginning and allocate from that. it would be better if we had an actual
//       memory monitoring gui.
// TODO: show message if we've hit a limit, such as with adding entities

// TODO: figure out a way to set and get meshes and materials easily of different entity types
//       - think we can just add get_mesh and set_mesh and pass in an entity then just do a switch block on
//         its type. that seems like the simplest solution.

// TODO: profiling (editor mode is super slow, most of the time is spent drawing UI)

// TODO: be able to make entities invisible (for walk meshes)

// TODO: entity list view

// TODO: fix issue where gizmo appears at last location for a frame before moving when adding a new entity
// TODO: prompt to save level if open pressed when changes have been made
// TODO: make sure after we've added entity deleting, that when entities are deleted, if the entity was being
//       used as a walk mesh, then the player's walk mesh properties are cleared. although, this might not be
//       an issue if we constantly reset is_grounded to false whenever we switch from editor to play mode
// TODO: be able to add point light entities, actually let's maybe wait until we replace the meshes with icons
// TODO: limit movement dt

// TODO: make struct that holds both the mesh type and mesh ID

// TODO: mesh parenting

// TODO: we can just add a star to the filename if a change has been made and needs to be saved
// TODO: dialog prompts.. (just use windows for this maybe?)
// TODO: keyboard shortcuts for level save/save as

// TODO: add collider information to levels

// TODO: implement granular steps if displacement vector is larger than some threshold value
// TODO: replace is_walkable with Walkable_Entity structs, i.e. a new Entity type.
//       we want to do this since we will probably replace the Mesh in the entity with a different type of
//       mesh. one that holds all the triangles so that we don't have to use indices. we also might use a BVH
//       so we don't have to check every single triangle all the time.
// TODO: cache triangles in a new array for walkable meshes (so that we don't have to use indices array)

// TODO: capsule collision and resolution with objects
// TODO: use oriented bounding boxes for collisions with objects instead of doing capsule vs triangle

// TODO: fix behaviour of going to higher triangle that's on the other side of an entity, but is inside
//       our walk radius, when we want to actually be on the triangle that's directly below us
//       - we can fix this by making walk_radius very small, but then that allows us to fall through gaps
//       - a different solution would be to check if the triangle is on the same entity or not
//       - actually, no, i think maybe we should just pick the closest that's within our radius?
//       - let's just make the walk radius small for now. i think that this can be fixed once we
//         implement capsule vs other colliders tests and collision responses. at that point, we might
//         also be able to use it for our walking as well.
//       - this also isn't really an issue since we can always author walk meshes manually with entities
//         that don't render, but are walkable

// TODO: use a while loop in platform_set_cursor_visible to make the API set the visibility to whatever is passed
//       in, since the current API is super annoying

// TODO: change level loading to use format similar to WAIT_FOR_ENTITY_BOOL, so we don't have to keep adding
//       new parser states for every property

// TODO: color selector improvements
//       - TODO: add sliders for RGB and HSV (should do manual input sliders first)
//       - TODO: draw little arrows on the side of the hue slider so that you can move the slider without hiding
//               the actual color with a line - we can just add a hitbox around where the current value is in
//               do_hue_slider(), and then when we draw it, draw arrows within that hitbox.

// TODO: text truncation when its containing box is too small

// TODO: change mesh parser to use new tokenizer_equals procedures, since we can actually read past bounds, since
//       file_contents is not null-terminated. when we read the file in, the size is just the size of the file,
//       which does not necessarily include a null-terminator.

// TODO: allocate Game_State in game_data_arena instead of having it on the stack
// TODO: add camera state to level
//       - would have to add way to set initial camera state, since camera can move

// TODO: add icons to some of the buttons for better recognition of buttons

// TODO: in-game console for outputting messages
// NOTE: we should not add functions to get materials or textures by their name, since, at least right now, their
//       names are NOT unique.
// TODO: error handling for mesh loading (should use in-game console when that's implemented)
// TODO: error handling for level loading

// TODO: scrollable UI region (mainly for material and texture libraries)

// TODO (done): make textbox use the string pool allocator and use UI states so we don't have to handle making the
//       string buffer ourselves
//       - this also allows us to validate the text box without having to create a temp buffer ourselves
//         (we would just validate the String_Buffer from the text box's state)
//       - use this in the material name textbox, since right now the material name can be empty
//       - this is kind of unnecessary since we use material and texture IDs now, materials and textures can have
//         an empty name
//       - actually, it is not unnecessary. materials and textures should not be allowed to have blank names,
//         since when an entity doesn't have a material or a material doesn't have a texture, the corresponding
//         name box is empty, which is useful to know what entities don't have a material or what materials
//         don't have a texture assigned.
//       - this isn't really that important right now, i don't think
// - textbox is created
// - create a string buffer of the size passed in
// - when the textbox loses focus or enter is pressed while it's active, it returns the text that was entered
// - or a struct that contains a boolean of whether or not a value was returned at all

// TODO: update string procedures to use memcpy (memcpy is probably optimized)

// TODO: be able to add and delete materials, textures, meshes
// TODO (done): make free list struct (start with using this for storing fixed length strings that could be deleted).
//              this can be used for storing names of materials and meshes. since when we rename a string, we can just
//              modify its string_buffer. and if the mesh or texture gets deleted, we can delete its strings.
//              this free list struct will also be used for text fields. since we often don't want a text field to
//              hold the direct contents of where it will eventually be stored. we will need to update our
//              immediate mode UI code to hold state for the UI elements.
//              (done note: this is our pool allocator)
// TODO: we may want to add some metadata to allocations, since if we're just given a pointer to some memory and
//       we want to deallocate it, it's impossible to know from where to deallocate it from unless its just known
//       where, such as when we deallocate the String_Buffer in UI_Slider_State.
// TODO: replace string arena with the string pool allocator

// TODO: entity instances? copies of an entity, where you can modify the parent entity and all the instances
//       will update as well.
// TODO: in the future we may want to have per entity/per entity instance materials or maybe material parameters.
//       it would be nice to have material parameters in general, so that multiple entities can have the same
//       material, but look slightly different.
// TODO: nicer UI (start with window to display selected entity properties)
// TODO: directional light (sun light)

// TODO: window resize handling (recreate framebuffer, modify display_output)

// TODO: game should have different Entity structs that have platform-independent data
//       that is then used by game_gl to render that data. for example: Text_Entity, which
//       is just some text with a font name and the game renders that.
//       there should be different make_x_entity procedures that accept that required data for that entity.
//       the entity shaders are handled by game_gl.cpp. all game does is say that some types of entities exist
//       and it is game_gl.cpp's job to render them.
//       this may make us want to have some time of push buffer for all entity types.
//       the alternative is to just have fixed size arrays for each entity type. the upside to that is that it's
//       less complex and most likely faster. the downside is that if the amounts of entity types that exist at
//       a given time differ by a large amount, you'll end up with a lot of unused memory.

// TODO: unicode support in win32 code
// TODO: unicode support with text drawing?

/*
you select an entity
some information of the entity is displayed
- position
- rotation
- entity name
- mesh name
 */

/*
This uses a left-handed coordinate system: positive x is right, positive y is up, positive z is into the screen.
Use your left hand to figure out the direction of the cross product of two vectors.
In 2D, (0, 0) is at the bottom left, positive x is right, positive y is up.
 */

#if 0
int32 gl_get_mesh_id(GL_State *gl_state, char *mesh_name) {
    Hash_Table<int32, GL_Mesh> *mesh_table = &gl_state->level_mesh_table;
    for (int32 i = 0; i < mesh_table->max_entries; i++) {
        Hash_Table_Entry<int32, GL_Mesh> *entry = &mesh_table->entries[i];
        if (!entry->is_occupied) continue;

        GL_Mesh *mesh = &entry->value;

        if (string_equals(make_string(mesh->name), make_string(mesh_name))) {
            return entry->key;
        }
    }

    return -1;
}
#endif

GL_Mesh make_gl_mesh(Mesh_Type mesh_type, uint32 vao, uint32 vbo, uint32 num_triangles) {
    GL_Mesh gl_mesh = { mesh_type, vao, vbo };
    return gl_mesh;
}

uint32 gl_create_shader(char *shader_source, uint32 shader_source_size, Shader_Type shader_type) {
    GLenum type = 0;
    if (shader_type == VERTEX) {
        type = GL_VERTEX_SHADER;
    } else if (shader_type == FRAGMENT) {
        type = GL_FRAGMENT_SHADER;
    }

    uint32 shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, (const char **) &shader_source, (int32 *) &shader_source_size);
    glCompileShader(shader_id);

    int32 success;
    char buffer[256];
    char error_buffer[256];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(shader_id, 256, NULL, error_buffer);
    if (!success) {
        snprintf(buffer, 256, "%s\n", error_buffer);
        debug_print(error_buffer);
        assert(success);
    }
    
    return shader_id;
}

uint32 gl_compile_and_link_shaders(char *vertex_shader_source, uint32 vertex_shader_source_size,
                                   char *fragment_shader_source, uint32 fragment_shader_source_size) {
    uint32 vertex_shader_id = gl_create_shader(vertex_shader_source, vertex_shader_source_size, VERTEX);
    uint32 fragment_shader_id = gl_create_shader(fragment_shader_source, fragment_shader_source_size, FRAGMENT);
    
    uint32 gl_program_id = glCreateProgram();
    glAttachShader(gl_program_id, vertex_shader_id);
    glAttachShader(gl_program_id, fragment_shader_id);
    glLinkProgram(gl_program_id);
    
    int32 success;
    char error_buffer[256];
    glGetProgramiv(gl_program_id, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(gl_program_id, 256, NULL, error_buffer);
    if (!success) {
        snprintf(error_buffer, 256, "%s\n", error_buffer);
        debug_print(error_buffer);
        assert(success);
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return gl_program_id;
}

// TODO: should these be inline?
inline void gl_set_uniform_mat4(uint32 shader_id, char* uniform_name, Mat4 *m) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (real32 *) m);
}

inline void gl_set_uniform_vec2(uint32 shader_id, char* uniform_name, Vec2 *v) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform2fv(uniform_location, 1, (real32 *) v);
}

inline void gl_set_uniform_vec3(uint32 shader_id, char* uniform_name, Vec3 *v) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform3fv(uniform_location, 1, (real32 *) v);
}

inline void gl_set_uniform_vec4(uint32 shader_id, char* uniform_name, Vec4 *v) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform4fv(uniform_location, 1, (real32 *) v);
}

inline void gl_set_uniform_int(uint32 shader_id, char* uniform_name, int32 i) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform1i(uniform_location, i);
}

inline void gl_set_uniform_uint(uint32 shader_id, char* uniform_name, uint32 ui) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform1ui(uniform_location, ui);
}

inline void gl_set_uniform_float(uint32 shader_id, char* uniform_name, real32 f) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform1f(uniform_location, f);
}

void gl_load_shader(GL_State *gl_state,
                    char *vertex_shader_filename, char *fragment_shader_filename, char *shader_name) {
    Marker m = begin_region();

    // NOTE: vertex shader
    Platform_File platform_file;
    bool32 file_exists = platform_open_file(vertex_shader_filename, &platform_file);
    assert(file_exists);

    File_Data vertex_shader_file_data = {};
    vertex_shader_file_data.contents = (char *) region_push(platform_file.file_size);
    bool32 result = platform_read_file(platform_file, &vertex_shader_file_data);
    assert(result);

    platform_close_file(platform_file);

    // NOTE: fragment shader
    file_exists = platform_open_file(fragment_shader_filename, &platform_file);
    assert(file_exists);

    File_Data fragment_shader_file_data = {};
    fragment_shader_file_data.contents = (char *) region_push(platform_file.file_size);
    result = platform_read_file(platform_file, &fragment_shader_file_data);
    assert(result);

    uint32 shader_id = gl_compile_and_link_shaders(vertex_shader_file_data.contents,
                                                   vertex_shader_file_data.size,
                                                   fragment_shader_file_data.contents,
                                                   fragment_shader_file_data.size);

    end_region(m);

    hash_table_add(&gl_state->shader_ids_table, make_string(shader_name), shader_id);
}

GL_Texture gl_load_texture(GL_State *gl_state, char *texture_filename, bool32 has_alpha=false) {
    Marker m = begin_region();
    File_Data texture_file_data = platform_open_and_read_file((Allocator *) &memory.global_stack,
                                                              texture_filename);

    int32 width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    uint8 *data = stbi_load_from_memory((uint8 *) texture_file_data.contents, texture_file_data.size,
                                        &width, &height, &num_channels, 0);
    assert(data);
    
    uint32 texture_id;
    glGenTextures(1, &texture_id);

    // TODO: we may want to be able to modify these parameters
    glBindTexture(GL_TEXTURE_2D, texture_id);
    if (has_alpha) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    end_region(m);

    GL_Texture gl_texture = { texture_id, width, height, num_channels };
    return gl_texture;
}

GL_Texture gl_load_texture(GL_State *gl_state, Texture texture, bool32 has_alpha=false) {
    Marker m = begin_region();
    Allocator *temp_allocator = (Allocator *) &memory.global_stack;
    char *temp_texture_filename = to_char_array(temp_allocator, texture.filename);
    File_Data texture_file_data = platform_open_and_read_file(temp_allocator,
                                                              temp_texture_filename);

    int32 width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    uint8 *data = stbi_load_from_memory((uint8 *) texture_file_data.contents, texture_file_data.size,
                                        &width, &height, &num_channels, 0);
    assert(data);
    
    uint32 texture_id;
    glGenTextures(1, &texture_id);

    // TODO: we may want to be able to modify these parameters
    glBindTexture(GL_TEXTURE_2D, texture_id);
    if (num_channels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    end_region(m);

    GL_Texture gl_texture = { texture_id, width, height, num_channels };
    return gl_texture;
}

// TODO: use the better stb_truetype packing procedures
void gl_init_font(GL_State *gl_state, Font *font) {
    Marker m = begin_region();
    uint8 *temp_bitmap = (uint8 *) allocate(temp_region, font->texture_width*font->texture_height);
    // NOTE: no guarantee that the bitmap will fit the font, so choose temp_bitmap dimensions carefully
    // TODO: we may want to maybe render this out to an image so that we can verify that the font fits
    int32 result = stbtt_BakeFontBitmap((uint8 *) font->file_data.contents, 0,
                                        font->height_pixels, temp_bitmap, font->texture_width, font->texture_height,
                                        font->first_char, font->num_chars,
                                        font->cdata);
    assert(result > 0);

    uint32 baked_texture_id;
    glGenTextures(1, &baked_texture_id);
    glBindTexture(GL_TEXTURE_2D, baked_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 font->texture_width, font->texture_height,
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    end_region(m);

    int32 font_id = gl_state->font_texture_table.total_added_ever;
    hash_table_add(&gl_state->font_texture_table, font_id, baked_texture_id);
}

GL_Mesh gl_load_mesh(GL_State *gl_state, Mesh mesh) {
    uint32 vao, vbo, ebo;
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
        
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.data_size, mesh.data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices_size, mesh.indices, GL_STATIC_DRAW);

    // vertices
    glVertexAttribPointer(0, mesh.n_vertex, GL_FLOAT, GL_FALSE,
                          mesh.vertex_stride * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    // normals
    glVertexAttribPointer(1, mesh.n_normal, GL_FLOAT, GL_FALSE,
                          mesh.vertex_stride * sizeof(real32),
                          (void *) (mesh.n_vertex * sizeof(real32)));
    glEnableVertexAttribArray(1);

    // UVs
    glVertexAttribPointer(2, mesh.n_uv, GL_FLOAT, GL_FALSE,
                          mesh.vertex_stride * sizeof(real32),
                          (void *) ((mesh.n_vertex + mesh.n_normal) * sizeof(real32)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    GL_Mesh gl_mesh = { mesh.type, vao, vbo, mesh.num_triangles };

    return gl_mesh;
}

uint32 gl_use_shader(GL_State *gl_state, char *shader_name) {
    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string(shader_name), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);
    return shader_id;
}

void gl_use_texture(GL_State *gl_state, int32 texture_id) {
    // TODO: will have to add parameter to specify which texture slot to use
    GL_Texture texture;
    uint32 texture_exists = hash_table_find(gl_state->texture_table, texture_id, &texture);
    assert(texture_exists);
    glBindTexture(GL_TEXTURE_2D, texture.id); 
}

void gl_use_rendering_texture(GL_State *gl_state, int32 texture_id) {
    // TODO: will have to add parameter to specify which texture slot to use
    GL_Texture texture;
    uint32 texture_exists = hash_table_find(gl_state->rendering_texture_table, texture_id, &texture);
    assert(texture_exists);
    glBindTexture(GL_TEXTURE_2D, texture.id); 
}

void gl_use_font_texture(GL_State *gl_state, int32 font_id) {
    uint32 texture_id;
    uint32 texture_exists = hash_table_find(gl_state->font_texture_table, font_id, &texture_id);
    assert(texture_exists);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

// TODO: think of a better API for this.. it's unclear which mesh table it's using
GL_Mesh gl_use_mesh(GL_State *gl_state, int32 mesh_id) {
    GL_Mesh gl_mesh = {};
    bool32 mesh_exists = false;
    mesh_exists = hash_table_find(gl_state->mesh_table, mesh_id, &gl_mesh);
    assert(mesh_exists);
    glBindVertexArray(gl_mesh.vao);
    return gl_mesh;
}

GL_Mesh gl_use_rendering_mesh(GL_State *gl_state, int32 mesh_id) {
    GL_Mesh gl_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->rendering_mesh_table, mesh_id, &gl_mesh);
    assert(mesh_exists);
    glBindVertexArray(gl_mesh.vao);
    return gl_mesh;
}

void gl_draw_solid_mesh(GL_State *gl_state, Render_State *render_state,
                        int32 mesh_id,
                        Material material,
                        Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "solid");
    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_id);

    if (!material.use_color_override && (material.texture_id < 0)) assert(!"No texture name provided.");
    if (!material.use_color_override) gl_use_texture(gl_state, material.texture_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &material.color_override);
    gl_set_uniform_int(shader_id, "use_color_override", material.use_color_override);
    
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_solid_color_mesh(GL_State *gl_state, Render_State *render_state,
                              int32 mesh_id, Vec4 color,
                              Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "solid");
    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    gl_set_uniform_int(shader_id, "use_color_override", true);
    
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_solid_color_mesh(GL_State *gl_state, Render_State *render_state,
                              GL_Mesh gl_mesh, Vec4 color,
                              Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "solid");
    glBindVertexArray(gl_mesh.vao);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    gl_set_uniform_int(shader_id, "use_color_override", true);
    
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

#if 0
void gl_draw_mesh(GL_State *gl_state, Render_State *render_state,
                  int32 mesh_id,
                  Material material,
                  Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "basic_3d");

    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_id);

    if (material.texture_id >= 0 && !material.use_color_override) gl_use_texture(gl_state, material.texture_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    // NOTE: we may need to think about this for transparent materials
    // TODO: using override color here is temporary.. ideally we will have finer control over things like
    //       ambient/diffuse/specular color
    Vec3 material_color = truncate_v4_to_v3(material.color_override);
    gl_set_uniform_vec3(shader_id, "material_color", &material_color);
    gl_set_uniform_float(shader_id, "gloss", material.gloss);
    
    gl_set_uniform_vec3(shader_id, "camera_pos", &render_state->camera.position);

    // if there is no texture, then just use color override, no matter what material.use_color_override is.
    // we do it this way since we don't save whether or not a material is using a color override when we delete
    // textures. it's kind of confusing. basically this is just so we don't have to also save use_color_overrides
    // when we delete textures/undo delete textures.
    gl_set_uniform_int(shader_id, "use_color_override", (material.texture_id < 0) || material.use_color_override);

    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}
#endif

void gl_draw_mesh(GL_State *gl_state, Render_State *render_state,
                  int32 mesh_id,
                  Material material,
                  Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "pbr");

    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_id);

    if (material.texture_id >= 0 && !material.use_color_override) gl_use_texture(gl_state, material.texture_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    // NOTE: we may need to think about this for transparent materials
    // TODO: using override color here is temporary.. ideally we will have finer control over things like
    //       ambient/diffuse/specular color
    Vec3 material_color = truncate_v4_to_v3(material.color_override);
    gl_set_uniform_vec3(shader_id, "albedo_color", &material_color);
    gl_set_uniform_float(shader_id, "metallic", 0.0f);
    gl_set_uniform_float(shader_id, "roughness", 0.5f);
    gl_set_uniform_float(shader_id, "ao", 1.0f);
    
    gl_set_uniform_vec3(shader_id, "camera_pos", &render_state->camera.position);

    // if there is no texture, then just use color override, no matter what material.use_color_override is.
    // we do it this way since we don't save whether or not a material is using a color override when we delete
    // textures. it's kind of confusing. basically this is just so we don't have to also save use_color_overrides
    // when we delete textures/undo delete textures.
    gl_set_uniform_int(shader_id, "use_color_override", (material.texture_id < 0) || material.use_color_override);

    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_textured_mesh(GL_State *gl_state, Render_State *render_state,
                           int32 mesh_id,
                           int32 texture_id, Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "basic_3d_textured");
    gl_use_texture(gl_state, texture_id);

    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);

    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

#if 0
void gl_draw_aabb(GL_State *gl_state, Render_State *render_state,
                  AABB aabb) {
    uint32 shader_id = gl_use_shader(gl_state, "solid");
    
    GL_Mesh gl_mesh = gl_use_rendering_mesh(
}
#endif

void gl_draw_wireframe(GL_State *gl_state, Render_State *render_state,
                       int32 mesh_id, Transform transform, Vec4 color) {
    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("debug_wireframe"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthFunc(GL_LESS);

    glUseProgram(0);
    glBindVertexArray(0);
}

    inline void gl_draw_wireframe(GL_State *gl_state, Render_State *render_state,
                           int32 mesh_id, Transform transform) {
        gl_draw_wireframe(gl_state, render_state,
                          mesh_id, transform, make_vec4(1.0f, 1.0f, 0.0f, 1.0f));
    }

void gl_draw_circle(GL_State *gl_state, Render_State *render_state, Transform transform, Vec4 color) {
    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("line_3d"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    GL_Mesh circle_mesh = gl_use_rendering_mesh(gl_state, gl_state->circle_mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    // offset by 1 since this only draws an outline
    //glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_CIRCLE_VERTICES + 2);
    glDrawArrays(GL_LINE_STRIP, 1, NUM_CIRCLE_VERTICES + 1);

    glUseProgram(0);
    glBindVertexArray(0);
}

// TODO: this doesn't non vertical capsules
void gl_draw_capsule(GL_State *gl_state, Render_State *render_state,
                     Vec3 base, Vec3 tip, real32 radius,
                     Vec4 color) {
    uint32 shader_id = gl_use_shader(gl_state, "debug_wireframe");

    Vec3 normal = normalize(tip - base);
    Vec3 a = base + normal*radius; // bottom sphere center
    Vec3 b = tip - normal*radius;  // top sphere center
    real32 length = distance(b - a);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GL_Mesh gl_mesh;
    Transform transform;
    Mat4 model_matrix;

    // capsule cylinder body
    gl_mesh = gl_use_rendering_mesh(gl_state, gl_state->capsule_cylinder_mesh_id);
    transform = make_transform(a, make_quaternion(), make_vec3(radius, length, radius));
    model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    // capsule cylinder bottom cap
    gl_mesh = gl_use_rendering_mesh(gl_state, gl_state->capsule_cap_mesh_id);
    transform = make_transform(a, make_quaternion(180.0f, x_axis), make_vec3(radius, radius, radius));
    model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    // capsule cylinder top cap
    gl_mesh = gl_use_rendering_mesh(gl_state, gl_state->capsule_cap_mesh_id);
    transform = make_transform(b, make_quaternion(), make_vec3(radius, radius, radius));
    model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_collider(GL_State *gl_state, Render_State *render_state, Collider_Variant collider) {
    Vec4 collider_color = make_vec4(1.0f, 0.0f, 1.0f, 1.0f);

    switch (collider.type) {
        case Collider_Type::NONE: return;
        case Collider_Type::CIRCLE: {
            Transform transform;
            Circle_Collider circle = collider.circle;
            transform.position = circle.center;
            // scale happens before rotation, so we scale on x and y since the circle primitive's vertices
            // exist on that plane
            transform.scale = make_vec3(circle.radius, circle.radius, 1.0f);
            transform.rotation = make_quaternion(90.0f, x_axis);
            gl_draw_circle(gl_state, render_state, transform, collider_color);
        } break;
        case Collider_Type::CAPSULE: {
            Capsule capsule = collider.capsule.capsule;
            gl_draw_capsule(gl_state, render_state, capsule.base, capsule.tip, capsule.radius,
                            collider_color);
        } break;
        default: {
            assert(!"Unhandled collider type");
            return;
        } break;
    }
}

void copy_aligned_quad_to_arrays(stbtt_aligned_quad q, real32 *vertices, real32 *uvs) {
    vertices[0] = q.x0;
    vertices[1] = q.y0;
    vertices[2] = q.x1;
    vertices[3] = q.y0;
    vertices[4] = q.x1;
    vertices[5] = q.y1;
    vertices[6] = q.x0;
    vertices[7] = q.y1;

    uvs[0] = q.s0;
    uvs[1] = q.t0;
    uvs[2] = q.s1;
    uvs[3] = q.t0;
    uvs[4] = q.s1;
    uvs[5] = q.t1;
    uvs[6] = q.s0;
    uvs[7] = q.t1;
}

void gl_draw_text(GL_State *gl_state, Render_State *render_state,
                  int32 font_id, Font *font,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  char *text, int32 num_chars, bool32 is_null_terminated,
                  Vec4 color,
                  bool32 has_shadow, Vec4 shadow_color, real32 shadow_offset) {
    uint32 text_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("text"), &text_shader_id);
    assert(shader_exists);
    glUseProgram(text_shader_id);

    GL_Mesh glyph_mesh = gl_use_rendering_mesh(gl_state, gl_state->glyph_quad_mesh_id);

    uint32 font_texture_id;
    uint32 font_texture_exists = hash_table_find(gl_state->font_texture_table, font_id,
                                                 &font_texture_id);
    assert(font_texture_exists);

    gl_set_uniform_mat4(text_shader_id, "cpv_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(text_shader_id, "color", &color);
    gl_set_uniform_int(text_shader_id, "has_shadow", has_shadow);

    if (has_shadow) {
        gl_set_uniform_vec4(text_shader_id, "shadow_color", &shadow_color);
        gl_set_uniform_float(text_shader_id, "shadow_offset", shadow_offset);
    }

    // NOTE: we disable depth test so that overlapping characters such as the "o" in "fo" doesn't cover the
    //       quad of the previous character, causing a cut off look.
    // NOTE: we assume that GL_DEPTH_TEST is disabled
    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_mesh.vbo);

    real32 quad_vertices[8];
    real32 quad_uvs[8];
    real32 line_advance = font->scale_for_pixel_height * (font->ascent - font->descent + font->line_gap);
    real32 start_x_pos_pixels = x_pos_pixels;

    int32 i = 0;
    while (*text && (is_null_terminated || (i < num_chars))) {
        if (*text >= 32 && *text < 128 || *text == '-') {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->cdata, 512, 512, *text - 32, &x_pos_pixels, &y_pos_pixels, &q, 1);

            copy_aligned_quad_to_arrays(q, quad_vertices, quad_uvs);

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
            glBufferSubData(GL_ARRAY_BUFFER, (int *) sizeof(quad_vertices), sizeof(quad_uvs), quad_uvs);

            if (has_shadow) {
                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 2);
            } else {
                glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);
            }
        } else if (*text == '\n') {
            x_pos_pixels = start_x_pos_pixels;
            y_pos_pixels += line_advance;
        }
        
        text++;
        i++;
    }

    //glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}


void gl_draw_text(GL_State *gl_state, Render_State *render_state,
                  int32 font_id, Font *font,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  char *text, Vec4 color,
                  Vec4 shadow_color, real32 shadow_offset) {
    return gl_draw_text(gl_state, render_state,
                        font_id, font,
                        x_pos_pixels, y_pos_pixels,
                        text, 0, true, color,
                        true, shadow_color, shadow_offset);
}

void gl_draw_text(GL_State *gl_state, Render_State *render_state,
                  int32 font_id, Font *font,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  String_Buffer buffer, Vec4 color,
                  Vec4 shadow_color, real32 shadow_offset) {
    return gl_draw_text(gl_state, render_state,
                        font_id, font,
                        x_pos_pixels, y_pos_pixels,
                        buffer.contents, buffer.current_length, false, color,
                        true, shadow_color, shadow_offset);
}

void gl_draw_text(GL_State *gl_state, Render_State *render_state,
                  int32 font_id, Font *font,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  char *text, Vec4 color) {
    return gl_draw_text(gl_state, render_state,
                        font_id, font,
                        x_pos_pixels, y_pos_pixels,
                        text, 0, true, color,
                        false, {}, 0);
}

void gl_draw_text(GL_State *gl_state, Render_State *render_state,
                  int32 font_id, Font *font,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  String_Buffer buffer, Vec4 color) {
    return gl_draw_text(gl_state, render_state,
                        font_id, font,
                        x_pos_pixels, y_pos_pixels,
                        buffer.contents, buffer.current_length, false, color,
                        false, {}, 0);
}

GLint gl_get_framebuffer_format(uint32 framebuffer_flags) {
    bool32 has_alpha = framebuffer_flags & FRAMEBUFFER_HAS_ALPHA;
    bool32 is_hdr = framebuffer_flags & FRAMEBUFFER_IS_HDR;

    GLint format;
    if (has_alpha) {
        if (is_hdr) {
            format = GL_RGBA16F;
        } else {
            format = GL_RGBA;
        }
    } else {
        if (is_hdr) {
            format = GL_RGB16F;
        } else {
            format = GL_RGB;
        }
    }

    return format;
}

GL_Framebuffer gl_make_framebuffer(int32 width, int32 height, uint32 flags) {
    GL_Framebuffer framebuffer = {};

    glGenFramebuffers(1, &framebuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

    // color buffer texture
    glGenTextures(1, &framebuffer.color_buffer_texture);
    glBindTexture(GL_TEXTURE_2D, framebuffer.color_buffer_texture);
    
    GLint format = gl_get_framebuffer_format(flags);
    glTexImage2D(GL_TEXTURE_2D, 0, format,
                 width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           framebuffer.color_buffer_texture, 0);

    // render buffer (depth only)
    glGenRenderbuffers(1, &framebuffer.render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.render_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        debug_print("Framebuffer is not complete.\n");
        assert(false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}

GL_Framebuffer gl_make_msaa_framebuffer(int32 width, int32 height, int32 num_samples, uint32 flags) {
    GL_Framebuffer framebuffer;

    framebuffer.is_multisampled = true;
    framebuffer.num_samples = num_samples;
    
    glGenFramebuffers(1, &framebuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

    // color buffer texture
    glGenTextures(1, &framebuffer.color_buffer_texture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.color_buffer_texture);
    GLint format = gl_get_framebuffer_format(flags);
    
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, num_samples,
                            format, width, height, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           framebuffer.color_buffer_texture, 0);

    // render buffer (depth only)
    glGenRenderbuffers(1, &framebuffer.render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.render_buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, num_samples, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.render_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        debug_print("Framebuffer is not complete.\n");
        assert(false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}

void gl_delete_framebuffer(GL_Framebuffer framebuffer) {
    // delete framebuffer, color buffer, render buffer
    glDeleteFramebuffers(1, &framebuffer.fbo);
    glDeleteTextures(1, &framebuffer.color_buffer_texture);
    glDeleteRenderbuffers(1, &framebuffer.render_buffer);
}

void gl_delete_mesh(GL_Mesh mesh) {
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteVertexArrays(1, &mesh.vao);
}

void gl_delete_texture(GL_Texture texture) {
    glDeleteTextures(1, &texture.id);
}

void generate_circle_vertices(real32 *buffer, int32 buffer_size,
                              int32 num_vertices, real32 radius, bool32 include_center = false) {
    assert(num_vertices > 0);

    // num_vertices doesn't initially include the start vertex twice, but we need it at the start and end so that
    // TRIANGLE_FAN draws the final triangle
    int32 total_num_vertices = num_vertices + 1;
    if (include_center) {
        total_num_vertices++;
    }

    int32 min_buffer_size = total_num_vertices * 3 * sizeof(real32);
    assert(buffer_size >= min_buffer_size);

    int32 vertex_offset = 0;
    if (include_center) {
        buffer[vertex_offset] = 0.0f;
        buffer[vertex_offset + 1] = 0.0f;
        buffer[vertex_offset + 2] = 0.0f;
        vertex_offset++;
    }
    
    // generate circle vertices on the xy-plane
    for (int32 i = 0; i < num_vertices + 1; i++) {
        real32 t = (real32) i / num_vertices;
        real32 angle = 2.0f * PI * t;
        real32 x = cosf(angle) * radius;
        real32 y = sinf(angle) * radius;
        buffer[vertex_offset * 3] = x,
        buffer[vertex_offset * 3 + 1] = y;
        buffer[vertex_offset * 3 + 2] = 0.0f;
        vertex_offset++;
    }
}

int32 gl_add_rendering_mesh(GL_State *gl_state, GL_Mesh gl_mesh) {
    int32 mesh_id = gl_state->rendering_mesh_table.total_added_ever;
    hash_table_add(&gl_state->rendering_mesh_table, mesh_id, gl_mesh);
    return mesh_id;
}

int32 gl_add_rendering_texture(GL_State *gl_state, GL_Texture gl_texture) {
    int32 texture_id = gl_state->rendering_texture_table.total_added_ever;
    hash_table_add(&gl_state->rendering_texture_table, texture_id, gl_texture);
    return texture_id;
}

void gl_init(GL_State *gl_state, Display_Output display_output) {
    gl_state->shader_ids_table = make_hash_table<String, uint32>((Allocator *) &memory.hash_table_stack,
                                                                 HASH_TABLE_SIZE, &string_equals);
    gl_state->rendering_mesh_table = make_hash_table<int32, GL_Mesh>((Allocator *) &memory.hash_table_stack,
                                                                     HASH_TABLE_SIZE, &int32_equals);
    gl_state->mesh_table = make_hash_table<int32, GL_Mesh>((Allocator *) &memory.hash_table_stack,
                                                           MAX_MESHES, &int32_equals);
    gl_state->rendering_texture_table = make_hash_table<int32, GL_Texture>((Allocator *) &memory.hash_table_stack,
                                                                           HASH_TABLE_SIZE, &int32_equals);
    gl_state->texture_table = make_hash_table<int32, GL_Texture>((Allocator *) &memory.hash_table_stack,
                                                                 HASH_TABLE_SIZE, &int32_equals);
    gl_state->font_texture_table = make_hash_table<int32, uint32>((Allocator *) &memory.hash_table_stack,
                                                                  HASH_TABLE_SIZE, &int32_equals);
    
    uint32 vbo, vao, ebo;

    // NOTE: triangle mesh
    real32 triangle_vertices[] = {
        0.0f, 0.0f, 0.0f,
        0.5f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    gl_state->triangle_mesh_id = gl_add_rendering_mesh(gl_state, make_gl_mesh(Mesh_Type::RENDERING, vao, vbo, 1));

    // NOTE: quad mesh
    // we store them separately like this because we use glBufferSubData to send the vertex positions
    // directly to the shader, and i don't think there's a way to easily modify interleaved data, unless
    // you're modifying all of the data, but when we modify the data we only modify the positions and not the uvs.
    // the values in these arrays don't matter; we're just filling these arrays up with enough values such that
    // sizeof() gives the correct values
    real32 quad_vertices[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    real32 quad_uvs[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    uint32 quad_indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices) + sizeof(quad_uvs), 0, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) sizeof(quad_vertices), sizeof(quad_uvs), quad_uvs);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(real32), (void *) sizeof(quad_vertices));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    gl_state->quad_mesh_id = gl_add_rendering_mesh(gl_state, make_gl_mesh(Mesh_Type::RENDERING, vao, vbo, 2));

    // NOTE: framebuffer mesh
    real32 framebuffer_mesh_data[] = {
        // positions  uvs
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,

         1.0f, 1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(framebuffer_mesh_data), framebuffer_mesh_data, GL_STATIC_DRAW);

    // positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    // uvs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(real32), (void *) (2 * sizeof(real32)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    gl_state->framebuffer_quad_mesh_id = gl_add_rendering_mesh(gl_state, make_gl_mesh(Mesh_Type::RENDERING,
                                                                                      vao, vbo, 2));

    // NOTE: glyph quad
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices) + sizeof(quad_uvs), 0, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) sizeof(quad_vertices), sizeof(quad_uvs), quad_uvs);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(real32), (void *) sizeof(quad_vertices));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    gl_state->glyph_quad_mesh_id = gl_add_rendering_mesh(gl_state, make_gl_mesh(Mesh_Type::RENDERING, vao, vbo, 2));

    // line mesh
    real32 line_vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    gl_state->line_mesh_id = gl_add_rendering_mesh(gl_state, make_gl_mesh(Mesh_Type::RENDERING, vao, vbo, 0));

    // circle mesh
    // add 2, since we need a space for the center of the circle and another space since the final vertex
    // is a duplicate of the first.
    real32 filled_circle_vertices[(NUM_CIRCLE_VERTICES + 2)*3];
    generate_circle_vertices(filled_circle_vertices, sizeof(filled_circle_vertices), NUM_CIRCLE_VERTICES, 1.0f, true);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(filled_circle_vertices), filled_circle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    gl_state->circle_mesh_id = gl_add_rendering_mesh(gl_state, make_gl_mesh(Mesh_Type::RENDERING, vao, vbo, 0));

    // sphere mesh
    Marker m = begin_region();
    Mesh sphere_mesh = read_and_load_mesh(temp_region, "blender/sphere.mesh", "sphere", Mesh_Type::RENDERING);
    gl_state->sphere_mesh_id = gl_add_rendering_mesh(gl_state, gl_load_mesh(gl_state, sphere_mesh));
    end_region(m);

    // capsule cylinder mesh
    m = begin_region();
    Mesh capsule_cylinder_mesh = read_and_load_mesh(temp_region,
                                                   "blender/capsule_cylinder.mesh", "capsule_cylinder",
                                                   Mesh_Type::RENDERING);
    gl_state->capsule_cylinder_mesh_id = gl_add_rendering_mesh(gl_state,
                                                               gl_load_mesh(gl_state, capsule_cylinder_mesh));
    end_region(m);

    // capsule cap mesh
    m = begin_region();
    Mesh capsule_cap_mesh = read_and_load_mesh(temp_region,
                                                    "blender/capsule_cap.mesh", "capsule_cap",
                                                    Mesh_Type::RENDERING);
    gl_state->capsule_cap_mesh_id = gl_add_rendering_mesh(gl_state,
                                                          gl_load_mesh(gl_state, capsule_cap_mesh));
    end_region(m);

    // NOTE: shaders
    gl_load_shader(gl_state,
                   "src/shaders/basic.vs", "src/shaders/basic.fs", "basic");
    gl_load_shader(gl_state,
                   "src/shaders/basic2.vs", "src/shaders/basic2.fs", "basic2");
    gl_load_shader(gl_state,
                   "src/shaders/text.vs", "src/shaders/text.fs", "text");
    gl_load_shader(gl_state,
                   "src/shaders/solid.vs", "src/shaders/solid.fs", "solid");
    gl_load_shader(gl_state,
                   "src/shaders/basic_3d.vs", "src/shaders/basic_3d.fs", "basic_3d");

    gl_load_shader(gl_state,
                   "src/shaders/basic_3d_textured.vs", "src/shaders/basic_3d_textured.fs", "basic_3d_textured");
    gl_load_shader(gl_state,
                   "src/shaders/debug_wireframe.vs",
                   "src/shaders/debug_wireframe.fs",
                   "debug_wireframe");
    gl_load_shader(gl_state,
                   "src/shaders/framebuffer.vs", "src/shaders/framebuffer.fs",
                   "framebuffer");
    gl_load_shader(gl_state,
                   "src/shaders/multisampled_framebuffer.vs", "src/shaders/multisampled_framebuffer.fs",
                   "multisampled_framebuffer");
    gl_load_shader(gl_state,
                   "src/shaders/hue_slider.vs", "src/shaders/hue_slider.fs",
                   "hue_slider");
    gl_load_shader(gl_state,
                   "src/shaders/hsv.vs", "src/shaders/hsv.fs",
                   "hsv");
    gl_load_shader(gl_state,
                   "src/shaders/mesh_2d.vs", "src/shaders/mesh_2d.fs",
                   "mesh_2d");
    gl_load_shader(gl_state,
                   "src/shaders/line_3d.vs", "src/shaders/line_3d.fs",
                   "line_3d");
    gl_load_shader(gl_state,
                   "src/shaders/constant_facing_quad.vs", "src/shaders/constant_facing_quad.fs",
                   "constant_facing_quad");
    gl_load_shader(gl_state,
                   "src/shaders/rounded_quad.vs", "src/shaders/rounded_quad.fs",
                   "rounded_quad");
    gl_load_shader(gl_state,
                   "src/shaders/pbr.vs", "src/shaders/pbr.fs",
                   "pbr");
    

    // NOTE: framebuffers
    int32 num_samples = NUM_MSAA_SAMPLES;
    uint32 framebuffer_flags = FRAMEBUFFER_IS_HDR | FRAMEBUFFER_HAS_ALPHA;
    gl_state->gizmo_framebuffer = gl_make_msaa_framebuffer(display_output.width, display_output.height,
                                                           num_samples, framebuffer_flags);
    gl_state->msaa_framebuffer = gl_make_msaa_framebuffer(display_output.width, display_output.height,
                                                          num_samples, framebuffer_flags);

    // NOTE: unified buffer object
    glGenBuffers(1, &gl_state->global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, gl_state->global_ubo);
    // TODO: not sure if 1024 bytes is enough
    // we add 1 to MAX_POINT_LIGHTS for the int to hold num_point_lights.
    // maybe we could put num_point lights at the end of the uniform buffer object?
    glBufferData(GL_UNIFORM_BUFFER, (MAX_POINT_LIGHTS + 1) * sizeof(GL_Point_Light), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //glBindBufferRange(GL_UNIFORM_BUFFER, 0, gl_state->global_ubo, 0, 4);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, gl_state->global_ubo);

    uint32 shader_id = gl_use_shader(gl_state, "basic_3d");
    uint32 uniform_block_index = glGetUniformBlockIndex(shader_id, "shader_globals");
    glUniformBlockBinding(shader_id, uniform_block_index, 0);
    glUseProgram(0);

    // NOTE: rendering textures
    GL_Texture debug_texture = gl_load_texture(gl_state, "src/textures/lightbulb.png", true);
    gl_state->light_icon_texture_id = gl_add_rendering_texture(gl_state, debug_texture);

    // NOTE: disable culling for now, just for easier debugging...
#if 1
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
#endif

    glEnable(GL_DEPTH_TEST);  
}

// NOTE: This draws a triangle that has its bottom left corner at position.
//       Position is based on percentages, so 50% x and 50%y would put the bottom left corner of the triangle
//       in the middle of the screen.
void gl_draw_triangle_p(GL_State *gl_state,
                        Display_Output display_output, Vec2 position,
                        real32 width_pixels, real32 height_pixels,
                        Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);
    
    GL_Mesh triangle_mesh = gl_use_rendering_mesh(gl_state, gl_state->triangle_mesh_id);

    Vec2 clip_space_position = make_vec2(position.x * 2.0f - 1.0f,
                                         position.y * -2.0f + 1.0f);
    
    real32 clip_space_width  = width_pixels / (display_output.width / 2.0f);
    real32 clip_space_height = height_pixels / (display_output.height / 2.0f);

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_position, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_height) *
                         make_scale_matrix(x_axis, clip_space_width));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_triangle(GL_State *gl_state,
                      Display_Output display_output,
                      real32 x_pos_pixels, real32 y_pos_pixels,
                      real32 width_pixels, real32 height_pixels,
                      Vec4 color) {
    // TODO: this might be wrong with the new screen-space coordinate-system (0, 0) in top left
    gl_draw_triangle_p(gl_state, display_output,
                       make_vec2(x_pos_pixels / display_output.width, y_pos_pixels / display_output.height),
                       width_pixels, height_pixels,
                       color);
}

#if 0
void gl_draw_line_p(GL_State *gl_state,
                    Display_Output display_output,
                    Vec2 start, Vec2 end,
                    Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh line_mesh = gl_use_rendering_mesh(gl_state, gl_state->line_mesh_id);

    Vec2 clip_space_start = make_vec2(start.x * 2.0f - 1.0f,
                                      start.y * -2.0f + 1.0f);
    Vec2 clip_space_end = make_vec2(end.x * 2.0f - 1.0f,
                                    end.y * -2.0f + 1.0f);
    Vec2 clip_space_length = clip_space_end - clip_space_start;

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_start, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_length.y) *
                         make_scale_matrix(x_axis, clip_space_length.x));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}
#endif

void gl_draw_line(GL_State *gl_state,
                  Render_State *render_state,
                  Vec2 start_pixels, Vec2 end_pixels,
                  Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic2"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh line_mesh = gl_use_rendering_mesh(gl_state, gl_state->line_mesh_id);

    real32 line_vertices[] = {
        start_pixels.x, start_pixels.y, 0.0f,
        end_pixels.x, end_pixels.y, 0.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, line_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_vertices), line_vertices);
    gl_set_uniform_mat4(basic_shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(basic_shader_id, "color", &color);
    gl_set_uniform_int(basic_shader_id, "use_color", true);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_line(GL_State *gl_state,
                  Render_State *render_state,
                  Vec3 start, Vec3 end,
                  Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("line_3d"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh line_mesh = gl_use_rendering_mesh(gl_state, gl_state->line_mesh_id);

    Mat4 model_matrix = make_mat4_identity();

    real32 line_vertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };
    glBindBuffer(GL_ARRAY_BUFFER, line_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_vertices), line_vertices);
    gl_set_uniform_mat4(basic_shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(basic_shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

#if 0
void gl_draw_line(GL_State *gl_state,
                  Display_Output display_output,
                  Vec3 start, Vec3 end,
                  Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic_3d"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh line_mesh = gl_use_rendering_mesh(gl_state, gl_state->line_mesh_id);

    Vec2 clip_space_start = make_vec2(start_pixels.x / display_output.width * 2.0f - 1.0f,
                                      start_pixels.y / display_output.height * -2.0f + 1.0f);
    Vec2 clip_space_end = make_vec2(end_pixels.x / display_output.width * 2.0f - 1.0f,
                                    end_pixels.y / display_output.height * -2.0f + 1.0f);
    Vec2 clip_space_length = clip_space_end - clip_space_start;

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_start, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_length.y) *
                         make_scale_matrix(x_axis, clip_space_length.x));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}
#endif

inline void gl_draw_line(GL_State *gl_state,
                         Render_State *render_state,
                         Vec2 start_pixels, Vec2 end_pixels,
                         Vec3 color) {
    gl_draw_line(gl_state, render_state, start_pixels, end_pixels, make_vec4(color, 1.0f));
}

// NOTE: percentage based position
void gl_draw_quad_p(GL_State *gl_state,
                    Display_Output display_output,
                    real32 x, real32 y,
                    real32 width_pixels, real32 height_pixels,
                    Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh square_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);

    Vec2 clip_space_position = make_vec2(x * 2.0f - 1.0f,
                                         y * -2.0f + 1.0f);
    
    real32 clip_space_width  = width_pixels / (display_output.width / 2.0f);
    real32 clip_space_height = height_pixels / (display_output.height / 2.0f);

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_position, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_height) *
                         make_scale_matrix(x_axis, clip_space_width));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_constant_facing_quad_view_space(GL_State *gl_state,
                                             Render_State *render_state,
                                             Vec3 view_space_center_position,
                                             real32 world_space_side_length,
                                             int32 texture_id, bool32 is_rendering_texture=false) {
    uint32 basic_shader_id = gl_use_shader(gl_state, "constant_facing_quad");
    GL_Mesh quad_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);
    if (is_rendering_texture) {
        gl_use_rendering_texture(gl_state, texture_id);
    } else {
        gl_use_texture(gl_state, texture_id);
    }

    real32 quad_vertices[8] = {
        -0.5f, -0.5f, // bottom left
        -0.5f, 0.5f,  // top left
        0.5f, 0.5f,   // top right
        0.5f, -0.5f   // bottom right
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);

    //Transform transform = make_transform();
    //transform.position = view_space_center_position;
    //Mat4 model = get_model_matrix(transform);
    //gl_set_uniform_mat4(basic_shader_id, "model_matrix", &model);
    //gl_set_uniform_mat4(basic_shader_id, "view_matrix", &render_state->view_matrix);

    gl_set_uniform_vec3(basic_shader_id, "view_space_center", &view_space_center_position);
    gl_set_uniform_mat4(basic_shader_id, "perspective_clip_matrix", &render_state->perspective_clip_matrix);
    gl_set_uniform_float(basic_shader_id, "side_length", world_space_side_length);
    //gl_set_uniform_mat4(basic_shader_id, "cpv_matrix", &render_state->cpv_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(GL_State *gl_state,
                  Render_State *render_state,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  int32 texture_id, bool32 is_rendering_texture=false) {
    uint32 basic_shader_id = gl_use_shader(gl_state, "basic2");
    GL_Mesh quad_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);
    if (is_rendering_texture) {
        gl_use_rendering_texture(gl_state, texture_id);
    } else {
        gl_use_texture(gl_state, texture_id);
    }

    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels + height_pixels,               // bottom left
        x_pos_pixels, y_pos_pixels,                               // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels // bottom right
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(basic_shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_int(basic_shader_id, "use_color", false);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(GL_State *gl_state,
                  Render_State *render_state,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  Vec4 color) {
    uint32 basic_shader_id = gl_use_shader(gl_state, "basic2");
    GL_Mesh quad_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);
    
    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels + height_pixels,               // bottom left
        x_pos_pixels, y_pos_pixels,                               // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels // bottom right
    };
    //real32 quad_uvs[8];
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(basic_shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(basic_shader_id, "color", &color);
    gl_set_uniform_int(basic_shader_id, "use_color", true);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(GL_State *gl_state,
                  Render_State *render_state,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  Vec3 color) {
    gl_draw_quad(gl_state, render_state, x_pos_pixels, y_pos_pixels, width_pixels, height_pixels,
                 make_vec4(color, 1.0f));
}

void gl_draw_rounded_quad(GL_State *gl_state, Render_State *render_state,
                          Rect rect, real32 corner_radius, uint32 corner_flags, Vec4 color) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    uint32 shader_id = gl_use_shader(gl_state, "rounded_quad");
    GL_Mesh quad_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);
    
    real32 quad_vertices[8] = {
        rect.x, rect.y + rect.height,             // bottom left
        rect.x, rect.y,                           // top left
        rect.x + rect.width, rect.y,              // top right
        rect.x + rect.width, rect.y + rect.height // bottom right
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    Vec2 position = make_vec2(rect.x, rect.y);
    gl_set_uniform_vec2(shader_id, "position", &position); 
    gl_set_uniform_float(shader_id, "width", rect.width);
    gl_set_uniform_float(shader_id, "height", rect.height);
    gl_set_uniform_float(shader_id, "corner_radius", corner_radius);
    gl_set_uniform_uint(shader_id, "corner_flags", corner_flags);

    //gl_set_uniform_int(shader_id, "use_color", true);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_hue_slider_quad(GL_State *gl_state,
                             Render_State *render_state,
                             real32 x_pos_pixels, real32 y_pos_pixels,
                             real32 width_pixels, real32 height_pixels) {
    uint32 shader_id = gl_use_shader(gl_state, "hue_slider");
    GL_Mesh quad_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);

    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels + height_pixels,               // bottom left
        x_pos_pixels, y_pos_pixels,                               // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels // bottom right
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_hsv_quad(GL_State *gl_state,
                      Render_State *render_state,
                      real32 x_pos_pixels, real32 y_pos_pixels,
                      real32 width_pixels, real32 height_pixels,
                      real32 hue_degrees) {
    uint32 shader_id = gl_use_shader(gl_state, "hsv");
    GL_Mesh quad_mesh = gl_use_rendering_mesh(gl_state, gl_state->quad_mesh_id);

    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels + height_pixels,               // bottom left
        x_pos_pixels, y_pos_pixels,                               // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels // bottom right
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_float(shader_id, "hue_degrees", hue_degrees);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_circle(GL_State *gl_state, Render_State *render_state,
                    real32 center_x, real32 center_y,
                    real32 radius,
                    Vec4 color,
                    bool32 is_filled) {
    // we could instead draw a circle by drawing a quad, then in the fragment shader checking if the fragment
    // is within some radius, but then, we would have to deal with aliasing, i think, and i think this method
    // would be slower as well.

    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("mesh_2d"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    GL_Mesh circle_mesh = gl_use_rendering_mesh(gl_state, gl_state->circle_mesh_id);

    Transform transform = {
        make_vec3(center_x, center_y, 0.0f),
        make_quaternion(),
        make_vec3(radius, radius, 1.0f)
    };
    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    if (is_filled) {
        glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_CIRCLE_VERTICES + 2);
    } else {
        glDrawArrays(GL_LINE_STRIP, 1, NUM_CIRCLE_VERTICES + 1);
    }

    glUseProgram(0);
    glBindVertexArray(0);
}

// TODO: these sound buffer drawing functions are probably messed up since we changed the line drawing code
void draw_sound_cursor(GL_State *gl_state, Render_State *render_state,
                       Display_Output display_output, Win32_Sound_Output *win32_sound_output,
                       real32 cursor_position, Vec3 color) {
    real32 cursor_width = 10.0f;
    real32 cursor_x = ((cursor_position *
                        display_output.width) - cursor_width / 2.0f);
    real32 cursor_height = 20.0f;
    gl_draw_triangle(gl_state, display_output,
                     cursor_x, display_output.height - 202 - cursor_height,
                     cursor_width, cursor_height,
                     make_vec4(color, 1.0f));

    gl_draw_line(gl_state, render_state,
                 make_vec2(cursor_position * display_output.width, display_output.height - 202.0f),
                 make_vec2(cursor_position * display_output.width, (real32) display_output.height),
                 make_vec4(color, 1.0f));
}

void draw_sound_buffer(GL_State *gl_state, Render_State *render_state,
                       Win32_Sound_Output *win32_sound_output) {
    Display_Output display_output = render_state->display_output;
    int32 max_samples = win32_sound_output->buffer_size / win32_sound_output->bytes_per_sample;

    real32 channel_height = 100.0;
    real32 height_offset = channel_height;
    gl_draw_quad(gl_state, render_state,
                 0.0f, display_output.height - height_offset,
                 (real32) display_output.width, channel_height,
                 make_vec3(0.1f, 0.1f, 0.1f));
    gl_draw_line(gl_state, render_state,
                 make_vec2(0.0f, display_output.height - height_offset - 1),
                 make_vec2((real32) display_output.width, display_output.height - height_offset - 1),
                 make_vec4(1.0f, 1.0f, 1.0f, 1.0f));

    height_offset += channel_height + 1;
    gl_draw_quad(gl_state, render_state,
                 0.0f, display_output.height - height_offset,
                 (real32) display_output.width, channel_height,
                 make_vec3(0.1f, 0.1f, 0.1f));
    gl_draw_line(gl_state, render_state,
                 make_vec2(0.0f, display_output.height - height_offset - 1),
                 make_vec2((real32) display_output.width, display_output.height - height_offset - 1),
                 make_vec4(1.0f, 1.0f, 1.0f, 1.0f));

    int32 increment = max_samples / display_output.width;
    for (int32 i = 0; i < max_samples; i += increment) {
        real32 sample_x = (real32) i / max_samples * display_output.width;

        int16 left_sample = win32_sound_output->accumulated_sound_buffer[2*i];
        int16 right_sample = win32_sound_output->accumulated_sound_buffer[2*i + 1];

        real32 sample_height = (real32) left_sample / 32768 * channel_height;
        real32 midline_offset = display_output.height - channel_height / 2.0f;

        gl_draw_line(gl_state, render_state,
                     make_vec2(sample_x, midline_offset),
                     make_vec2(sample_x, midline_offset - sample_height),
                     make_vec4(0.0f, 1.0f, 0.0f, 1.0f));

        
        sample_height = (real32) right_sample / 32768 * channel_height;
        midline_offset -= channel_height + 1;

        gl_draw_line(gl_state, render_state,
                     make_vec2(sample_x, midline_offset),
                     make_vec2(sample_x, midline_offset - sample_height),
                     make_vec4(0.0f, 1.0f, 0.0f, 1.0f));

    }

    real32 play_cursor_position = (real32) win32_sound_output->current_play_cursor / win32_sound_output->buffer_size;
    draw_sound_cursor(gl_state, render_state, display_output, win32_sound_output,
                      play_cursor_position, make_vec3(1.0f, 1.0f, 1.0f));
    real32 write_cursor_position = (real32) win32_sound_output->current_write_cursor / win32_sound_output->buffer_size;
    draw_sound_cursor(gl_state, render_state, display_output, win32_sound_output,
                      write_cursor_position, make_vec3(1.0f, 0.0f, 0.0f));
}

void gl_draw_ui_text(GL_State *gl_state, Asset_Manager *asset_manager,
                     Render_State *render_state,
                     UI_Text ui_text) {
    UI_Text_Style style = ui_text.style;

    Font font = get_font(asset_manager, ui_text.font_id);

    real32 x = ui_text.x;

    if (style.text_align_flags & TEXT_JUSTIFY_CENTER) {
        x -= 0.5f * get_width(font, ui_text.text);
    }

    if (style.use_offset_shadow) {
        gl_draw_text(gl_state, render_state, ui_text.font_id, &font,
                     x, ui_text.y,
                     ui_text.text, style.color,
                     style.offset_shadow_color, TEXT_SHADOW_OFFSET);
    } else {
        gl_draw_text(gl_state, render_state, ui_text.font_id, &font,
                     x, ui_text.y,
                     ui_text.text, style.color);
    }
}

void gl_draw_ui_text_button(GL_State *gl_state, Asset_Manager *asset_manager,
                            Render_State *render_state,
                            UI_Manager *ui_manager, UI_Text_Button button) {
    Vec4 color;

    Font font = get_font(asset_manager, button.font_id);

    UI_Text_Button_Style style = button.style;

    if (ui_id_equals(ui_manager->hot, button.id)) {
        color = style.hot_color;
        if (ui_id_equals(ui_manager->active, button.id)) {
            color = style.active_color;
        }
    } else {
        color = style.normal_color;
    }

    if (button.is_disabled) {
        color = style.disabled_color;
    }

    Rect rect = button.rect;
    gl_draw_rounded_quad(gl_state, render_state, rect, style.corner_radius, style.corner_flags, color);

    real32 adjusted_text_height = get_adjusted_font_height(font);

    // center text
    real32 text_width = get_width(font, button.text);
    real32 x_offset = 0;
    real32 y_offset = 0;

    if (style.text_align_flags & TEXT_ALIGN_X) {
        x_offset = get_center_x_offset(rect.width, text_width);
    }
    if (style.text_align_flags & TEXT_ALIGN_Y) {
        y_offset = get_center_baseline_offset(rect.height, adjusted_text_height);
    }
    
    UI_Text_Style text_style = button.text_style;

    if (text_style.use_offset_shadow) {
        gl_draw_text(gl_state, render_state, button.font_id, &font,
                     rect.x + x_offset, rect.y + y_offset,
                     button.text, text_style.color,
                     text_style.offset_shadow_color, TEXT_SHADOW_OFFSET);
    } else {
        gl_draw_text(gl_state, render_state, button.font_id, &font,
                     rect.x + x_offset, rect.y + y_offset,
                     button.text, text_style.color);
    }
}

void gl_draw_ui_image_button(GL_State *gl_state,
                             Asset_Manager *asset_manager, Render_State *render_state,
                             UI_Manager *ui_manager,
                             UI_Image_Button button) {
    UI_Image_Button_Style style = button.style;
    Vec4 color;

    if (ui_id_equals(ui_manager->hot, button.id)) {
        color = style.hot_color;
        if (ui_id_equals(ui_manager->active, button.id)) {
            color = style.active_color;
        }
    } else {
        color = style.normal_color;
    }

    Rect rect = { button.x, button.y, button.width, button.height + style.footer_height };
    gl_draw_rounded_quad(gl_state, render_state, rect, style.corner_radius, style.corner_flags, color);

    GL_Texture texture;
    uint32 texture_exists = hash_table_find(gl_state->texture_table, button.texture_id, &texture);
    assert(texture_exists);

    real32 width_to_height_ratio = (real32) texture.width / texture.height;

    uint32 image_constraint_flags = style.image_constraint_flags;

    real32 inner_image_width = button.width - style.padding_x * 2;
    real32 inner_image_height = button.height - style.padding_y * 2;

    real32 image_width = (real32) texture.width;
    real32 image_height = (real32) texture.height;
    if (image_constraint_flags & CONSTRAINT_FILL_BUTTON_WIDTH) {
        image_width = inner_image_width;
        if (image_constraint_flags & CONSTRAINT_KEEP_IMAGE_PROPORTIONS) {
            image_height = image_width / width_to_height_ratio;
        }
    }
    if (image_constraint_flags & CONSTRAINT_FILL_BUTTON_HEIGHT) {
        image_height = inner_image_height;
        if (image_constraint_flags & CONSTRAINT_KEEP_IMAGE_PROPORTIONS) {
            image_width = image_height * width_to_height_ratio;
        }
    }

    gl_draw_quad(gl_state, render_state, button.x + style.padding_x, button.y + style.padding_y,
                 image_width, image_height, button.texture_id);

    if (button.has_text) {
        real32 footer_height = style.footer_height + style.padding_y;
        assert(footer_height >= 0);

        Font font = get_font(asset_manager, button.font_id);
        real32 adjusted_text_height = get_adjusted_font_height(font);

        // center text
        real32 text_width = get_width(font, button.text);
        real32 x_offset = get_center_x_offset(button.width, text_width);
        real32 y_offset = get_center_baseline_offset(footer_height, adjusted_text_height);
    
        UI_Text_Style text_style = button.text_style;

        real32 text_x = button.x;
        real32 text_y = button.y + image_height + style.padding_y;
        
        if (text_style.use_offset_shadow) {
            gl_draw_text(gl_state, render_state, button.font_id, &font,
                         text_x + x_offset, text_y + y_offset,
                         button.text, text_style.color,
                         text_style.offset_shadow_color, TEXT_SHADOW_OFFSET);
        } else {
            gl_draw_text(gl_state, render_state, button.font_id, &font,
                         text_x + x_offset, text_y + y_offset,
                         button.text, text_style.color);        
        }
    }
}

void gl_draw_ui_color_button(GL_State *gl_state, Render_State *render_state,
                             UI_Manager *ui_manager,
                             UI_Color_Button button) {
    UI_Color_Button_Style style = button.style;
    Vec4 button_color;

    if (ui_id_equals(ui_manager->hot, button.id)) {
        button_color = style.hot_color;
        if (ui_id_equals(ui_manager->active, button.id)) {
            button_color = style.active_color;
        }
    } else {
        button_color = style.normal_color;
    }

    gl_draw_quad(gl_state, render_state, button.x, button.y,
                 button.width, button.height, button_color);
    gl_draw_quad(gl_state, render_state, button.x + style.padding_x, button.y + style.padding_y,
                 button.width - style.padding_x*2, button.height - style.padding_y*2, button.color);
}

void gl_draw_ui_text_box(GL_State *gl_state, Asset_Manager *asset_manager,
                         Render_State *render_state,
                         UI_Manager *ui_manager, UI_Text_Box text_box) {
    Display_Output display_output = render_state->display_output;
    UI_Text_Box_Style style = text_box.style;
    Vec4 color = style.normal_color;

    Font font = get_font(asset_manager, text_box.font_id);

    if (ui_id_equals(ui_manager->active, text_box.id)) {
        color = style.active_color;
    } else if (ui_id_equals(ui_manager->hot, text_box.id)) {
        color = style.hot_color;
    }

    gl_draw_quad(gl_state, render_state, text_box.x, text_box.y,
                 text_box.width, text_box.height,
                 color);

    UI_Text_Style text_style = text_box.text_style;

    glEnable(GL_SCISSOR_TEST);
    // TODO: should move where the text renders depending on if the cursor moves outside of the
    //       text box's bounds.
    glScissor((int32) (text_box.x + style.padding_x),
              (int32) (display_output.height - text_box.y - text_box.height - style.padding_y),
              (int32) (text_box.width - style.padding_x * 2), (int32) display_output.height);

    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 text_y = text_box.y + text_box.height - style.padding_y;
    if (style.text_align_flags & TEXT_ALIGN_Y) {
        real32 inner_height = (text_box.height - 2*style.padding_y);
        text_y += -0.5f*inner_height + 0.5f*adjusted_text_height;
        //y_offset = 0.5f * (-inner_height + adjusted_text_height);
    }

    if (text_style.use_offset_shadow) {
        gl_draw_text(gl_state, render_state, text_box.font_id, &font,
                     text_box.x + style.padding_x, text_y,
                     text_box.buffer, text_style.color,
                     text_style.offset_shadow_color, TEXT_SHADOW_OFFSET);
    } else {
        gl_draw_text(gl_state, render_state, text_box.font_id, &font,
                     text_box.x + style.padding_x, text_y,
                     text_box.buffer, text_style.color);
    }
    
    glDisable(GL_SCISSOR_TEST);

    real32 cursor_width = get_width(font, "I");

    if (ui_id_equals(ui_manager->active, text_box.id)) {
        // TODO: this cursor should actually be calculated using focus_cursor_index. we need to
        //       split the text string on that index and draw the cursor at the width of the left
        //       split. when we draw it, it has to be offset if it is outside the bounds of the text
        //       box.

        // in focus
        real32 text_width = get_width(font, text_box.buffer);
        gl_draw_quad(gl_state, render_state,
                     text_box.x + text_width + style.padding_x, text_box.y + style.padding_y,
                     cursor_width, text_box.height - style.padding_y * 2 + TEXT_SHADOW_OFFSET,
                     make_vec3(0.0f, 1.0f, 0.0f));
    }
}

void gl_draw_ui_slider(GL_State *gl_state, Asset_Manager *asset_manager,
                       Render_State *render_state,
                       UI_Manager *ui_manager, UI_Slider slider) {
    Display_Output display_output = render_state->display_output;
    UI_Slider_Style style = slider.style;



    if (slider.is_text_box) {
        UI_Text_Box_Style text_box_style = { TEXT_ALIGN_X | TEXT_ALIGN_Y, 5.0f, 5.0f,
                                             style.normal_color, style.hot_color, style.normal_color };

        UI_Text_Box text_box = make_ui_text_box(slider.x, slider.y, slider.width, slider.height,
                                                slider.buffer, slider.font_id,
                                                text_box_style, slider.text_style,
                                                slider.layer, (char *) slider.id.string_ptr, slider.id.index);

        gl_draw_ui_text_box(gl_state, asset_manager, render_state, ui_manager, text_box);
        return;
    }

    // background box
    Vec4 color = style.normal_color;
    if (ui_id_equals(ui_manager->hot, slider.id)) {
        color = style.hot_color;
        if (ui_id_equals(ui_manager->active, slider.id)) {
            color = style.active_color;
        }
    }

    gl_draw_quad(gl_state, render_state, slider.x, slider.y,
                 slider.width, slider.height,
                 color);

    // slider bar
    if (slider.is_bounded) {
        glEnable(GL_SCISSOR_TEST);
        glScissor((int32) (slider.x),
                  (int32) (display_output.height - slider.y - slider.height),
                  (int32) slider.width, (int32) slider.height);
    
        Vec4 slider_color = style.slider_normal_color;
        if (ui_id_equals(ui_manager->hot, slider.id)) {
            slider_color = style.slider_hot_color;
        }
        if (ui_id_equals(ui_manager->active, slider.id)) {
            slider_color = style.slider_active_color;
        }

        real32 bar_width = (slider.value / (slider.max - slider.min)) * slider.width;
        gl_draw_quad(gl_state, render_state, slider.x, slider.y,
                     bar_width, slider.height,
                     slider_color);
        glDisable(GL_SCISSOR_TEST);
    }

    // text
    UI_Text_Style text_style = slider.text_style;
    Font font = get_font(asset_manager, slider.font_id);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 text_width = get_width(font, slider.buffer);
    real32 text_x = slider.x + 0.5f*slider.width - 0.5f*text_width;
    real32 text_y = slider.y + 0.5f*slider.height + 0.5f*adjusted_text_height;

    if (text_style.use_offset_shadow) {
        gl_draw_text(gl_state, render_state, slider.font_id, &font,
                     text_x, text_y,
                     slider.buffer, text_style.color,
                     text_style.offset_shadow_color, TEXT_SHADOW_OFFSET);
    } else {
        gl_draw_text(gl_state, render_state, slider.font_id, &font,
                     text_x, text_y,
                     slider.buffer, text_style.color);
    }
}

void gl_draw_ui_hue_slider(GL_State *gl_state, Render_State *render_state,
                           UI_Manager *ui_manager, UI_Hue_Slider slider) {
    gl_draw_hue_slider_quad(gl_state, render_state,
                            slider.x, slider.y,
                            slider.width, slider.height);

    real32 line_y = slider.y + (slider.height - ((real32) slider.hue_degrees / 360.0f) * slider.height);
    real32 line_x = slider.x;

    gl_draw_quad(gl_state, render_state,
                 line_x, line_y,
                 slider.width, 1.0f,
                 make_vec4(1.0f, 1.0f, 1.0f, 1.0f));
}

void gl_draw_ui_hsv_picker(GL_State *gl_state, Render_State *render_state,
                           UI_Manager *ui_manager, UI_HSV_Picker picker) {
    gl_draw_hsv_quad(gl_state, render_state,
                     picker.x, picker.y,
                     picker.width, picker.height,
                     picker.hsv_color.h);

    gl_draw_circle(gl_state, render_state,
                   picker.x + picker.relative_cursor_x, picker.y + picker.relative_cursor_y,
                   14.0f, make_vec4(1.0f, 1.0f, 1.0f, 1.0f), true);

    RGB_Color rgb_color = hsv_to_rgb(picker.hsv_color);
    Vec4 color = make_vec4(rgb_to_vec3(rgb_color), 1.0f);
    gl_draw_circle(gl_state, render_state,
                   picker.x + picker.relative_cursor_x, picker.y + picker.relative_cursor_y,
                   12.0f, color, true);
                   

/*
    real32 line_y = slider.y + (slider.height - ((real32) slider.hue_degrees / 360.0f) * slider.height);
    real32 line_x = slider.x;

    gl_draw_quad(gl_state, render_state,
                 line_x, line_y,
                 slider.width, 1.0f,
                 make_vec4(1.0f, 1.0f, 1.0f, 1.0f));
*/
}

void gl_draw_ui_box(GL_State *gl_state, Render_State *render_state,
                    UI_Manager *ui_manager, UI_Box box) {
    UI_Box_Style style = box.style;

    gl_draw_quad(gl_state, render_state,
                 box.x, box.y,
                 box.width, box.height, style.background_color);

    // TODO: using box_* names for border quad data is confusing
    uint32 border_flags = box.border_flags;
    if (border_flags) {
        if (border_flags & SIDE_LEFT) {
            real32 box_x = box.x;
            if (!style.inside_border) box_x -= style.border_width;

            gl_draw_quad(gl_state, render_state,
                         box_x, box.y,
                         style.border_width, box.height, style.border_color);
        }
        if (border_flags & SIDE_BOTTOM) {
            real32 box_x = box.x;
            real32 box_y = box.y + box.height - style.border_width;
            real32 box_width = box.width;
            if (!style.inside_border) {
                box_y = box.y + box.height;
                if (border_flags & SIDE_LEFT) {
                    box_x -= style.border_width;
                    box_width += style.border_width;
                }
                if (border_flags & SIDE_RIGHT) {
                    box_width += style.border_width;
                }
            } 

            gl_draw_quad(gl_state, render_state,
                         box_x, box_y,
                         box_width, style.border_width, style.border_color);
        }
        if (border_flags & SIDE_TOP) {
            real32 box_x = box.x;
            real32 box_y = box.y;
            real32 box_width = box.width;
            if (!style.inside_border) {
                box_y = box.y - style.border_width;
                if (border_flags & SIDE_LEFT) {
                    box_x -= style.border_width;
                    box_width += style.border_width;
                }
                if (border_flags & SIDE_RIGHT) {
                    box_width += style.border_width;
                }
            }

            gl_draw_quad(gl_state, render_state,
                         box_x, box_y,
                         box_width, style.border_width, style.border_color);
        }
        if (border_flags & SIDE_RIGHT) {
            real32 box_x = box.x + box.width - style.border_width;
            if (!style.inside_border) box_x += style.border_width;
            gl_draw_quad(gl_state, render_state,
                         box_x, box.y,
                         style.border_width, box.height, style.border_color);
        }
    }
}

void gl_draw_ui_line(GL_State *gl_state, Render_State *render_state,
                     UI_Manager *ui_manager, UI_Line line) {
    UI_Line_Style style = line.style;

    // TODO: use style.line_width in gl_draw_line()
    gl_draw_line(gl_state, render_state,
                 line.start, line.end,
                 style.color);
}

// TODO: we could, along with gl_draw_quad, replace the model_matrix stuff with just updating the VBO.
//       the issue with this is that it could make it harder for us to do more interesting transformations like
//       rotation.
void gl_draw_ui(GL_State *gl_state,
                Asset_Manager *asset_manager, Render_State *render_state,
                UI_Manager *ui_manager) {
    UI_Push_Buffer *push_buffer = &ui_manager->push_buffer;
    uint8 *address = (uint8 *) push_buffer->base;
    //Render_State *render_state = &game_state->render_state;

    // NOTE: we don't store the lowest layer, so technically you could do a bunch of push_layer()'s, and
    //       only add an element on the highest layer, which would cause us to loop a bunch of times until
    //       we reach that layer. i.e. this procedure assumes that every layer has something on it.

    int32 current_layer = 0;

    UI_Element *element = next_element(NULL, push_buffer);

    bool32 found_greater_layer_element = false;
    UI_Element *first_greater_layer_element = element;

    while (first_greater_layer_element) {
        // start from the first element that was not on the last layer, i.e. skip all the one's before
        // it, since those are guaranteed to have been on the last layer and thus already rendered.
        element = first_greater_layer_element;
        
        while (element) {
            if (element->layer != current_layer) {
                if (!found_greater_layer_element && (element->layer > current_layer)) {
                    found_greater_layer_element = true;
                    first_greater_layer_element = element;
                }
                element = next_element(element, push_buffer);
                continue;
            }

            switch (element->type) {
                case UI_TEXT: {
                    UI_Text *ui_text = (UI_Text *) element;
                    gl_draw_ui_text(gl_state, asset_manager, render_state, *ui_text);
                } break;
                case UI_TEXT_BUTTON: {
                    UI_Text_Button *ui_text_button = (UI_Text_Button *) element;
                    gl_draw_ui_text_button(gl_state, asset_manager, render_state, ui_manager, *ui_text_button);
                } break;
                case UI_IMAGE_BUTTON: {
                    UI_Image_Button *ui_image_button = (UI_Image_Button *) element;
                    gl_draw_ui_image_button(gl_state, asset_manager, render_state, ui_manager, *ui_image_button);
                } break;
                case UI_COLOR_BUTTON: {
                    UI_Color_Button *ui_color_button = (UI_Color_Button *) element;
                    gl_draw_ui_color_button(gl_state, render_state, ui_manager, *ui_color_button);
                } break;
                case UI_TEXT_BOX: {
                    UI_Text_Box *ui_text_box = (UI_Text_Box *) element;
                    gl_draw_ui_text_box(gl_state, asset_manager, render_state, ui_manager, *ui_text_box);
                } break;
                case UI_SLIDER: {
                    UI_Slider *ui_slider = (UI_Slider *) element;
                    gl_draw_ui_slider(gl_state, asset_manager, render_state, ui_manager, *ui_slider);
                } break;
                case UI_BOX: {
                    UI_Box *ui_box = (UI_Box *) element;
                    gl_draw_ui_box(gl_state, render_state, ui_manager, *ui_box);
                } break;
                case UI_LINE: {
                    UI_Line *ui_line = (UI_Line *) element;
                    gl_draw_ui_line(gl_state, render_state, ui_manager, *ui_line);
                } break;
                case UI_HUE_SLIDER: {
                    UI_Hue_Slider *ui_hue_slider = (UI_Hue_Slider *) element;
                    gl_draw_ui_hue_slider(gl_state, render_state, ui_manager, *ui_hue_slider);
                } break;
                case UI_HSV_PICKER: {
                    UI_HSV_Picker *ui_hsv_picker = (UI_HSV_Picker *) element;
                    gl_draw_ui_hsv_picker(gl_state, render_state, ui_manager, *ui_hsv_picker);
                } break;
                case UI_COLOR_PICKER: {
                    UI_Color_Picker *ui_color_picker = (UI_Color_Picker *) element;
                } break;
                default: {
                    assert(!"Unhandled UI element type.");
                }
            }

            element = next_element(element, push_buffer);
        }

        if (!found_greater_layer_element) {
            // on this loop, all the elements we visited were on the same layer or lower, so that means
            // we've rendered up to and including the elements on the final layer, so we're done.
            break;
        } else {
            found_greater_layer_element = false;
            current_layer++;
        }
    }
}

void gl_draw_framebuffer(GL_State *gl_state, GL_Framebuffer framebuffer) {
    // use premultiplied alpha to prevent dark edges when transitioning from opaque to transparent
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    if (framebuffer.is_multisampled) {
        int32 shader_id = gl_use_shader(gl_state, "multisampled_framebuffer");
        gl_set_uniform_int(shader_id, "num_samples", framebuffer.num_samples);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.color_buffer_texture);
    } else {
        gl_use_shader(gl_state, "framebuffer");
        glBindTexture(GL_TEXTURE_2D, framebuffer.color_buffer_texture);
    }
    
    GL_Mesh gl_mesh = gl_use_rendering_mesh(gl_state, gl_state->framebuffer_quad_mesh_id);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_gizmo(GL_State *gl_state, Render_State *render_state, Gizmo_State *gizmo_state) {
    Transform_Mode transform_mode = gizmo_state->transform_mode;

    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic_3d"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    Transform x_transform, y_transform, z_transform;

    // this is for a world-space gizmo
    // TODO: if we have the model matrix of the mesh, we could use the columns of that to create the model
    //       matrix of the gizmo when in local mode. but this is fine too.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo_state->transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo_state->transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo_state->transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);
    } else {
        x_transform = gizmo_state->transform;
        y_transform = gizmo_state->transform;
        y_transform.rotation = gizmo_state->transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo_state->transform;
        z_transform.rotation = gizmo_state->transform.rotation*make_quaternion(-90.0f, y_axis);
    }
    
    using namespace Gizmo_Constants;

    Gizmo_Handle hovered_handle = gizmo_state->hovered_gizmo_handle;

    // translation arrows
    Vec4 x_handle_color = default_x_handle_color;
    Vec4 y_handle_color = default_y_handle_color;
    Vec4 z_handle_color = default_z_handle_color;

    if (hovered_handle == GIZMO_TRANSLATE_X) {
        x_handle_color = x_handle_hover;
    } else if (hovered_handle == GIZMO_TRANSLATE_Y) {
        y_handle_color = y_handle_hover;
    } else if (hovered_handle == GIZMO_TRANSLATE_Z) {
        z_handle_color = z_handle_hover;
    }

    GL_Mesh arrow_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->mesh_table, gizmo_state->arrow_mesh_id, &arrow_mesh);
    assert(mesh_exists);
    GL_Mesh sphere_mask_mesh;
    mesh_exists = hash_table_find(gl_state->mesh_table, gizmo_state->sphere_mesh_id, &sphere_mask_mesh);
    assert(mesh_exists);
    GL_Mesh ring_mesh;
    mesh_exists = hash_table_find(gl_state->mesh_table, gizmo_state->ring_mesh_id, &ring_mesh);
    assert(mesh_exists);
    GL_Mesh cube_mesh;
    mesh_exists = hash_table_find(gl_state->mesh_table, gizmo_state->cube_mesh_id, &cube_mesh);
    assert(mesh_exists);

    gl_draw_solid_color_mesh(gl_state, render_state, arrow_mesh, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, arrow_mesh, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, arrow_mesh, z_handle_color, z_transform);

    // scale handles
    x_handle_color = default_x_handle_color;
    y_handle_color = default_y_handle_color;
    z_handle_color = default_z_handle_color;

    if (hovered_handle == GIZMO_SCALE_X) {
        x_handle_color = x_handle_hover;
    } else if (hovered_handle == GIZMO_SCALE_Y) {
        y_handle_color = y_handle_hover;
    } else if (hovered_handle == GIZMO_SCALE_Z) {
        z_handle_color = z_handle_hover;
    }

    gl_draw_solid_color_mesh(gl_state, render_state, cube_mesh, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, cube_mesh, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, cube_mesh, z_handle_color, z_transform);

    Transform sphere_mask_transform = gizmo_state->transform;
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    gl_draw_solid_color_mesh(gl_state, render_state, sphere_mask_mesh,
                             make_vec4(0.0f, 0.0f, 0.0f, 1.0f), sphere_mask_transform);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // rotation rings
    x_handle_color = default_x_handle_color;
    y_handle_color = default_y_handle_color;
    z_handle_color = default_z_handle_color;
    
    if (hovered_handle == GIZMO_ROTATE_X) {
        x_handle_color = x_handle_hover;
    } else if (hovered_handle == GIZMO_ROTATE_Y) {
        y_handle_color = y_handle_hover;
    } else if (hovered_handle == GIZMO_ROTATE_Z) {
        z_handle_color = z_handle_hover;
    }

    gl_draw_solid_color_mesh(gl_state, render_state, ring_mesh, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, ring_mesh, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, ring_mesh, z_handle_color, z_transform);
}

void gl_render_editor(GL_State *gl_state, Render_State *render_state, GL_Framebuffer framebuffer,
                      Editor_State *editor_state) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    Display_Output display_output = render_state->display_output;
    
    Marker m = begin_region();

    if (editor_state->should_unload_level_gpu_data) {
        // clear texture table
        // we don't need to set is_occupied to false, since all the textures in this table belong to the level,
        // so we just clear it all.
        {
            FOR_VALUE_POINTERS(int32, GL_Texture, gl_state->texture_table) {
                gl_delete_texture(*value);
            }
        }
        hash_table_reset(&gl_state->texture_table);

        // delete level meshes
        int32 num_reset = 0;
        {
            FOR_ENTRY_POINTERS(int32, GL_Mesh, gl_state->mesh_table) {
                if (entry->value.type != Mesh_Type::LEVEL) continue;

                GL_Mesh mesh = entry->value;
                gl_delete_mesh(mesh);
                entry->is_occupied = false;
                num_reset++;
            }
        }
        gl_state->mesh_table.num_entries -= num_reset;
        assert(gl_state->mesh_table.num_entries >= 0);

        editor_state->should_unload_level_gpu_data = false;
    }

    // load level meshes
    {
        FOR_ENTRY_POINTERS(int32, Mesh, asset_manager->mesh_table) {
            int32 mesh_key = entry->key;
            Mesh *mesh = &entry->value;
            
            if (!mesh->is_loaded) {
                if (!hash_table_exists(gl_state->mesh_table, mesh_key)) {
                    GL_Mesh gl_mesh = gl_load_mesh(gl_state, *mesh);
                    hash_table_add(&gl_state->mesh_table, mesh_key, gl_mesh);
                } else {
                    debug_print("Mesh \"%s\" already loaded.\n", mesh->name);
                }

                mesh->is_loaded = true;
            }
        }
    }

    // load textures
    {
        FOR_ENTRY_POINTERS(int32, Texture, asset_manager->texture_table) {
            int32 texture_key = entry->key;
            Texture *texture = &entry->value;
            
            if (!texture->is_loaded) {
                if (!hash_table_exists(gl_state->texture_table, texture_key)) {
                    GL_Texture gl_texture = gl_load_texture(gl_state, *texture);
                    hash_table_add(&gl_state->texture_table, texture_key, gl_texture);
                } else {
                    debug_print("Texture \"%s\" already loaded.\n", texture->name);
                }

                texture->is_loaded = true;
            }
        }
    }

    // delete GL mesh data for meshes that no longer exist in level
    {
        FOR_ENTRY_POINTERS(int32, GL_Mesh, gl_state->mesh_table) {
            int32 mesh_key = entry->key;
            if (!hash_table_exists(asset_manager->mesh_table, mesh_key)) {
                hash_table_remove(&gl_state->mesh_table, mesh_key);
                gl_delete_mesh(entry->value);
            }
        }
    }

    // delete GL texture data for textures that no longer exist in level
    {
        FOR_ENTRY_POINTERS(int32, GL_Texture, gl_state->texture_table) {
            int32 texture_key = entry->key;
            if (!hash_table_exists(asset_manager->texture_table, texture_key)) {
                hash_table_remove(&gl_state->texture_table, texture_key);
                gl_delete_texture(entry->value);
            }
        }
    }

    // gather entities by type
    Linked_List<Point_Light_Entity *> point_light_entities;
    make_and_init_linked_list(Point_Light_Entity *, &point_light_entities, temp_region);
    Linked_List<Normal_Entity *> normal_entities;
    make_and_init_linked_list(Normal_Entity *, &normal_entities, temp_region);

    Editor_Level *level = &editor_state->level;
    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        switch (entity->type) {
            case ENTITY_NORMAL: {
                add(&normal_entities, (Normal_Entity *) entity);
            } break;
            case ENTITY_POINT_LIGHT: {
                add(&point_light_entities, (Point_Light_Entity *) entity);
            } break;
            default: {
                assert(!"Unhandled entity type.");
            }
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, gl_state->global_ubo);
    int64 ubo_offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) ubo_offset, sizeof(int32),
                    &point_light_entities.num_entries);
    // NOTE: not sure why we use 16 here, instead of 32, which is the size of the GL_Point_Light struct.
    //       i think we just use the aligned offset of the first member of the struct, which is a vec4, so we offset
    //       by 16 since it's the closest multiple.
    ubo_offset += 16;
    FOR_LIST_NODES(Point_Light_Entity *, point_light_entities) {
        Point_Light_Entity *entity = current_node->value;
        if (entity->type == ENTITY_POINT_LIGHT) {
            // TODO: we may just want to replace position and light_color with vec4s in Point_Light_Entity.
            //       although this would be kind of annoying since we would have to modify the Transform struct.
            GL_Point_Light gl_point_light = {
                make_vec4(entity->transform.position, 1.0f),
                make_vec4(entity->light_color, 1.0f),
                entity->falloff_start,
                entity->falloff_end
            };

            glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) ubo_offset,
                            sizeof(GL_Point_Light), &gl_point_light);
            ubo_offset += sizeof(GL_Point_Light) + 8; // add 8 bytes of padding so that it aligns to size of vec4
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    Entity *selected_entity = get_selected_entity(editor_state);

    // entities
    {
        FOR_LIST_NODES(Normal_Entity *, normal_entities) {
            Normal_Entity *entity = current_node->value;

            int32 mesh_id = entity->mesh_id;

            Material material;
            if (entity->material_id >= 0) {
                material = get_material(asset_manager, entity->material_id);
            } else {
                material = default_material;
            }

            gl_draw_mesh(gl_state, render_state,
                         mesh_id, material,
                         entity->transform);

            if (editor_state->show_wireframe &&
                selected_entity &&
                selected_entity->type == ENTITY_NORMAL &&
                selected_entity->id == entity->id) {
                gl_draw_wireframe(gl_state, render_state, mesh_id, entity->transform);
            }

            if (editor_state->show_colliders) {
                gl_draw_collider(gl_state, render_state, entity->collider);
            }
        }
    }

    // point light icons
    
    int32 num_point_lights = point_light_entities.num_entries;

    Vec3 *positions = (Vec3 *) allocate(temp_region, sizeof(Vec3)*num_point_lights);
    int32 current_index = 0;
    FOR_LIST_NODES(Point_Light_Entity *, point_light_entities) {
        Point_Light_Entity *entity = current_node->value;
        positions[current_index++] = truncate_v4_to_v3(render_state->view_matrix *
                                                       make_vec4(entity->transform.position, 1.0f));
    }

    // insertion sort
    for (int32 i = 1; i < num_point_lights; i++) {
        Vec3 key = positions[i];
        int32 j = i - 1;
        for (; j >= 0 && (positions[j].z < key.z); j--) {
            positions[j + 1] = positions[j];
        }
        positions[j + 1] = key;
    }

    glDepthMask(GL_FALSE);
    for (int32 i = 0; i < num_point_lights; i++) {
        Vec3 view_space_position = positions[i];
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl_draw_constant_facing_quad_view_space(gl_state, render_state,
                                                view_space_position, Editor_Constants::point_light_side_length,
                                                gl_state->light_icon_texture_id, true);
    }
    glDepthMask(GL_TRUE);

    glBindFramebuffer(GL_FRAMEBUFFER, gl_state->gizmo_framebuffer.fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (editor_state->selected_entity_id >= 0) {
        gl_draw_gizmo(gl_state, render_state, &editor_state->gizmo_state);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    gl_draw_framebuffer(gl_state, gl_state->gizmo_framebuffer);
    
    end_region(m);
}

void gl_render_game(GL_State *gl_state, Render_State *render_state, GL_Framebuffer framebuffer,
                    Game_State *game_state) {
    Asset_Manager *asset_manager = &game_state->asset_manager;
    Display_Output display_output = render_state->display_output;
    
    Marker m = begin_region();

    // TODO: if we ever change levels in the game, we should clear its GPU data, but since everything in the game
    //       is static right now, we only handle loading and not unloading.

    // NOTE: we currently share the same asset_manager when switching from edit mode to game mode.

    // load level meshes
    {
        FOR_ENTRY_POINTERS(int32, Mesh, asset_manager->mesh_table) {
            int32 mesh_key = entry->key;
            Mesh *mesh = &entry->value;
            
            if (!mesh->is_loaded) {
                if (!hash_table_exists(gl_state->mesh_table, mesh_key)) {
                    GL_Mesh gl_mesh = gl_load_mesh(gl_state, *mesh);
                    hash_table_add(&gl_state->mesh_table, mesh_key, gl_mesh);
                } else {
                    debug_print("Mesh \"%s\" already loaded.\n", mesh->name);
                }

                mesh->is_loaded = true;
            }
        }
    }

    // load textures
    {
        FOR_ENTRY_POINTERS(int32, Texture, asset_manager->texture_table) {
            int32 texture_key = entry->key;
            Texture *texture = &entry->value;
            
            if (!texture->is_loaded) {
                if (!hash_table_exists(gl_state->texture_table, texture_key)) {
                    GL_Texture gl_texture = gl_load_texture(gl_state, *texture);
                    hash_table_add(&gl_state->texture_table, texture_key, gl_texture);
                } else {
                    debug_print("Texture \"%s\" already loaded.\n", texture->name);
                }

                texture->is_loaded = true;
            }
        }
    }

    // delete GL mesh data for meshes that no longer exist in level
    {
        FOR_ENTRY_POINTERS(int32, GL_Mesh, gl_state->mesh_table) {
            int32 mesh_key = entry->key;
            if (!hash_table_exists(asset_manager->mesh_table, mesh_key)) {
                hash_table_remove(&gl_state->mesh_table, mesh_key);
                gl_delete_mesh(entry->value);
            }
        }
    }

    // delete GL texture data for textures that no longer exist in level
    {
        FOR_ENTRY_POINTERS(int32, GL_Texture, gl_state->texture_table) {
            int32 texture_key = entry->key;
            if (!hash_table_exists(asset_manager->texture_table, texture_key)) {
                hash_table_remove(&gl_state->texture_table, texture_key);
                gl_delete_texture(entry->value);
            }
        }
    }

    // gather entities by type
    Linked_List<Point_Light_Entity *> point_light_entities;
    make_and_init_linked_list(Point_Light_Entity *, &point_light_entities, temp_region);
    Linked_List<Normal_Entity *> normal_entities;
    make_and_init_linked_list(Normal_Entity *, &normal_entities, temp_region);

    Game_Level *level = &game_state->level;
    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        switch (entity->type) {
            case ENTITY_NORMAL: {
                add(&normal_entities, (Normal_Entity *) entity);
            } break;
            case ENTITY_POINT_LIGHT: {
                add(&point_light_entities, (Point_Light_Entity *) entity);
            } break;
            default: {
                assert(!"Unhandled entity type.");
            }
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, gl_state->global_ubo);
    int64 ubo_offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) ubo_offset, sizeof(int32),
                    &point_light_entities.num_entries);
    // NOTE: not sure why we use 16 here, instead of 32, which is the size of the GL_Point_Light struct.
    //       i think we just use the aligned offset of the first member of the struct, which is a vec4, so we offset
    //       by 16 since it's the closest multiple.
    ubo_offset += 16;
    FOR_LIST_NODES(Point_Light_Entity *, point_light_entities) {
        Point_Light_Entity *entity = current_node->value;
        if (entity->type == ENTITY_POINT_LIGHT) {
            // TODO: we may just want to replace position and light_color with vec4s in Point_Light_Entity.
            //       although this would be kind of annoying since we would have to modify the Transform struct.
            GL_Point_Light gl_point_light = {
                make_vec4(entity->transform.position, 1.0f),
                make_vec4(entity->light_color, 1.0f),
                entity->falloff_start,
                entity->falloff_end
            };

            glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) ubo_offset,
                            sizeof(GL_Point_Light), &gl_point_light);
            ubo_offset += sizeof(GL_Point_Light) + 8; // add 8 bytes of padding so that it aligns to size of vec4
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // entities
    {
        FOR_LIST_NODES(Normal_Entity *, normal_entities) {
            Normal_Entity *entity = current_node->value;

            int32 mesh_id = entity->mesh_id;

            Material material;
            if (entity->material_id >= 0) {
                material = get_material(asset_manager, entity->material_id);
            } else {
                material = default_material;
            }

            gl_draw_mesh(gl_state, render_state,
                         mesh_id, material,
                         entity->transform);
        }
    }

    Player *player = &game_state->player;
    Transform player_circle_transform = make_transform(player->position, make_quaternion(-90.0f, x_axis),
                                                       make_vec3(Player_Constants::capsule_radius,
                                                                 Player_Constants::capsule_radius,
                                                                 1.0f));
    glDisable(GL_DEPTH_TEST);
    gl_draw_circle(gl_state, render_state, player_circle_transform, make_vec4(0.0f, 1.0f, 1.0f, 1.0f));
    glEnable(GL_DEPTH_TEST);    

    end_region(m);    
}

void gl_render(GL_State *gl_state, Game_State *game_state,
               Controller_State *controller_state,
               Win32_Sound_Output *win32_sound_output) {
    Render_State *render_state = &game_state->render_state;
    Display_Output display_output = render_state->display_output;
    
    Asset_Manager *asset_manager;
    if (game_state->mode == Game_Mode::PLAYING) {
        asset_manager = &game_state->asset_manager;
    } else {
        asset_manager = &game_state->editor_state.asset_manager;
    }
    
    // load fonts
    Hash_Table<int32, Font> *font_table = &asset_manager->font_table;
    FOR_ENTRY_POINTERS(int32, Font, *font_table) {
        Font *font = &entry->value;

        if (!font->is_baked) {
            // NOTE: the font names are always in read-only memory
            if (!hash_table_exists(gl_state->font_texture_table, entry->key)) {
                gl_init_font(gl_state, font);
            } else {
                debug_print("%s already loaded.\n", font->name);
            }

            font->is_baked = true;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, gl_state->msaa_framebuffer.fbo);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(1.0f);

    if (game_state->mode == Game_Mode::PLAYING) {
        gl_render_game(gl_state, render_state, gl_state->msaa_framebuffer, game_state);

        glDisable(GL_DEPTH_TEST);
        // TODO: for some reason, if we comment out this line, nothing renders at all, other than the gizmos
        //       if we happen to click in an area where there is an entity
        //       - pretty sure it has to do with gl_draw_text(), since if we never call that, then nothing
        //         renders
        gl_draw_ui(gl_state, asset_manager, render_state, &game_state->ui_manager);
        glEnable(GL_DEPTH_TEST);
    } else {
        gl_render_editor(gl_state, render_state, gl_state->msaa_framebuffer,
                         &game_state->editor_state);

        glDisable(GL_DEPTH_TEST);
        // TODO: for some reason, if we comment out this line, nothing renders at all, other than the gizmos
        //       if we happen to click in an area where there is an entity
        //       - pretty sure it has to do with gl_draw_text(), since if we never call that, then nothing
        //         renders
        gl_draw_ui(gl_state, asset_manager, render_state, &game_state->ui_manager);
        glEnable(GL_DEPTH_TEST);
    }

    // debug lines
    glDisable(GL_DEPTH_TEST);
    glLineWidth(6.0f);
    Debug_State *debug_state = &game_state->debug_state;
    for (int32 i = 0; i < debug_state->num_debug_lines; i++) {
        Debug_Line *line = &debug_state->debug_lines[i];
        gl_draw_line(gl_state, render_state,
                     line->start, line->end, line->color);
    }
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);

    // draw msaa framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_state->msaa_framebuffer.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, display_output.width, display_output.height, 0, 0,
                      display_output.width, display_output.height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
