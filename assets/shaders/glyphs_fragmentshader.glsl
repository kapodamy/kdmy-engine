#define SDF_FONT

out vec4 FragColor;

in vec2 v_texcoord;
in float v_texalt;
in float v_coloralt;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;

uniform vec4 u_color;
uniform vec4 u_color_outline;
uniform bool u_grayscale;
uniform bool u_color_by_diff;

uniform bool u_offsetcolor_mul_or_diff;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;

#ifdef DOTTED
uniform bool u_dotted;

const float MOD_A = 0.0016;
const float MOD_B = MOD_A / 2.0;
#endif

#ifdef SDF_FONT
uniform float u_sdf_width;
uniform float u_sdf_edge;

const float THICKNESS = 0.20;
#endif

void main() {
    bool is_outline = v_coloralt != 0.0;
    vec4 texture_color;
    vec4 source_color = is_outline ? u_color_outline : u_color;
    vec4 color;

    if(bool(v_texalt))
        texture_color = texture(u_texture1, v_texcoord);
    else
        texture_color = texture(u_texture0, v_texcoord);

    if(u_grayscale) {
        float luminance = texture_color.r;

		#ifdef SDF_FONT
        /*if(is_outline) {
            luminance = 1.0 - smoothstep(THICKNESS + u_sdf_width, THICKNESS + u_sdf_width + u_sdf_edge, 1.0 - luminance);
        } else {*/
            luminance = 1.0 - smoothstep(u_sdf_width, u_sdf_width + u_sdf_edge, 1.0 - luminance);
        /*}*/
		#endif

        color = vec4(1.0, 1.0, 1.0, luminance) * source_color;
    } else {
        if(u_color_by_diff) {
            color.r = source_color.r - texture_color.r;
            color.g = source_color.g - texture_color.g;
            color.b = source_color.b - texture_color.b;
            color.a = texture_color.a * source_color.a;
        } else {
            color = texture_color * source_color;
        }
    }

    if(u_offsetcolor_enabled) {
        if(u_offsetcolor_mul_or_diff)
            color *= u_offsetcolor;
        else
            color = u_offsetcolor - color;
    }

    if(color.a <= 0.0)
        discard;

#ifdef DOTTED
    if(u_dotted && mod(v_texcoord.x, MOD_A) < MOD_B && mod(v_texcoord.y, MOD_A) < MOD_B) {
        color.a *= 0.25;
    }
#endif

    FragColor = color;
}
