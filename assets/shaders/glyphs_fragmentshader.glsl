out vec4 FragColor;

in vec2 v_texcoord;
in float v_texalt;
in float v_coloralt;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;

uniform vec4 u_color;
uniform vec4 u_color_outline;
uniform bool u_grayscale;
uniform bool u_color_by_add;

uniform bool u_offsetcolor_mul_or_add;
uniform bool u_offsetcolor_enabled;
uniform vec4 u_offsetcolor;

#ifdef SDF_FONT
uniform float u_sdf_size;// this is the 25% of FONTTYPE_GLYPHS_HEIGHT
uniform float u_sdf_padding;// padding percent

float calculate_sdf_pixel_alpha(float lumma, ivec2 tex_size) {
    //
    // SDF antialiasing, credits to:
    //      drewcassidy
    //      mortoray
    //

    float dist = (0.0 - lumma) * u_sdf_size;
    vec2 duv = fwidth(v_texcoord);
    float dtex = length(duv * vec2(tex_size));
    float pixelDist = (dist * 2.0) / dtex;

    return clamp(0.5 - pixelDist, 0.0, 1.0);
}

vec4 calculate_sdf() {
    float luminance;
    ivec2 tex_size;
    vec4 color;

    if(bool(v_texalt)) {
        luminance = texture(u_texture1, v_texcoord).r;
        tex_size = textureSize(u_texture1, 0);
    } else {
        luminance = texture(u_texture0, v_texcoord).r;
        tex_size = textureSize(u_texture0, 0);
    }

    if(luminance <= 0.0) {
        // avoid tranparent pixels become visible with outlines
        return vec4(0.0);
    }

    // change range from [0.0, 1.0] to [-0.5, 0.5]
    float lumma = luminance - 0.5;

    float alpha = calculate_sdf_pixel_alpha(lumma, tex_size);

    if(bool(v_coloralt)) {
        // u_sdf_padding is calculated as (border_size / FONTTYPE_GLYPHS_HEIGHT)
        float lumma_outline = luminance - mix(0.5, 0.0, u_sdf_padding);

        float alpha_outline = calculate_sdf_pixel_alpha(lumma_outline, tex_size);

        if(alpha != 0.0) {
            // do not draw the outline behind the glyph
            return vec4(0.0);
        }

        alpha = alpha_outline;
        color = u_color_outline;
    } else {
        color = u_color;
    }

    // apply alpha
    color.a *= alpha;

    return color;
}
#endif

vec4 calculate_lumma() {
    float luminance;
    vec4 color;

    if(bool(v_texalt))
        luminance = texture(u_texture1, v_texcoord).r;
    else
        luminance = texture(u_texture0, v_texcoord).r;

    if(bool(v_coloralt))
        color = vec4(u_color_outline.rgb, u_color_outline.a * luminance);
    else
        color = vec4(u_color.rgb, u_color.a * luminance);

    return color;
}

vec4 calculate_rgb() {
    vec4 color;
    vec4 tint;

    if(bool(v_texalt))
        color = texture(u_texture1, v_texcoord);
    else
        color = texture(u_texture0, v_texcoord);

    if(bool(v_coloralt))
        tint = u_color_outline;
    else
        tint = u_color;

    if(u_color_by_add) {
        color.rgb += tint.rgb;
        color.a *= tint.a;
    } else {
        color *= tint;
    }

    return color;
}

void main() {
    vec4 color;

    if(u_grayscale) {
#ifdef SDF_FONT
        color = calculate_sdf();
#else
        color = calculate_lumma();
#endif
    } else {
        color = calculate_rgb();
    }

    if(u_offsetcolor_enabled) {
        if(u_offsetcolor_mul_or_add)
            color *= u_offsetcolor;
        else
            color += u_offsetcolor;
    }

    if(color.a <= 0.0) {
        discard;
        return;
    }

    FragColor = color;
}
