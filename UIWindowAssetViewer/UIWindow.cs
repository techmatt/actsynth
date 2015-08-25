using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.IO.Pipes;

namespace UIWindow
{
    public partial class UIWindow : Form
    {
        const string pipeBaseName = "SceneAssetViewer";
        const string resultsDir = @"V:\data\synthesis\results\";

        NamedPipeServerStream server;
        NamedPipeClientStream client;
        StreamReader reader;
        StreamWriter writer;

        bool ignoreEvents = false;

        ConcurrentQueue<string> messages = new ConcurrentQueue<string>();

        public UIWindow()
        {
            InitializeComponent();
        }

        private void LaunchServer(string pipeName)
        {
            try
            {
                Console.WriteLine("Creating server: " + pipeName);
                server = new NamedPipeServerStream(pipeName, PipeDirection.InOut, 4);
                Console.WriteLine("Waiting for connection");
                server.WaitForConnection();
                reader = new StreamReader(server);

                Task.Factory.StartNew(() =>
                {
                    Console.WriteLine("Begin server read loop");
                    while (true)
                    {
                        try
                        {
                            var line = reader.ReadLine();
                            messages.Enqueue(line);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("ServerLoop exception: {0}", ex.Message);
                        }
                    }
                });
            }
            catch (Exception ex)
            {
                Console.WriteLine("LaunchServer exception: {0}", ex.Message);
            }
        }

        private void SendMessage(string message)
        {
            if (ignoreEvents) return;
            if (writer != null)
            {
                try
                {
                    writer.Write(message);
                    writer.Flush();
                }
                catch (Exception ex)
                {
                    Console.WriteLine("SendMessage exception: {0}", ex.Message);
                }
            }
        }

        private void ProcessMessage(string message)
        {
            ignoreEvents = true;
            var parts = message.Split(' ');
            if (parts.Length == 2)
            {
                if (parts[0] == "sceneList")
                {
                    comboBoxSceneList.Items.Clear();
                    foreach (string s in parts[1].Split('|'))
                    {
                        if(s.Length > 0)
                            comboBoxSceneList.Items.Add(s);
                    }
                }
                if (parts[0] == "helpText")
                {
                    textBoxCommands.Text = parts[1].Replace("@", " ").Replace("|", Environment.NewLine);
                }
            }
            ignoreEvents = false;
        }

        private void LaunchClient(string pipeName)
        {
            try
            {
                Console.WriteLine("Creating client: " + pipeName);
                client = new NamedPipeClientStream(pipeName);
                Console.WriteLine("Connecting to client");
                client.Connect();
                writer = new StreamWriter(client);
            }
            catch (Exception ex)
            {
                Console.WriteLine("LaunchClient exception: {0}", ex.Message);
            }
        }

        private void timerProcessMessages_Tick(object sender, EventArgs e)
        {
            string message;
            if (messages.TryDequeue(out message) && message != null)
            {
                ProcessMessage(message);
            }
        }

        private void timerInitialize_Tick(object sender, EventArgs e)
        {
            timerInitialize.Enabled = false;

            LaunchClient(pipeBaseName + "ReadFromUI");
            LaunchServer(pipeBaseName + "WriteToUI");

            this.Text = "Asset Viewer UI";
        }

        private void UIWindow_Load(object sender, EventArgs e)
        {
            this.Left = System.Windows.Forms.Screen.PrimaryScreen.WorkingArea.Right - this.Width;
            this.Top = 0;
            this.KeyPreview = true;

            foreach(string s in Directory.EnumerateDirectories(resultsDir))
            {
                comboBoxResultFolderList.Items.Add(s.Split('\\').Last());
            }
        }

        private void timerDisconnectCheck_Tick(object sender, EventArgs e)
        {
            if (server != null && reader != null && !server.IsConnected)
            {
                Application.Exit();
            }
        }

        private void UIWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            SendMessage("terminate");
        }

        private void UIWindow_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape)
            {
                SendMessage("terminate");
                Close();
            }
        }

        private void comboBoxSceneList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (ignoreEvents) return;
            if (comboBoxSceneList.SelectedIndex < comboBoxSceneList.Items.Count)
            {
                string selectedScene = comboBoxSceneList.Items[comboBoxSceneList.SelectedIndex].ToString();
                if (selectedScene.Length > 1)
                    SendMessage("loadScene " + selectedScene);
            }
        }

        private void checkBoxBBoxes_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("showBBoxes " + checkBoxBBoxes.Checked);
        }

        private void modeAgentPlacement_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode AgentPlacement");
        }

        private void modeAgentOwnership_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode AgentOwnership");
        }

        private void modeInteractionMap_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode InteractionMap");
        }

        private void modeScore_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode Score");
        }

        private void modeResults_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode Results");
        }

        private void modeScan_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode Scan");
        }

        private void buttonRecategorize_Click(object sender, EventArgs e)
        {
            SendMessage("recategorize " + textBoxRecategorize.Text);
        }

        private void comboBoxResultFolderList_SelectedIndexChanged(object sender, EventArgs e)
        {
            comboBoxResultsSceneList.Items.Clear();
            foreach (string s in Directory.EnumerateFiles(resultsDir + comboBoxResultFolderList.SelectedItem, "*.sss"))
            {
                comboBoxResultsSceneList.Items.Add(s.Split('\\').Last());
            }
            labelResultsFolder.Text = comboBoxResultFolderList.SelectedItem.ToString();
        }

        private void comboBoxResultsSceneList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (ignoreEvents) return;
            if (comboBoxResultsSceneList.SelectedIndex < comboBoxResultsSceneList.Items.Count)
            {
                string selectedScene = comboBoxResultsSceneList.SelectedItem.ToString();
                string resultFolder = comboBoxResultFolderList.SelectedItem.ToString();
                if (selectedScene.Length > 1 && resultFolder.Length > 1)
                    SendMessage("loadResultScene " + resultFolder + "/" + selectedScene);
            }
        }

        private void buttonSavePBRT_Click(object sender, EventArgs e)
        {
          SendMessage("SavePBRT");
          /*FolderBrowserDialog folder_browser_dialog = new FolderBrowserDialog();

          folder_browser_dialog.Description = "Select the directory that you want to use for storing PBRT files";
          folder_browser_dialog.SelectedPath = "V:\\data\\synthesis\\";

          if (folder_browser_dialog.ShowDialog() == DialogResult.OK)
          {
            SendMessage("SavePBRT " + folder_browser_dialog.SelectedPath);
          }*/
        }

        private void buttonRenderFolder_Click(object sender, EventArgs e)
        {
            string resultFolder = comboBoxResultFolderList.SelectedItem.ToString();
            SendMessage("renderResultsFolder " + resultFolder + "/");
        }

        private void radioButtonHeatmap_CheckedChanged(object sender, EventArgs e)
        {
            SendMessage("mode Heatmap");
        }

        private void buttonBatchPBRTRendering_Click(object sender, EventArgs e)
        {
          SendMessage("batchPBRTRendering");
        }

        private void buttonSavePBRTCamera_Click(object sender, EventArgs e)
        {
          string resultFolder = comboBoxResultFolderList.SelectedItem.ToString();
          SendMessage("savePBRTCamera " + resultFolder);
        }
    }
}
