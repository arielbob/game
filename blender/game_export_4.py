import os
import bpy
import bmesh
from mathutils import Matrix
from bpy.types import PropertyGroup

# to swap the y and z of matrix_local to fit the game's coordinate-space
swap_matrix = Matrix(((1.0, 0.0, 0.0, 0.0),
    (0.0, 0.0, 1.0, 0.0),
    (0.0, 1.0, 0.0, 0.0),
    (0.0, 0.0, 0.0, 1.0)))

bone_to_game = Matrix(((1.0, 0.0, 0.0, 0.0),
    (0.0, 1.0, 0.0, 0.0),
    (0.0, 0.0, -1.0, 0.0),
    (0.0, 0.0, 0.0, 1.0)))

armature_to_game = swap_matrix

class Vertex:
    def __init__(self, co, normal, uv, bone_indices=[], bone_weights=[]):
        self.co = co
        self.normal = normal
        self.uv = uv
        self.bone_indices = bone_indices
        self.bone_weights = bone_weights
    def __repr__(self):
        # honestly have no clue what bone_indices and bone_weights prints here
        return f'Vertex(co={self.co}, normal={self.normal}, uv={self.uv}, bone_indices={self.bone_indices}, bone_weights={self.bone_weights})\n'
    def __eq__(self, other):
        if isinstance(other, Vertex):
            # TODO: pretty sure we don't need to compare bone_indices and bone_weights here?
            return ((self.co == other.co) and (self.normal == other.normal) and (self.uv == other.uv))
        else:
            return False
    def __hash__(self):
        sum = 0
        for e in self.co:
            sum += e
        for e in self.normal:
            sum += e
        for e in self.uv:
            sum += e
        return int(sum)

def apply_modifiers(obj):
    ctx = bpy.context.copy()
    ctx.view_layer.objects.active = obj
    
    for modifier in obj.modifiers:
        bpy.ops.object.modifier_apply(modifier=modifier.name)
#    ctx.object = obj
#    for _, m in enumerate(obj.modifiers):
#        try:
#            ctx['modifier'] = m
#            bpy.ops.object.modifier_apply(ctx, modifier=m.name)
#        except RuntimeError:
#            print(f"Error applying {m.name} to {obj.name}, removing it instead.")
#            obj.modifiers.remove(m)

#    for m in obj.modifiers:
#        obj.modifiers.remove(m)

def get_ordered_bones(skeleton_data):
    bones = []
    # we start by ordering the bones such that parents always come before
    # their children
    bone_stack = []

    # first, find all the bones without parents
    for bone in skeleton_data.bones:
        if bone.parent == None:
            bone_stack.append(bone)
            
    while len(bone_stack):
        current = bone_stack.pop()
        bones.append(current)
        for child in current.children:
            bone_stack.append(child)
            
    return bones

