out vec4 FragColor;
in vec2 v_texcoord;

uniform sampler2D u_texture;
uniform float u_alpha;

void main() {
    FragColor = texture(u_texture, v_texcoord) * u_alpha;
}
