using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
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

        public const int ICON_WIDTH = 32;
        public const int ICON_HEIGHT = 32;


        [DllImport("uxtheme", CharSet = CharSet.Unicode)]
        private extern static int SetWindowTheme(IntPtr hWnd, string pszSubAppName, string pszSubIdList);

        private readonly string expansions_dir;
        private readonly Bitmap blank_bitmap;
        private Expansion[] expansions;
        private string selected_expansion_name;
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

        public string SelectedExpansion { get => selected_expansion_name; }

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
            selected_expansion_name = null;
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
                expansion.name = JSONParser.ReadString(json, "name", dir.Name);
                expansion.version = JSONParser.ReadString(json, "version", null);
                expansion.submiter = JSONParser.ReadString(json, "submiter", null);
                expansion.description = JSONParser.ReadString(json, "description", null);
                string screenshoot_path = JSONParser.ReadString(json, "screenshoot", null);
                string icon_path = JSONParser.ReadString(json, "icon", null);
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
                this.textBox_description.Text = null;
                return;
            }

            Expansion expansion = expansions[selections[0]];

            this.pictureBox_screenshoot.Image = expansion.screenshoot;
            this.label_submiter.Text = expansion.submiter;
            this.textBox_description.Text = expansion.description;
        }

        private void button_launch_Click(object sender, EventArgs e) {
            var selections = listView.SelectedIndices;
            if (selections.Count < 1) return;

            do_launch = true;
            selected_expansion_name = expansions[selections[0]].directory;
            Close();
        }

        private void button_refreshList_Click(object sender, EventArgs e) {
            foreach (Expansion expansion in expansions) expansion.Dispose();
            expansions = LoadExpansions();

            imagelist.Images.Clear();
            foreach (Expansion expansion in expansions) imagelist.Images.Add(expansion.icon ?? blank_bitmap);

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

    }
}
