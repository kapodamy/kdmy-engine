out vec4 FragColor;
in vec2 v_texcoord;

uniform sampler2D u_texture;
uniform float u_alpha;

uniform bool u_offsetcolor_mul_or_add;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;
uniform bool u_darken;

void main() {
    vec4 color = texture(u_texture, v_texcoord);
    color.a *= u_alpha;

    if (color.a <= 0.0) {
        discard;
        return;
    }

    if (u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if (u_offsetcolor_mul_or_add)
            color *= u_offsetcolor;
        else
            color += u_offsetcolor;
    }

    if (color.a <= 0.0) {
        discard;
        return;
    }

    if (u_darken) {
        color.rgb *= u_alpha;
    }

    FragColor = color;
}
