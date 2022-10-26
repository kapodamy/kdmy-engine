export { };

declare global {
    interface Window {
        run: number | boolean
    }

    interface DedicatedWorkerGlobalScope {
        postMessage(message: any, transfer?: Transferable[]): void;
    }

    /*interface NavigatorUAData {
        brands: FrozenArray<object>;
        mobile: boolean;
        platform: string;
        getHighEntropyValues(hints: string[]): any[];
        toJSON(): object;
    };

    declare var NavigatorUAData: {
        prototype: NavigatorUAData
    };*/

    interface ICanvas {
        /**
        * Returns an object that provides methods and properties for drawing and manipulating images and graphics on a canvas element in a document. A context object includes information about colors, line widths, fonts, and other graphic parameters that can be drawn on a canvas.
        * @param contextId The identifier (ID) of the type of canvas to create. Internet Explorer 9 and Internet Explorer 10 support only a 2-D context using canvas.getContext("2d"); IE11 Preview also supports 3-D or WebGL context using canvas.getContext("experimental-webgl");
        */
        getContext(contextId: "2d", options?: CanvasRenderingContext2DSettings): CanvasRenderingContext2D | null;
        getContext(contextId: "bitmaprenderer", options?: ImageBitmapRenderingContextSettings): ImageBitmapRenderingContext | null;
        getContext(contextId: "webgl", options?: WebGLContextAttributes): WebGLRenderingContext | null;
        getContext(contextId: "webgl2", options?: WebGLContextAttributes): WebGL2RenderingContext | null;
        getContext(contextId: string, options?: any): RenderingContext | null;

        width: number;
        height: number;

    }

    /**
     * The OffscreenCanvas interface provides a canvas that can be rendered off screen. It is available in both the window and worker contexts.
     */
    interface OffscreenCanvas extends HTMLCanvasElement {

    }

    declare var OffscreenCanvas: {
        prototype: OffscreenCanvas;
        new(width, height): OffscreenCanvas;
    };

    // used is some js-docs
    type bool = (number | boolean | 1 | 0 | false | true);
}