namespace UIWindow
{
    partial class UIWindow
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
            this.timerProcessMessages = new System.Windows.Forms.Timer(this.components);
            this.timerInitialize = new System.Windows.Forms.Timer(this.components);
            this.timerDisconnectCheck = new System.Windows.Forms.Timer(this.components);
            this.checkBoxBBoxes = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.modeField = new System.Windows.Forms.RadioButton();
            this.helpBox = new System.Windows.Forms.GroupBox();
            this.textBoxCommands = new System.Windows.Forms.TextBox();
            this.buttonViewMax = new System.Windows.Forms.Button();
            this.checkBoxScan = new System.Windows.Forms.CheckBox();
            this.checkBoxPlanes = new System.Windows.Forms.CheckBox();
            this.checkBoxHeatMap = new System.Windows.Forms.CheckBox();
            this.checkBoxScene = new System.Windows.Forms.CheckBox();
            this.checkBoxScanColumns = new System.Windows.Forms.CheckBox();
            this.checkBoxShowAgents = new System.Windows.Forms.CheckBox();
            this.checkBoxCollisionMesh = new System.Windows.Forms.CheckBox();
            this.groupBox1.SuspendLayout();
            this.helpBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // timerProcessMessages
            // 
            this.timerProcessMessages.Enabled = true;
            this.timerProcessMessages.Interval = 1;
            this.timerProcessMessages.Tick += new System.EventHandler(this.timerProcessMessages_Tick);
            // 
            // timerInitialize
            // 
            this.timerInitialize.Enabled = true;
            this.timerInitialize.Tick += new System.EventHandler(this.timerInitialize_Tick);
            // 
            // timerDisconnectCheck
            // 
            this.timerDisconnectCheck.Enabled = true;
            this.timerDisconnectCheck.Interval = 500;
            this.timerDisconnectCheck.Tick += new System.EventHandler(this.timerDisconnectCheck_Tick);
            // 
            // checkBoxBBoxes
            // 
            this.checkBoxBBoxes.AutoSize = true;
            this.checkBoxBBoxes.Location = new System.Drawing.Point(151, 9);
            this.checkBoxBBoxes.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxBBoxes.Name = "checkBoxBBoxes";
            this.checkBoxBBoxes.Size = new System.Drawing.Size(65, 19);
            this.checkBoxBBoxes.TabIndex = 2;
            this.checkBoxBBoxes.Text = "BBoxes";
            this.checkBoxBBoxes.UseVisualStyleBackColor = true;
            this.checkBoxBBoxes.CheckedChanged += new System.EventHandler(this.checkBoxBBoxes_CheckedChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.modeField);
            this.groupBox1.Location = new System.Drawing.Point(12, 11);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(133, 132);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Mode";
            // 
            // modeField
            // 
            this.modeField.AutoSize = true;
            this.modeField.Location = new System.Drawing.Point(6, 22);
            this.modeField.Name = "modeField";
            this.modeField.Size = new System.Drawing.Size(87, 19);
            this.modeField.TabIndex = 0;
            this.modeField.TabStop = true;
            this.modeField.Text = "View Fields";
            this.modeField.UseVisualStyleBackColor = true;
            this.modeField.CheckedChanged += new System.EventHandler(this.modeAgentPlacement_CheckedChanged);
            // 
            // helpBox
            // 
            this.helpBox.Controls.Add(this.textBoxCommands);
            this.helpBox.Location = new System.Drawing.Point(151, 81);
            this.helpBox.Name = "helpBox";
            this.helpBox.Size = new System.Drawing.Size(247, 166);
            this.helpBox.TabIndex = 4;
            this.helpBox.TabStop = false;
            this.helpBox.Text = "Commands";
            // 
            // textBoxCommands
            // 
            this.textBoxCommands.BackColor = System.Drawing.SystemColors.Control;
            this.textBoxCommands.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBoxCommands.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textBoxCommands.Location = new System.Drawing.Point(8, 21);
            this.textBoxCommands.Multiline = true;
            this.textBoxCommands.Name = "textBoxCommands";
            this.textBoxCommands.Size = new System.Drawing.Size(233, 130);
            this.textBoxCommands.TabIndex = 0;
            // 
            // buttonViewMax
            // 
            this.buttonViewMax.Location = new System.Drawing.Point(357, 5);
            this.buttonViewMax.Name = "buttonViewMax";
            this.buttonViewMax.Size = new System.Drawing.Size(97, 25);
            this.buttonViewMax.TabIndex = 1;
            this.buttonViewMax.Text = "View Max";
            this.buttonViewMax.UseVisualStyleBackColor = true;
            this.buttonViewMax.Click += new System.EventHandler(this.buttonViewMax_Click);
            // 
            // checkBoxScan
            // 
            this.checkBoxScan.AutoSize = true;
            this.checkBoxScan.Checked = true;
            this.checkBoxScan.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxScan.Location = new System.Drawing.Point(260, 57);
            this.checkBoxScan.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxScan.Name = "checkBoxScan";
            this.checkBoxScan.Size = new System.Drawing.Size(52, 19);
            this.checkBoxScan.TabIndex = 2;
            this.checkBoxScan.Text = "Scan";
            this.checkBoxScan.UseVisualStyleBackColor = true;
            this.checkBoxScan.CheckedChanged += new System.EventHandler(this.checkBoxScan_CheckedChanged);
            // 
            // checkBoxPlanes
            // 
            this.checkBoxPlanes.AutoSize = true;
            this.checkBoxPlanes.Location = new System.Drawing.Point(151, 33);
            this.checkBoxPlanes.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxPlanes.Name = "checkBoxPlanes";
            this.checkBoxPlanes.Size = new System.Drawing.Size(63, 19);
            this.checkBoxPlanes.TabIndex = 2;
            this.checkBoxPlanes.Text = "Planes";
            this.checkBoxPlanes.UseVisualStyleBackColor = true;
            this.checkBoxPlanes.CheckedChanged += new System.EventHandler(this.checkBoxPlanes_CheckedChanged);
            // 
            // checkBoxHeatMap
            // 
            this.checkBoxHeatMap.AutoSize = true;
            this.checkBoxHeatMap.Location = new System.Drawing.Point(231, 34);
            this.checkBoxHeatMap.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxHeatMap.Name = "checkBoxHeatMap";
            this.checkBoxHeatMap.Size = new System.Drawing.Size(75, 19);
            this.checkBoxHeatMap.TabIndex = 2;
            this.checkBoxHeatMap.Text = "Heatmap";
            this.checkBoxHeatMap.UseVisualStyleBackColor = true;
            this.checkBoxHeatMap.CheckedChanged += new System.EventHandler(this.checkBoxHeatMap_CheckedChanged);
            // 
            // checkBoxScene
            // 
            this.checkBoxScene.AutoSize = true;
            this.checkBoxScene.Checked = true;
            this.checkBoxScene.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxScene.Location = new System.Drawing.Point(312, 33);
            this.checkBoxScene.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxScene.Name = "checkBoxScene";
            this.checkBoxScene.Size = new System.Drawing.Size(57, 19);
            this.checkBoxScene.TabIndex = 2;
            this.checkBoxScene.Text = "Scene";
            this.checkBoxScene.UseVisualStyleBackColor = true;
            this.checkBoxScene.CheckedChanged += new System.EventHandler(this.checkBoxScene_CheckedChanged);
            // 
            // checkBoxScanColumns
            // 
            this.checkBoxScanColumns.AutoSize = true;
            this.checkBoxScanColumns.Location = new System.Drawing.Point(151, 57);
            this.checkBoxScanColumns.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxScanColumns.Name = "checkBoxScanColumns";
            this.checkBoxScanColumns.Size = new System.Drawing.Size(103, 19);
            this.checkBoxScanColumns.TabIndex = 2;
            this.checkBoxScanColumns.Text = "Scan Columns";
            this.checkBoxScanColumns.UseVisualStyleBackColor = true;
            this.checkBoxScanColumns.CheckedChanged += new System.EventHandler(this.checkBoxScanColumns_CheckedChanged);
            // 
            // checkBoxShowAgents
            // 
            this.checkBoxShowAgents.AutoSize = true;
            this.checkBoxShowAgents.Location = new System.Drawing.Point(375, 33);
            this.checkBoxShowAgents.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxShowAgents.Name = "checkBoxShowAgents";
            this.checkBoxShowAgents.Size = new System.Drawing.Size(62, 19);
            this.checkBoxShowAgents.TabIndex = 2;
            this.checkBoxShowAgents.Text = "Agents";
            this.checkBoxShowAgents.UseVisualStyleBackColor = true;
            this.checkBoxShowAgents.CheckedChanged += new System.EventHandler(this.checkBoxShowAgents_CheckedChanged);
            // 
            // checkBoxCollisionMesh
            // 
            this.checkBoxCollisionMesh.AutoSize = true;
            this.checkBoxCollisionMesh.Location = new System.Drawing.Point(222, 9);
            this.checkBoxCollisionMesh.Name = "checkBoxCollisionMesh";
            this.checkBoxCollisionMesh.Size = new System.Drawing.Size(109, 19);
            this.checkBoxCollisionMesh.TabIndex = 5;
            this.checkBoxCollisionMesh.Text = "Collision Mesh";
            this.checkBoxCollisionMesh.UseVisualStyleBackColor = true;
            this.checkBoxCollisionMesh.CheckedChanged += new System.EventHandler(this.checkBoxCollisionMesh_CheckedChanged);
            // 
            // UIWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(466, 276);
            this.Controls.Add(this.checkBoxCollisionMesh);
            this.Controls.Add(this.buttonViewMax);
            this.Controls.Add(this.helpBox);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.checkBoxHeatMap);
            this.Controls.Add(this.checkBoxShowAgents);
            this.Controls.Add(this.checkBoxScanColumns);
            this.Controls.Add(this.checkBoxScene);
            this.Controls.Add(this.checkBoxPlanes);
            this.Controls.Add(this.checkBoxScan);
            this.Controls.Add(this.checkBoxBBoxes);
            this.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "UIWindow";
            this.Opacity = 0.8D;
            this.Text = "Waiting for connection from application...";
            this.TopMost = true;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.UIWindow_FormClosing);
            this.Load += new System.EventHandler(this.UIWindow_Load);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.UIWindow_KeyUp);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.helpBox.ResumeLayout(false);
            this.helpBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Timer timerProcessMessages;
        private System.Windows.Forms.Timer timerInitialize;
        private System.Windows.Forms.Timer timerDisconnectCheck;
        private System.Windows.Forms.CheckBox checkBoxBBoxes;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton modeField;
        private System.Windows.Forms.GroupBox helpBox;
        private System.Windows.Forms.TextBox textBoxCommands;
        private System.Windows.Forms.Button buttonViewMax;
        private System.Windows.Forms.CheckBox checkBoxScan;
        private System.Windows.Forms.CheckBox checkBoxPlanes;
        private System.Windows.Forms.CheckBox checkBoxHeatMap;
        private System.Windows.Forms.CheckBox checkBoxScene;
        private System.Windows.Forms.CheckBox checkBoxScanColumns;
        private System.Windows.Forms.CheckBox checkBoxShowAgents;
        private System.Windows.Forms.CheckBox checkBoxCollisionMesh;
    }
}

