attribute vec4 a_position;

uniform mat4 u_matrix_proyection;
uniform mat4 u_matrix_model;// location + size
uniform mat4 u_matrix_transform;// applied modifiers
uniform mat4 u_matrix_texture;// subtexture (from atlas entry)

varying vec2 v_texcoord;

void main() {
    gl_Position = u_matrix_proyection * u_matrix_transform * u_matrix_model * a_position;
    v_texcoord = (u_matrix_texture * a_position).xy;
}

