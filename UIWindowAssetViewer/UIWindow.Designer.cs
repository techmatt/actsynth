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
            this.label1 = new System.Windows.Forms.Label();
            this.comboBoxSceneList = new System.Windows.Forms.ComboBox();
            this.checkBoxBBoxes = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.modeResults = new System.Windows.Forms.RadioButton();
            this.modeScan = new System.Windows.Forms.RadioButton();
            this.modeScore = new System.Windows.Forms.RadioButton();
            this.modeInteractionMap = new System.Windows.Forms.RadioButton();
            this.modeAgentOwnership = new System.Windows.Forms.RadioButton();
            this.modeAgentPlacement = new System.Windows.Forms.RadioButton();
            this.helpBox = new System.Windows.Forms.GroupBox();
            this.textBoxCommands = new System.Windows.Forms.TextBox();
            this.textBoxRecategorize = new System.Windows.Forms.TextBox();
            this.buttonRecategorize = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.comboBoxResultFolderList = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.comboBoxResultsSceneList = new System.Windows.Forms.ComboBox();
            this.buttonSavePBRT = new System.Windows.Forms.Button();
            this.buttonRenderFolder = new System.Windows.Forms.Button();
            this.labelResultsFolder = new System.Windows.Forms.Label();
            this.buttonBatchPBRTRendering = new System.Windows.Forms.Button();
            this.buttonSavePBRTCamera = new System.Windows.Forms.Button();
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
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(151, 12);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(47, 15);
            this.label1.TabIndex = 0;
            this.label1.Text = "Scenes:";
            // 
            // comboBoxSceneList
            // 
            this.comboBoxSceneList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxSceneList.FormattingEnabled = true;
            this.comboBoxSceneList.Items.AddRange(new object[] {
            "a",
            "b",
            "c",
            "d"});
            this.comboBoxSceneList.Location = new System.Drawing.Point(204, 9);
            this.comboBoxSceneList.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.comboBoxSceneList.Name = "comboBoxSceneList";
            this.comboBoxSceneList.Size = new System.Drawing.Size(197, 23);
            this.comboBoxSceneList.TabIndex = 1;
            this.comboBoxSceneList.SelectedIndexChanged += new System.EventHandler(this.comboBoxSceneList_SelectedIndexChanged);
            // 
            // checkBoxBBoxes
            // 
            this.checkBoxBBoxes.AutoSize = true;
            this.checkBoxBBoxes.Location = new System.Drawing.Point(154, 36);
            this.checkBoxBBoxes.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.checkBoxBBoxes.Name = "checkBoxBBoxes";
            this.checkBoxBBoxes.Size = new System.Drawing.Size(97, 19);
            this.checkBoxBBoxes.TabIndex = 2;
            this.checkBoxBBoxes.Text = "Show BBoxes";
            this.checkBoxBBoxes.UseVisualStyleBackColor = true;
            this.checkBoxBBoxes.CheckedChanged += new System.EventHandler(this.checkBoxBBoxes_CheckedChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.modeResults);
            this.groupBox1.Controls.Add(this.modeScan);
            this.groupBox1.Controls.Add(this.modeScore);
            this.groupBox1.Controls.Add(this.modeInteractionMap);
            this.groupBox1.Controls.Add(this.modeAgentOwnership);
            this.groupBox1.Controls.Add(this.modeAgentPlacement);
            this.groupBox1.Location = new System.Drawing.Point(12, 11);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(133, 215);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Mode";
            // 
            // modeResults
            // 
            this.modeResults.AutoSize = true;
            this.modeResults.Location = new System.Drawing.Point(6, 147);
            this.modeResults.Name = "modeResults";
            this.modeResults.Size = new System.Drawing.Size(65, 19);
            this.modeResults.TabIndex = 1;
            this.modeResults.TabStop = true;
            this.modeResults.Text = "Results";
            this.modeResults.UseVisualStyleBackColor = true;
            this.modeResults.CheckedChanged += new System.EventHandler(this.modeResults_CheckedChanged);
            // 
            // modeScan
            // 
            this.modeScan.AutoSize = true;
            this.modeScan.Location = new System.Drawing.Point(6, 122);
            this.modeScan.Name = "modeScan";
            this.modeScan.Size = new System.Drawing.Size(51, 19);
            this.modeScan.TabIndex = 0;
            this.modeScan.TabStop = true;
            this.modeScan.Text = "Scan";
            this.modeScan.UseVisualStyleBackColor = true;
            this.modeScan.CheckedChanged += new System.EventHandler(this.modeScan_CheckedChanged);
            // 
            // modeScore
            // 
            this.modeScore.AutoSize = true;
            this.modeScore.Location = new System.Drawing.Point(6, 97);
            this.modeScore.Name = "modeScore";
            this.modeScore.Size = new System.Drawing.Size(55, 19);
            this.modeScore.TabIndex = 0;
            this.modeScore.TabStop = true;
            this.modeScore.Text = "Score";
            this.modeScore.UseVisualStyleBackColor = true;
            this.modeScore.CheckedChanged += new System.EventHandler(this.modeScore_CheckedChanged);
            // 
            // modeInteractionMap
            // 
            this.modeInteractionMap.AutoSize = true;
            this.modeInteractionMap.Location = new System.Drawing.Point(6, 72);
            this.modeInteractionMap.Name = "modeInteractionMap";
            this.modeInteractionMap.Size = new System.Drawing.Size(113, 19);
            this.modeInteractionMap.TabIndex = 0;
            this.modeInteractionMap.TabStop = true;
            this.modeInteractionMap.Text = "Interaction Map";
            this.modeInteractionMap.UseVisualStyleBackColor = true;
            this.modeInteractionMap.CheckedChanged += new System.EventHandler(this.modeInteractionMap_CheckedChanged);
            // 
            // modeAgentOwnership
            // 
            this.modeAgentOwnership.AutoSize = true;
            this.modeAgentOwnership.Location = new System.Drawing.Point(6, 47);
            this.modeAgentOwnership.Name = "modeAgentOwnership";
            this.modeAgentOwnership.Size = new System.Drawing.Size(118, 19);
            this.modeAgentOwnership.TabIndex = 0;
            this.modeAgentOwnership.TabStop = true;
            this.modeAgentOwnership.Text = "Agent Ownership";
            this.modeAgentOwnership.UseVisualStyleBackColor = true;
            this.modeAgentOwnership.CheckedChanged += new System.EventHandler(this.modeAgentOwnership_CheckedChanged);
            // 
            // modeAgentPlacement
            // 
            this.modeAgentPlacement.AutoSize = true;
            this.modeAgentPlacement.Location = new System.Drawing.Point(6, 22);
            this.modeAgentPlacement.Name = "modeAgentPlacement";
            this.modeAgentPlacement.Size = new System.Drawing.Size(115, 19);
            this.modeAgentPlacement.TabIndex = 0;
            this.modeAgentPlacement.TabStop = true;
            this.modeAgentPlacement.Text = "Agent Placement";
            this.modeAgentPlacement.UseVisualStyleBackColor = true;
            this.modeAgentPlacement.CheckedChanged += new System.EventHandler(this.modeAgentPlacement_CheckedChanged);
            // 
            // helpBox
            // 
            this.helpBox.Controls.Add(this.textBoxCommands);
            this.helpBox.Location = new System.Drawing.Point(154, 60);
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
            // textBoxRecategorize
            // 
            this.textBoxRecategorize.Location = new System.Drawing.Point(11, 232);
            this.textBoxRecategorize.Name = "textBoxRecategorize";
            this.textBoxRecategorize.Size = new System.Drawing.Size(134, 23);
            this.textBoxRecategorize.TabIndex = 5;
            // 
            // buttonRecategorize
            // 
            this.buttonRecategorize.Location = new System.Drawing.Point(9, 262);
            this.buttonRecategorize.Name = "buttonRecategorize";
            this.buttonRecategorize.Size = new System.Drawing.Size(136, 24);
            this.buttonRecategorize.TabIndex = 6;
            this.buttonRecategorize.Text = "Recategorize";
            this.buttonRecategorize.UseVisualStyleBackColor = true;
            this.buttonRecategorize.Click += new System.EventHandler(this.buttonRecategorize_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(168, 235);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(50, 15);
            this.label2.TabIndex = 7;
            this.label2.Text = "Results:";
            // 
            // comboBoxResultFolderList
            // 
            this.comboBoxResultFolderList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxResultFolderList.FormattingEnabled = true;
            this.comboBoxResultFolderList.Location = new System.Drawing.Point(221, 232);
            this.comboBoxResultFolderList.Name = "comboBoxResultFolderList";
            this.comboBoxResultFolderList.Size = new System.Drawing.Size(180, 23);
            this.comboBoxResultFolderList.Sorted = true;
            this.comboBoxResultFolderList.TabIndex = 8;
            this.comboBoxResultFolderList.SelectedIndexChanged += new System.EventHandler(this.comboBoxResultFolderList_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(177, 265);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(41, 15);
            this.label3.TabIndex = 7;
            this.label3.Text = "Scene:";
            // 
            // comboBoxResultsSceneList
            // 
            this.comboBoxResultsSceneList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxResultsSceneList.FormattingEnabled = true;
            this.comboBoxResultsSceneList.Location = new System.Drawing.Point(221, 262);
            this.comboBoxResultsSceneList.Name = "comboBoxResultsSceneList";
            this.comboBoxResultsSceneList.Size = new System.Drawing.Size(180, 23);
            this.comboBoxResultsSceneList.Sorted = true;
            this.comboBoxResultsSceneList.TabIndex = 8;
            this.comboBoxResultsSceneList.SelectedIndexChanged += new System.EventHandler(this.comboBoxResultsSceneList_SelectedIndexChanged);
            // 
            // buttonSavePBRT
            // 
            this.buttonSavePBRT.Location = new System.Drawing.Point(9, 292);
            this.buttonSavePBRT.Name = "buttonSavePBRT";
            this.buttonSavePBRT.Size = new System.Drawing.Size(136, 25);
            this.buttonSavePBRT.TabIndex = 9;
            this.buttonSavePBRT.Text = "Save PBRT";
            this.buttonSavePBRT.UseVisualStyleBackColor = true;
            this.buttonSavePBRT.Click += new System.EventHandler(this.buttonSavePBRT_Click);
            // 
            // buttonRenderFolder
            // 
            this.buttonRenderFolder.Location = new System.Drawing.Point(221, 323);
            this.buttonRenderFolder.Name = "buttonRenderFolder";
            this.buttonRenderFolder.Size = new System.Drawing.Size(180, 25);
            this.buttonRenderFolder.TabIndex = 10;
            this.buttonRenderFolder.Text = "Render Results Folder";
            this.buttonRenderFolder.UseVisualStyleBackColor = true;
            this.buttonRenderFolder.Click += new System.EventHandler(this.buttonRenderFolder_Click);
            // 
            // labelResultsFolder
            // 
            this.labelResultsFolder.AutoSize = true;
            this.labelResultsFolder.Location = new System.Drawing.Point(9, 358);
            this.labelResultsFolder.Name = "labelResultsFolder";
            this.labelResultsFolder.Size = new System.Drawing.Size(117, 15);
            this.labelResultsFolder.TabIndex = 11;
            this.labelResultsFolder.Text = "<no folder selected>";
            // 
            // buttonBatchPBRTRendering
            // 
            this.buttonBatchPBRTRendering.Location = new System.Drawing.Point(9, 323);
            this.buttonBatchPBRTRendering.Name = "buttonBatchPBRTRendering";
            this.buttonBatchPBRTRendering.Size = new System.Drawing.Size(136, 23);
            this.buttonBatchPBRTRendering.TabIndex = 12;
            this.buttonBatchPBRTRendering.Text = "Batch PBRT Rendering";
            this.buttonBatchPBRTRendering.UseVisualStyleBackColor = true;
            this.buttonBatchPBRTRendering.Click += new System.EventHandler(this.buttonBatchPBRTRendering_Click);
            // 
            // buttonSavePBRTCamera
            // 
            this.buttonSavePBRTCamera.Location = new System.Drawing.Point(151, 294);
            this.buttonSavePBRTCamera.Name = "buttonSavePBRTCamera";
            this.buttonSavePBRTCamera.Size = new System.Drawing.Size(125, 23);
            this.buttonSavePBRTCamera.TabIndex = 13;
            this.buttonSavePBRTCamera.Text = "Save PBRT Camera";
            this.buttonSavePBRTCamera.UseVisualStyleBackColor = true;
            this.buttonSavePBRTCamera.Click += new System.EventHandler(this.buttonSavePBRTCamera_Click);
            // 
            // UIWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(408, 382);
            this.Controls.Add(this.buttonSavePBRTCamera);
            this.Controls.Add(this.buttonBatchPBRTRendering);
            this.Controls.Add(this.labelResultsFolder);
            this.Controls.Add(this.buttonRenderFolder);
            this.Controls.Add(this.buttonSavePBRT);
            this.Controls.Add(this.comboBoxResultsSceneList);
            this.Controls.Add(this.comboBoxResultFolderList);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.buttonRecategorize);
            this.Controls.Add(this.textBoxRecategorize);
            this.Controls.Add(this.helpBox);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.checkBoxBBoxes);
            this.Controls.Add(this.comboBoxSceneList);
            this.Controls.Add(this.label1);
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
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxSceneList;
        private System.Windows.Forms.CheckBox checkBoxBBoxes;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton modeScore;
        private System.Windows.Forms.RadioButton modeInteractionMap;
        private System.Windows.Forms.RadioButton modeAgentOwnership;
        private System.Windows.Forms.RadioButton modeAgentPlacement;
        private System.Windows.Forms.GroupBox helpBox;
        private System.Windows.Forms.TextBox textBoxCommands;
        private System.Windows.Forms.RadioButton modeScan;
        private System.Windows.Forms.TextBox textBoxRecategorize;
        private System.Windows.Forms.Button buttonRecategorize;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox comboBoxResultFolderList;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ComboBox comboBoxResultsSceneList;
        private System.Windows.Forms.RadioButton modeResults;
        private System.Windows.Forms.Button buttonSavePBRT;
        private System.Windows.Forms.Button buttonRenderFolder;
        private System.Windows.Forms.Label labelResultsFolder;
        private System.Windows.Forms.Button buttonBatchPBRTRendering;
        private System.Windows.Forms.Button buttonSavePBRTCamera;
    }
}

