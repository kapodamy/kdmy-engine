
in vec4 a_position;
in vec4 a_source_coords;
in vec4 a_draw_coords;
in float a_texture_alt;
in float a_color_alt;

uniform mat4 u_matrix_proyection;
uniform mat4 u_matrix_transform;

out float v_texalt;
out vec2 v_texcoord;
out float v_coloralt;


mat4 tmp_matrix = mat4(1.0);

void main() {
    tmp_matrix[0][0] = a_draw_coords.z;
    tmp_matrix[1][1] = a_draw_coords.w;
    tmp_matrix[3][0] = a_draw_coords.x;
    tmp_matrix[3][1] = a_draw_coords.y;

    gl_Position = u_matrix_proyection * u_matrix_transform * tmp_matrix * a_position;

    tmp_matrix[3][0] = a_source_coords.x;
    tmp_matrix[3][1] = a_source_coords.y;
    tmp_matrix[0][0] = a_source_coords.z;
    tmp_matrix[1][1] = a_source_coords.w;

    v_texcoord = (tmp_matrix * a_position).xy;
    v_texalt = a_texture_alt;
    v_coloralt = a_color_alt;
}
