export { };

declare global {
    interface Window {
        run: number | boolean
    }

    interface DedicatedWorkerGlobalScope {
        postMessage(message: any, transfer?: Transferable[]): void;
    }
}
