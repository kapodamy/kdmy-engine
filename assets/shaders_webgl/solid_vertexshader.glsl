precision highp float;
attribute vec4 a_position;

uniform mat4 u_matrix_proyection;
uniform mat4 u_matrix_model;// location + size
uniform mat4 u_matrix_transform;// applied modifiers

#ifdef DOTTED
varying vec2 v_coord;
#endif

void main() {
    vec4 pos = u_matrix_transform * u_matrix_model * a_position;
#ifdef DOTTED
    v_coord = pos.xy;
#endif
    gl_Position = u_matrix_proyection * pos;
}
