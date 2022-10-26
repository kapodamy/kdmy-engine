#version 330 core

out vec4 FragColor;

uniform vec4 u_vetex_color;

uniform bool u_offsetcolor_mul_or_diff;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;

void main() {
	vec4 color = u_vetex_color;
	
    if(u_offsetcolor_enabled/*u_offsetcolor.a >= 0*/) {
        if(u_offsetcolor_mul_or_diff)
            color *= u_offsetcolor;
        else
            color = vec4(u_offsetcolor.rgb - color.rgb, u_offsetcolor.a * color.a);
    }
	
	FragColor = color;
}

