#version 330 core

out vec4 FragColor;

in vec2 v_texcoord;

uniform sampler2D u_texture;
uniform float u_alpha;

uniform bool u_offsetcolor_mul_or_diff;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;

void main() {
    vec4 color = texture2D(u_texture, v_texcoord);
    color.a *= u_alpha;

    if(color.a <= 0.0)
        discard;

    if(u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if(u_offsetcolor_mul_or_diff)
            color *= u_offsetcolor;
        else
            color = vec4(u_offsetcolor.rgb - color.rgb, u_offsetcolor.a * color.a);
    }

    if(color.a <= 0.0)
        discard;

    FragColor = color;
}
