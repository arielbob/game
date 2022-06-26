import bpy
import bmesh
from bpy.types import PropertyGroup

class Vertex:
    def __init__(self, co, normal, uv):
        self.co = co
        self.normal = normal
        self.uv = uv
    def __repr__(self):
        return f'Vertex(co={self.co}, normal={self.normal}, uv={self.uv})\n'
    def __eq__(self, other):
        if isinstance(other, Vertex):
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

def game_export(context, filename, replace_existing):
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

    # TODO: apply modifiers on the copy, so we don't have to apply them and change what we're exporting

    #bpy.ops.object.make_single_user(object=True, obdata=True, material=True, animation=False)
    
    mesh_copy_data = mesh_copy.data.copy()
    
    # Get a BMesh representation
    bm = bmesh.new()
    bm.from_mesh(mesh_copy_data)
    
    bmesh.ops.triangulate(bm, faces=bm.faces)

    bm.to_mesh(mesh_copy_data)
    bm.free()
    
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
            
        new_face_indices = []
        for i in range(3):
            vertex_to_add = Vertex(vertex_positions[i], vertex_normals[i], vertex_uvs[i])

            # TODO: this is slow, but is very simple
            #       we could add them to a set instead, but then we would have to set the face vertex
            #       indices later, which would mean we would have to loop through for every vertex in
            #       the face to find its index, and we would also need to store the vertex data for each
            #       face, so we can find the index again later.
            
            vertex_index = vertex_indices_by_vertex.get(vertex_to_add)
            if vertex_index == None:
                vertex_indices_by_vertex[vertex_to_add] = num_unique_vertices
                vertex_index = num_unique_vertices
                num_unique_vertices += 1
            
            new_face_indices.append(vertex_index)
            
        faces_list.append(new_face_indices)

    vertices_list = list(vertex_indices_by_vertex.items())
    print(vertices_list)
    vertices_list = [entry[0] for entry in sorted(vertices_list, key=lambda x: x[1])]
    print(vertices_list)

    # TODO: write data to file
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
        game_export(context, props.filename, props.replace_existing)
        return {'FINISHED'}

class GameExportPropertyGroup(PropertyGroup):
    filename: bpy.props.StringProperty(name="Filename")
    replace_existing: bpy.props.BoolProperty(name="Replace Existing")

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
    