precision highp float;

uniform vec4 u_vetex_color;

#ifdef DOTTED
varying vec2 v_coord;
uniform bool u_dotted;

const lowp float MOD_A = 4.0;
const lowp float MOD_B = MOD_A / 2.0;
#endif

void main() {
#ifdef DOTTED
    if(u_dotted && mod(v_coord.x, MOD_A) < MOD_B && mod(v_coord.y, MOD_A) < MOD_B) {
        discard;
    }
#endif

    gl_FragColor = u_vetex_color;
}
