namespace PDBManager
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
      this.components = new System.ComponentModel.Container();
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
      this.menuMain = new System.Windows.Forms.MenuStrip();
      this.menuMain_DataBase = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_ConnDatabase = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_CloseDataBase = new System.Windows.Forms.ToolStripMenuItem();
      this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
      this.menuMain_Exit = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_Table = new System.Windows.Forms.ToolStripMenuItem();
      this.menuTable_newTable = new System.Windows.Forms.ToolStripMenuItem();
      this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
      this.menuTable_delTable = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_detach = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_Query = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_newQuery = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_help = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain_about = new System.Windows.Forms.ToolStripMenuItem();
      this.statusStrip1 = new System.Windows.Forms.StatusStrip();
      this.statusLbl1 = new System.Windows.Forms.ToolStripStatusLabel();
      this.statusLbl_Status = new System.Windows.Forms.ToolStripStatusLabel();
      this.statusLbl3 = new System.Windows.Forms.ToolStripStatusLabel();
      this.statusLbl_localTime = new System.Windows.Forms.ToolStripStatusLabel();
      this.splitMain = new System.Windows.Forms.SplitContainer();
      this.tvNav = new System.Windows.Forms.TreeView();
      this.imgListNav = new System.Windows.Forms.ImageList(this.components);
      this.stripNavgation = new System.Windows.Forms.ToolStrip();
      this.stripNav_Connect = new System.Windows.Forms.ToolStripButton();
      this.stripNav_DisConnect = new System.Windows.Forms.ToolStripButton();
      this.stripNav_Refresh = new System.Windows.Forms.ToolStripButton();
      this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
      this.stripNav_NewTable = new System.Windows.Forms.ToolStripButton();
      this.stripNav_NewQuery = new System.Windows.Forms.ToolStripButton();
      this.tabMain = new System.Windows.Forms.TabControl();
      this.imgListTab = new System.Windows.Forms.ImageList(this.components);
      this.menuStripNav = new System.Windows.Forms.ContextMenuStrip(this.components);
      this.MenuItem_newTable = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_newQuery = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_selectTop1000 = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_delTable = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_detachTable = new System.Windows.Forms.ToolStripMenuItem();
      this.timerState = new System.Windows.Forms.Timer(this.components);
      this.menuStripSystem = new System.Windows.Forms.ContextMenuStrip(this.components);
      this.MenuItem_Query = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_select1000 = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuStripDatabase = new System.Windows.Forms.ContextMenuStrip(this.components);
      this.MenuItem_connect = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_disconnect = new System.Windows.Forms.ToolStripMenuItem();
      this.MenuItem_refresh = new System.Windows.Forms.ToolStripMenuItem();
      this.menuMain.SuspendLayout();
      this.statusStrip1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.splitMain)).BeginInit();
      this.splitMain.Panel1.SuspendLayout();
      this.splitMain.Panel2.SuspendLayout();
      this.splitMain.SuspendLayout();
      this.stripNavgation.SuspendLayout();
      this.menuStripNav.SuspendLayout();
      this.menuStripSystem.SuspendLayout();
      this.MenuStripDatabase.SuspendLayout();
      this.SuspendLayout();
      // 
      // menuMain
      // 
      this.menuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_DataBase,
            this.menuMain_Table,
            this.menuMain_Query,
            this.menuMain_help});
      this.menuMain.Location = new System.Drawing.Point(0, 0);
      this.menuMain.Name = "menuMain";
      this.menuMain.ShowItemToolTips = true;
      this.menuMain.Size = new System.Drawing.Size(784, 25);
      this.menuMain.TabIndex = 0;
      this.menuMain.Text = "menuStrip1";
      // 
      // menuMain_DataBase
      // 
      this.menuMain_DataBase.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_ConnDatabase,
            this.menuMain_CloseDataBase,
            this.toolStripSeparator1,
            this.menuMain_Exit});
      this.menuMain_DataBase.Name = "menuMain_DataBase";
      this.menuMain_DataBase.Size = new System.Drawing.Size(56, 21);
      this.menuMain_DataBase.Text = "数据库";
      // 
      // menuMain_ConnDatabase
      // 
      this.menuMain_ConnDatabase.Image = global::PDBManager.Properties.Resources.database_new;
      this.menuMain_ConnDatabase.Name = "menuMain_ConnDatabase";
      this.menuMain_ConnDatabase.Size = new System.Drawing.Size(136, 22);
      this.menuMain_ConnDatabase.Text = "连接数据库";
      this.menuMain_ConnDatabase.Click += new System.EventHandler(this.menuMain_ConnDatabase_Click);
      // 
      // menuMain_CloseDataBase
      // 
      this.menuMain_CloseDataBase.Image = global::PDBManager.Properties.Resources.database_close;
      this.menuMain_CloseDataBase.Name = "menuMain_CloseDataBase";
      this.menuMain_CloseDataBase.Size = new System.Drawing.Size(136, 22);
      this.menuMain_CloseDataBase.Text = "断开数据库";
      this.menuMain_CloseDataBase.Click += new System.EventHandler(this.menuMain_CloseDataBase_Click);
      // 
      // toolStripSeparator1
      // 
      this.toolStripSeparator1.Name = "toolStripSeparator1";
      this.toolStripSeparator1.Size = new System.Drawing.Size(133, 6);
      // 
      // menuMain_Exit
      // 
      this.menuMain_Exit.Image = global::PDBManager.Properties.Resources.database_exit;
      this.menuMain_Exit.Name = "menuMain_Exit";
      this.menuMain_Exit.Size = new System.Drawing.Size(136, 22);
      this.menuMain_Exit.Text = "退出";
      this.menuMain_Exit.Click += new System.EventHandler(this.menuMain_Exit_Click);
      // 
      // menuMain_Table
      // 
      this.menuMain_Table.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuTable_newTable,
            this.toolStripSeparator2,
            this.menuTable_delTable,
            this.MenuItem_detach});
      this.menuMain_Table.Name = "menuMain_Table";
      this.menuMain_Table.Size = new System.Drawing.Size(32, 21);
      this.menuMain_Table.Text = "表";
      // 
      // menuTable_newTable
      // 
      this.menuTable_newTable.Image = global::PDBManager.Properties.Resources.table_new;
      this.menuTable_newTable.Name = "menuTable_newTable";
      this.menuTable_newTable.Size = new System.Drawing.Size(112, 22);
      this.menuTable_newTable.Text = "新建表";
      this.menuTable_newTable.Click += new System.EventHandler(this.menuTable_newTable_Click);
      // 
      // toolStripSeparator2
      // 
      this.toolStripSeparator2.Name = "toolStripSeparator2";
      this.toolStripSeparator2.Size = new System.Drawing.Size(109, 6);
      // 
      // menuTable_delTable
      // 
      this.menuTable_delTable.Image = global::PDBManager.Properties.Resources.error;
      this.menuTable_delTable.Name = "menuTable_delTable";
      this.menuTable_delTable.Size = new System.Drawing.Size(112, 22);
      this.menuTable_delTable.Text = "删除表";
      this.menuTable_delTable.Click += new System.EventHandler(this.menuTable_delTable_Click);
      // 
      // MenuItem_detach
      // 
      this.MenuItem_detach.Image = global::PDBManager.Properties.Resources.plus_ident;
      this.MenuItem_detach.Name = "MenuItem_detach";
      this.MenuItem_detach.Size = new System.Drawing.Size(112, 22);
      this.MenuItem_detach.Text = "分离表";
      this.MenuItem_detach.Click += new System.EventHandler(this.MenuItem_detach_Click);
      // 
      // menuMain_Query
      // 
      this.menuMain_Query.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_newQuery});
      this.menuMain_Query.Name = "menuMain_Query";
      this.menuMain_Query.Size = new System.Drawing.Size(44, 21);
      this.menuMain_Query.Text = "查询";
      // 
      // menuMain_newQuery
      // 
      this.menuMain_newQuery.Image = global::PDBManager.Properties.Resources.search;
      this.menuMain_newQuery.Name = "menuMain_newQuery";
      this.menuMain_newQuery.Size = new System.Drawing.Size(124, 22);
      this.menuMain_newQuery.Text = "新建查询";
      this.menuMain_newQuery.Click += new System.EventHandler(this.menuMain_newQuery_Click);
      // 
      // menuMain_help
      // 
      this.menuMain_help.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuMain_about});
      this.menuMain_help.Name = "menuMain_help";
      this.menuMain_help.Size = new System.Drawing.Size(44, 21);
      this.menuMain_help.Text = "帮助";
      // 
      // menuMain_about
      // 
      this.menuMain_about.Image = global::PDBManager.Properties.Resources.info;
      this.menuMain_about.Name = "menuMain_about";
      this.menuMain_about.Size = new System.Drawing.Size(124, 22);
      this.menuMain_about.Text = "关于我们";
      this.menuMain_about.Click += new System.EventHandler(this.menuMain_about_Click);
      // 
      // statusStrip1
      // 
      this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusLbl1,
            this.statusLbl_Status,
            this.statusLbl3,
            this.statusLbl_localTime});
      this.statusStrip1.Location = new System.Drawing.Point(0, 540);
      this.statusStrip1.Name = "statusStrip1";
      this.statusStrip1.Size = new System.Drawing.Size(784, 22);
      this.statusStrip1.TabIndex = 1;
      this.statusStrip1.Text = "statusStrip1";
      // 
      // statusLbl1
      // 
      this.statusLbl1.Name = "statusLbl1";
      this.statusLbl1.Size = new System.Drawing.Size(35, 17);
      this.statusLbl1.Text = "状态:";
      // 
      // statusLbl_Status
      // 
      this.statusLbl_Status.ForeColor = System.Drawing.Color.DarkGreen;
      this.statusLbl_Status.Name = "statusLbl_Status";
      this.statusLbl_Status.Size = new System.Drawing.Size(138, 17);
      this.statusLbl_Status.Text = "已连接，127.0.0.1:8105";
      // 
      // statusLbl3
      // 
      this.statusLbl3.Name = "statusLbl3";
      this.statusLbl3.Size = new System.Drawing.Size(59, 17);
      this.statusLbl3.Text = "本地时间:";
      // 
      // statusLbl_localTime
      // 
      this.statusLbl_localTime.Name = "statusLbl_localTime";
      this.statusLbl_localTime.Size = new System.Drawing.Size(126, 17);
      this.statusLbl_localTime.Text = "2019-01-01 12:00:00";
      // 
      // splitMain
      // 
      this.splitMain.Dock = System.Windows.Forms.DockStyle.Fill;
      this.splitMain.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
      this.splitMain.Location = new System.Drawing.Point(0, 25);
      this.splitMain.Name = "splitMain";
      // 
      // splitMain.Panel1
      // 
      this.splitMain.Panel1.Controls.Add(this.tvNav);
      this.splitMain.Panel1.Controls.Add(this.stripNavgation);
      // 
      // splitMain.Panel2
      // 
      this.splitMain.Panel2.Controls.Add(this.tabMain);
      this.splitMain.Size = new System.Drawing.Size(784, 515);
      this.splitMain.SplitterDistance = 231;
      this.splitMain.SplitterWidth = 12;
      this.splitMain.TabIndex = 2;
      // 
      // tvNav
      // 
      this.tvNav.Dock = System.Windows.Forms.DockStyle.Fill;
      this.tvNav.ImageIndex = 0;
      this.tvNav.ImageList = this.imgListNav;
      this.tvNav.Location = new System.Drawing.Point(0, 25);
      this.tvNav.Name = "tvNav";
      this.tvNav.SelectedImageIndex = 0;
      this.tvNav.Size = new System.Drawing.Size(231, 490);
      this.tvNav.TabIndex = 1;
      this.tvNav.MouseDown += new System.Windows.Forms.MouseEventHandler(this.tvNav_MouseDown);
      // 
      // imgListNav
      // 
      this.imgListNav.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgListNav.ImageStream")));
      this.imgListNav.TransparentColor = System.Drawing.Color.Transparent;
      this.imgListNav.Images.SetKeyName(0, "database.png");
      this.imgListNav.Images.SetKeyName(1, "info.png");
      this.imgListNav.Images.SetKeyName(2, "db_field.png");
      this.imgListNav.Images.SetKeyName(3, "db_field_key.png");
      this.imgListNav.Images.SetKeyName(4, "db_table.png");
      this.imgListNav.Images.SetKeyName(5, "folder.png");
      // 
      // stripNavgation
      // 
      this.stripNavgation.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.stripNav_Connect,
            this.stripNav_DisConnect,
            this.stripNav_Refresh,
            this.toolStripSeparator4,
            this.stripNav_NewTable,
            this.stripNav_NewQuery});
      this.stripNavgation.Location = new System.Drawing.Point(0, 0);
      this.stripNavgation.Name = "stripNavgation";
      this.stripNavgation.Size = new System.Drawing.Size(231, 25);
      this.stripNavgation.TabIndex = 0;
      this.stripNavgation.Text = "toolStrip1";
      // 
      // stripNav_Connect
      // 
      this.stripNav_Connect.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.stripNav_Connect.Image = global::PDBManager.Properties.Resources.db_connect;
      this.stripNav_Connect.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.stripNav_Connect.Name = "stripNav_Connect";
      this.stripNav_Connect.Size = new System.Drawing.Size(23, 22);
      this.stripNav_Connect.Text = "连接数据库";
      this.stripNav_Connect.Click += new System.EventHandler(this.stripNav_Connect_Click);
      // 
      // stripNav_DisConnect
      // 
      this.stripNav_DisConnect.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.stripNav_DisConnect.Image = global::PDBManager.Properties.Resources.db_disconnect;
      this.stripNav_DisConnect.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.stripNav_DisConnect.Name = "stripNav_DisConnect";
      this.stripNav_DisConnect.Size = new System.Drawing.Size(23, 22);
      this.stripNav_DisConnect.Text = "断开连接";
      this.stripNav_DisConnect.Click += new System.EventHandler(this.stripNav_DisConnect_Click);
      // 
      // stripNav_Refresh
      // 
      this.stripNav_Refresh.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.stripNav_Refresh.Image = global::PDBManager.Properties.Resources.db_refresh;
      this.stripNav_Refresh.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.stripNav_Refresh.Name = "stripNav_Refresh";
      this.stripNav_Refresh.Size = new System.Drawing.Size(23, 22);
      this.stripNav_Refresh.Text = "刷新";
      this.stripNav_Refresh.Click += new System.EventHandler(this.stripNav_Refresh_Click);
      // 
      // toolStripSeparator4
      // 
      this.toolStripSeparator4.Name = "toolStripSeparator4";
      this.toolStripSeparator4.Size = new System.Drawing.Size(6, 25);
      // 
      // stripNav_NewTable
      // 
      this.stripNav_NewTable.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.stripNav_NewTable.Image = global::PDBManager.Properties.Resources.table_new;
      this.stripNav_NewTable.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.stripNav_NewTable.Name = "stripNav_NewTable";
      this.stripNav_NewTable.Size = new System.Drawing.Size(23, 22);
      this.stripNav_NewTable.Text = "新建表";
      this.stripNav_NewTable.Click += new System.EventHandler(this.stripNav_NewTable_Click);
      // 
      // stripNav_NewQuery
      // 
      this.stripNav_NewQuery.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.stripNav_NewQuery.Image = global::PDBManager.Properties.Resources.search;
      this.stripNav_NewQuery.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.stripNav_NewQuery.Name = "stripNav_NewQuery";
      this.stripNav_NewQuery.Size = new System.Drawing.Size(23, 22);
      this.stripNav_NewQuery.Text = "新建查询";
      this.stripNav_NewQuery.Click += new System.EventHandler(this.stripNav_NewQuery_Click);
      // 
      // tabMain
      // 
      this.tabMain.Dock = System.Windows.Forms.DockStyle.Fill;
      this.tabMain.DrawMode = System.Windows.Forms.TabDrawMode.OwnerDrawFixed;
      this.tabMain.HotTrack = true;
      this.tabMain.ImageList = this.imgListTab;
      this.tabMain.ItemSize = new System.Drawing.Size(80, 19);
      this.tabMain.Location = new System.Drawing.Point(0, 0);
      this.tabMain.Name = "tabMain";
      this.tabMain.SelectedIndex = 0;
      this.tabMain.Size = new System.Drawing.Size(541, 515);
      this.tabMain.TabIndex = 0;
      this.tabMain.DrawItem += new System.Windows.Forms.DrawItemEventHandler(this.tabMain_DrawItem);
      this.tabMain.MouseClick += new System.Windows.Forms.MouseEventHandler(this.tabMain_MouseClick);
      // 
      // imgListTab
      // 
      this.imgListTab.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgListTab.ImageStream")));
      this.imgListTab.TransparentColor = System.Drawing.Color.Transparent;
      this.imgListTab.Images.SetKeyName(0, "info.png");
      this.imgListTab.Images.SetKeyName(1, "search.png");
      this.imgListTab.Images.SetKeyName(2, "table_new.png");
      // 
      // menuStripNav
      // 
      this.menuStripNav.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuItem_newTable,
            this.MenuItem_newQuery,
            this.MenuItem_selectTop1000,
            this.MenuItem_delTable,
            this.MenuItem_detachTable});
      this.menuStripNav.Name = "menuStripNav";
      this.menuStripNav.Size = new System.Drawing.Size(153, 114);
      // 
      // MenuItem_newTable
      // 
      this.MenuItem_newTable.ForeColor = System.Drawing.SystemColors.ControlText;
      this.MenuItem_newTable.ImageTransparentColor = System.Drawing.SystemColors.ActiveCaption;
      this.MenuItem_newTable.Name = "MenuItem_newTable";
      this.MenuItem_newTable.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_newTable.Text = "新建表";
      this.MenuItem_newTable.Click += new System.EventHandler(this.MenuItem_newTable_Click);
      // 
      // MenuItem_newQuery
      // 
      this.MenuItem_newQuery.Name = "MenuItem_newQuery";
      this.MenuItem_newQuery.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_newQuery.Text = "新建查询";
      this.MenuItem_newQuery.Click += new System.EventHandler(this.MenuItem_newQuery_Click);
      // 
      // MenuItem_selectTop1000
      // 
      this.MenuItem_selectTop1000.Name = "MenuItem_selectTop1000";
      this.MenuItem_selectTop1000.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_selectTop1000.Text = "选择前1000行";
      this.MenuItem_selectTop1000.Click += new System.EventHandler(this.MenuItem_selectTop1000_Click);
      // 
      // MenuItem_delTable
      // 
      this.MenuItem_delTable.Name = "MenuItem_delTable";
      this.MenuItem_delTable.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_delTable.Text = "删除表";
      this.MenuItem_delTable.Click += new System.EventHandler(this.MenuItem_delTable_Click);
      // 
      // MenuItem_detachTable
      // 
      this.MenuItem_detachTable.Name = "MenuItem_detachTable";
      this.MenuItem_detachTable.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_detachTable.Text = "分离表";
      this.MenuItem_detachTable.Click += new System.EventHandler(this.MenuItem_detachTable_Click);
      // 
      // timerState
      // 
      this.timerState.Interval = 1000;
      this.timerState.Tick += new System.EventHandler(this.timerState_Tick);
      // 
      // menuStripSystem
      // 
      this.menuStripSystem.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuItem_Query,
            this.MenuItem_select1000});
      this.menuStripSystem.Name = "menuStripSystem";
      this.menuStripSystem.Size = new System.Drawing.Size(153, 48);
      // 
      // MenuItem_Query
      // 
      this.MenuItem_Query.Name = "MenuItem_Query";
      this.MenuItem_Query.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_Query.Text = "新建查询";
      this.MenuItem_Query.Click += new System.EventHandler(this.MenuItem_Query_Click);
      // 
      // MenuItem_select1000
      // 
      this.MenuItem_select1000.Name = "MenuItem_select1000";
      this.MenuItem_select1000.Size = new System.Drawing.Size(152, 22);
      this.MenuItem_select1000.Text = "选择前1000行";
      this.MenuItem_select1000.Click += new System.EventHandler(this.MenuItem_select_Click);
      // 
      // MenuStripDatabase
      // 
      this.MenuStripDatabase.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.MenuItem_connect,
            this.MenuItem_disconnect,
            this.MenuItem_refresh});
      this.MenuStripDatabase.Name = "MenuStripDatabase";
      this.MenuStripDatabase.Size = new System.Drawing.Size(137, 70);
      // 
      // MenuItem_connect
      // 
      this.MenuItem_connect.Name = "MenuItem_connect";
      this.MenuItem_connect.Size = new System.Drawing.Size(136, 22);
      this.MenuItem_connect.Text = "连接数据库";
      this.MenuItem_connect.Click += new System.EventHandler(this.MenuItem_connect_Click);
      // 
      // MenuItem_disconnect
      // 
      this.MenuItem_disconnect.Name = "MenuItem_disconnect";
      this.MenuItem_disconnect.Size = new System.Drawing.Size(136, 22);
      this.MenuItem_disconnect.Text = "断开连接";
      this.MenuItem_disconnect.Click += new System.EventHandler(this.MenuItem_disconnect_Click);
      // 
      // MenuItem_refresh
      // 
      this.MenuItem_refresh.Name = "MenuItem_refresh";
      this.MenuItem_refresh.Size = new System.Drawing.Size(136, 22);
      this.MenuItem_refresh.Text = "刷新";
      this.MenuItem_refresh.Click += new System.EventHandler(this.MenuItem_refresh_Click);
      // 
      // MainForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(784, 562);
      this.Controls.Add(this.splitMain);
      this.Controls.Add(this.statusStrip1);
      this.Controls.Add(this.menuMain);
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MainMenuStrip = this.menuMain;
      this.Name = "MainForm";
      this.Text = "松果时序数据库(PinusDB)-管理工具                   ";
      this.Load += new System.EventHandler(this.MainForm_Load);
      this.SizeChanged += new System.EventHandler(this.MainForm_SizeChanged);
      this.menuMain.ResumeLayout(false);
      this.menuMain.PerformLayout();
      this.statusStrip1.ResumeLayout(false);
      this.statusStrip1.PerformLayout();
      this.splitMain.Panel1.ResumeLayout(false);
      this.splitMain.Panel1.PerformLayout();
      this.splitMain.Panel2.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)(this.splitMain)).EndInit();
      this.splitMain.ResumeLayout(false);
      this.stripNavgation.ResumeLayout(false);
      this.stripNavgation.PerformLayout();
      this.menuStripNav.ResumeLayout(false);
      this.menuStripSystem.ResumeLayout(false);
      this.MenuStripDatabase.ResumeLayout(false);
      this.ResumeLayout(false);
      this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuMain;
        private System.Windows.Forms.ToolStripMenuItem menuMain_DataBase;
        private System.Windows.Forms.ToolStripMenuItem menuMain_ConnDatabase;
        private System.Windows.Forms.ToolStripMenuItem menuMain_CloseDataBase;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem menuMain_Exit;
        private System.Windows.Forms.ToolStripMenuItem menuMain_Table;
        private System.Windows.Forms.ToolStripMenuItem menuTable_newTable;
        private System.Windows.Forms.ToolStripMenuItem menuMain_Query;
        private System.Windows.Forms.ToolStripMenuItem menuMain_newQuery;
        private System.Windows.Forms.ToolStripMenuItem menuMain_help;
        private System.Windows.Forms.ToolStripMenuItem menuMain_about;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel statusLbl1;
        private System.Windows.Forms.ToolStripStatusLabel statusLbl_Status;
        private System.Windows.Forms.ToolStripStatusLabel statusLbl3;
        private System.Windows.Forms.ToolStripStatusLabel statusLbl_localTime;
        private System.Windows.Forms.SplitContainer splitMain;
        private System.Windows.Forms.ToolStrip stripNavgation;
        private System.Windows.Forms.ToolStripButton stripNav_Connect;
        private System.Windows.Forms.ToolStripButton stripNav_DisConnect;
        private System.Windows.Forms.ToolStripButton stripNav_NewTable;
        private System.Windows.Forms.ToolStripButton stripNav_NewQuery;
        private System.Windows.Forms.TreeView tvNav;
        private System.Windows.Forms.TabControl tabMain;
        private System.Windows.Forms.ImageList imgListNav;
        private System.Windows.Forms.ImageList imgListTab;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem menuTable_delTable;
        private System.Windows.Forms.ContextMenuStrip menuStripNav;
        private System.Windows.Forms.ToolStripMenuItem MenuItem_newQuery;
        private System.Windows.Forms.ToolStripMenuItem MenuItem_delTable;
    private System.Windows.Forms.Timer timerState;
    private System.Windows.Forms.ToolStripButton stripNav_Refresh;
    private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_newTable;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_detachTable;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_detach;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_selectTop1000;
    private System.Windows.Forms.ContextMenuStrip menuStripSystem;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_select1000;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_Query;
    private System.Windows.Forms.ContextMenuStrip MenuStripDatabase;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_connect;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_disconnect;
    private System.Windows.Forms.ToolStripMenuItem MenuItem_refresh;
  }
}