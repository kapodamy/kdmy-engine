#pragma header

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Note:Tthe engine replaces "#pragma header" with:
//      out     vec4      FragColor;
//      in      vec2      TexCoord;
//      uniform sampler2D Texture;
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

////////////////////////////////////////////////////////////////////////////////////////////////
//
// use "#pragma shadertoy_mainImage" for shadertoy shaders. The following code is added:
//
//        #define iResolution           vec3(1.0)
//        #define iTime                 u_kdy_time
//        #define iTimeDelta            u_kdy_delta
//        #define iFrame                u_kdy_frame
//        #define iChannel0             Texture
//        #define iFrameRate            (1.0/u_kdy_delta)
//        #define iChannelTime          (float[1](u_kdy_time))
//        #define iChannelResolution    (float[1](u_kdy_texsize))
//
//        void mainImage(out vec4, in vec2);
//        void main() { mainImage(FragColor, TexCoord); }
//
////////////////////////////////////////////////////////////////////////////////////////////////


void main() { 
    FragColor = texture(Texture, TexCoord);
}
