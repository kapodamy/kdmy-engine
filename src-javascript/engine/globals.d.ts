export { };

declare global {
    interface Window {
        run: number | boolean
    }

    interface DedicatedWorkerGlobalScope {
        postMessage(message: any, transfer?: Transferable[]): void;
    }

    // used is some js-docs
    type bool = (number | boolean | 1 | 0 | false | true);
}