using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Engine.Utils;

namespace CsharpWrapper {

    public partial class ExpansionsLoader : Form {

        private class Expansion : IDisposable {
            public string name;
            public string directory;
            public string version = "";
            public string submiter = "";
            public string description = "";
            public Image screenshoot;
            public Image icon;
            public byte[] window_icon;
            public string window_title;

            public Expansion(string directory_name) {
                name = directory = directory_name;
            }

            public void Dispose() {
                if (screenshoot != null) {
                    screenshoot.Dispose();
                    screenshoot = null;
                }
                if (icon != null) {
                    icon.Dispose();
                    icon = null;
                }
            }

        }

        public const int ICON_WIDTH = 80;
        public const int ICON_HEIGHT = 45;


        [DllImport("uxtheme", CharSet = CharSet.Unicode)]
        private extern static int SetWindowTheme(IntPtr hWnd, string pszSubAppName, string pszSubIdList);

        private readonly string expansions_dir;
        private readonly Bitmap blank_bitmap;
        private Expansion[] expansions;
        private Expansion selected_expansion;
        private readonly ImageList imagelist;
        private bool do_launch;
        private Icon icon;

        public ExpansionsLoader() {
            expansions_dir = Path.Combine(EngineSettings.EngineDir, "expansions");
            expansions = new Expansion[0];
            imagelist = new ImageList();
            imagelist.ImageSize = new Size(ICON_WIDTH, ICON_HEIGHT);

            InitializeComponent();

            if (Environment.OSVersion.Platform == PlatformID.Win32NT && Environment.OSVersion.Version.Major >= 6) {
                SetWindowTheme(this.listView.Handle, "explorer", null);
            }

            listView.SmallImageList = imagelist;

            blank_bitmap = new Bitmap(ICON_WIDTH, ICON_HEIGHT, PixelFormat.Format32bppArgb);
            using (Graphics graphics = Graphics.FromImage(blank_bitmap)) {
                graphics.Clear(Color.Transparent);
            }

            using (MemoryStream stream = new MemoryStream(Engine.Properties.Resources.icon)) {
                icon = new Icon(stream);
            }
            Icon = icon;
        }

        public string SelectedExpansionDirectory { get => selected_expansion.directory; }

        public byte[] SelectedExpansionWindowIcon { get => selected_expansion.window_icon; }

        public string SelectedExpansionWindowTitle { get => selected_expansion.window_title; }

        public bool NotLaunchAndExit { get => !do_launch; }

        public void Destroy() {
            foreach (Expansion expansion in expansions) expansion.Dispose();
            blank_bitmap.Dispose();
            imagelist.Dispose();
            icon.Dispose();

            Dispose();
        }

        protected override void OnLoad(EventArgs e) {
            base.OnShown(e);
            button_refreshList_Click(this, e);
            selected_expansion = null;
        }


        private Expansion[] LoadExpansions() {
            List<Expansion> expansions = new List<Expansion>();

            if (!Directory.Exists(expansions_dir)) return new Expansion[0];
            DirectoryInfo directory = new DirectoryInfo(expansions_dir);

            foreach (DirectoryInfo dir in directory.EnumerateDirectories()) {
                string name = dir.Name.ToLowerInvariant();

                if (name == "funkin") continue;

                Expansion expansion = new Expansion(dir.Name);

                string about_src = Path.Combine(dir.FullName, Engine.Platform.Expansions.ABOUT_FILENAME);
                if (!File.Exists(about_src)) {
                    Console.WriteLine($"ExpansionLoader: missing file {about_src}");
                    expansions.Add(expansion);
                    continue;
                }

                JSONParser json = JSONParser.LoadDirectFrom(about_src);
                if (json == null) {
                    Console.Error.WriteLine("ExpansionsLoader::LoadExpansions() can not open: " + about_src);
                    continue;
                }

                expansion.name = JSONParser.ReadString(json, "name", dir.Name);
                expansion.version = JSONParser.ReadString(json, "version", null);
                expansion.submiter = JSONParser.ReadString(json, "submiter", null);
                expansion.description = JSONParser.ReadString(json, "description", null);
                string screenshoot_path = JSONParser.ReadString(json, "screenshoot", null);
                string icon_path = JSONParser.ReadString(json, "icon", null);
                string window_icon_path = JSONParser.ReadString(json, "windowIcon", null);
                expansion.window_title = JSONParser.ReadString(json, "windowTitle", null);
                JSONParser.Destroy(json);

                if (screenshoot_path != null) {
                    screenshoot_path = Path.Combine(dir.FullName, screenshoot_path);
                    if (File.Exists(screenshoot_path)) {
                        expansion.screenshoot = Image.FromFile(screenshoot_path);
                    }
                }
                if (icon_path != null) {
                    icon_path = Path.Combine(dir.FullName, icon_path);
                    if (File.Exists(icon_path)) {
                        expansion.icon = Image.FromFile(icon_path);
                    }
                }
                if (window_icon_path != null) {
                    window_icon_path = Path.Combine(dir.FullName, window_icon_path);
                    if (File.Exists(window_icon_path)) {
                        expansion.window_icon = File.ReadAllBytes(window_icon_path);
                    } else {
                        Console.Error.WriteLine(
                            $"[WARN] ExpansionsLoader::LoadExpansios() file '{window_icon_path}' not found"
                        );
                        expansion.window_icon = null;
                    }
                }

                // TODO: markdown to RTF (https://es.wikipedia.org/wiki/Rich_Text_Format)
                if (expansion.submiter != null) {
                    expansion.submiter = expansion.submiter.Replace("\r", "").Replace("\n", "\r\n");
                }
                if (expansion.description != null) {
                    expansion.description = expansion.description.Replace("\r", "").Replace("\n", "\r\n");
                }
                if (expansion.version != null) {
                    expansion.version = expansion.version.Replace("\r", "").Replace("\n", "\r\n");
                }
                if (expansion.name != null) {
                    expansion.name = expansion.name.Replace("\r", "").Replace("\n", "\r\n");
                }

                expansions.Add(expansion);
            }

            expansions.Insert(0, new Expansion(null) {
                name = "(Launch without any expansion)",
                version = ""
            });

            return expansions.ToArray();
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e) {
            var selections = listView.SelectedIndices;
            if (selections.Count < 1) {
                this.pictureBox_screenshoot.Image = null;
                this.label_submiter.Text = null;
                this.richTextBox_description.Text = null;
                return;
            }

            Expansion expansion = expansions[selections[0]];

            this.pictureBox_screenshoot.Image = expansion.screenshoot;
            this.label_submiter.Text = expansion.submiter;
            this.richTextBox_description.Text = expansion.description;
        }

