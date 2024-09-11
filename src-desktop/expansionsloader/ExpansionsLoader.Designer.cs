namespace CsharpWrapperExpansionsLoader
{
    partial class ExpansionsLoader
    {
        /// <summary>
        /// Variable del diseñador necesaria.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Limpiar los recursos que se estén usando.
        /// </summary>
        /// <param name="disposing">true si los recursos administrados se deben desechar; false en caso contrario.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Código generado por el Diseñador de Windows Forms

        /// <summary>
        /// Método necesario para admitir el Diseñador. No se puede modificar
        /// el contenido de este método con el editor de código.
        /// </summary>
        private void InitializeComponent()
        {
            this.listView = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label_submiter = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.pictureBox_screenshoot = new System.Windows.Forms.PictureBox();
            this.button_launch = new System.Windows.Forms.Button();
            this.button_refreshList = new System.Windows.Forms.Button();
            this.button_openExpansionsFolder = new System.Windows.Forms.Button();
            this.richTextBox_description = new System.Windows.Forms.RichTextBox();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox_screenshoot)).BeginInit();
            this.SuspendLayout();
            // 
            // listView
            // 
            this.listView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
            | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.listView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
            this.listView.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listView.HideSelection = false;
            this.listView.Location = new System.Drawing.Point(12, 25);
            this.listView.MultiSelect = false;
            this.listView.Name = "listView";
            this.listView.ShowGroups = false;
            this.listView.Size = new System.Drawing.Size(380, 404);
            this.listView.TabIndex = 0;
            this.listView.UseCompatibleStateImageBehavior = false;
            this.listView.View = System.Windows.Forms.View.Details;
            this.listView.FullRowSelect = true;
            this.listView.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
            this.listView.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listView1_DoubleClick);
            this.listView.KeyUp += new System.Windows.Forms.KeyEventHandler(this.listView1_KeyUp);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Version";
            this.columnHeader1.Width = 160;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Name";
            this.columnHeader2.Width = 250;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Installed expansions:";
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            /*| System.Windows.Forms.AnchorStyles.Left*/) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.richTextBox_description);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label_submiter);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.pictureBox_screenshoot);
            this.groupBox1.Location = new System.Drawing.Point(398, 25);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(428, 375);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Selected expansion";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(7, 288);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(75, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Description:";
            // 
            // label_submiter
            // 
            this.label_submiter.AutoSize = true;
            this.label_submiter.Location = new System.Drawing.Point(73, 259);
            this.label_submiter.Name = "label_submiter";
            this.label_submiter.Size = new System.Drawing.Size(0, 13);
            this.label_submiter.TabIndex = 3;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(7, 259);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(60, 13);
            this.label4.TabIndex = 2;
            this.label4.Text = "Submiter:";
            // 
            // pictureBox_screenshoot
            // 
            this.pictureBox_screenshoot.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.pictureBox_screenshoot.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Center;
            this.pictureBox_screenshoot.Location = new System.Drawing.Point(10, 19);
            this.pictureBox_screenshoot.Name = "pictureBox_screenshoot";
            this.pictureBox_screenshoot.Size = new System.Drawing.Size(412, 224);
            this.pictureBox_screenshoot.TabIndex = 0;
            this.pictureBox_screenshoot.TabStop = false;
            this.pictureBox_screenshoot.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            // 
            // button_launch
            // 
            this.button_launch.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.button_launch.Location = new System.Drawing.Point(398, 406);
            this.button_launch.Name = "button_launch";
            this.button_launch.Size = new System.Drawing.Size(138, 23);
            this.button_launch.TabIndex = 3;
            this.button_launch.Text = "Launch";
            this.button_launch.UseVisualStyleBackColor = true;
            this.button_launch.Click += new System.EventHandler(this.button_launch_Click);
            // 
            // button_refreshList
            // 
            this.button_refreshList.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.button_refreshList.Location = new System.Drawing.Point(543, 406);
            this.button_refreshList.Name = "button_refreshList";
            this.button_refreshList.Size = new System.Drawing.Size(139, 23);
            this.button_refreshList.TabIndex = 4;
            this.button_refreshList.Text = "Refresh list";
            this.button_refreshList.UseVisualStyleBackColor = true;
            this.button_refreshList.Click += new System.EventHandler(this.button_refreshList_Click);
            // 
            // button_openExpansionsFolder
            // 
            this.button_openExpansionsFolder.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.button_openExpansionsFolder.Location = new System.Drawing.Point(689, 406);
            this.button_openExpansionsFolder.Name = "button_openExpansionsFolder";
            this.button_openExpansionsFolder.Size = new System.Drawing.Size(138, 23);
            this.button_openExpansionsFolder.TabIndex = 5;
            this.button_openExpansionsFolder.Text = "Open expansions folder";
            this.button_openExpansionsFolder.UseVisualStyleBackColor = true;
            this.button_openExpansionsFolder.Click += new System.EventHandler(this.button_openExpansionsFolder_Click);
            // 
            // textBox_description
            // 
            this.richTextBox_description.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBox_description.Location = new System.Drawing.Point(6, 304);
            this.richTextBox_description.Multiline = true;
            this.richTextBox_description.Name = "richTextBox_description";
            this.richTextBox_description.ReadOnly = true;
            this.richTextBox_description.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.Both;
            this.richTextBox_description.DetectUrls = true;
            this.richTextBox_description.Size = new System.Drawing.Size(416, 65);
            this.richTextBox_description.TabIndex = 5;
            this.richTextBox_description.LinkClicked += new System.Windows.Forms.LinkClickedEventHandler(richTextBox_description_LinkClicked);
            // 
            // ExpansionsLoader
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(838, 441);
            this.Controls.Add(this.button_openExpansionsFolder);
            this.Controls.Add(this.button_refreshList);
            this.Controls.Add(this.button_launch);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.listView);
            this.Name = "ExpansionsLoader";
            this.Text = "Expansions loader";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox_screenshoot)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView listView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.PictureBox pictureBox_screenshoot;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label_submiter;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button button_launch;
        private System.Windows.Forms.Button button_refreshList;
        private System.Windows.Forms.Button button_openExpansionsFolder;
        private System.Windows.Forms.RichTextBox richTextBox_description;
    }
}

