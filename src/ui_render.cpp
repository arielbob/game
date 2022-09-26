#include "ui.h"

void ui_push_triangle_list(UI_Vertex *vertices, int32 num_vertices,
                           uint32 *indices,     int32 num_indices) {
    assert(num_indices % 3 == 0);
    int32 num_triangles = num_indices / 3;

    int32 indices_base = ui_manager->num_vertices;
    for (int32 i = 0; i < num_vertices; i++) {
        assert(ui_manager->num_vertices < UI_MAX_VERTICES);
        ui_manager->vertices[ui_manager->num_vertices++] = vertices[i];
    }

    for (int32 i = 0; i < num_triangles; i++) {
        assert((ui_manager->num_indices / 3) < UI_MAX_TRIANGLES);
        // convert the indices to be relative to the ui_manager's vertex list instead of relative to
        // the vertices argument
        uint32 i1 = indices_base + indices[3*i + 0];
        uint32 i2 = indices_base + indices[3*i + 1];
        uint32 i3 = indices_base + indices[3*i + 2];
        assert(i1 < (uint32) num_vertices);
        assert(i2 < (uint32) num_vertices);
        assert(i3 < (uint32) num_vertices);
        ui_manager->indices[ui_manager->num_indices++] = i1;
        ui_manager->indices[ui_manager->num_indices++] = i2;
        ui_manager->indices[ui_manager->num_indices++] = i3;
    }
}

// assumes clockwise order of vertices
void ui_push_quad(Vec2 vertices[4], Vec2 uvs[4], Vec4 color) {
    UI_Vertex ui_vertices[4];
    for (int32 i = 0; i < 4; i++) {
        ui_vertices[i] = { vertices[i], uvs[i], color };
    }

    int32 num_triangles = 2;
    uint32 indices[] = { 0, 1, 2, 0, 2, 3 };
    ui_push_triangle_list(ui_vertices, 4, indices, num_triangles*3);
}