def game_export(context, filename, replace_existing, is_skinned):
    temp_output_file = None
    open_flag = ''
    
    if (replace_existing):
        open_flag = 'w'
    else:
        open_flag = 'x'
    
    try:
        # convert relative path from blender file to absolute path
        filepath = os.path.abspath(bpy.path.abspath('//' + filename))
        temp_output_file = open(filepath, open_flag)
    except FileExistsError:
        show_message_box('File already exists', 'Error', 'ERROR')
        return

    if context.active_object.type != 'MESH':
        show_message_box('Object must be a mesh!', 'Error', 'ERROR')
        temp_output_file.close()
        return

    bpy.ops.object.mode_set(mode='OBJECT')
    mesh_copy = context.active_object.copy()
    mesh_copy.data = mesh_copy.data.copy()
    bpy.context.collection.objects.link(mesh_copy)
    
    ctx = bpy.context.copy()
    ctx["object"] = mesh_copy
    for _, m in enumerate(mesh_copy.modifiers):
        ctx["modifier"] = m
        bpy.ops.object.modifier_apply(ctx, modifier=m.name)        

    #apply_modifiers(mesh_copy)

    # TODO: apply modifiers on the copy, so we don't have to apply them and change what we're exporting

    #bpy.ops.object.make_single_user(object=True, obdata=True, material=True, animation=False)
    
    mesh_copy_data = mesh_copy.data.copy()
    
    # Get a BMesh representation
    bm = bmesh.new()
    bm.from_mesh(mesh_copy_data)
    
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bmesh.ops.dissolve_degenerate(bm, dist=0.000001, edges=bm.edges)

    bm.to_mesh(mesh_copy_data)
    bm.free()
    
    # export skeleton data
    # this goes at the end of the file, but we need the data before we append it 
    skeleton_data_string = ''

    bones = []
    bone_names = []
    
    if is_skinned:
        skeleton_data = mesh_copy.parent.data
        
        if mesh_copy.location != mesh_copy.parent.location:
            show_message_box('Mesh origin and armature origin must match!', 'Error', 'ERROR')
            temp_output_file.close()
            return
        
        bones = get_ordered_bones(skeleton_data)
        bone_names = [bone.name for bone in bones]
        
        skeleton_data_string += 'skeleton {\n'
        skeleton_data_string += 'num_bones {:d}\n'.format(len(bones))
        
        for bone in bones:
            skeleton_data_string += '\nbone "{:s}" {{\n'.format(bone.name)
            skeleton_data_string += 'inverse_bind '
            
            # TODO: for some reason, matrix_local starts out with swapped y and z rows - DONE
            # - matrix_local goes from bone space to model space
            # - what the default matrix is saying that, to get to model space, make z negative,
            #   then swap the y and z-coordinates
            # - what this is saying is that in bone-space, y is up, and +z is out of the screen.
            # - in model space (blender space, i guess) z is up, and -y is out of the screen.
            # - so if you have, let's say, z=1 in bone-space, that point in model-space would be
            #   y=-1.
            # - in our engine, x is to the right, y is up, and +z is into the screen. so, we just
            #   take the matrix_local and swap 2nd and 3rd rows. the 1.0 for the z is already
            #   negative, so we don't need to do anything there.
            # - easier way of thinking about it is just that matrix_local goes from bone_space to
            #   blender's model-space, then to convert to game's model space, just swap y and
            #   z-axes.
            # - to go from bone-space to game-space, just negate z
            
            # so, this model_to_bone converts from game's model space to blender's bone space!!!
            # so, if we're doing transforms in bone-space, it's expected that we're transforming
            # in that weird swapped coordinate-system (i.e. in blender's bone-space).
            
            # armature-space -> game model-space
            swapped = swap_matrix @ bone.matrix_local
            model_to_bone = swapped.inverted()
            
            # don't need last row, since it's expected to just always have the homogenous 1
            for i in range(3):
                skeleton_data_string += '{:.5f} {:.5f} {:.5f} {:.5f}\n'.format(*model_to_bone[i]) # * is spread
                
            # append parent
            if bone.parent:
                skeleton_data_string += 'parent {:d}\n'.format(bone_names.index(bone.parent.name))
            
            skeleton_data_string += '}\n'
            # YOU NEED TO SAVE THE FILE BEFORE TRYING THINGS IN THE CONSOLE, OR ELSE WHAT YOU GET
            # BACK FROM THE THINGS YOU CALL WILL BE OUTDATED!!!!!
            
            # TODO: figure out how to get local transforms (though this isn't really necessary
            #       for this part - they're necessary only for animation exporting, not model
            #       exporting, but i still would like to figure this out.. because it's weird)
            
            # should we expect the bone origin to be the same as the object origin?
            # what are the implications if we just find the difference between them and apply
            # that offset to the bone matrices and the transforms?
            # it's much simpler if we just expect the animator to set the armature origin to
            # be the same as the mesh's origin.
            # i would assume that it's easier to work that way anyways..
            # but, it really isn't an issue if we do calculate the offset and apply it. in the
            # case they're the same, it'll just be 0 and nothing will happen.
            # but in the case that the origins are different, it will be a lot nicer, since you
            # won't have to debug why your animation is fucked up.
            # i really don't know the implications of doing that though. i'm just not gonna
            # bother, and maybe just show a message that the origins don't match, but they
            # probably should.
            
        skeleton_data_string += '}\n'        
        
    
    # we don't need to deal with the same vertex having different bone indices or weights,
    # so just loop through the object's vertices and get their bone stuff
    
    # indexed by same index into object.data.vertices (i.e. non-duplicated vertices)
    vertex_bone_data = []
    if is_skinned:
        # get object's skeleton
        if mesh_copy.parent == None or mesh_copy.parent.type != 'ARMATURE':
            show_message_box('Object must have an armature object as its parent', 'Error', 'ERROR')
            temp_output_file.close()
            return
        
        skeleton_data = mesh_copy.parent.data
        
        # i think we actually want to export all the bones, even if they aren't associated with any weights.
        # so, go through all the bone names and append the names to an array.
        # then, for each vertex, go through all of their groups and try and find a corresponding bone.
        # if it exists, then we append the bone index and weight.
        # note that we export the entire skeleton, even if a bone doesn't have any vertex weights associated
        # with it.
        # we don't query for mesh_copy.vertex_groups[bone_name] because when a bone isn't associated with any
        # vertex, then there is never a vertex_group assigned to that mesh for that bone.

        # TODO: when we export animations, all bones in a skeleton need to have at least one frame.
        #       i don't think it necessarily needs to be the first frame?
        #       they all need at least one because if a bone has a parent, but that parent doesn't have any frames,
        #       then the child will not know how to go to model-space. since the child's transform goes from bone
        #       to parent-space, and the parent's computed transform goes from parent to world space.

        for v in mesh_copy_data.vertices:
            bone_indices = []
            bone_weights = []
            for group in v.groups:
                # group.group is the index into the object's vertex groups list
                # we need this because the group on the vertex object doesn't have the name.. we have to
                # get the group from the mesh's vertex groups.
                global_group_index = group.group
                group_name = mesh_copy.vertex_groups[global_group_index].name
                
                if group_name in bone_names:
                    bone_weight = group.weight
                    if bone_weight > 0.001:
                        bone_index = bone_names.index(group_name)
                        bone_indices.append(bone_index)
                        bone_weights.append(bone_weight)    

            vertex_bone_data.append((bone_indices, bone_weights))
    
    #vertices_list = []
    faces_list = []
    
    vertex_indices_by_vertex = {}
    
    # TODO: can we just have a dict that maps vertex -> vertex id?
    #       that way we can query if the vertex exists AND get the vertex id (based on how many we've added so far)
    
    # we loop through the faces and generate the unique vertices based on position, normal, and UV
    # for each face. faces can share vertices, but if they don't share both UV and normal and only
    # share position, we create a new vertex.
    
    num_unique_vertices = 0
    for face_index, face in enumerate(mesh_copy_data.polygons):
        vertex_positions = [mesh_copy_data.vertices[index].co.copy() for index in face.vertices]
        vertex_normals = []
        if face.use_smooth:
            vertex_normals = [mesh_copy_data.vertices[index].normal.copy() for index in face.vertices]
        else:
            vertex_normals = [face.normal, face.normal, face.normal]
            
        vertex_uvs = []
        for i in range(3):
            uv = mesh_copy_data.uv_layers.active.data[face_index*3 + i].uv.copy()
            vertex_uvs.append(uv)
            
        vertex_bone_indices = []
        vertex_bone_weights = []
        
        if is_skinned:
            for index in face.vertices:
                vertex_bone_indices.append(vertex_bone_data[index][0]);
                vertex_bone_weights.append(vertex_bone_data[index][1]);
            
        new_face_indices = []
        for i in range(3):
            vertex_to_add = None
            if is_skinned:
                vertex_to_add = Vertex(vertex_positions[i], vertex_normals[i], vertex_uvs[i], vertex_bone_indices[i], vertex_bone_weights[i])
            else:
                vertex_to_add = Vertex(vertex_positions[i], vertex_normals[i], vertex_uvs[i])

            # TODO: this is slow, but is very simple
            #       we could add them to a set instead, but then we would have to set the face vertex
            #       indices later, which would mean we would have to loop through for every vertex in
            #       the face to find its index, and we would also need to store the vertex data for each
            #       face, so we can find the index again later.
            
            vertex_index = vertex_indices_by_vertex.get(vertex_to_add)
            if vertex_index == None:
                # note that these indices refer to the new indices in the final vertex list we export
                # which includes the "duplicated" ones. this is used for the indices data in the file.
                vertex_indices_by_vertex[vertex_to_add] = num_unique_vertices
                vertex_index = num_unique_vertices
                num_unique_vertices += 1
            
            new_face_indices.append(vertex_index)
            
        faces_list.append(new_face_indices)

        vertices_list = list(vertex_indices_by_vertex.items())
    print(vertices_list)
    vertices_list = [entry[0] for entry in sorted(vertices_list, key=lambda x: x[1])]
    print(vertices_list)

    temp_output_file.write('is_skinned {:d}\n\n'.format(is_skinned))
    temp_output_file.write(str(len(vertices_list)) + '\n')
    temp_output_file.write(str(len(faces_list)) + '\n\n')
    
    temp_output_file.write(';; vertices\n\n')
    
    for i in range(len(vertices_list)):
        vert = vertices_list[i]
        
        x, y, z = vert.co
        out_x, out_y, out_z = x, z, y
        temp_output_file.write('v {:.5f} {:.5f} {:.5f}\n'.format(out_x, out_y, out_z))
        
        x, y, z = vert.normal
        out_x, out_y, out_z = x, z, y
        temp_output_file.write('n {:.5f} {:.5f} {:.5f}\n'.format(out_x, out_y, out_z))
        
        u, v = vert.uv
        temp_output_file.write('uv {:.5f} {:.5f}\n'.format(u, v))
        
        if is_skinned:
            if len(vert.bone_indices):
                if len(vert.bone_indices) > 4:
                    show_message_box('Each vertex can only have a maximum of 4 bones influencing it', 'Error', 'ERROR')
                    temp_output_file.close()
                    return
                
                temp_output_file.write('bi')
                num_bone_indices = 0
                for bone_index in vert.bone_indices:
                    temp_output_file.write(' {:d}'.format(bone_index))
                    num_bone_indices += 1
                    
                # just fill it out with bone indices that will later be weighted 0 because
                # we don't support variable amounts yet in the file format
                while num_bone_indices < 4:
                    temp_output_file.write(' 0')
                    num_bone_indices += 1
                
                temp_output_file.write('\nbw')
                num_bone_weights = 0
                for bone_weight in vert.bone_weights:
                    temp_output_file.write(' {:f}'.format(bone_weight))
                    num_bone_weights += 1
                
                while num_bone_weights < 4:
                    temp_output_file.write(' 0')
                    num_bone_weights += 1
                    
                temp_output_file.write('\n')
        
        temp_output_file.write('\n')
    
    temp_output_file.write(';; indices\n\n')
    
    # NOTE: when store indices in a manner such that if we're looking at the front of a face, we can
    #       take the cross product of the lines made by its indices in the order that they exist in the data
    #       to get a normal that points towards us.
    #       it's just nice to know when taking the cross product, what the output will be, and this just
    #       helps make that convention known.
    #       the indices are stored in blender counter-clockwise when looking at a face's front, so we just
    #       reverse it to make it so the cross product's result is pointing out from the front side of the face.
    # NOTE: actually, i don't think blender enforces an exact winding order at all.
    #       you have to make sure that the vertices are in the correct order based on the face's normal.
    for face in faces_list:
        index_1, index_2, index_3 = face
        temp_output_file.write('{} {} {}\n'.format(index_3, index_2, index_1))    
    
    # TODO: store a mapping of object names -> file names so that we don't have to keep changing the name
    #       of the output file inside the input box
    
    if is_skinned:
        temp_output_file.write('\n')
        temp_output_file.write(skeleton_data_string)
    
    print('closing file and deleting copy of mesh\n')
    
    temp_output_file.close()
    # delete the temporary mesh
    bpy.data.objects.remove(mesh_copy)
    
