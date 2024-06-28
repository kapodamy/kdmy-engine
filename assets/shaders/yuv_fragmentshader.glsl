out vec4 FragColor;
in vec2 v_texcoord;

uniform sampler2D u_texture[3];
uniform float u_alpha;

uniform bool u_offsetcolor_mul_or_add;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;
uniform bool u_darken;

void main() {
    float y = texture(u_texture[0], v_texcoord).r;
    float u = texture(u_texture[1], v_texcoord).r - 0.5;
    float v = texture(u_texture[2], v_texcoord).r - 0.5;

    float r = y + (1.403 * v);
    float g = y - (0.344 * u) - (0.714 * v);
    float b = y + (1.770 * u);

    vec4 color = vec4(r, g, b, u_alpha);

    if(u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if(u_offsetcolor_mul_or_add)
            color *= u_offsetcolor;
        else
            color += u_offsetcolor;
    }

    if(color.a <= 0.0) {
        discard;
        return;
    }

    if(u_darken) {
        color.rgb *= u_alpha;
    }

    FragColor = color;
}
