using Engine.Externals.LuaScriptInterop;
using Engine.Utils;

namespace Engine.Game;

public class HealthWatcher {

    private ArrayList<CharacterInfo> players;
    private ArrayList<CharacterInfo> opponents;

    public HealthWatcher() {
        this.players = new ArrayList<CharacterInfo>(1);
        this.opponents = new ArrayList<CharacterInfo>(1);
    }

    public void Destroy() {
        Luascript.DropShared(this);

        this.players.Destroy();
        this.opponents.Destroy();
        //free(this);
    }


    public bool AddOpponent(PlayerStats playerstats, bool can_recover, bool can_die) {
        bool success = InternalAdd(
            this.opponents, this.players, playerstats, can_recover, can_die
        );
        if (success) {
            playerstats.SetHealth(0.0);
        }
        return success;
    }

    public bool AddPlayer(PlayerStats playerstats, bool can_recover, bool can_die) {
        bool success = InternalAdd(
            this.players, this.opponents, playerstats, can_recover, can_die
        );
        if (success) {
            playerstats.SetHealth(playerstats.GetMaximumHealth() / 2.0);
        }
        return success;
    }


    public int HasDeads(bool in_players_or_opponents) {
        ArrayList<CharacterInfo> arraylist = in_players_or_opponents ? this.players : this.opponents;
        int deads = 0;

        foreach (CharacterInfo character in arraylist) {
            if (character.playerstats.IsDead()) deads++;
        }

        return deads;
    }


    public bool EnableDead(PlayerStats playerstats, bool can_die) {
        CharacterInfo character = InternalGetCharacter(playerstats);
        if (character == null) return false;

        character.can_die = can_die;
        return true;
    }

    public bool EnableRecover(PlayerStats playerstats, bool can_recover) {
        CharacterInfo character = InternalGetCharacter(playerstats);
        if (character == null) return false;
        playerstats.EnableHealthRecover(can_recover);
        return true;
    }

    public void Clear() {
        this.opponents.Clear();
        this.players.Clear();
    }

    public void Balance(HealthBar healthbar) {
        double opponents_total = 0.0;
        double accumulated = 0.0;
        double maximum = 0.0;
        int players_count = this.players.Size();

        foreach (CharacterInfo character in this.opponents) {
            double health = character.playerstats.GetHealth();
            if (health > 0.0) opponents_total += health;

            if (character.can_die && health < 0.0)
                character.playerstats.Kill();
            else if (players_count > 0)
                character.playerstats.SetHealth(0.0);

            if (players_count < 1) maximum += character.playerstats.GetMaximumHealth();
        }

        if (players_count > 0) {
            double amount = (-opponents_total) / players_count;

            foreach (CharacterInfo character in this.players) {
                if (amount != 0.0)
                    character.playerstats.AddHealth(amount, character.can_die);

                if (character.can_die)
                    character.playerstats.KillIfNegativeHealth();
                else
                    character.playerstats.Raise(false);

                accumulated += character.playerstats.GetHealth();
                maximum += character.playerstats.GetMaximumHealth();
            }
        }

        if (healthbar != null) {
            bool opponents_recover = opponents_total > 0;

            // Note: originally was "if (players_count < 0)" change if something breaks
            if (players_count < 1) opponents_recover = !opponents_recover;

            healthbar.SetHealthPosition((float)maximum, (float)accumulated, opponents_recover);
        }
    }

    public void ResetOpponents() {
        foreach (CharacterInfo character in this.opponents) {
            character.playerstats.SetHealth(0);
        }
    }




    private bool InternalAdd(ArrayList<CharacterInfo> arraylist1, ArrayList<CharacterInfo> arraylist2, PlayerStats playerstats, bool can_recover, bool can_die) {
        foreach (CharacterInfo character in arraylist1) {
            if (character.playerstats == playerstats) return false;
        }
        foreach (CharacterInfo character in arraylist2) {
            if (character.playerstats == playerstats) return false;
        }
        arraylist1.Add(new CharacterInfo() { playerstats = playerstats, can_die = can_die });
        playerstats.EnableHealthRecover(can_recover);
        return true;
    }

    private CharacterInfo InternalGetCharacter(PlayerStats playerstats) {
        foreach (CharacterInfo character in this.players) {
            if (character.playerstats == playerstats) return character;
        }
        foreach (CharacterInfo character in this.opponents) {
            if (character.playerstats == playerstats) return character;
        }
        return null;
    }

    private class CharacterInfo {
        public PlayerStats playerstats;
        public bool can_die;
    }

}
