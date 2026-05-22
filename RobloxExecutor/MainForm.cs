using System;
using System.Drawing;
using System.Windows.Forms;
using System.Reflection;

namespace RobloxExecutor {
    public class MainForm : Form {
        private object apiInstance = null;
        private MethodInfo launchMethod = null;
        private MethodInfo sendScriptMethod = null;

        RichTextBox txtEditor = new RichTextBox();
        Button btnInject = new Button();
        Button btnExec = new Button();
        Label lblStatus = new Label();

        public MainForm() {
            this.Text = "Stealth Executor v1.0";
            this.Size = new Size(600, 420);
            this.BackColor = Color.FromArgb(25, 25, 25);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;

            txtEditor.Location = new Point(10, 10);
            txtEditor.Size = new Size(560, 260);
            txtEditor.BackColor = Color.FromArgb(35, 35, 35);
            txtEditor.ForeColor = Color.LightGreen;
            txtEditor.Font = new Font("Consolas", 10F);
            txtEditor.Text = "-- Roblox Lua kodunuzu buraya yazin\nprint(\"Hello from Stealth\")";

            btnInject.Text = "Inject";
            btnInject.Location = new Point(10, 290);
            btnInject.Size = new Size(100, 35);
            btnInject.BackColor = Color.FromArgb(45, 45, 45);
            btnInject.ForeColor = Color.White;
            btnInject.FlatStyle = FlatStyle.Flat;
            btnInject.Click += btnInject_Click;

            btnExec.Text = "Execute";
            btnExec.Location = new Point(120, 290);
            btnExec.Size = new Size(100, 35);
            btnExec.BackColor = Color.FromArgb(45, 45, 45);
            btnExec.ForeColor = Color.White;
            btnExec.FlatStyle = FlatStyle.Flat;
            btnExec.Click += btnExec_Click;

            lblStatus.Text = "Durum: DLL Yukleniyor...";
            lblStatus.Location = new Point(10, 345);
            lblStatus.Size = new Size(300, 20);
            lblStatus.ForeColor = Color.Yellow;

            this.Controls.Add(txtEditor);
            this.Controls.Add(btnInject);
            this.Controls.Add(btnExec);
            this.Controls.Add(lblStatus);

            InitializeAPI();
        }

        private void InitializeAPI() {
            try {
                // Çalışma dizinindeki DLL dosyasını dinamik olarak yükler
                string dllPath = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "wearedevs_exploit_api.dll");
                if (System.IO.File.Exists(dllPath)) {
                    Assembly asm = Assembly.LoadFrom(dllPath);
                    foreach (Type t in asm.GetTypes()) {
                        if (t.Name.Contains("ExploitAPI")) {
                            apiInstance = Activator.CreateInstance(t);
                            launchMethod = t.GetMethod("LaunchExploit");
                            sendScriptMethod = t.GetMethod("SendLuaScript");
                            break;
                        }
                    }
                    lblStatus.Text = "Durum: Hazir (DLL Baglandi)";
                    lblStatus.ForeColor = Color.LimeGreen;
                } else {
                    lblStatus.Text = "Durum: DLL Bulunamadi (wearedevs_exploit_api.dll eksik)";
                    lblStatus.ForeColor = Color.Red;
                }
            } catch (Exception) {
                lblStatus.Text = "Durum: DLL Yukleme Hatasi!";
                lblStatus.ForeColor = Color.Red;
            }
        }

        private void btnInject_Click(object sender, EventArgs e) {
            if (launchMethod != null && apiInstance != null) {
                lblStatus.Text = "Durum: Inject Ediliyor...";
                lblStatus.ForeColor = Color.Orange;
                try {
                    launchMethod.Invoke(apiInstance, null);
                    lblStatus.Text = "Durum: Inject Komutu Gönderildi!";
                    lblStatus.ForeColor = Color.LimeGreen;
                } catch {
                    lblStatus.Text = "Durum: Enjeksiyon Başarısız.";
                    lblStatus.ForeColor = Color.Red;
                }
            } else {
                MessageBox.Show("API kütüphanesi aktif değil veya yüklenemedi.", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnExec_Click(object sender, EventArgs e) {
            if (sendScriptMethod != null && apiInstance != null) {
                try {
                    sendScriptMethod.Invoke(apiInstance, new object[] { txtEditor.Text });
                } catch (Exception ex) {
                    MessageBox.Show("Yürütme hatası: " + ex.Message, "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            } else {
                MessageBox.Show("Önce başarılı şekilde Inject etmeniz veya DLL dosyasını eklemeniz gerekir.", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
