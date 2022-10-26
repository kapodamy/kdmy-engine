using System;

namespace Engine.Utils {

    public class Tokenizer {

        private Tokenizer() { }

        private string tokens;
        private string str;
        private int length;
        private bool empty;
        private int index;
        private bool last_char_is_token;

        public static Tokenizer Init(string token_chars, bool ignore_empty_tokens, bool duplicate_string_before_init, string str) {
            if (str == null) return null;
            if (String.IsNullOrEmpty(token_chars)) throw new ArgumentException("token_chars");

            if (!duplicate_string_before_init) str = String.Intern(str);

            return new Tokenizer() {
                tokens = String.Intern(token_chars),
                str = str,
                length = str.Length,
                empty = !ignore_empty_tokens,
                index = 0,
                last_char_is_token = str.Length < 1 ? false : (token_chars.IndexOf(str[str.Length - 1]) >= 0)
            };
        }

        public void Destroy() {
            //free(tokenizer);
        }

        public int CountOccurrences() {
            int count = 0;
            int readed = 0;

            for (int i = 0 ; i < this.length ; i++) {
                char chr = this.str[i];

                if (this.tokens.IndexOf(chr) >= 0) {
                    if (readed > 0 || this.empty) {
                        readed = 0;
                        count++;
                    }
                    continue;
                }
                readed++;
            }

            if (readed > 0) count++;

            return count;
        }

        public string ReadNext() {
            if (this.index >= this.length) {
                if (
                    this.index == this.length &&
                    this.empty &&
                    this.length > 0 &&
                    this.last_char_is_token
                ) {
                    this.index++;
                    return String.Empty;
                }
                return null;
            }

            int index = this.index;
            int length = 0;

            for (; index < this.length ; index++) {
                char chr = this.str[index];
                if (this.tokens.IndexOf(chr) >= 0) {
                    if (length > 0 || this.empty) break;
                    continue;
                }
                length++;
            }

            string str_copy;

            if (length > 0) {
                int start_index = index - length;
                str_copy = this.str.SubstringKDY(start_index, index);
            } else if (this.empty) {
                str_copy = String.Empty;
            } else {
                str_copy = null;
            }

            this.index = index + 1;
            return str_copy;
        }

        public void Restart() {
            this.index = 0;
        }
    }
}