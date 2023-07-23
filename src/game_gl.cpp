#include "platform.h"
#include "math.h"
#include "hash_table.h"
#include "game_gl.h"
#include "asset.h"

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

// TODO (done): for some reason compiling with -O2 makes it so the text in the slider gets pushed by the slider

// TODO: add collider controls to entities, like have collider be the mesh, or an OBB, or a sphere, etc.

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

// TODO: just rename this to gl_state once we remove all the parameters named it
global_variable GL_State *g_gl_state;

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

#if 0
Mesh *gl_get_mesh(String name) {
    uint32 hash = get_hash(name, NUM_MESH_BUCKETS);

    GL_Mesh *current = g_gl_state->mesh_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
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

inline void gl_set_uniform_bool(uint32 shader_id, char* uniform_name, bool32 b) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform1ui(uniform_location, b);
}

inline void gl_set_uniform_float(uint32 shader_id, char* uniform_name, real32 f) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform1f(uniform_location, f);
}

GL_Shader *gl_get_shader(char *shader_name) {
    uint32 hash = get_hash(shader_name, NUM_SHADER_BUCKETS);
    GL_Shader *current = g_gl_state->shader_table[hash];
    while (current) {
        if (string_equals(current->name, shader_name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

#if 0
GL_Texture *gl_get_texture(String texture_name) {
    for (int32 i = 0; i < NUM_TEXTURE_BUCKETS; i++) {
        GL_Texture *current = g_gl_state->texture_table[i];
        while (current) {
            if (string_equals(current->name, texture_name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

inline GL_Texture *gl_get_texture(char *texture_name) {
    return gl_get_texture(make_string(texture_name));
}
#endif

GL_Texture *gl_get_texture(int32 texture_id) {
    uint32 hash = get_hash(texture_id, NUM_TEXTURE_BUCKETS);
    GL_Texture *current = g_gl_state->texture_table[hash];
    while (current) {
        if (current->id == texture_id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

#if 0
GL_Texture *gl_get_texture(char *texture_name) {
    GL_GET_TEXTURE_DEF;
}

GL_Texture *gl_get_texture(String texture_name) {
    GL_GET_TEXTURE_DEF;
}
#endif

#define GL_GET_FONT_DEF                                         \
    uint32 hash = get_hash(font_name, NUM_FONT_BUCKETS);        \
    GL_Font *current = g_gl_state->font_table[hash];            \
    while (current) {                                           \
        if (string_equals(current->name, font_name)) {          \
            return current;                                     \
        }                                                       \
                                                                \
        current = current->table_next;                          \
    }                                                           \
                                                                \
    return NULL;                                                \
    
GL_Font *gl_get_font(char *font_name) {
    GL_GET_FONT_DEF
}

GL_Font *gl_get_font(String font_name) {
    GL_GET_FONT_DEF
}

// TODO: replace calls to this to use the gl_get_mesh(int32 mesh_id) version
#if 0
GL_Mesh *gl_get_mesh(String mesh_name) {
    for (int32 i = 0; i < NUM_MESH_BUCKETS; i++) {
        GL_Mesh *current = g_gl_state->mesh_table[i];
        while (current) {
            if (string_equals(current->name, mesh_name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

// TODO: maybe just use the macro like GL_GET_TEXTURE_DEF
inline GL_Mesh *gl_get_mesh(char *mesh_name) {
    return gl_get_mesh(make_string(mesh_name));
}
#endif

GL_Mesh *gl_get_mesh(int32 mesh_id) {
    uint32 hash = get_hash(mesh_id, NUM_MESH_BUCKETS);
    GL_Mesh *current = g_gl_state->mesh_table[hash];
    while (current) {
        if (current->id == mesh_id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

bool32 gl_load_shader(char *vertex_shader_filename, char *fragment_shader_filename, char *shader_name) {
    Allocator *temp_region = begin_region();

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

    end_region(temp_region);

    // add shader to gl shader table
    uint32 hash = get_hash(make_string(shader_name), NUM_SHADER_BUCKETS);
    GL_Shader *current = g_gl_state->shader_table[hash];
    GL_Shader *last_visited = current;
    while (current) {
        if (string_equals(current->name, shader_name)) {
            assert(!"GL_Shader with this name already exists!");
            return false;
        }

        last_visited = current;
        current = current->table_next;
    }

    GL_Shader *gl_shader = (GL_Shader *) allocate(&g_gl_state->heap, sizeof(GL_Shader));
    *gl_shader = { shader_id };
    gl_shader->name = copy((Allocator *) &g_gl_state->heap, make_string(shader_name));
    gl_shader->table_prev = last_visited;
    gl_shader->table_next = NULL;
    
    if (!last_visited) {
        g_gl_state->shader_table[hash] = gl_shader;
    } else {
        last_visited->table_next = gl_shader;
    }

    return true;
}

#if 0
GL_Texture gl_load_texture(GL_State *gl_state, char *texture_filename, bool32 has_alpha=false) {
    Allocator *temp_region = begin_region();
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

    end_region(temp_region);

    GL_Texture gl_texture = { texture_id, width, height, num_channels };
    return gl_texture;
}
#endif

bool32 gl_load_texture(Texture *texture, bool32 has_alpha = false) {
    Allocator *temp_region = begin_region();
    Allocator *temp_allocator = (Allocator *) &memory.global_stack;
    char *temp_texture_filename = to_char_array(temp_allocator, texture->filename);
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

    end_region(temp_region);

    // add gl_texture to the gl texture table
    uint32 hash = get_hash(texture->id, NUM_TEXTURE_BUCKETS);
    GL_Texture *current = g_gl_state->texture_table[hash];
    GL_Texture *last_visited = current;
    while (current) {
        if (current->id == texture->id) {
            assert(!"GL_Texture with this ID already exists!");
            return false;
        }

        last_visited = current;
        current = current->table_next;
    }

    GL_Texture *gl_texture = (GL_Texture *) allocate(&g_gl_state->heap, sizeof(GL_Texture));
    *gl_texture = { texture->type, texture->id, texture_id, width, height, num_channels };
    //gl_texture->name = copy((Allocator *) &g_gl_state->heap, texture->name);
    gl_texture->table_prev = last_visited;
    gl_texture->table_next = NULL;
    
    if (!last_visited) {
        g_gl_state->texture_table[hash] = gl_texture;
    } else {
        last_visited->table_next = gl_texture;
    }

    return true;
}

// TODO: use the better stb_truetype packing procedures
// we don't have per-level fonts right now, so we don't have an unloading font procedure.
// not sure if we ever will need per-level fonts.
void gl_init_font(Font *font) {
    Allocator *temp_region = begin_region();

    assert(font->is_baked);
    
    uint32 baked_texture_id;
    glGenTextures(1, &baked_texture_id);
    glBindTexture(GL_TEXTURE_2D, baked_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 font->texture_width, font->texture_height,
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, font->bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    end_region(temp_region);

    // add baked font to the gl_font table
    uint32 hash = get_hash(font->name, NUM_FONT_BUCKETS);
    GL_Font *current = g_gl_state->font_table[hash];
    GL_Font *last_visited = current;
    while (current) {
        if (string_equals(current->name, font->name)) {
            assert(!"GL_Font with this name already exists!");
            return;
        }

        last_visited = current;
        current = current->table_next;
    }

    GL_Font *gl_font = (GL_Font *) allocate(&g_gl_state->heap, sizeof(GL_Font));
    gl_font->name = font->name;
    gl_font->baked_texture_id = baked_texture_id;
    gl_font->table_prev = last_visited;
    gl_font->table_next = NULL;
    
    if (!last_visited) {
        g_gl_state->font_table[hash] = gl_font;
    } else {
        last_visited->table_next = gl_font;
    }
}

bool32 gl_add_mesh(int32 id, Mesh_Type type, uint32 vao, uint32 vbo, uint32 num_triangles) {
    if (type != Mesh_Type::LEVEL) {
        assert(id < 0);
    }
    
    GL_Mesh *mesh = gl_get_mesh(id);
    if (mesh) {
        assert(!"GL_Mesh with this ID already exists!");
        return false;
    }

#if 0
    mesh = gl_get_mesh(name);
    if (mesh) {
        assert(!"GL_Mesh with this name already exists!");
        return false;
    }
#endif

    // add gl_mesh to the gl mesh table
    uint32 hash = get_hash(id, NUM_MESH_BUCKETS);
    GL_Mesh *current = g_gl_state->mesh_table[hash];
    GL_Mesh *last = NULL;
    while (current) {
        last = current;
        current = current->table_next;
    }
    
    GL_Mesh *gl_mesh = (GL_Mesh *) allocate(&g_gl_state->heap, sizeof(GL_Mesh));
    *gl_mesh = { type, vao, vbo, num_triangles };
    gl_mesh->id         = id;
    //gl_mesh->name       = copy((Allocator *) &g_gl_state->heap, name);
    gl_mesh->table_prev = last;
    gl_mesh->table_next = NULL;
    
    if (!last) {
        g_gl_state->mesh_table[hash] = gl_mesh;
    } else {
        last->table_next = gl_mesh;
    }

    return true;
}

bool32 gl_load_mesh(Mesh *mesh) {
    uint32 vao, vbo, ebo;
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
        
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->data_size, mesh->data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_size, mesh->indices, GL_STATIC_DRAW);

    // vertices
    glVertexAttribPointer(0, mesh->n_vertex, GL_FLOAT, GL_FALSE,
                          mesh->vertex_stride * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    // normals
    glVertexAttribPointer(1, mesh->n_normal, GL_FLOAT, GL_FALSE,
                          mesh->vertex_stride * sizeof(real32),
                          (void *) (mesh->n_vertex * sizeof(real32)));
    glEnableVertexAttribArray(1);

    // UVs
    glVertexAttribPointer(2, mesh->n_uv, GL_FLOAT, GL_FALSE,
                          mesh->vertex_stride * sizeof(real32),
                          (void *) ((mesh->n_vertex + mesh->n_normal) * sizeof(real32)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // add gl_mesh to the gl mesh table
    return gl_add_mesh(mesh->id, mesh->type, vao, vbo, mesh->num_triangles);
}

uint32 gl_use_shader(char *shader_name) {
    GL_Shader *shader = gl_get_shader(shader_name);
    
    if (shader) {
        glUseProgram(shader->id);
    } else {
        assert(!"Shader does not exist!");
        return 0;
    }

    return shader->id;
}

uint32 gl_use_texture(int32 texture_id, int32 slot_index = 0) {
    GL_Texture *texture = gl_get_texture(texture_id);

    GLenum slots[3] = { GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2 };
    
    if (texture) {
        glActiveTexture(slots[slot_index]);
        glBindTexture(GL_TEXTURE_2D, texture->gl_texture_id);
    } else {
        assert(!"Texture does not exist!");
        return 0;
    }

    glActiveTexture(GL_TEXTURE0);

    return texture->gl_texture_id;
}

uint32 gl_use_font_texture(String font_name) {
    GL_Font *font = gl_get_font(font_name);
    
    if (font) {
        glBindTexture(GL_TEXTURE_2D, font->baked_texture_id);
    } else {
        assert(!"Font does not exist!");
        return 0;
    }

    return font->baked_texture_id;
}

inline uint32 gl_use_font_texture(char *font_name) {
    return gl_use_font_texture(make_string(font_name));
}

#if 0
GL_Mesh *gl_use_mesh(String mesh_name) {
    GL_Mesh *mesh = gl_get_mesh(mesh_name);
    
    if (mesh) {
        glBindVertexArray(mesh->vao);
    } else {
        assert(!"Mesh does not exist!");
        return NULL;
    }

    return mesh;
}

inline GL_Mesh *gl_use_mesh(char *mesh_name) {
    GL_Mesh *mesh = gl_get_mesh(mesh_name);
    
    if (mesh) {
        glBindVertexArray(mesh->vao);
    } else {
        assert(!"Mesh does not exist!");
        return NULL;
    }

    return mesh;
}
#endif

inline GL_Mesh *gl_use_mesh(int32 mesh_id) {
    GL_Mesh *mesh = gl_get_mesh(mesh_id);
    
    if (mesh) {
        glBindVertexArray(mesh->vao);
    } else {
        assert(!"Mesh does not exist!");
        return NULL;
    }

    return mesh;
}

void gl_draw_solid_color_mesh(int32 mesh_id, Vec4 color,
                              Transform transform) {
    uint32 shader_id = gl_use_shader("solid");
    GL_Mesh *gl_mesh = gl_use_mesh(mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    gl_set_uniform_int(shader_id, "use_color_override", true);
    
    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

#if 0
void gl_draw_solid_color_mesh(GL_Mesh gl_mesh, Vec4 color,
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
#endif

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

void gl_draw_mesh(int32 mesh_id,
                  Material *material,
                  Transform transform) {
    uint32 shader_id = gl_use_shader("pbr");

    GL_Mesh *gl_mesh = gl_use_mesh(mesh_id);
    gl_use_texture(material->albedo_texture_id,    0);
    gl_use_texture(material->metalness_texture_id, 1);
    gl_use_texture(material->roughness_texture_id, 2);

    gl_set_uniform_int(shader_id, "albedo_texture", 0);
    gl_set_uniform_int(shader_id, "metalness_texture", 1);
    gl_set_uniform_int(shader_id, "roughness_texture", 2);
    
    // TODO: call gl_use_texture with different slots based on material_use_x_texture flags

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    // TODO: we may need to think about this for transparent materials
    gl_set_uniform_bool(shader_id, "use_albedo_texture",    material->flags & MATERIAL_USE_ALBEDO_TEXTURE);
    gl_set_uniform_bool(shader_id, "use_metalness_texture", material->flags & MATERIAL_USE_METALNESS_TEXTURE);
    gl_set_uniform_bool(shader_id, "use_roughness_texture", material->flags & MATERIAL_USE_ROUGHNESS_TEXTURE);
    
    gl_set_uniform_vec3(shader_id, "u_albedo_color", &material->albedo_color);
    gl_set_uniform_float(shader_id, "u_metalness", material->metalness);
    gl_set_uniform_float(shader_id, "u_roughness", material->roughness);
    gl_set_uniform_float(shader_id, "ao", 1.0f);
    
    gl_set_uniform_vec3(shader_id, "camera_pos", &render_state->camera.position);
    
    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_textured_mesh(int32 mesh_id, int32 texture_id, Transform transform) {
    uint32 shader_id = gl_use_shader("basic_3d_textured");
    gl_use_texture(texture_id);

    GL_Mesh *gl_mesh = gl_use_mesh(mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);

    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void gl_draw_wireframe(int32 mesh_id, Vec4 color, Transform transform) {
    uint32 shader_id = gl_use_shader("debug_wireframe");

    GL_Mesh *gl_mesh = gl_use_mesh(mesh_id);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthFunc(GL_LESS);

    glUseProgram(0);
    glBindVertexArray(0);
}

inline void gl_draw_wireframe(int32 mesh_id, Transform transform) {
    gl_draw_wireframe(mesh_id, make_vec4(1.0f, 1.0f, 0.0f, 1.0f), transform);
}

void gl_draw_circle(Vec4 color, Transform transform) {
    uint32 shader_id = gl_use_shader("line_3d");
    GL_Mesh *circle_mesh = gl_use_mesh(ENGINE_CIRCLE_MESH_ID);

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
void gl_draw_capsule(Vec3 base, Vec3 tip, real32 radius,
                     Vec4 color) {
    uint32 shader_id = gl_use_shader("debug_wireframe");

    Vec3 normal = normalize(tip - base);
    Vec3 a = base + normal*radius; // bottom sphere center
    Vec3 b = tip - normal*radius;  // top sphere center
    real32 length = distance(b - a);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GL_Mesh *gl_mesh;
    Transform transform;
    Mat4 model_matrix;

    // capsule cylinder body
    gl_mesh = gl_use_mesh(ENGINE_CAPSULE_CYLINDER_MESH_ID);
    transform = make_transform(a, make_quaternion(), make_vec3(radius, length, radius));
    model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);

    // capsule cylinder bottom cap
    gl_mesh = gl_use_mesh(ENGINE_CAPSULE_CAP_MESH_ID);
    transform = make_transform(a, make_quaternion(180.0f, x_axis), make_vec3(radius, radius, radius));
    model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);

    // capsule cylinder top cap
    transform = make_transform(b, make_quaternion(), make_vec3(radius, radius, radius));
    model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    glDrawElements(GL_TRIANGLES, gl_mesh->num_triangles * 3, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_collider(Collider_Variant collider) {
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
            gl_draw_circle(collider_color, transform);
        } break;
        case Collider_Type::CAPSULE: {
            Capsule capsule = collider.capsule.capsule;
            gl_draw_capsule(capsule.base, capsule.tip, capsule.radius, collider_color);
        } break;
        default: {
            assert(!"Unhandled collider type");
            return;
        } break;
    }
}

void gl_draw_text(char *font_name,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  char *text, int32 num_chars, bool32 is_null_terminated,
                  Vec4 color,
                  bool32 has_shadow, Vec4 shadow_color, real32 shadow_offset) {
    uint32 text_shader_id = gl_use_shader("text");

    GL_Mesh *glyph_mesh = gl_use_mesh(ENGINE_GLYPH_QUAD_MESH_ID);

    gl_set_uniform_mat4(text_shader_id, "cpv_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(text_shader_id, "color", &color);
    //gl_set_uniform_int(text_shader_id, "has_shadow", has_shadow);

    if (has_shadow) {
        //gl_set_uniform_vec4(text_shader_id, "shadow_color", &shadow_color);
        //gl_set_uniform_float(text_shader_id, "shadow_offset", shadow_offset);
    }

    // NOTE: we disable depth test so that overlapping characters such as the "o" in "fo" doesn't cover the
    //       quad of the previous character, causing a cut off look.
    // NOTE: we assume that GL_DEPTH_TEST is disabled
    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    
    gl_use_font_texture(font_name);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_mesh->vbo);

    Font *font = get_font(font_name);
    real32 line_advance = font->scale_for_pixel_height * (font->ascent - font->descent + font->line_gap);
    real32 start_x_pos_pixels = x_pos_pixels;

    Allocator *temp_region = begin_region();

    uint32 text_buffer_size = sizeof(Vec2)*GL_MAX_TEXT_CHARACTERS;
    Vec2 *glyph_positions = (Vec2 *) allocate(temp_region, text_buffer_size);
    Vec2 *glyph_sizes     = (Vec2 *) allocate(temp_region, text_buffer_size);
    Vec2 *uv_positions    = (Vec2 *) allocate(temp_region, text_buffer_size);
    Vec2 *uv_sizes        = (Vec2 *) allocate(temp_region, text_buffer_size);
        
    int32 i = 0;
    while (*text && (is_null_terminated || (i < num_chars))) {
        assert(i < GL_MAX_TEXT_CHARACTERS);
        
        if (*text >= 32 && *text < 128 || *text == '-') {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->cdata, 512, 512, *text - 32, &x_pos_pixels, &y_pos_pixels, &q, 1);

            Vec2 glyph_position = { q.x0, q.y0 };
            Vec2 glyph_size     = { q.x1 - q.x0, q.y1 - q.y0 };

            // our UVs are have 0,0 in the bottom left, so to make the 0,0 UV equal to the bottom left UV
            // returned by stb, we need to flip them. kinda confusing, but if you plug in values, you'll
            // see that it works.
            Vec2 uv_position    = { q.s0, q.t1 };
            Vec2 uv_size        = { q.s1 - q.s0, q.t0 - q.t1 };

            glyph_positions[i] = glyph_position;
            glyph_sizes[i]     = glyph_size;

            uv_positions[i]    = uv_position;
            uv_sizes[i]        = uv_size;
        } else if (*text == '\n') {
            x_pos_pixels = start_x_pos_pixels;
            y_pos_pixels += line_advance;
        }
        
        text++;
        i++;
    }

    int32 num_chars_added = i;
    GLsizeiptr data_size = (GLsizeiptr) (num_chars_added*sizeof(Vec2));
    // TODO: maybe try different data layout where instance data is grouped together instead of spread out in different arrays?
    //       like xyzxyzxyz instead of xxxyyyzzz
    #if 1
    glBufferSubData(GL_ARRAY_BUFFER, 0,                                    data_size, glyph_positions);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) (int64) (text_buffer_size),   data_size, glyph_sizes);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) (int64) (text_buffer_size*2), data_size, uv_positions);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) (int64) (text_buffer_size*3), data_size, uv_sizes);
    
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    #endif

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, num_chars_added);
    
    end_region(temp_region);
    
    //glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

#if 0
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
#endif

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

#if 0
void gl_init_alpha_mask_stack(int32 width, int32 height) {
    GL_Alpha_Mask_Stack *stack = &g_gl_state->alpha_mask_stack;
    glGenTextures(MAX_ALPHA_MASKS, stack->texture_ids);

    for (int32 i = 0; i < MAX_ALPHA_MASKS; i++) {
        uint32 texture_id = stack->texture_ids[i];
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);        
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

uint32 gl_push_new_alpha_mask_texture() {
    GL_Alpha_Mask_Stack *stack = &g_gl_state->alpha_mask_stack;
    stack->index++;
    assert(stack->index < MAX_ALPHA_MASKS);
    return stack->texture_ids[stack->index];
}

// pushes new alpha mask onto the alpha mask stack and binds the framebuffer for drawing
void gl_push_alpha_mask() {
    glBindFramebuffer(GL_FRAMEBUFFER, g_gl_state->alpha_mask_framebuffer.fbo);
    uint32 alpha_texture_id = gl_push_new_alpha_mask_texture();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           alpha_texture_id, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gl_pop_alpha_mask() {
    GL_Alpha_Mask_Stack *stack = &g_gl_state->alpha_mask_stack;
    stack->index--;
    assert(stack->index >= -1);
}

// TODO: add procedure to deallocate and reset alpha masks and framebuffer when window dimensions change
GL_Framebuffer gl_make_alpha_mask_framebuffer(int32 width, int32 height) {
    GL_Framebuffer framebuffer = {};

    glGenFramebuffers(1, &framebuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}
#endif

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

void gl_unload_mesh(int32 id) {
    uint32 hash = get_hash(id, NUM_MESH_BUCKETS);

    GL_Mesh *current = g_gl_state->mesh_table[hash];
    while (current) {
        if (current->id == id) {
            if (current->table_prev) {
                current->table_prev->table_next = current->table_next;
            } else {
                // if we're first in list, we need to update bucket array when we delete
                g_gl_state->mesh_table[hash] = current->table_next;
            }
            
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            // we should only ever be deleting level meshes
            assert(current->type == Mesh_Type::LEVEL);
            
            glDeleteBuffers(1, &current->vbo);
            glDeleteVertexArrays(1, &current->vao);
            
            deallocate(current);
            deallocate((Allocator *) &g_gl_state->heap, current);

            return;
        }

        current = current->table_next;
    }
    
    assert(!"GL mesh does not exist!");
}

void gl_unload_texture(int32 id) {
    uint32 hash = get_hash(id, NUM_TEXTURE_BUCKETS);

    GL_Texture *current = g_gl_state->texture_table[hash];
    while (current) {
        if (current->id == id) {
            if (current->table_prev) {
                current->table_prev->table_next = current->table_next;
            } else {
                // if we're first in list, we need to update bucket array when we delete
                g_gl_state->texture_table[hash] = current->table_next;
            }
            
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            glDeleteTextures(1, &current->gl_texture_id);
            
            deallocate(current);
            deallocate((Allocator *) &g_gl_state->heap, current);

            return;
        }

        current = current->table_next;
    }
    
    assert(!"GL texture does not exist!");
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

void gl_init_ui() {
    uint32 vao, vbo, ebo;
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
        
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UI_Vertex) * UI_MAX_VERTICES, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * UI_MAX_INDICES, NULL, GL_DYNAMIC_DRAW);

    // vertices
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          sizeof(UI_Vertex), (void *) 0);
    glEnableVertexAttribArray(0);
    
    // UVs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(UI_Vertex),
                          (void *) (offsetof(UI_Vertex, uv)));
    glEnableVertexAttribArray(1);

    // color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
                          sizeof(UI_Vertex),
                          (void *) (offsetof(UI_Vertex, color)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    g_gl_state->ui_data = {
        vao, vbo, ebo
    };
}

void gl_init(Arena_Allocator *game_data, Display_Output display_output) {
    g_gl_state = (GL_State *) allocate((Allocator *) game_data,
                                       sizeof(GL_State), true);

    uint32 heap_size = GL_HEAP_SIZE;
    void *heap_base = arena_push(game_data, heap_size);
    g_gl_state->heap = make_heap_allocator(heap_base, heap_size);

    // TODO: move these to where we init the game, so we have a single place for all the mesh loading
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

    gl_add_mesh(ENGINE_TRIANGLE_MESH_ID, Mesh_Type::ENGINE, vao, vbo, 1);

    // NOTE: 3D quad mesh
    // we store them separately like this because we use glBufferSubData to send the vertex positions
    // directly to the shader, and i don't think there's a way to easily modify interleaved data, unless
    // you're modifying all of the data, but when we modify the data we only modify the positions and not the uvs.
    // the values in these arrays don't matter; we're just filling these arrays up with enough values such that
    // sizeof() gives the correct values

    // the numbers in the vertices array don't matter since we end up replacing them in the vbo. for the glyph quad however,
    // they do matter since they are used as a base for calculating instance vertex positions.
    // NOTE: the numbers in the UVs array does matter though, since we don't ever really change those. and they shouldn't
    //       be changed. as a consequence of this, the vertices that we update the buffer with should correspond with the
    //       correct UV in the UVs array.
    real32 quad_vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    real32 quad_uvs[] = {
        0.0f, 1.0f, // top left
        1.0f, 1.0f, // top right
        1.0f, 0.0f, // bottom right
        0.0f, 0.0f  // bottom left
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
    
    gl_add_mesh(ENGINE_QUAD_MESH_ID, Mesh_Type::ENGINE, vao, vbo, 2);
    
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
    
    gl_add_mesh(ENGINE_FRAMEBUFFER_QUAD_MESH_ID, Mesh_Type::ENGINE, vao, vbo, 2);
    
    // NOTE: glyph quad
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // these are in window-space, so x increases from left to right and y increases from top to bottom
    real32 glyph_quad_vertices[] = {
        0.0f, 0.0f, // top left
        1.0f, 0.0f, // top right
        1.0f, 1.0f, // bottom right
        0.0f, 1.0f  // bottom left
    };
    real32 glyph_quad_uvs[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
    };
    uint32 glyph_quad_indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // UV position, UV size, glyph position, glyph size = 4 Vec2's
    // also store space for the basic quad vertices and UVs that we use as a base
    uint32 buffer_size = (sizeof(glyph_quad_vertices) + sizeof(glyph_quad_uvs)) + (sizeof(Vec2)*4*GL_MAX_TEXT_CHARACTERS);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, 0, GL_DYNAMIC_DRAW);

    int64 text_buffer_size = sizeof(Vec2)*GL_MAX_TEXT_CHARACTERS;
    // we put these at the end so that we don't have to offset by them when we set the instance data arrays
    glBufferSubData(GL_ARRAY_BUFFER, (int *) (text_buffer_size*4),
                    sizeof(glyph_quad_vertices), glyph_quad_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) (text_buffer_size*4 + sizeof(glyph_quad_vertices)),
                    sizeof(glyph_quad_uvs), glyph_quad_uvs);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glyph_quad_indices), glyph_quad_indices, GL_STATIC_DRAW); 

    // positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          sizeof(real32)*2, (void *) (text_buffer_size*4));
    glEnableVertexAttribArray(0);
    
    // uvs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(real32)*2, (void *) (text_buffer_size*4+sizeof(glyph_quad_vertices)));
    glEnableVertexAttribArray(1);

    // glyph positions
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(real32)*2, (void *) 0);
    glEnableVertexAttribArray(2);
    
    // glyph sizes
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                          sizeof(real32)*2, (void *) (text_buffer_size));
    glEnableVertexAttribArray(3);

    // uv positions
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE,
                          sizeof(real32)*2, (void *) (text_buffer_size*2));
    glEnableVertexAttribArray(4);
    
    // uv sizes
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE,
                          sizeof(real32)*2, (void *) (text_buffer_size*3));
    glEnableVertexAttribArray(5);

    glBindVertexArray(0);
    
    gl_add_mesh(ENGINE_GLYPH_QUAD_MESH_ID, Mesh_Type::ENGINE, vao, vbo, 2);

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
    
    gl_add_mesh(ENGINE_DEBUG_LINE_MESH_ID, Mesh_Type::ENGINE, vao, vbo, 0);

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
    
    gl_add_mesh(ENGINE_CIRCLE_MESH_ID, Mesh_Type::ENGINE, vao, vbo, 0);

    // NOTE: shaders
    gl_load_shader("src/shaders/basic.vs",                    "src/shaders/basic.fs",
                   "basic");
    gl_load_shader("src/shaders/basic2.vs",                   "src/shaders/basic2.fs",
                   "basic2");
    gl_load_shader("src/shaders/text.vs",                     "src/shaders/text.fs",
                   "text");
    gl_load_shader("src/shaders/solid.vs",                    "src/shaders/solid.fs",
                   "solid");
    gl_load_shader("src/shaders/basic_3d.vs",                 "src/shaders/basic_3d.fs",
                   "basic_3d");
    gl_load_shader("src/shaders/basic_3d_textured.vs",        "src/shaders/basic_3d_textured.fs",
                   "basic_3d_textured");
    gl_load_shader("src/shaders/debug_wireframe.vs",          "src/shaders/debug_wireframe.fs",
                   "debug_wireframe");
    gl_load_shader("src/shaders/framebuffer.vs",              "src/shaders/framebuffer.fs",
                   "framebuffer");
    gl_load_shader("src/shaders/multisampled_framebuffer.vs", "src/shaders/multisampled_framebuffer.fs",
                   "multisampled_framebuffer");
    gl_load_shader("src/shaders/hue_slider.vs",               "src/shaders/hue_slider.fs",
                   "hue_slider");
    gl_load_shader("src/shaders/hsv.vs",                      "src/shaders/hsv.fs",
                   "hsv");
    gl_load_shader("src/shaders/mesh_2d.vs",                  "src/shaders/mesh_2d.fs",
                   "mesh_2d");
    gl_load_shader("src/shaders/line_3d.vs",                  "src/shaders/line_3d.fs",
                   "line_3d");
    gl_load_shader("src/shaders/constant_facing_quad.vs",     "src/shaders/constant_facing_quad.fs",
                   "constant_facing_quad");
    gl_load_shader("src/shaders/rounded_quad.vs",             "src/shaders/rounded_quad.fs",
                   "rounded_quad");
    gl_load_shader("src/shaders/pbr.vs",                      "src/shaders/pbr.fs",
                   "pbr");
    gl_load_shader("src/shaders/ui.vs",                       "src/shaders/ui.fs",
                   "ui");

    // NOTE: ui buffer
    gl_init_ui();

    // NOTE: framebuffers
    int32 num_samples = NUM_MSAA_SAMPLES;
    uint32 framebuffer_flags = FRAMEBUFFER_IS_HDR | FRAMEBUFFER_HAS_ALPHA;
    g_gl_state->gizmo_framebuffer = gl_make_msaa_framebuffer(display_output.width, display_output.height,
                                                             num_samples, framebuffer_flags);
    g_gl_state->msaa_framebuffer = gl_make_msaa_framebuffer(display_output.width, display_output.height,
                                                            num_samples, framebuffer_flags);
    //g_gl_state->alpha_mask_framebuffer = gl_make_alpha_mask_framebuffer(display_output.width, display_output.height);

    // NOTE: alpha mask textures
    //gl_init_alpha_mask_stack(display_output.width, display_output.height);
    
    // NOTE: unified buffer object
    glGenBuffers(1, &g_gl_state->global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, g_gl_state->global_ubo);

    // make sure to increase GLOBAL_UBO_SIZE if more size is needed
    glBufferData(GL_UNIFORM_BUFFER, GLOBAL_UBO_SIZE, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_gl_state->global_ubo);

    uint32 shader_id = gl_use_shader("pbr");
    uint32 uniform_block_index = glGetUniformBlockIndex(shader_id, "shader_globals");
    assert(uniform_block_index != GL_INVALID_INDEX);
    glUniformBlockBinding(shader_id, uniform_block_index, 0);
    glUseProgram(0);

    // NOTE: disable culling for now, just for easier debugging...
#if 1
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
#endif

    glEnable(GL_DEPTH_TEST);  
}

// NOTE: This draws a triangle that has its bottom left corner at position.
//       Position is based on percentages, so 50% x and 50%y would put the bottom left corner of the triangle
//       in the middle of the screen.
void gl_draw_triangle_p(Vec2 position,
                        real32 width_pixels, real32 height_pixels,
                        Vec4 color) {
    uint32 basic_shader_id = gl_use_shader("basic");
    GL_Mesh *triangle_mesh = gl_use_mesh(ENGINE_TRIANGLE_MESH_ID);

    Vec2 clip_space_position = make_vec2(position.x * 2.0f - 1.0f,
                                         position.y * -2.0f + 1.0f);

    Display_Output display_output = render_state->display_output;
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

void gl_draw_triangle(real32 x_pos_pixels, real32 y_pos_pixels,
                      real32 width_pixels, real32 height_pixels,
                      Vec4 color) {
    // TODO: this might be wrong with the new screen-space coordinate-system (0, 0) in top left
    Display_Output display_output = render_state->display_output;
    gl_draw_triangle_p(make_vec2(x_pos_pixels / display_output.width, y_pos_pixels / display_output.height),
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

void gl_draw_line(Vec2 start_pixels, Vec2 end_pixels,
                  Vec4 color) {
    uint32 basic_shader_id = gl_use_shader("basic2");
    GL_Mesh *line_mesh = gl_use_mesh(ENGINE_DEBUG_LINE_MESH_ID);

    real32 line_vertices[] = {
        start_pixels.x + 0.5f, start_pixels.y + 0.5f, 0.0f,
        end_pixels.x + 0.5f, end_pixels.y + 0.5f, 0.0f,
    };
    glBindBuffer(GL_ARRAY_BUFFER, line_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_vertices), line_vertices);
    gl_set_uniform_mat4(basic_shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(basic_shader_id, "color", &color);
    gl_set_uniform_int(basic_shader_id, "use_color", true);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_line(Vec3 start, Vec3 end,
                  Vec4 color) {
    uint32 basic_shader_id = gl_use_shader("line_3d");
    GL_Mesh *line_mesh = gl_use_mesh(ENGINE_DEBUG_LINE_MESH_ID);

    Mat4 model_matrix = make_mat4_identity();

    real32 line_vertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };
    glBindBuffer(GL_ARRAY_BUFFER, line_mesh->vbo);
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

inline void gl_draw_line(Vec2 start_pixels, Vec2 end_pixels,
                         Vec3 color) {
    gl_draw_line(start_pixels, end_pixels, make_vec4(color, 1.0f));
}

// NOTE: percentage based position
void gl_draw_quad_p(real32 x, real32 y,
                    real32 width_pixels, real32 height_pixels,
                    Vec4 color) {
    uint32 basic_shader_id = gl_use_shader("basic");
    GL_Mesh *square_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);

    Vec2 clip_space_position = make_vec2(x * 2.0f - 1.0f,
                                         y * -2.0f + 1.0f);

    Display_Output display_output = render_state->display_output;
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

void gl_draw_constant_facing_quad_view_space(Vec3 view_space_center_position,
                                             real32 world_space_side_length,
                                             int32 texture_id) {
    uint32 basic_shader_id = gl_use_shader("constant_facing_quad");
    GL_Mesh *quad_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);
    gl_use_texture(texture_id);
    
    // these positions are in view space, i.e., same coordinate system as world-space
    real32 quad_vertices[8] = {
        -0.5f, 0.5f, // top left
        0.5f, 0.5f,  // top right
        0.5f, -0.5f, // bottom right
        -0.5f, -0.5f // bottom left
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);

    gl_set_uniform_vec3(basic_shader_id, "view_space_center", &view_space_center_position);
    gl_set_uniform_mat4(basic_shader_id, "perspective_clip_matrix", &render_state->perspective_clip_matrix);
    gl_set_uniform_float(basic_shader_id, "side_length", world_space_side_length);
    //gl_set_uniform_mat4(basic_shader_id, "cpv_matrix", &render_state->cpv_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  int32 texture_id) {
    uint32 basic_shader_id = gl_use_shader("basic2");
    GL_Mesh *quad_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);
    gl_use_texture(texture_id);
    
    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels,                                // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                 // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels, // bottom right
        x_pos_pixels, y_pos_pixels + height_pixels                 // bottom left
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(basic_shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_int(basic_shader_id, "use_color", false);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  Vec4 color, bool32 has_alpha = false) {
    uint32 basic_shader_id = gl_use_shader("basic2");
    GL_Mesh *quad_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);
    
    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels,                                // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                 // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels, // bottom right
        x_pos_pixels, y_pos_pixels + height_pixels                 // bottom left
    };
    //real32 quad_uvs[8];
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(basic_shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(basic_shader_id, "color", &color);
    gl_set_uniform_int(basic_shader_id, "use_color", true);
    gl_set_uniform_int(basic_shader_id, "has_alpha", has_alpha);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  Vec3 color) {
    gl_draw_quad(x_pos_pixels, y_pos_pixels, width_pixels, height_pixels,
                 make_vec4(color, 1.0f));
}

#if 0
void gl_draw_rounded_quad(Vec2 position, Vec2 size,
                          Vec4 color,
                          real32 corner_radius, uint32 corner_flags,
                          Vec4 border_color, real32 border_width, uint32 border_side_flags,
                          bool32 is_alpha_pass = false, bool32 has_alpha = false) {
    uint32 shader_id = gl_use_shader("rounded_quad");
    GL_Mesh *quad_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);
    
    real32 quad_vertices[8] = {
        position.x, position.y + size.y,         // bottom left
        position.x, position.y,                  // top left
        position.x + size.x, position.y,         // top right
        position.x + size.x, position.y + size.y // bottom right
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    gl_set_uniform_vec2(shader_id, "position", &position); 
    gl_set_uniform_vec2(shader_id, "size", &size);
    gl_set_uniform_float(shader_id, "corner_radius", corner_radius);
    gl_set_uniform_uint(shader_id, "corner_flags", corner_flags);
    gl_set_uniform_float(shader_id, "border_width", border_width);
    gl_set_uniform_uint(shader_id, "border_side_flags", border_side_flags);
    gl_set_uniform_vec4(shader_id, "border_color", &border_color);
    gl_set_uniform_int(shader_id, "is_alpha_pass", is_alpha_pass);
    gl_set_uniform_int(shader_id, "has_alpha", has_alpha);
    
    //gl_set_uniform_int(shader_id, "use_color", true);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}
#endif

void gl_draw_hue_slider_quad(real32 x_pos_pixels, real32 y_pos_pixels,
                             real32 width_pixels, real32 height_pixels) {
    uint32 shader_id = gl_use_shader("hue_slider");
    GL_Mesh *quad_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);

    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels + height_pixels,               // bottom left
        x_pos_pixels, y_pos_pixels,                               // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels // bottom right
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_hsv_quad(real32 x_pos_pixels, real32 y_pos_pixels,
                      real32 width_pixels, real32 height_pixels,
                      real32 hue_degrees) {
    uint32 shader_id = gl_use_shader("hsv");
    GL_Mesh *quad_mesh = gl_use_mesh(ENGINE_QUAD_MESH_ID);

    real32 quad_vertices[8] = {
        x_pos_pixels, y_pos_pixels + height_pixels,               // bottom left
        x_pos_pixels, y_pos_pixels,                               // top left
        x_pos_pixels + width_pixels, y_pos_pixels,                // top right
        x_pos_pixels + width_pixels, y_pos_pixels + height_pixels // bottom right
    };
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    gl_set_uniform_mat4(shader_id, "ortho_matrix", &render_state->ortho_clip_matrix);
    gl_set_uniform_float(shader_id, "hue_degrees", hue_degrees);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_circle(real32 center_x, real32 center_y,
                    real32 radius,
                    Vec4 color,
                    bool32 is_filled) {
    // we could instead draw a circle by drawing a quad, then in the fragment shader checking if the fragment
    // is within some radius, but then, we would have to deal with aliasing, i think, and i think this method
    // would be slower as well.

    uint32 shader_id = gl_use_shader("mesh_2d");
    GL_Mesh *circle_mesh = gl_use_mesh(ENGINE_CIRCLE_MESH_ID);

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
void draw_sound_cursor(Win32_Sound_Output *win32_sound_output,
                       real32 cursor_position, Vec3 color) {
    Display_Output display_output = render_state->display_output;
    real32 cursor_width = 10.0f;
    real32 cursor_x = ((cursor_position *
                        display_output.width) - cursor_width / 2.0f);
    real32 cursor_height = 20.0f;
    gl_draw_triangle(cursor_x, display_output.height - 202 - cursor_height,
                     cursor_width, cursor_height,
                     make_vec4(color, 1.0f));

    gl_draw_line(make_vec2(cursor_position * display_output.width, display_output.height - 202.0f),
                 make_vec2(cursor_position * display_output.width, (real32) display_output.height),
                 make_vec4(color, 1.0f));
}

void draw_sound_buffer(Win32_Sound_Output *win32_sound_output) {
    Display_Output display_output = render_state->display_output;
    int32 max_samples = win32_sound_output->buffer_size / win32_sound_output->bytes_per_sample;

    real32 channel_height = 100.0;
    real32 height_offset = channel_height;
    gl_draw_quad(0.0f, display_output.height - height_offset,
                 (real32) display_output.width, channel_height,
                 make_vec3(0.1f, 0.1f, 0.1f));
    gl_draw_line(make_vec2(0.0f, display_output.height - height_offset - 1),
                 make_vec2((real32) display_output.width, display_output.height - height_offset - 1),
                 make_vec4(1.0f, 1.0f, 1.0f, 1.0f));

    height_offset += channel_height + 1;
    gl_draw_quad(0.0f, display_output.height - height_offset,
                 (real32) display_output.width, channel_height,
                 make_vec3(0.1f, 0.1f, 0.1f));
    gl_draw_line(make_vec2(0.0f, display_output.height - height_offset - 1),
                 make_vec2((real32) display_output.width, display_output.height - height_offset - 1),
                 make_vec4(1.0f, 1.0f, 1.0f, 1.0f));

    int32 increment = max_samples / display_output.width;
    for (int32 i = 0; i < max_samples; i += increment) {
        real32 sample_x = (real32) i / max_samples * display_output.width;

        int16 left_sample = win32_sound_output->accumulated_sound_buffer[2*i];
        int16 right_sample = win32_sound_output->accumulated_sound_buffer[2*i + 1];

        real32 sample_height = (real32) left_sample / 32768 * channel_height;
        real32 midline_offset = display_output.height - channel_height / 2.0f;

        gl_draw_line(make_vec2(sample_x, midline_offset),
                     make_vec2(sample_x, midline_offset - sample_height),
                     make_vec4(0.0f, 1.0f, 0.0f, 1.0f));

        
        sample_height = (real32) right_sample / 32768 * channel_height;
        midline_offset -= channel_height + 1;

        gl_draw_line(make_vec2(sample_x, midline_offset),
                     make_vec2(sample_x, midline_offset - sample_height),
                     make_vec4(0.0f, 1.0f, 0.0f, 1.0f));

    }

    real32 play_cursor_position = (real32) win32_sound_output->current_play_cursor / win32_sound_output->buffer_size;
    draw_sound_cursor(win32_sound_output,
                      play_cursor_position, make_vec3(1.0f, 1.0f, 1.0f));
    real32 write_cursor_position = (real32) win32_sound_output->current_write_cursor / win32_sound_output->buffer_size;
    draw_sound_cursor(win32_sound_output,
                      write_cursor_position, make_vec3(1.0f, 0.0f, 0.0f));
}

// NOTE: we don't use this anymore; see ui_render2.cpp. just keeping it for reference because i haven't
//       reimplemented some of the stuff that was here.
void gl_draw_ui_widget(Asset_Manager *asset, UI_Manager *manager, UI_Widget *widget) {
    Vec2 computed_position = widget->computed_position;
    Vec2 computed_size = widget->computed_size;

    #if 0
    GL_Alpha_Mask_Stack *alpha_mask_stack = &gl_state->alpha_mask_stack;
    bool32 has_alpha = alpha_mask_stack->index >= 0;

    if (has_alpha) {
        glBindTexture(GL_TEXTURE_2D, alpha_mask_stack->texture_ids[alpha_mask_stack->index]);
    }
    #endif
    bool32 has_alpha = false;

    if (widget->flags & UI_WIDGET_DRAW_BACKGROUND) {
        Vec4 color = widget->background_color;
        if (widget->flags & UI_WIDGET_IS_CLICKABLE) {
            real32 transition_time = 0.1f;
            
            if (is_hot(widget)) {
                real32 t = min(ui_manager->hot_t / transition_time, 1.0f);
                t = 1.0f-(1.0f-t)*(1.0f-t);
                color = mix(color, widget->hot_background_color, t);
            }

            if (is_active(widget)) {
                real32 t = min(ui_manager->active_t / transition_time, 1.0f);
                t = 1.0f-(1.0f-t)*(1.0f-t);
                color = mix(color, widget->active_background_color, t);
                //color = widget->active_background_color;
            }
            
        }

        // this flag is for both border drawing and corner rounding
        if (widget->flags & UI_WIDGET_DRAW_BORDER) {
            #if 0
            gl_draw_rounded_quad(computed_position, computed_size,
                                 color,
                                 widget->corner_radius, widget->corner_flags,
                                 widget->border_color, widget->border_width, widget->border_flags,
                                 false, has_alpha);

            // draw inner area alpha mask
            gl_push_alpha_mask();
            glDisable(GL_BLEND);
            gl_draw_rounded_quad(computed_position, computed_size,
                                 color,
                                 widget->corner_radius, widget->corner_flags,
                                 widget->border_color, widget->border_width, widget->border_flags,
                                 true, has_alpha);
            glEnable(GL_BLEND);
            
            glBindFramebuffer(GL_FRAMEBUFFER, g_gl_state->msaa_framebuffer.fbo);
            #endif
        } else {
            gl_draw_quad(computed_position.x, computed_position.y, computed_size.x, computed_size.y,
                         color, has_alpha);
        }
    } else {
        Vec4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
        if (widget->flags & UI_WIDGET_DRAW_BORDER) {
            #if 0
            gl_draw_rounded_quad(gl_state, render_state,
                                 computed_position, computed_size,
                                 color,
                                 widget->corner_radius, widget->corner_flags,
                                 widget->border_color, widget->border_width, widget->border_flags,
                                 false, has_alpha);

            // draw inner area alpha mask
            gl_push_alpha_mask(gl_state);
            glDisable(GL_BLEND);
            gl_draw_rounded_quad(gl_state, render_state,
                                 computed_position, computed_size,
                                 color,
                                 widget->corner_radius, widget->corner_flags,
                                 widget->border_color, widget->border_width, widget->border_flags,
                                 true, has_alpha);
            glEnable(GL_BLEND);
            
            glBindFramebuffer(GL_FRAMEBUFFER, gl_state->msaa_framebuffer.fbo);
            #endif
        }
    }
    
    if (widget->flags & UI_WIDGET_DRAW_TEXT) {
        #if 0
        int32 font_id;
        Font *font = get_font(asset, widget->font, &font_id);
        gl_draw_text(gl_state, render_state,
                     font_id, &font,
                     computed_position.x, computed_position.y + computed_size.y - (font.scale_for_pixel_height*font.line_gap),
                     widget->text, widget->text_color);
        #endif
    }
}

void gl_scissor(Vec2_int32 position, Vec2_int32 dimensions) {
    // our dimensions are based from the top left of boxes with 0,0 being top left,
    // but the scissor dimensions are based from the bottom left with 0,0 being bottom left (on opengl).
    int32 display_height = game_state->render_state.display_output.height;
    int32 adjusted_y = (display_height - position.y) - dimensions.y;
    adjusted_y = max(adjusted_y, 0);
    position.y = adjusted_y;

    bool32 same_scissor_position = g_gl_state->scissor_position == position;
    bool32 same_scissor_dimensions = g_gl_state->scissor_dimensions == dimensions;

    if (g_gl_state->scissor_enabled && same_scissor_position && same_scissor_dimensions) {
        return;
    }

    if (!g_gl_state->scissor_enabled) {
        g_gl_state->scissor_enabled = true;
        glEnable(GL_SCISSOR_TEST);
    } else {
        
    }

    glScissor(position.x, position.y, dimensions.x, dimensions.y);

    g_gl_state->scissor_position = position;
    g_gl_state->scissor_dimensions = dimensions;
}

void gl_disable_scissor() {
    if (g_gl_state->scissor_enabled) {
        g_gl_state->scissor_enabled = false;
        glDisable(GL_SCISSOR_TEST);
    }
}

uint32 gl_use_ui_shader(UI_Shader_Type type, UI_Shader_Uniforms uniforms) {
    switch (type) {
        case UI_Shader_Type::HSV: {
            uint32 shader_id = gl_use_shader("hsv");
            gl_set_uniform_float(shader_id, "hue_degrees", uniforms.hsv.degrees);
            return shader_id;
        } break;
        case UI_Shader_Type::HSV_SLIDER: {
            uint32 shader_id = gl_use_shader("hue_slider");
            //gl_set_uniform_float(shader_id, "hue_degrees", uniforms.hsv_slider.hue);
            return shader_id;
        } break;
        case UI_Shader_Type::NONE: {} break;
        default: {
            assert(!"Unhandled UI_Shader_Type!");
        }
    }

    return 0;
}

void gl_draw_ui_commands() {
    UI_Shader_Type current_shader_type = UI_Shader_Type::NONE;
    uint32 shader_id = gl_use_shader("ui");
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->ortho_clip_matrix);

    for (int32 i = 0; i < ui_manager->num_render_commands; i++) {
        UI_Render_Command *current = &ui_manager->render_commands[i];

        if (current->use_scissor) {
            gl_scissor(current->scissor_position, current->scissor_dimensions);
        } else {
            gl_disable_scissor();
        }

        if (current->num_indices == 0) {
            // we can have empty render commands that just set a scissor region
            continue;
        }
        
        if (current->shader_type != UI_Shader_Type::NONE) {
            current_shader_type = current->shader_type;
            shader_id = gl_use_ui_shader(current_shader_type, current->shader_uniforms);
            gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->ortho_clip_matrix);
        } else {
            if (current_shader_type != UI_Shader_Type::NONE) {
                current_shader_type = UI_Shader_Type::NONE;
                shader_id = gl_use_shader("ui");
                gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->ortho_clip_matrix);
            }
        }

        if (current_shader_type == UI_Shader_Type::NONE) {
            // this stuff is only for the default UI shader
            if (current->texture_type == UI_Texture_Type::UI_TEXTURE_IMAGE) {
                gl_set_uniform_bool(shader_id, "use_texture", true);
                gl_set_uniform_bool(shader_id, "is_text", false);
                gl_use_texture(current->texture_id);
            } else if (current->texture_type == UI_Texture_Type::UI_TEXTURE_FONT) {
                gl_set_uniform_bool(shader_id, "use_texture", true);
                gl_set_uniform_bool(shader_id, "is_text", true);
                gl_use_font_texture(current->font_name);
            } else {
                gl_set_uniform_bool(shader_id, "use_texture", false);
            }
        }

        if (current->rendering_mode == UI_Rendering_Mode::TRIANGLE_FAN) {
            glDrawElements(GL_TRIANGLE_FAN, current->num_indices, GL_UNSIGNED_INT,
                           (void *) (current->indices_start * sizeof(uint32)));
        } else if (current->rendering_mode == UI_Rendering_Mode::TRIANGLES) {
            glDrawElements(GL_TRIANGLES, current->num_indices, GL_UNSIGNED_INT,
                           (void *) (current->indices_start * sizeof(uint32)));
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    gl_disable_scissor();
}

void gl_draw_ui() {
    Allocator *temp_region = begin_region();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    UI_Render_Data *render_data = &ui_manager->render_data;
    
    // upload the vertex and index arrays to the GPU
    glBindVertexArray(g_gl_state->ui_data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl_state->ui_data.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    render_data->num_vertices*sizeof(UI_Vertex), render_data->vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_gl_state->ui_data.ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    render_data->num_indices*sizeof(uint32), render_data->indices);
    
    gl_draw_ui_commands();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    end_region(temp_region);
}

#if 0
void gl_draw_ui() {
    // go through ui_manager->render_groups and copy the vertex and index data to two arrays
    // then upload both arrays to GPU (idk if this is actually faster than just uploading each command data directly)
    // while doing this, update the commands with an indices_start member
    // go through the render groups again and this time go through the command lists and draw the indices

    Allocator *temp_region = begin_region();

    int32 total_num_vertices = 0;
    int32 total_num_indices  = 0;
    UI_Vertex *vertices_to_upload = (UI_Vertex *) allocate(temp_region, UI_MAX_VERTICES*sizeof(UI_Vertex));
    uint32    *indices_to_upload = (uint32 *) allocate(temp_region, UI_MAX_INDICES*sizeof(uint32));

    // copy vertex and index data to two arrays for all the commands
    UI_Render_Group *groups = ui_manager->render_groups;
    for (int32 i = 0; i < ui_manager->num_render_groups; i++) {
        UI_Render_Group *group = &groups[i];

        UI_Render_Command *current = group->triangles.first;
        while (current) {
            copy_ui_command_to_buffers(vertices_to_upload, &total_num_vertices,
                                       indices_to_upload, &total_num_indices,
                                       current);
            current = current->next;
        }

        current = group->text_quads.first;
        while (current) {
            copy_ui_command_to_buffers(vertices_to_upload, &total_num_vertices,
                                       indices_to_upload, &total_num_indices,
                                       current);
            current = current->next;
        }
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    uint32 shader_id = gl_use_shader("ui");
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->ortho_clip_matrix);

    // upload the vertex and index arrays to the GPU
    glBindVertexArray(g_gl_state->ui_data.vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl_state->ui_data.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    total_num_vertices*sizeof(UI_Vertex), vertices_to_upload);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_gl_state->ui_data.ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    total_num_indices*sizeof(uint32), indices_to_upload);
    
    for (int32 i = 0; i < ui_manager->num_render_groups; i++) {
        UI_Render_Group *group = &groups[i];
        
        gl_draw_ui_command_list(&group->triangles, shader_id);
        gl_draw_ui_command_list(&group->text_quads, shader_id);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    end_region(temp_region);
}
#endif

#if 0
void gl_draw_ui(Asset_Manager *asset, UI_Manager *manager) {
    #if 0
    glBindFramebuffer(GL_FRAMEBUFFER, gl_state->alpha_mask_framebuffer.fbo);
    uint32 alpha_texture_id = gl_push_alpha_mask(gl_state);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           alpha_texture_id, 0);
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

#if 1
    gl_draw_rounded_quad(gl_state, render_state, { 200.0f, 20.0f }, { 200.0f, 100.0f },
                         { 1.0f, 1.0f, 1.0f, 1.0f },
                         50.0f, CORNER_ALL,
                         { 1.0f, 0.0f, 0.0f, 1.0f }, 5.0f, BORDER_ALL, true);
#endif
    
    glBindFramebuffer(GL_FRAMEBUFFER, gl_state->msaa_framebuffer.fbo);

    
    glBindTexture(GL_TEXTURE_2D, alpha_texture_id);
    #endif
    
    // pre-order traversal
    UI_Widget *current = manager->root;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (current) {
        gl_draw_ui_widget(asset, manager, current);
        
        if (current->first) {
            current = current->first;
        } else {
            if (current->next) {
                current = current->next;
            } else {
                if (!current->parent) {
                    // root should not push an alpha mask
                    assert(!(current->flags & UI_WIDGET_DRAW_BORDER));
                    break;
                }

                // as we go up the tree, checking for next nodes on ancestors, we pop off alpha masks if necessary
                if (current->flags & UI_WIDGET_DRAW_BORDER) {
                    //gl_pop_alpha_mask(gl_state);
                }

                UI_Widget *current_ancestor = current;

                do {
                    current_ancestor = current_ancestor->parent;

                    if (current_ancestor->flags & UI_WIDGET_DRAW_BORDER) {
                        //gl_pop_alpha_mask(gl_state);
                    }
                    
                    if (!current_ancestor->parent) {
                        // root
                        // this will break out of outer loop as well, since root->next is NULL
                        break;
                    }
                } while (!current_ancestor->next);

                current = current_ancestor->next;
            }
        }
    }
}
#endif

void gl_draw_framebuffer(GL_Framebuffer framebuffer) {
    // use premultiplied alpha to prevent dark edges when transitioning from opaque to transparent
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    if (framebuffer.is_multisampled) {
        int32 shader_id = gl_use_shader("multisampled_framebuffer");
        gl_set_uniform_int(shader_id, "num_samples", framebuffer.num_samples);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebuffer.color_buffer_texture);
    } else {
        gl_use_shader("framebuffer");
        glBindTexture(GL_TEXTURE_2D, framebuffer.color_buffer_texture);
    }
    
    GL_Mesh *gl_mesh = gl_use_mesh(ENGINE_FRAMEBUFFER_QUAD_MESH_ID);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_gizmo(Gizmo_State *gizmo_state) {
    Transform_Mode transform_mode = gizmo_state->transform_mode;

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

    gl_draw_solid_color_mesh(ENGINE_GIZMO_ARROW_MESH_ID, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_ARROW_MESH_ID, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_ARROW_MESH_ID, z_handle_color, z_transform);

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

    gl_draw_solid_color_mesh(ENGINE_GIZMO_CUBE_MESH_ID, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_CUBE_MESH_ID, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_CUBE_MESH_ID, z_handle_color, z_transform);

    Transform sphere_mask_transform = gizmo_state->transform;
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_SPHERE_MESH_ID,
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

    gl_draw_solid_color_mesh(ENGINE_GIZMO_RING_MESH_ID, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_RING_MESH_ID, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(ENGINE_GIZMO_RING_MESH_ID, z_handle_color, z_transform);
}

void gl_render_editor(GL_Framebuffer framebuffer,
                      Editor_State *editor_state) {
    Display_Output display_output = render_state->display_output;
    
    Allocator *temp_region = begin_region();

    Level *level = &game_state->level;

    // TODO: will need to use different macro when adding more lights
    Vec3 *light_positions = (Vec3 *) allocate(temp_region, sizeof(Vec3)*MAX_POINT_LIGHTS);
    int32 current_light_index = 0;

    // fill in UBO with point light data
    uint32 padded_point_light_struct_size = sizeof(GL_Point_Light) + 8;
    uint32 ubo_offset = 16; // start at 16 because we set num_point_lights after adding the lights
    uint8 *ubo_buffer = (uint8 *) allocate(temp_region, GLOBAL_UBO_SIZE);

    int32 num_point_lights = 0;
    Entity *current = level->entities;
    while (current) {
        if (current->flags & ENTITY_LIGHT) {
            if (current->light_type == LIGHT_POINT) {
                num_point_lights++;
                assert(num_point_lights <= MAX_POINT_LIGHTS);

                GL_Point_Light gl_point_light = {
                    make_vec4(current->transform.position, 1.0f),
                    make_vec4(current->light_color, 1.0f),
                    current->falloff_start,
                    current->falloff_end
                };
                memcpy(ubo_buffer + ubo_offset, &gl_point_light, sizeof(GL_Point_Light));
                ubo_offset += padded_point_light_struct_size;

                // for point light icon
                light_positions[current_light_index++] = truncate_v4_to_v3(render_state->view_matrix *
                                                                           make_vec4(current->transform.position,
                                                                                     1.0f));
            }
        }

        current = current->next;
    }
    *((int32 *) ubo_buffer) = num_point_lights;

    glBindBuffer(GL_UNIFORM_BUFFER, g_gl_state->global_ubo);
    // only need to send up to ubo_offset bytes (this assumes ubo_offset is right after final byte set)
    glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) 0, ubo_offset, ubo_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // render entities
    Entity *selected_entity = get_selected_entity(editor_state);
    current = level->entities;

    // TODO: maybe just have one loop and do everything there?
    while (current) {
        bool32 has_mesh     = current->flags & ENTITY_MESH;
        bool32 has_material = current->flags & ENTITY_MATERIAL;
        bool32 has_collider = current->flags & ENTITY_COLLIDER;
        
        if (has_mesh && has_material) {
            Material *material = get_material(current->material_id);
            assert(material);
            gl_draw_mesh(current->mesh_id, material, current->transform);
        }

        if (editor_state->show_wireframe &&
            (selected_entity == current) &&
            has_mesh) {
            gl_draw_wireframe(current->mesh_id, current->transform);
        }

        if (editor_state->show_colliders && has_collider) {
            gl_draw_collider(current->collider);
        }
        
        current = current->next;
    }

    // insertion sort on light positions, to render back to front with transparency
    for (int32 i = 1; i < num_point_lights; i++) {
        Vec3 key = light_positions[i];
        int32 j = i - 1;
        for (; j >= 0 && (light_positions[j].z < key.z); j--) {
            light_positions[j + 1] = light_positions[j];
        }
        light_positions[j + 1] = key;
    }

    // draw light icons
    for (int32 i = 0; i < num_point_lights; i++) {
        Vec3 view_space_position = light_positions[i];
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl_draw_constant_facing_quad_view_space(view_space_position, Editor_Constants::point_light_side_length,
                                                ENGINE_LIGHTBULB_TEXTURE_ID);
    }

    // draw gizmo
    glBindFramebuffer(GL_FRAMEBUFFER, g_gl_state->gizmo_framebuffer.fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (editor_state->selected_entity_id >= 0) {
        gl_draw_gizmo(&editor_state->gizmo_state);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    gl_draw_framebuffer(g_gl_state->gizmo_framebuffer);
    
    end_region(temp_region);
}

#if 0
void gl_render_game(GL_State *gl_state, Render_State *render_state, GL_Framebuffer framebuffer,
                    Game_State *game_state) {
    Asset_Manager *asset_manager = &game_state->asset_manager;
    Display_Output display_output = render_state->display_output;
    
    Allocator *temp_region = begin_region();

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

    end_region(temp_region);    
}
#endif

void gl_render(Controller_State *controller_state,
               Win32_Sound_Output *win32_sound_output) {
    Display_Output display_output = render_state->display_output;
    
    Command_Queue *command_queue = &render_state->command_queue;
    for (int32 i = 0; i < command_queue->num_commands; i++) {
        Command *command = &command_queue->commands[i];

        switch (command->type) {
            case Command_Type::LOAD_FONT: {
                Command_Load_Font c = command->load_font;
                Font *font = get_font(c.font_name);
                assert(font->is_baked);
                gl_init_font(font);
            } break;
            case Command_Type::LOAD_MESH: {
                // we use the mesh object here, but not when we unload. when we unload, it's not
                // guaranteed that the mesh object will be in the asset manager and it doesn't
                // matter if it's there. we separate the game object and the gl object in this
                // way so that we don't have to do any second pass things to unload assets
                // from gl state that are deleted in the game. instead, if we want to delete
                // a mesh, we delete it from the asset manager, then add a command to the
                // command queue to unload the mesh from our gl state.
                Command_Load_Mesh c = command->load_mesh;
                Mesh *mesh = get_mesh(c.mesh_id);
                gl_load_mesh(mesh);
            } break;
            case Command_Type::UNLOAD_MESH: {
                Command_Unload_Mesh c = command->unload_mesh;
                gl_unload_mesh(c.mesh_id);
            } break;
            case Command_Type::LOAD_TEXTURE: {
                Command_Load_Texture c = command->load_texture;
                Texture *texture = get_texture(c.texture_id);
                gl_load_texture(texture);
            } break;
            case Command_Type::UNLOAD_TEXTURE: {
                Command_Unload_Texture c = command->unload_texture;
                gl_unload_texture(c.texture_id);
            } break;
            default: {
                assert(!"Unhandled command type.");
            } break;
        }
    }
        
    glBindFramebuffer(GL_FRAMEBUFFER, g_gl_state->msaa_framebuffer.fbo);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(1.0f);
    
    if (game_state->mode == Game_Mode::PLAYING) {
        #if 0
        gl_render_game(gl_state, render_state, gl_state->msaa_framebuffer, game_state);

        glDisable(GL_DEPTH_TEST);
        //gl_draw_ui(gl_state, asset_manager, render_state, &game_state->ui_manager);
        glEnable(GL_DEPTH_TEST);
        #endif
    } else {
        gl_render_editor(g_gl_state->msaa_framebuffer,
                         &game_state->editor_state);

        glDisable(GL_DEPTH_TEST);
        gl_draw_ui();
        glEnable(GL_DEPTH_TEST);
        #if 0
        glDisable(GL_DEPTH_TEST);
        // TODO: for some reason, if we comment out this line, nothing renders at all, other than the gizmos
        //       if we happen to click in an area where there is an entity
        //       - pretty sure it has to do with gl_draw_text(), since if we never call that, then nothing
        //         renders
        //       - don't think this is true anymore
        //gl_draw_ui(gl_state, asset_manager, render_state, &game_state->ui_manager);
        gl_draw_ui(gl_state, render_state, asset_manager, &game_state->ui_manager);
        glEnable(GL_DEPTH_TEST);
        #endif
    }

    // debug lines
    glDisable(GL_DEPTH_TEST);
    glLineWidth(6.0f);
    Debug_State *debug_state = &game_state->debug_state;
    for (int32 i = 0; i < debug_state->num_debug_lines; i++) {
        Debug_Line *line = &debug_state->debug_lines[i];
        gl_draw_line(line->start, line->end, line->color);
    }
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);

    // draw msaa framebuffer
    #if 1
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl_state->msaa_framebuffer.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, display_output.width, display_output.height, 0, 0,
                      display_output.width, display_output.height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    #endif

    #if 0
    glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl_state->alpha_mask_framebuffer.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, display_output.width, display_output.height, 0, 0,
                      display_output.width, display_output.height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    #endif
}
