precision highp float;

varying vec2 v_texcoord;

uniform sampler2D u_texture;
uniform float u_alpha;

uniform bool u_offsetcolor_mul_or_diff;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;

#ifdef DOTTED
uniform bool u_dotted;

const lowp float MOD_A = 4.0;
const lowp float MOD_B = MOD_A / 2.0;
const lowp float RESOLUTION = 1024.0;
#endif

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

#ifdef DOTTED
    if(u_dotted && mod(RESOLUTION * v_texcoord.x, MOD_A) < MOD_B && mod(RESOLUTION * v_texcoord.y, MOD_A) < MOD_B) {
        color.a *= 0.25;
    }
#endif

    gl_FragColor = color;
}
