
namespace Engine.Externals;

public interface IWebGLExtension { }

public class WEBGL_compressed_texture_s3tc : IWebGLExtension {
    public readonly GLenum COMPRESSED_RGB_S3TC_DXT1_EXT = GLenum.GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    public readonly GLenum COMPRESSED_RGBA_S3TC_DXT1_EXT = GLenum.GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    public readonly GLenum COMPRESSED_RGBA_S3TC_DXT3_EXT = GLenum.GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    public readonly GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = GLenum.GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
}

public class EXT_texture_compression_bptc : IWebGLExtension {
    public readonly GLenum COMPRESSED_RGBA_BPTC_UNORM_EXT = GLenum.GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
    public readonly GLenum COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT = GLenum.GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
    public readonly GLenum COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT = GLenum.GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
    public readonly GLenum COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT = GLenum.GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
}