#    print(vertices_list)
#    print(faces_list)
    
    show_message_box('Model exported succcessfully', 'Success', 'CHECKMARK')

def anim_export(context, filename, replace_existing):
    temp_output_file = None
    open_flag = ''
    
    if (replace_existing):
        open_flag = 'w'
    else:
        open_flag = 'x'
    
    try:
        # convert relative path from blender file to absolute path
        filepath = os.path.abspath(bpy.path.abspath('//' + filename))
        temp_output_file = open(filepath, open_flag)
    except FileExistsError:
        show_message_box('File already exists', 'Error', 'ERROR')
        return

    bpy.ops.object.mode_set(mode='OBJECT')
    active_object = context.active_object
    
    armature_object = active_object.parent
    assert(armature_object.type == 'ARMATURE')
    
    keyframe_positions_by_bone = {}

    # NOTE: we might eventually have interpolations other than linear, but for now we
    #       just linearly interpolate between an animation's keyframes
    
    # TODO: export keyframe positions - DONE
    # TODO: create transform samples from those positions
    #       - https://blender.stackexchange.com/questions/8387/how-to-get-keyframe-data-from-python
    #       - just do that from the link
    #       - note that that link just samples every frame, instead of only
    #         the frames that have keyframes (we want to do the latter)
    
    # TODO: verify that frame_start has a keyframe
    # TODO: export from frame_start to frame_end only
    #       - only put in keyframes that are inside the range
    #       - need to create key frame and frame_end if it doesn't exist
    #       - since you can have keyframes outside of the bounds
    
    scene = bpy.context.scene
    
    frame_start = scene.frame_start
    frame_end = scene.frame_end # inclusive
    
    for fcurve in armature_object.animation_data.action.fcurves:
        bone_name = None
        data_path = fcurve.data_path
        
        assert(data_path.find('[') > -1)
        assert(data_path.find(']') > -1)
        
        # we need to do this because bones can have [] in them
        open_square_bracket_index = data_path.find('[')
        close_square_bracket_index = len(data_path) - 1 - data_path[::-1].find(']')
        
        # skip the quotes
        name_start = open_square_bracket_index + 2 # inclusive
        name_end = close_square_bracket_index - 1 # exclusive
        
        bone_name = data_path[name_start:name_end]
        
        for keyframe in fcurve.keyframe_points:
            keyframe_pos = int(keyframe.co.x)
            if keyframe_pos < frame_start or keyframe_pos > frame_end:
                continue
            
            if bone_name in keyframe_positions_by_bone:
                if keyframe_pos not in keyframe_positions_by_bone[bone_name]:
                    keyframe_positions_by_bone[bone_name].append(keyframe_pos)
            else:
                keyframe_positions_by_bone[bone_name] = [keyframe_pos]
                
        # we really don't need any animation about the transform at the keyframe.
        # we just need to know where keyframes exist for any property of the bone.
        #       print(key.co) # prints the position on the timeline
    
    # PoseBones are basically the same as regular Bones
    # they have all the properties that we use on regular Bones, so we can use
    # this function interchangeably.
    pose_bones = get_ordered_bones(armature_object.pose)
    # we need some data that we can only get from regular Bones, such as use_inherit_rotation
    normal_bones = get_ordered_bones(armature_object.data)
    
    samples_str = ''
    
    # generate keyframe points for frame_start/frame_end if they don't exist.
    # also, generate keyframe points for bones that don't inherit rotation.
    for idx, pose_bone in enumerate(pose_bones):
        use_inherit_rotation = normal_bones[idx].use_inherit_rotation
        
        if pose_bone.name not in keyframe_positions_by_bone:
            if not use_inherit_rotation and pose_bone.parent:
                # need to generate keyframes based on parent keyframes
                parent_keyframe_positions = keyframe_positions_by_bone[pose_bone.parent.name]
                for parent_keyframe_pos in parent_keyframe_positions:
                    keyframe_positions_by_bone[pose_bone.name] = parent_keyframe_positions.copy()
            else:
                keyframe_positions_by_bone[pose_bone.name] = [frame_start]
        else:
            keyframe_positions = keyframe_positions_by_bone[pose_bone.name]
            assert(len(keyframe_positions))

            if not use_inherit_rotation and pose_bone.parent:
                # generate keyframes based on parent keyframes if there isn't already
                # a keyframe at that position
                parent_keyframe_positions = keyframe_positions_by_bone[pose_bone.parent.name]
                for parent_keyframe_pos in parent_keyframe_positions:
                    if parent_keyframe_pos not in keyframe_positions_by_bone[pose_bone.name]:
                        keyframe_positions.append(parent_keyframe_pos)
                
        keyframe_positions = keyframe_positions_by_bone[pose_bone.name]        
        assert(len(keyframe_positions))
        keyframe_positions.sort()
                
        # verify that we have keyframes at frame_start and frame_end
        if keyframe_positions[0] != frame_start:
            # TODO: this might be slow; may look into using deque, or just do
            #       this in the initial generation of these keyframes
            keyframe_positions.insert(0, frame_start)
        if keyframe_positions[-1] != frame_end:
            keyframe_positions.append(frame_end)
    
    # TODO: generate the samples
    old_current_frame = scene.frame_current

    # to swap rows to convert armature coordinate-space to bone coordinate-space
    armature_to_bone_swap = Matrix(((1.0, 0.0, 0.0, 0.0),
                                    (0.0, 0.0, 1.0, 0.0),
                                    (0.0, -1.0, 0.0, 0.0),
                                    (0.0, 0.0, 0.0, 1.0)))

    # bone->parent transforms (still in bone-space coordinate-space)
    keyframe_transforms_by_bone = {}
    for pose_bone in pose_bones:
        assert(pose_bone.rotation_mode == 'QUATERNION')
        
        transforms = []
        keyframe_positions = keyframe_positions_by_bone[pose_bone.name]
        for keyframe_pos in keyframe_positions:
            scene.frame_set(keyframe_pos)
            
            # this is in bone-space, but relative to the parent
            bone_to_parent_matrix = None
            if pose_bone.parent:
                # armature->parent * bone->armature
                # so, it's in parent-space (still using bone-space coordinate-space (the weird one))
                
                bone_to_parent_matrix = pose_bone.parent.matrix.inverted() @ pose_bone.matrix
            else:
                # no need for multiply when bone has parent because in that case, the multiply we do
                # makes it end up in bone-space
                bone_to_parent_matrix = armature_to_bone_swap @ pose_bone.matrix
            
            '''
            - you could get the local transform by multiplying by inverse of the rest pose matrix
            - bpy.context.active_object.data.bones[0].matrix_local.inverted() @ bpy.context.active_object.pose.bones[0].matrix
            - so we have the bone local transform
            - but we don't actually want the bone local transform.. we still need the offset!
            - it feels like we just want to do what we were doing before
              - that works for the bones with a parent
              - but what if we don't have a parent?
              - like what is the transform and in what coordinate-space is it happening in?
              
            - is our swap matrix wrong? swap_matrix assumes we're going from blender->game
              - it's not the same as bone->blender->game
            
            - TODO: try the new bone_to_game matrix instead of swap_matrix above
              - are the transforms from the decomposed matrix still weird?
              - yes, i'm pretty sure they will still be weird because you're changing the handedness of the coordinate-space
              
            - TODO: just figure out the offset the root bone is from its rest pose - NEVERMIND, see below
            
            - the root is just armature_to_bone @ bone.matrix
              - no need to offset, since bone.matrix is just bone->armature
              - armature_to_bone is just a swap matrix to make it so we don't swap axes or invert them
                - this is just so the transforms we get when we decompose actually make sense
                - i'm not sure if there's a way to do a swap, then decompose, and have the transforms actually work,
                  so just gonna do this
            - for children, you just do pose_bone.parent.matrix.inverted() @ pose_bone.matrix
            
            - if we wanted to be able to have everything in the game's coordinate-space, then we would first need to
              change the inverse_bind matrices, such that the bone-space is assumed to be in the game's coordinate-space
              - but, we would still need to do the weird math to deal with decomposing a transform matrix
                that has swapped rows (something about negative determinants)

            - TODO: export the parent swap matrix
            '''
            
            position, rotation, scale = bone_to_parent_matrix.decompose()
            transforms.append({
                'frame_num': keyframe_pos,
                'position': position,
                'rotation': rotation,
                'scale': scale
            })

        keyframe_transforms_by_bone[pose_bone.name] = transforms

    # TODO: actually output the text!    
    # TODO: remember to output the initial parent->model matrix (swap_matrix) - DONE
    # - just put this in animation_info probably
    
    # animation_info
    temp_output_file.write('animation_info {\n')
    # offset frame_end to start at zero
    offset_frame_end = frame_end - frame_start
    
    temp_output_file.write('frame_end {:d}\n'.format(offset_frame_end))
    temp_output_file.write('fps {:d}\n'.format(scene.render.fps))
    temp_output_file.write('num_bones {:d}\n'.format(len(normal_bones)))
    
    # the initial matrix to convert points in bone-space to model-space
    temp_output_file.write('bone_to_model ')
    for i in range(4):
        temp_output_file.write('{:.5f} {:.5f} {:.5f} {:.5f}\n'.format(*bone_to_game[i])) # * is spread

    temp_output_file.write('}\n\n')

    temp_output_file.write('bones {')
    for bone in normal_bones:
        samples = keyframe_transforms_by_bone[bone.name]
        num_samples = len(samples)
        
        temp_output_file.write('\nbone "{:s}" {{\n'.format(bone.name))
        temp_output_file.write('num_samples {:d}\n'.format(num_samples))
        temp_output_file.write('samples {')
        
        # output samples
        for sample in samples:
            # frame numbers are zero-indexed in the file-format, but 1-indexed when
            # we're working with them above
            temp_output_file.write('\n{:d} {{\n'.format(sample['frame_num'] - 1))
            
            temp_output_file.write('pos {:f} {:f} {:f}\n'.format(sample['position'].x, sample['position'].y, sample['position'].z))
            temp_output_file.write('rot {:f} {:f} {:f} {:f}\n'.format(sample['rotation'].w, sample['rotation'].x, sample['rotation'].y, sample['rotation'].z))
            temp_output_file.write('scale {:f} {:f} {:f}\n'.format(sample['scale'].x, sample['scale'].y, sample['scale'].z))
            
            temp_output_file.write('}\n')

        # samples end brace        
        temp_output_file.write('}\n')
        # bone end brace
        temp_output_file.write('}\n')

    # bones end brace        
    temp_output_file.write('}\n')

    print(keyframe_positions_by_bone)
    print(keyframe_transforms_by_bone)
    
    temp_output_file.close()    
    show_message_box('Animation exported succcessfully', 'Success', 'CHECKMARK')  
            
            
            
            
            
            # output the transform
            # this can have parent shit though can't it???
            # actually, it's fine, i think? because the parent transform will already
            # be applied to it
            # oh yeah, if the parent has a transform and it's applied at some keyframe,
            # we will automatically generate a keyframe for the child bone?
            # that's not actually the case...
            # yeah, it's weird.. we just should NOT use the "inherit rotation" checkbox
            # under the relations options for a bone
            # actually, it's somewhat unreasonable to do that, i think...
            
            # does our C++ code automatically inherit rotations? i don't think so.
            # i think it does actually... yeah, fuck, it does automatically inherit.
            
            # so, let's say it does automatically inherit..
            # the matrix of the child bone is affected, but not the transform
            # i'm thinking that we should get the transforms from the matrix and not
            # from the transform.
            
            # IMPORTANT:
            # the bone matrix is affected by whether or not "inherit rotation" is
            # enabled.
            # when it's enabled, it does what we expect. so blender's parent matrix
            # multiplication is the same as ours, i.e. blender also by default, applies
            # parent rotations to its children.
            
            # when you disable it, the child bone's matrix is changed to compensate.
            # since the multiplication that blender does automatically applies the 
            # parent's rotation.
            
            # so BASICALLY, we really don't need to care about whether "inherit rotation"
            # is enabled or disabled.
            
            # there is still the issue of, do we need to create keyframes for these
            # bones that don't inherit rotation? yes we do, because then the parent will
            # be rotating or whatever and the child will be rotating along with it when
            # it shouldn't because it's supposed to be ignoring its parent's rotation.
            
            # basically if we're not inheriting rotation, the child node should always
            # have a keyframe wherever the parent has a keyframe. i'm pretty sure that
            # should work and we wouldn't have to sample every single frame.
            # you basically just have to process the bones in the parent to child order,
            # which is the order we get from get_ordered_bones()
            
            # something we might want to do is creating a linear curve from a curve.
            # i think blender might already have a feature like that though.
            # - yeah, you can do this manually in blender. you just go to the graph
            #   editor, then Key > Density > Sample Keyframes, then in the same menu,
            #   do Clean Keyframes, and you can specifiy a threshold.
            #   what it's basically doing, i think, is, actually, i have no clue.
        
        #for pos in keyframe_positions:
            # TODO: write the keyframe stuff to the string
            # - if we aren't inheriting rotation, create keyframes for every parent keyframe
            # - actually, this can probably end up in some weird results.. ACTUALLY, i think
            #   it's fine, since if the parent is being linearly interpolated, so will the
            #   child. it's fine. just do it.
            

