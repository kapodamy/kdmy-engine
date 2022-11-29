out vec4 FragColor;

uniform vec4 u_vertex_color;

uniform bool u_offsetcolor_mul_or_diff;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;

#ifdef DOTTED
varying vec2 v_coord;
uniform bool u_dotted;

const lowp float MOD_A = 4.0;
const lowp float MOD_B = MOD_A / 2.0;
#endif

void main() {
    vec4 color = u_vertex_color;

    if(u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if(u_offsetcolor_mul_or_diff)
            color *= u_offsetcolor;
        else
            color = vec4(u_offsetcolor.rgb - color.rgb, u_offsetcolor.a * color.a);
    }

#ifdef DOTTED
    if(u_dotted && mod(v_coord.x, MOD_A) < MOD_B && mod(v_coord.y, MOD_A) < MOD_B) {
        discard;
    }
#endif

    FragColor = color;
}
