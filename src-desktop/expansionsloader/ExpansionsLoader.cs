using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace CsharpWrapper {

    public struct ExpansionInfo {
        public string name;
        public string directory;
        public string version;
        public string submiter;
        public string description;
        public string screenshoot_native_path;
        public string icon_native_path;
        public string window_icon_native_path;
        public string window_title;

        public ExpansionInfo(string directory_name) {
            this.name = this.directory = directory_name;
            this.version = "";
            this.submiter = "";
            this.description = "";
            this.screenshoot_native_path = null;
            this.icon_native_path = null;
            this.window_icon_native_path = null;
            this.window_title = null;
        }

    }

    public delegate ExpansionInfo[] QueryExpansionsCallback(out int loaded_count);


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

        private const int ICON_WIDTH = 80;
        private const int ICON_HEIGHT = 45;


        [DllImport("uxtheme", CharSet = CharSet.Unicode)]
        private extern static int SetWindowTheme(IntPtr hWnd, string pszSubAppName, string pszSubIdList);


        private readonly string expansions_dir;
        private readonly Bitmap blank_bitmap;
        private Expansion[] expansions;
        private Expansion selected_expansion;
        private readonly ImageList imagelist;
        private bool do_launch;
        private readonly Icon icon;
        private readonly QueryExpansionsCallback query_callback;


        [STAThread]
        public static string Main(byte[] formIcon, QueryExpansionsCallback queryCallback, string expansionsDir) {
            Application.SetCompatibleTextRenderingDefault(false);
            Application.EnableVisualStyles();

            using (ExpansionsLoader form = new ExpansionsLoader(formIcon, queryCallback, expansionsDir)) {

                Application.Run(form);
                Application.Exit();

                string selected_expansion_directory;

                if (form.do_launch)
                    selected_expansion_directory = form.selected_expansion.directory;
                else
                    selected_expansion_directory = null;

                return selected_expansion_directory;
            }
        }


        private ExpansionsLoader(byte[] formIcon, QueryExpansionsCallback queryCallback, string expansionsDir) {
            this.query_callback = queryCallback;
            this.expansions_dir = expansionsDir;
            this.expansions = new Expansion[0];
            this.imagelist = new ImageList();
            this.imagelist.ImageSize = new Size(ICON_WIDTH, ICON_HEIGHT);

            InitializeComponent();

            if (Environment.OSVersion.Platform == PlatformID.Win32NT && Environment.OSVersion.Version.Major >= 6) {
                SetWindowTheme(this.listView.Handle, "explorer", null);
            }

            this.listView.SmallImageList = imagelist;

            this.blank_bitmap = new Bitmap(ICON_WIDTH, ICON_HEIGHT, PixelFormat.Format32bppArgb);
            using (Graphics graphics = Graphics.FromImage(this.blank_bitmap)) {
                graphics.Clear(Color.Transparent);
            }

            using (MemoryStream stream = new MemoryStream(formIcon)) {
                icon = new Icon(stream);
            }
            Icon = icon;
        }

        public new void Dispose() {
            foreach (Expansion expansion in expansions) expansion.Dispose();
            blank_bitmap.Dispose();
            imagelist.Dispose();
            icon.Dispose();

            base.Dispose();
        }

        protected override void OnLoad(EventArgs e) {
            base.OnShown(e);
            button_refreshList_Click(this, e);
            selected_expansion = null;
        }


        private Expansion[] LoadExpansions() {
            int expansions_found_size;
            ExpansionInfo[] expansions_found = this.query_callback(out expansions_found_size);

            Expansion[] expansions = new Expansion[expansions_found_size];

            // load all icons
            for (int i = 0 ; i < expansions_found_size ; i++) {
                if (expansions_found[i].screenshoot_native_path != null) {
                    expansions[i].screenshoot = Image.FromFile(expansions_found[i].screenshoot_native_path);
                }
                if (expansions_found[i].icon_native_path != null) {
                    expansions[i].icon = Image.FromFile(expansions_found[i].icon_native_path);
                }
            }

            return expansions;
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
