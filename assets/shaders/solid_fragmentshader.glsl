out vec4 FragColor;

uniform vec4 u_vertex_color;

uniform bool u_offsetcolor_mul_or_add;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;
uniform bool u_darken;

void main() {
    vec4 color = u_vertex_color;

    if(u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if(u_offsetcolor_mul_or_add)
            color *= u_offsetcolor;
        else
            color += u_offsetcolor;
    }

    if (u_darken) {
        color.rgb *= color.a;
    }

    FragColor = color;
}
