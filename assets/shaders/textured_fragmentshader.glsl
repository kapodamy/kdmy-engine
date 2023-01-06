out vec4 FragColor;
in vec2 v_texcoord;

uniform sampler2D u_texture;
uniform float u_alpha;

uniform bool u_offsetcolor_mul_or_diff;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;
uniform bool u_darken;

#ifdef DOTTED
uniform bool u_dotted;

const lowp float MOD_A = 4.0;
const lowp float MOD_B = MOD_A / 2.0;
#endif

void main() {
    vec4 color = texture(u_texture, v_texcoord);
    color.a *= u_alpha;

    if(color.a <= 0.0)
        discard;

    if(u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if(u_offsetcolor_mul_or_diff)
            color *= u_offsetcolor;
        else
            color = vec4(u_offsetcolor.rgb - color.rgb, u_offsetcolor.a * color.a);
    }

#ifdef DOTTED
    if(u_dotted && mod(gl_FragCoord.x, MOD_A) < MOD_B && mod(gl_FragCoord.y, MOD_A) < MOD_B) {
        discard;
    }
#endif

    if(color.a <= 0.0)
        discard;
        
    if (u_darken)
        color.rgb *= u_alpha;

    FragColor = color;
}
