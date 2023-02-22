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
uniform float u_sdf_smoothing;
uniform float u_sdf_thickness;

const float THICKNESS = 0.20;
#endif

void main() {
    bool is_outline = v_coloralt != 0.0;
    vec4 source_color = is_outline ? u_color_outline : u_color;
    vec4 texture_color;
    vec4 color;

    if(bool(v_texalt))
        texture_color = texture(u_texture1, v_texcoord);
    else
        texture_color = texture(u_texture0, v_texcoord);

    if(u_grayscale) {
        float luminance = texture_color.r;

        if(luminance <= 0.0) {
            discard;
            return;
        }

#ifdef SDF_FONT
        float distance = smoothstep(0.5 - u_sdf_smoothing, 0.5 + u_sdf_smoothing, luminance);

        if(is_outline && u_sdf_thickness >= 0.0) {
            float alpha = smoothstep(u_sdf_thickness - u_sdf_smoothing, u_sdf_thickness + u_sdf_smoothing, luminance);
            color = vec4(u_color_outline.rgb, u_color_outline.a * alpha * (1.0 - distance));
        } else {
            color = vec4(source_color.rgb, source_color.a * distance);
        }
#else
        color = vec4(source_color.rgb, source_color.a * distance);
#endif
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
