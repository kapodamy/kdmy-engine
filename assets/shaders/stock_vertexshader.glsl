#pragma header

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Note: The engine replaces "#pragma header" with:
//          in  vec4 a_position;
//          in  vec2 a_texcoord;
//          out vec2 TexCoord;
//
// Also the following uniforms are added:
//          uniform float u_kdy_time      seconds since the engine is running
//          uniform int   u_kdy_frame     frame number (increments by one on every draw)
//          uniform float u_kdy_delta     last render time in seconds
//          uniform vec3  u_kdy_screen    screen {width, height, aspect ratio}
//          uniform vec3  u_kdy_texsize   framebuffer {width, height, aspect ratio}
//
// Important: The engine always add "#version 330 core" or "#version 300 es" at the
//            start of the shader.
//
////////////////////////////////////////////////////////////////////////////////////////////////


void main() {
    gl_Position = a_position;
    TexCoord = a_texcoord;
}
