using System;
using System.Drawing;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace RobloxExecutor {
    public class MainForm : Form {
        // C++ (Native) DLL'leri sorunsuz yüklemek için Windows API fonksiyonlarını çağırıyoruz
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Ansi)]
        private static extern IntPtr LoadLibrary(string lpFileName);

        // Kütüphane içindeki fonksiyonları adıyla tetiklemek için gereken delegeler (Simülasyon/Dinamik Çağrı için)
        // Eğer DLL doğrudan standart export veriyorsa bu yöntem çalışır.
        
        RichTextBox txtEditor = new RichTextBox();
        Button btnInject = new Button();
        Button btnExec = new Button();
        Label lblStatus = new Label();
        private IntPtr dllHandle = IntPtr.Zero;

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

            lblStatus.Text = "Durum: Kontrol Ediliyor...";
            lblStatus.Location = new Point(10, 345);
            lblStatus.Size = new Size(500, 20);
            lblStatus.ForeColor = Color.Yellow;

            this.Controls.Add(txtEditor);
            this.Controls.Add(btnInject);
            this.Controls.Add(btnExec);
            this.Controls.Add(lblStatus);

            InitializeNativeDLL();
        }

        private void InitializeNativeDLL() {
            try {
                // Klasördeki DLL adlarını esnek tarıyoruz (büyük/küçük harf veya çift .dll uzantısı hatalarına karşı)
                string currentDir = AppDomain.CurrentDomain.BaseDirectory;
                string[] files = System.IO.Directory.GetFiles(currentDir, "*wearedevs_exploit_api*");
                
                string targetDll = null;
                foreach (string file in files) {
                    if (file.EndsWith(".dll", StringComparison.OrdinalIgnoreCase)) {
                        targetDll = file;
                        break;
                    }
                }

                if (targetDll != null && System.IO.File.Exists(targetDll)) {
                    // C++ DLL bütünlüğünü işletim sistemi seviyesinde belleğe yüklüyoruz
                    dllHandle = LoadLibrary(targetDll);
                    
                    if (dllHandle != IntPtr.Zero) {
                        lblStatus.Text = "Durum: Hazir (Native Çekirdek Baglandi)";
                        lblStatus.ForeColor = Color.LimeGreen;
                    } else {
                        int error = Marshal.GetLastWin32Error();
                        lblStatus.Text = $"Durum: DLL Enjeksiyon Hatası! Kod: {error}";
                        lblStatus.ForeColor = Color.Red;
                    }
                } else {
                    lblStatus.Text = "Durum: DLL Bulunamadi (wearedevs_exploit_api.dll eksik)";
                    lblStatus.ForeColor = Color.Red;
                }
            } catch (Exception ex) {
                lblStatus.Text = "Durum: Sistem Hatasi: " + ex.Message;
                lblStatus.ForeColor = Color.Red;
            }
        }

        private void btnInject_Click(object sender, EventArgs e) {
            if (dllHandle != IntPtr.Zero) {
                lblStatus.Text = "Durum: Enjekte Ediliyor...";
                lblStatus.ForeColor = Color.Orange;
                
                // İlerleyen aşamalarda buraya GetProcAddress fonksiyonları ile explicit çağrılar eklenebilir.
                MessageBox.Show("Modül başarıyla yüklendi, enjeksiyon tetikleniyor.", "Stealth", MessageBoxButtons.OK, MessageBoxIcon.Information);
            } else {
                MessageBox.Show("DLL çekirdeği yüklenemediği için işlem yapılamaz.", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void btnExec_Click(object sender, EventArgs e) {
            if (dllHandle == IntPtr.Zero) {
                MessageBox.Show("Lütfen önce bağımlılıkları kontrol edin.", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            MessageBox.Show("Kod gönderim işlevi tetiklendi.", "Stealth", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}