class GameExportOperator(bpy.types.Operator):
    """Tooltip"""
    bl_idname = "object.game_export"
    bl_label = "Export Mesh"

    filename: bpy.props.StringProperty(name='filename')

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        props = context.scene.game_export_props
        print('filename: ' + props.filename)
        game_export(context, props.filename, props.replace_existing, props.is_skinned)
        return {'FINISHED'}

class AnimExportOperator(bpy.types.Operator):
    """Tooltip"""
    bl_idname = "object.anim_export"
    bl_label = "Export Animation"

    #filename: bpy.props.StringProperty(name='anim_filename')

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        props = context.scene.game_export_props
        print('anim_filename: ' + props.anim_filename)
        anim_export(context, props.anim_filename, props.anim_replace_existing)
        return {'FINISHED'}

def filename_update_func(self, context):
    object_to_filename[context.active_object] = self.filename
#    print(context.active_object)
#    print("filename updated!", self.filename)
    
class GameExportPropertyGroup(PropertyGroup):
    filename: bpy.props.StringProperty(name="Filename", update=filename_update_func)
    replace_existing: bpy.props.BoolProperty(name="Replace Existing")
    is_skinned: bpy.props.BoolProperty(name="Enable Skinning")
    anim_filename: bpy.props.StringProperty(name="Animation filename")
    anim_replace_existing: bpy.props.BoolProperty(name="Replace Existing Animation")
