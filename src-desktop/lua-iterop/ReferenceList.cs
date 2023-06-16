using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Engine.Externals.LuaInterop;

internal unsafe class ReferenceList {

    private readonly List<GCHandle> handles;

    public ReferenceList() {
        this.handles = new List<GCHandle>();
    }

    public void RevokeAllReferences() {
        for (int i = 0, count = this.handles.Count ; i < count ; i++) {
            GCHandle handle = this.handles[i];
            if (handle.IsAllocated) handle.Free();
        }
        this.handles.Clear();
    }

    public void* GetReference(object obj) {
        GCHandle handle;

        for (int i = 0, count = this.handles.Count ; i < count ; i++) {
            handle = this.handles[i];

            if (!handle.IsAllocated) {
                this.handles.RemoveAt(i);
                i--;
                continue;
            }

            if (Object.ReferenceEquals(handle.Target, obj)) {
                return GCHandle.ToIntPtr(handle).ToPointer();
            }
        }

        return null;
    }

    public void* AddReference(object obj, bool strong_reference) {
        void* obj_ptr = this.GetReference(obj);
        if (obj_ptr != null) return obj_ptr;

        GCHandleType type = strong_reference ? GCHandleType.Normal : GCHandleType.WeakTrackResurrection;
        GCHandle handle = GCHandle.Alloc(obj, type);
        this.handles.Add(handle);

        return GCHandle.ToIntPtr(handle).ToPointer();
    }

    public void RevokeReference(object obj) {
        for (int i = 0, count = this.handles.Count ; i < count ; i++) {
            GCHandle handle = this.handles[i];

            if (!handle.IsAllocated) {
                this.handles.RemoveAt(i);
                i--;
                continue;
            }

            if (Object.ReferenceEquals(handle.Target, obj)) {
                handle.Free();
            }
        }
    }

}

