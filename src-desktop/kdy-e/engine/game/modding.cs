using System;
using Engine.Game.Gameplay;
using Engine.Platform;
using Newtonsoft.Json.Linq;

namespace Engine.Game {

    public class Modding {
        public readonly WeekScript script;
        public readonly Layout layout;
        public bool has_exit;
        public bool has_halt;
        public bool ui_visible;
        public bool has_funkinsave_changes;

        public Modding(Layout layout, string src_script) {

            this.script = null;
            this.layout = layout;
            this.has_exit = false;
            this.has_halt = false;
            this.ui_visible = true;
            this.has_funkinsave_changes = false;

            if (!String.IsNullOrEmpty(src_script) && FS.FileExists(src_script))
                this.script = WeekScript.Init(src_script, this, false);

        }

        public void Destroy() {
            if (this.has_funkinsave_changes) FunkinSave.WriteToVMU();
            if (this.script != null) this.script.Destroy();
            //free(this);
        }

        public Layout GetLayout() {
            return this.layout;
        }

        public void Exit() {
            this.has_exit = true;
        }

        public void SetHalt(bool halt) {
            this.has_halt = halt;
        }


        public void SetUiVisibility(bool visible) {
            this.ui_visible = !!visible;
        }

        public void UnlockdirectiveCreate(string name, double value) {
            this.has_funkinsave_changes = true;
            FunkinSave.CreateUnlockDirective(name, value);
        }

        public bool UnlockdirectiveHas(string name) {
            return FunkinSave.ContainsUnlockDirective(name);
        }

        public double UnlockdirectiveGet(string name) {
            double value;
            FunkinSave.ReadUnlockDirective(name, out value);
            return value;
        }

        public void UnlockdirectiveRemove(string name) {
            this.has_funkinsave_changes = true;
            FunkinSave.DeleteUnlockDirective(name);
        }


    }

}