        private void listView1_DoubleClick(object sender, MouseEventArgs e) {
            ListViewHitTestInfo info = listView.HitTest(e.X, e.Y);
            ListViewItem item = info.Item;

            if (item == null) return;

            button_launch_Click(sender, EventArgs.Empty);
        }

        private void listView1_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e) {
            if (this.Focused) return;
            if (e.KeyCode != Keys.Enter) return;

            button_launch_Click(sender, EventArgs.Empty);
        }

        private void button_launch_Click(object sender, EventArgs e) {
            var selections = listView.SelectedIndices;
            if (selections.Count < 1) return;

            do_launch = true;
            selected_expansion = expansions[selections[0]];
            Close();
        }

        private void button_refreshList_Click(object sender, EventArgs e) {
            foreach (Expansion expansion in expansions) expansion.Dispose();
            expansions = LoadExpansions();

            foreach (Image thumbnail in imagelist.Images) thumbnail.Dispose();
            imagelist.Images.Clear();

            foreach (Expansion expansion in expansions) {
                Image thumbnail = CreateThumbnail(expansion.icon ?? blank_bitmap);
                imagelist.Images.Add(thumbnail);
            }

            listView.BeginUpdate();

            listView.SelectedIndices.Clear();
            listView.Items.Clear();

            for (int i = 0 ; i < expansions.Length ; i++) {
                Expansion expansion = expansions[i];
                string[] columns = new string[] { expansion.version, expansion.name };
                listView.Items.Add(new ListViewItem(columns, i));
            }
            listView.EndUpdate();
        }

        private void button_openExpansionsFolder_Click(object sender, EventArgs e) {
            Process.Start(new ProcessStartInfo() {
                FileName = expansions_dir,
                UseShellExecute = true,
                Verb = "open"
            });
        }

        private void richTextBox_description_LinkClicked(object sender, LinkClickedEventArgs e) {
            Process.Start(new ProcessStartInfo() {
                //Verb = "open",
                FileName = e.LinkText,
                UseShellExecute = true
            });
        }

        private Image CreateThumbnail(Image image) {
            Bitmap bitmap = new Bitmap(ICON_WIDTH, ICON_HEIGHT, PixelFormat.Format32bppArgb);

            using (Graphics graphics = Graphics.FromImage(bitmap)) {
                float width = (float)ICON_WIDTH / image.Width;
                float height = (float)ICON_HEIGHT / image.Height;
                float scale = width > height ? width : height;

                width = image.Width * scale;
                height = image.Height * scale;

                float x = (ICON_WIDTH - width) / 2f;
                float y = (ICON_HEIGHT - height) / 2f;

                graphics.Clear(Color.Transparent);
                graphics.InterpolationMode = InterpolationMode.HighQualityBicubic;
                graphics.CompositingQuality = CompositingQuality.HighQuality;
                graphics.DrawImage(image, x, y, width, height);
            }

            return bitmap;
        }

    }
}
