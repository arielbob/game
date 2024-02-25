import bpy
import bmesh
from mathutils import Matrix
from bpy.types import PropertyGroup

class Vertex:
    def __init__(self, co, normal, uv, bone_indices, bone_weights):
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

def game_export(context, filename, replace_existing, is_skinned):
    temp_output_file = None
    open_flag = ''
    
    if (replace_existing):
        open_flag = 'w'
    else:
        open_flag = 'x'
    
    try:
        filepath = bpy.path.abspath('//' + filename)
        temp_output_file = open(filepath, open_flag)
    except FileExistsError:
        show_message_box('File already exists', 'Error', 'ERROR')
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
            return
        
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
        
        bone_names = [bone.name for bone in bones]
        
        skeleton_data_string += 'skeleton {\n'
        skeleton_data_string += 'num_bones {:d}\n'.format(len(bones))
        
        # to swap the y and z of matrix_local to fit the game's coordinate-space
        swap_matrix = Matrix(((1.0, 0.0, 0.0, 0.0),
            (0.0, 0.0, 1.0, 0.0),
            (0.0, 1.0, 0.0, 0.0),
            (0.0, 0.0, 0.0, 1.0)))
        
        for bone in bones:
            skeleton_data_string += '\nbone "{:s}" {{\n'.format(bone.name)
            skeleton_data_string += 'inverse_bind '
            
            # TODO: for some reason, matrix_local starts out with swapped y and z rows
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
            #   blender's model-space, then to convert to blender's model space, just swap y and
            #   z-axes.
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
                    if bone_weight > 0.0001:
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
            vertex_to_add = Vertex(vertex_positions[i], vertex_normals[i], vertex_uvs[i], vertex_bone_indices[i], vertex_bone_weights[i])

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
                    return
                
                temp_output_file.write('bi')
                for bone_index in vert.bone_indices:
                    temp_output_file.write(' {:d}'.format(bone_index))
                
                temp_output_file.write('\nbw')
                for bone_weight in vert.bone_weights:
                    temp_output_file.write(' {:f}'.format(bone_weight))
                    
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


class GameExportOperator(bpy.types.Operator):
    """Tooltip"""
    bl_idname = "object.game_export"
    bl_label = "Game Export Operator"

    filename: bpy.props.StringProperty(name='filename')

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        props = context.scene.game_export_props
        print('filename: ' + props.filename)
        game_export(context, props.filename, props.replace_existing, props.is_skinned)
        return {'FINISHED'}

class GameExportPropertyGroup(PropertyGroup):
    filename: bpy.props.StringProperty(name="Filename")
    replace_existing: bpy.props.BoolProperty(name="Replace Existing")
    is_skinned: bpy.props.BoolProperty(name="Enable Skinning")

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
        row.prop(props, "filename", text="File name")
        row = layout.row()
        row.prop(props, "replace_existing", text="Replace Existing")
        row = layout.row()
        row.prop(props, "is_skinned", text="Skinned")

        row = layout.row()
        row.operator("object.game_export")

#def unregister_game_export_panel():
#    bpy.utils.unregister_class(HelloWorldPanel)

def unregister():
    bpy.utils.unregister_class(GameExportOperator)
    del bpy.types.Scene.GameExportPropertyGroup

if __name__ == "__main__":
    bpy.utils.register_class(GameExportPropertyGroup)
    bpy.types.Scene.game_export_props = bpy.props.PointerProperty(type=GameExportPropertyGroup)
    bpy.utils.register_class(GameExportOperator)
    bpy.utils.register_class(GameExportPanel)
    