using System;
using Engine.Game.Gameplay;
using Engine.Platform;

namespace Engine.Game {

    public class Modding {
        public readonly WeekScript script;
        public readonly Layout layout;
        public bool has_exit;

        public Modding(Layout layout, string src_script) {

            this.script = null;
            this.layout = layout;
            this.has_exit = false;

            if (!String.IsNullOrEmpty(src_script) && FS.FileExists(src_script))
                this.script = WeekScript.Init(src_script, this, false);

        }

        public void Destroy() {
            if (this.script != null) this.script.Destroy();
            //free(this);
        }

        public Layout GetLayout() {
            return this.layout;
        }

        public void Exit() {
            this.has_exit = true;
        }

    }

}


