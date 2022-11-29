using System;
using Engine.Font;

namespace Engine.Game.Helpers {

    public class WeekSelectorWeekMSG {

        private const float DISABLED_ALPHA = 0.5f;

        private TextSprite title;
        private TextSprite message;
        private bool no_message;


        public WeekSelectorWeekMSG(Layout layout, string title, string placeholder_title_name, string placeholder_message_name) {

            this.title = layout.GetTextsprite(placeholder_title_name);
            this.message = layout.GetTextsprite(placeholder_message_name);
            this.no_message = true;

            if (this.title != null) this.title.SetTextIntern(true, title);
            Visible(false);

        }

        public void Destroy() {
            //free(this);
        }

        public void SetMessage(string message) {
            this.no_message = String.IsNullOrEmpty(message);

            if (this.title != null) this.title.SetAlpha(1.0f);

            if (!this.no_message) {
                this.message.SetTextIntern(true, message);
                this.message.SetAlpha(1.0f);
            }

            Visible(true);
        }

        public void Visible(bool visible) {
            if (visible && this.no_message) visible = false;
            if (this.title != null) this.title.SetVisible(visible);
            if (this.message != null) this.message.SetVisible(visible);
        }

        public void Disabled(bool disabled) {
            float alpha = disabled ? WeekSelectorWeekMSG.DISABLED_ALPHA : 1.0f;
            if (this.title != null) this.title.SetAlpha(alpha);
            if (this.message != null) this.message.SetAlpha(alpha);
        }

    }

}