#    actions: bpy.props.EnumProperty(
#        name="",
#        description="select something",
#        items=[('op1', "cube", "")]
#    )

def show_message_box(message, title, icon='INFO'):
    def draw(self, context):
        self.layout.label(text=message)

    bpy.context.window_manager.popup_menu(draw, title=title, icon=icon)

class GameExportPanel(bpy.types.Panel):
    """Creates a Panel in the Object properties window"""
    bl_label = "Game Export"
    bl_idname = "OBJECT_PT_game_export"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        layout = self.layout

        row = layout.row()
        obj = context.object
        row.label(text="Active object is: " + obj.name)

        props = context.scene.game_export_props
        row = layout.row()
       
#        if context.object not in object_to_filename:
#            object_to_filename[context.object] = ""
#        filename = str(object_to_filename[context.object])
#        props.filename = filename
       
        row.prop(obj, 'filename_string', text="File name")
        row = layout.row()
        row.prop(props, "replace_existing", text="Replace Existing")
        row = layout.row()
        row.prop(props, "is_skinned", text="Skinned")

        row = layout.row()
        row.operator("object.game_export")
        
        action_name = None
        if obj.parent:
            if obj.parent.type == 'ARMATURE' and obj.parent.animation_data.action:
                action_name = obj.parent.animation_data.action.name
        row = layout.row()
        row.label(text="Current action: " + (action_name if action_name else 'None' ))
        
        if action_name:
            row = layout.row()
