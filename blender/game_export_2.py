import bpy
import bmesh
from bpy.types import PropertyGroup

class Vertex:
    def __init__(self, co, normal):
        self.co = co
        self.normal = normal

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

    #bpy.ops.object.make_single_user(object=True, obdata=True, material=True, animation=False)
    
    mesh_copy_data = mesh_copy.data.copy()
    
    # Get a BMesh representation
    bm = bmesh.new()
    bm.from_mesh(mesh_copy_data)
    
    bmesh.ops.triangulate(bm, faces=bm.faces)

    bm.to_mesh(mesh_copy_data)
    bm.free()
    
    # NOTE: it is very important that you copy these attributes and not just store references
    #       to the original vertices. if you store references, blender may move the data and your
    #       reference will become out of date, which will cause a crash due to invalid memory
    #       access later.
    vertices_list = [Vertex(v.co.copy(), v.normal.copy()) for v in mesh_copy_data.vertices]
    num_vertices = len(vertices_list)
    
    faces_list = [face.vertices for face in mesh_copy_data.polygons]
    num_triangles = len(faces_list)
    
    vertex_uvs = [None] * len(vertices_list)
    indices_for_vert = [None] * len(vertices_list)
    
    for i in range(len(mesh_copy_data.uv_layers.active.data)):
        index_uv = mesh_copy_data.uv_layers.active.data[i]
        uv = index_uv.uv
        
        # NOTE: we assume that all faces have 3 verts
        face_index = i // 3
        vertex_index = faces_list[face_index][i % 3]
        
        if (vertex_uvs[vertex_index] == None):
            vertex_uvs[vertex_index] = uv
        elif (vertex_uvs[vertex_index] == uv):
            continue
        elif (vertex_uvs[vertex_index] != uv):

            found_shared_index = False
            if (indices_for_vert[vertex_index] != None):
                for v_index in indices_for_vert[vertex_index]:
                    if (vertex_uvs[v_index] == uv):
                        faces_list[face_index][i % 3] = v_index
                        found_shared_index = True
            
            if not found_shared_index:      
                # add a new vertex
                vertices_list.append(mesh_copy_data.vertices[vertex_index])
                new_index = num_vertices
                vertex_uvs.append(uv)
                faces_list[face_index][i % 3] = new_index
                
                if (indices_for_vert[vertex_index] == None):
                    indices_for_vert[vertex_index] = [new_index]
                else:
                    indices_for_vert[vertex_index].append(new_index)
                num_vertices += 1
                
                
                
    temp_output_file.write(str(num_vertices) + '\n')
    temp_output_file.write(str(num_triangles) + '\n\n')
    
    temp_output_file.write(';; vertices\n\n')
    
    for i in range(num_vertices):
        vert = vertices_list[i]
        
        # TODO: we may want to find a way to not always have to store 16 places after
        #       the decimal, for example we don't want to store 1.0 as 1.000000... whatever
        #       .. but usually models don't really fall on integer boundaries like that, so..
        #       it probably doesn't really matter
        x, y, z = vert.co
        out_x, out_y, out_z = x, z, y
        temp_output_file.write('v {:.16f} {:.16f} {:.16f}\n'.format(out_x, out_y, out_z))
        
        x, y, z = vert.normal
        out_x, out_y, out_z = x, z, y
        temp_output_file.write('n {:.16f} {:.16f} {:.16f}\n'.format(out_x, out_y, out_z))
        
        u, v = vertex_uvs[i]
        temp_output_file.write('uv {:.16f} {:.16f}\n'.format(u, v))
        
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
    
    print('closing file and deleting copy of mesh\n')
    
    temp_output_file.close()
    
    # delete the temporary mesh
    bpy.data.objects.remove(mesh_copy)
    
    # TODO: store a mapping of object names -> file names so that we don't have to keep changing the name
    #       of the output file inside the input box
    
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
    