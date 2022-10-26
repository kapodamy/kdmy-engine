using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Engine.Externals {

    public interface IWebGLExtension { }

    public class ANGLE_instanced_arrays : IWebGLExtension {
        public void vertexAttribDivisorANGLE(uint index, uint divisor) {
            WebGLRenderingContext.NativeMethods.glVertexAttribDivisor(index, divisor);

        }
        public void drawElementsInstancedANGLE(GLenum mode, int count, GLenum type, int offset, int primcount) {
            WebGLRenderingContext.KDY_draw_calls_count++;
            WebGLRenderingContext.NativeMethods.glDrawElementsInstanced(mode, count, type, (IntPtr)offset, primcount);
        }
    }

    public class OES_element_index_uint : IWebGLExtension { }
}
