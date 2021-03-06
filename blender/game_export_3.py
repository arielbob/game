import bpy
import bmesh
from bpy.types import PropertyGroup
import mathutils

class Vertex:
    def __init__(self, co, normal):
        self.co = co
        self.normal = normal
    def __repr__(self):
        return f'Vertex(co={self.co}, normal={self.normal})\n'
    def __eq__(self, other):
        if isinstance(other, Vertex):
            return ((self.co == other.co) and (self.normal == other.normal))
        else:
            return False
    def __hash__(self):
        return int(self.co[0] + self.co[1]+ self.co[2] + self.normal[0] + self.normal[1] + self.normal[2])

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
    
    active_object = context.active_object
    object_data = active_object.data

    object_data.calc_loop_triangles()
    object_data.calc_normals_split()
    
    vertices_set = set()

    bm = bmesh.new()
    bm.from_mesh(object_data)
    
    
    
    uv_layer = bm.loops.layers.uv.active
    
    for face in bm.faces:
        for loop_vertex in face.loops:
            uv = loop_vertex[uv_layer].uv
            vert = loop_vertex.vert.co
            normal = loop_vertex.calc_normal()
            
    
#    object_data.calc_loop_triangles()
#    object_data.calc_normals_split()
#    for loop in object_data.loop_triangles:
#        for i in range(3):
#            vertex_index = loop.vertices[i]
#            
#            vertex_coord = object_data.vertices[vertex_index].co.copy()
#            split_normal = loop.split_normals[i]
#            vertex_normal = mathutils.Vector((split_normal[0], split_normal[1], split_normal[2]))
#            
#            v_to_add = Vertex(vertex_coord, vertex_normal)
#            vertices_set.add(v_to_add)

    print(vertices_set)
    
    # TODO: store the UVs in 
    # TODO: get UVs.
    #       we may want to use BMesh instead: https://docs.blender.org/api/current/bmesh.html#customdata-access
    #       TODO: see if we can get the vertices, split normals, and UVs of a triangulated face using BMesh
    #             https://docs.blender.org/api/current/bmesh.types.html#bmesh.types.BMFace
    #       NEVERMIND, BMesh does not give you the interpolated normals for smooth faces

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
    