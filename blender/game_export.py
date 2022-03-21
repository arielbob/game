import bpy
import bmesh
from bpy.types import PropertyGroup

def game_export(context, filename, replace_existing):

    temp_output_file = None
    open_flag = ''
    
    if (replace_existing):
        open_flag = 'w'
    else:
        open_flag = 'x'
    
    try:
        temp_output_file = open(filename, open_flag)
    except FileExistsError:
        show_message_box('File already exists', 'Error', 'ERROR')
        return


    bpy.ops.object.mode_set(mode='OBJECT')
    mesh_copy = context.active_object.copy()

    bpy.ops.object.make_single_user(object=True, obdata=True, material=True, animation=False)
    
    mesh_copy_data = mesh_copy.data
    
    # Get a BMesh representation
    bm = bmesh.new()
    bm.from_mesh(mesh_copy_data)
    
    bmesh.ops.triangulate(bm, faces=bm.faces)

    bm.to_mesh(mesh_copy_data)
    bm.free()
    
    
    num_vertices = len(mesh_copy_data.vertices)
    num_triangles = len(mesh_copy_data.polygons)
    
    temp_output_file.write(str(num_vertices) + '\n')
    temp_output_file.write(str(num_triangles) + '\n\n')
    
    temp_output_file.write(';; vertices\n\n')
    
    # get the UVs for each vertex
    # NOTE: We keep the last index UV we see to get the vertex UV we export.
    #       In Blender, there isn't a 1-1 correspondence between UV and
    #       vertex, since if you imagine the cube case, where each side
    #       is a different colour, what would the vertex color be?
    #       Exactly, you would need 3 different values for the same vertex,
    #       and that's where index-based UVs come in.
    #       But in the majority of cases where there is a smooth transition
    #       from one vertex to another, just taking the last UV is completely
    #       fine.
    #
    #       Also imagine the case where UV islands are not connected. In that
    #       case, a single vertex is seen as being mapped to 2 different
    #       positions in UV space. But since Blender uses indices, it's fine.
    #
    #       What would we need to do if we wanted a faceted look like this?
    #       Well, it would be similar to how you would do flat shading. In flat
    #       shading, you have n copies of a vertex with n adjacent faces. In the
    #       example of a cube, each vertex would have 3 copies of itself and
    #       each vertex would have the normal and uv of the face that it's
    #       meant for.
    #
    #       Why do we do this in the first place? Why can't we just have face
    #       normals and not think about vertices?
    #       The reason why we can't do that is because the graphics rendering
    #       pipeline when using graphics hardware operates on vertices. It
    #       expects us to give them vertex data and not face data. The face
    #       data, such as face normals and face UVs are all interpolated using
    #       the vertex data.
    #
    #       And I'm not sure if "face data" is an entirely accurate term. Maybe
    #       it should just be "fragment data." The fragments are the actual
    #       pixels that make up the face of a polygon.
    
    # get the vertex indices for all the faces in a flattened array
    flattened_face_indices = []
    for face in bpy.context.active_object.data.polygons:
        face_vertices = list(face.vertices)
        for item in face_vertices:
            flattened_face_indices.append(item)
    
    # get the UVs for each vertex
    # we use the flattened_face_indices array to get the vertex index of the
    # current UV, since the UVs are stored in the same order as the face indices
    vertex_uvs = [0] * len(bpy.context.active_object.data.vertices)
    for i in range(len(bpy.context.active_object.data.uv_layers.active.data)):
        index_uv = bpy.context.active_object.data.uv_layers.active.data[i]
        uv = index_uv.uv
        vertex_uvs[flattened_face_indices[i]] = uv

#        print('vertex index: ' + str(flattened_face_indices[i]) + '\tuv: ' + str(uv))
    
    
    for i in range(len(mesh_copy_data.vertices)):
        vert = mesh_copy_data.vertices[i]
        
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
    for face in mesh_copy_data.polygons:
        index_1, index_2, index_3 = face.vertices
        temp_output_file.write('{} {} {}\n'.format(index_3, index_2, index_1))
    
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
    