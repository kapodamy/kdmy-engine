using Engine.Externals.LuaScriptInterop;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game;

public class MissNoteFX {

    private SoundPlayer missnote1;
    private SoundPlayer missnote2;
    private SoundPlayer missnote3;
    private bool disabled;



    public MissNoteFX() {

        this.missnote1 = SoundPlayer.Init("/assets/common/sound/missnote1.ogg");
        this.missnote2 = SoundPlayer.Init("/assets/common/sound/missnote2.ogg");
        this.missnote3 = SoundPlayer.Init("/assets/common/sound/missnote3.ogg");
        this.disabled = false;

    }

    public void Destroy() {
        Luascript.DropShared(this);

        if (this.missnote1 != null) this.missnote1.Destroy();
        if (this.missnote2 != null) this.missnote2.Destroy();
        if (this.missnote3 != null) this.missnote3.Destroy();
        //free(this);
    }

    public void Stop() {
        if (this.missnote1 != null) this.missnote1.Stop();
        if (this.missnote2 != null) this.missnote2.Stop();
        if (this.missnote3 != null) this.missnote3.Stop();
    }

    public void Disable(bool disabled) {
        this.disabled = disabled;
    }

    public void PlayEffect() {
        if (this.disabled) return;

        int soundplayer_id = Math2D.RandomInt(0, 3);
        float volume = Math2D.Lerp(0.1f, 0.3f, Math2D.RandomFloat());

        SoundPlayer soundplayer;
        switch (soundplayer_id) {
            case 2:
                soundplayer = this.missnote3;
                break;
            case 1:
                soundplayer = this.missnote2;
                break;
            default:
            case 0:
                soundplayer = this.missnote1;
                break;
        }

        if (soundplayer != null) {
            soundplayer.SetVolume(volume);
            soundplayer.Replay();
        }
    }

}
