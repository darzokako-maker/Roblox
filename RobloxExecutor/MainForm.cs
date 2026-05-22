using System;
using System.Drawing;
using System.Windows.Forms;
using WeAreDevs_API;

namespace RobloxExecutor {
    public class MainForm : Form {
        ExploitAPI api = new ExploitAPI();
        RichTextBox txtEditor = new RichTextBox();
        Button btnInject = new Button();
        Button btnExec = new Button();

        public MainForm() {
            this.Text = "Stealth Executor";
            this.Size = new Size(600, 400);
            this.BackColor = Color.FromArgb(25, 25, 25);

            txtEditor.Location = new Point(10, 10);
            txtEditor.Size = new Size(560, 280);
            txtEditor.BackColor = Color.FromArgb(35, 35, 35);
            txtEditor.ForeColor = Color.LightGreen;
            txtEditor.Text = "-- Roblox Lua kodunuzu buraya yazin\nprint(\"Hello from Stealth\")";

            btnInject.Text = "Inject";
            btnInject.Location = new Point(10, 300);
            btnInject.Size = new Size(80, 30);
            btnInject.Click += (s, e) => { api.LaunchExploit(); };

            btnExec.Text = "Execute";
            btnExec.Location = new Point(100, 300);
            btnExec.Size = new Size(80, 30);
            btnExec.Click += (s, e) => { api.SendLuaScript(txtEditor.Text); };

            this.Controls.Add(txtEditor);
            this.Controls.Add(btnInject);
            this.Controls.Add(btnExec);
        }
    }
}