#            if obj not in object_to_anim_filename:
#                object_to_anim_filename[obj] = ""
                
#            props.anim_filename = object_to_anim_filename[obj]
            row.prop(props, "anim_filename", text="File name")
            
            row = layout.row()
            row.prop(props, "anim_replace_existing", text="Replace Existing")
            
            row = layout.row()
            row.operator("object.anim_export")
        
        # TODO: you need to put the action on the armature, not the mesh - DONE
        # TODO: button to export action - DONE
        
        # bone.matrix converts from bone-space to armature space
        # - i think armature space is actually same coordinate-system as blender world-space
        #
        # bpy.context.active_object.pose.bones[0].matrix.inverted() @ bpy.context.active_object.pose.bones[0].matrix
        # = pose.parent_bone.matrix.inverted() * pose.bone.matrix
        # = the bone's local transform matrix
        # 
        # - should we extract the transforms here? or should we like somehow use the transforms
        #   that are listed in the "N" menu?
        # - does it matter? i think it might matter, honestly..
        # - actually, i don't think it does. the translation, rotation, scale are like
        #   independent from each other in the matrix, aren't they?
        # - the sign ambiguity of the rotation (since we're using quaternions, -q = q), does
        #   not matter, since we always just take shortest path when we slerp
        # - the thing you can't do is interpolate the matrices themselves; you must extract
        #   the components of the transform out of the matrix and interpolate those
        #
        # - so we have the bone-local transform (i.e. relative to the parent)
        # - this is enough data to export. we just have to go through all the frames now
        # TODO: try extracting the stuff, and see if it's in bone-space
        # - bone.matrix is in armature-space, not bone-space. we need to do the parent inverse
        #   multiplication to get it in bone-space
        # NOTE: when we 
        # bone.matrix is bone->armature
        # so transform extracted from decomposed bone.matrix is in armature-space.
        # so we MUST multiply by inverse(bone.matrix) to go from armature->bone.
        # and now the transforms will work properly when we're in bone-space.
        #
        # what do we multiply the root bone by?
        # just do swap_matrix @ bone.matrix, and that'll convert it from 
        # this is confusing. my C++ code for getting the matrices kind of assumes that
        # the bone and model-space coordinate systems are the same, but they're not in
        # blender. but should that even matter?
        #
        # let's start at the root bone.
        # - parent_to_model = identity
        # - no parent
        # - get_model_matrix(interpolated_transform);
        #   - this is matrix to pose the bone, but it's still in bone-space
        # - parent_to_model * get_model_matrix(interpolated_transform);
        #   - this is wrong, because parent_to_model in this case does not transform to
        #     model-space. it's still just a posed bone, but still in bone-space.
        # - the interpolated_transform should really transform it from bone-space to
        #   armature-space. so, does the local bone transform, but then converts it to
        #   armature-space's coordinate system.
        # - we want the transform that goes from bone-space to armature-space.
        #   - so, we assume that parent-space is in armature-space
        #
        # okay, so the bone.matrix is in armature-space.
        # if we decompose it, the transforms are in armature-space's coordinate-space, i.e.
        # blender's normal coordinate-space.
        # the model->bone (inverse_bind) matrix converts points to bone-space, though.
        # so, first, it's expected that the local transform transforms to armature-space
        #
        # rel_to_parent = bpy.context.active_object.pose.bones[0].matrix.inverted() @ bpy.context.active_object.pose.bones[1].matrix
        # = pose.parent.matrix.inverted() * pose.bone.matrix()
        # parent armature->bone * bone->armature * bone_p
        # so, if you imagine a point bone_p here in bone_space (we get from multiplying our
        # model-space point by inverse_bind), multiplying by bone->armature converts it to
        # armature-space.
        # - at this point, it's posed and in model-space (good coordinate-space), but the
        #   problem is that it's not relative to the parent.
        # - we want it relative to the parent, but still in good coordinate-space.
        # - imagine the transforms in your head...
        # - if you're in armature-space, and you multiply by parent armature->bone, now
        #   your point is just relative to the parent bone. which is what we want, is it not?
        # - now that we're relative to the parent, we just need to multiply by swap_matrix
        #   - and then, just extract?
        # - i think we're good?
        #
        # so final equation is just swap_matrix * parent.matrix.inverted() * bone.matrix.
        # then, decompose it, and the pos, rot, scale are the correct parent-relative
        # transforms that go from bone-space to parent-space.
        # it makes sense, i think.
        # imagine just like a vertical stack of two bones.
        # we have a point in the top bone's space. bone.matrix * p = point in armature-space.
        # multiply by parent.matrix.inverted(), now that's relative to parent.
        # finally, multiply by swap, and you have the proper matrix that you can extract the
        # parent-relative transform out of.
        #
        # to remind us what we're doing: we're trying to figure out the transform that
        # transforms points in bone-local space to the pose, except with the new point being
        # in that bone's parent space, but also using the game's coordinate-system.
        # - an alternative is to just have us pass a special matrix and use that as the
        #   initial parent_to_model matrix, so then we automatically convert from bone-space
        #   to the game's coordinate system..
        # - i actually think this might be better... maybe just do this
        
        # TODO: test this method out on some test armature - DONE, seems like it works?
        # - why is rotation not the same?
        # - scale is the same, but all negatives..
        # - ACTUALLY, this is expected!!! because it's converting from bone-space to
        #   armature-space!!! the weird rotation and scales are expected
        # - this is kind of complicated, but i think any alternative would just have us
        #   doing these same conversions elsewhere..

        # TODO: figure out how to go through all the frames
        # active_object.animation_data.action.fcurves[1].keyframe_points[0].co
        # for fc in fcurves:
        #   for key in keyframe_points:
        #       print(key.co) # prints the position on the timeline
        #
        # you could literally just find the positions of all the keyframes, then loop
        # through them from start to finish, and just create samples for pos, rot, scale.
        # (it would be every part of the transform, since our format expects that for now)
        # - you do need to categorize the keyframes by bone, then loop through each bone
        #   and then loop for each keyframe within each bone
        # - for an fcurve, you can do fcurve.data_path, which shows you which object it's
        #   operating on. you can see if it's a location or rotation or scale, but i don't
        #   know how to see the specific axis. but we don't really need that since we're
        #   sampling everything anyways.
        # - anyways, you do need to do some special data_path parsing, i think, to extract
        #   the bone name
        #   - right, just split on ", then get whatever's at index 1

        # TODO: export the data for each frame for every bone
        # TODO: use the special initial parent_to_model matrix thing, so that we can keep
        #       the weird transforming stuff out of this code..
        #       - i think it makes more sense, anyways, since parent-space is still technically
        #         relative to a bone, and i think should use the bone's local coordinate-system.
        #       - and the naming of parent_to_model in the C++ exporting code is still valid,
        #         because the initial matrix will convert from parent to model, then every thing
        #         after will eventually be multiplied by that.

