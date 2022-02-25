#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 model;
/*
uniform mat4 cpv_matrix;
uniform vec4 color;
*/

//out vec3 frag_pos;
out vec4 frag_color;
//out vec3 normal;
//out vec2 uv;

void main() {
    // frag_pos = vec3(model * vec4(pos, 1.0));
    gl_Position = model * vec4(pos, 1.0);
    // gl_Position = vec4(pos, 1.0);
    frag_color = vec4(1.0, 1.0, 1.0, 1.0);

    // TODO: add transform to normal vector
    // TODO: this transform doesn't handle non-uniform scale
    // NOTE: w of vec4 is 0 to ignore the translation of the model matrix
    //normal = vec3(model * vec4(vert_normal, 0.0));
    //uv = _uv;
}