#def unregister_game_export_panel():
#    bpy.utils.unregister_class(HelloWorldPanel)

def unregister():
    bpy.utils.unregister_class(GameExportOperator)
    bpy.utils.unregister_class(AnimExportOperator)
    del bpy.types.Scene.GameExportPropertyGroup

if __name__ == "__main__":
    bpy.utils.register_class(GameExportPropertyGroup)
    bpy.types.Scene.game_export_props = bpy.props.PointerProperty(type=GameExportPropertyGroup)
    bpy.utils.register_class(GameExportOperator)
    bpy.utils.register_class(AnimExportOperator)
    bpy.utils.register_class(GameExportPanel)
    
old_active_object = None
    
def active_object_callback():
    global old_active_object
    
    active_object = bpy.context.object
    
    if active_object == old_active_object:
        return
    
    old_active_object = active_object
    
    if active_object not in object_to_filename:
        object_to_filename[active_object] = '../assets/meshes/DEFAULT.mesh'
        
    bpy.context.scene.game_export_props.filename = object_to_filename[active_object]
    print(object_to_filename[active_object])
    print("current active object: ", active_object)

owner = object()

def subscribe_to_active_object_change():
    subscribe_to = bpy.types.LayerObjects

    bpy.msgbus.subscribe_rna(
        key=subscribe_to,
        # owner of msgbus subcribe (for clearing later)
        owner=owner,
        # Args passed to callback function (tuple)
        args=(),
        # Callback function for property update
        notify=active_object_callback,
    )

subscribe_to_active_object_change()

# the filename
bpy.types.Object.filename_string = bpy.props.StringProperty(default='../assets/meshes/DEFAULT.mesh')
bpy.types.Object.anim_filename_string = bpy.props.StringProperty('../assets/animations/DEFAULT.mesh')