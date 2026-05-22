using System;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace RobloxExecutor {
    public class MainForm : Form {
        // --- WINDOWS API (WIN32) TANIMLAMALARI ---
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenProcess(uint processAccess, bool bInheritHandle, int processId);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, out IntPtr lpThreadId);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);

        // Erişim Yetkileri
        const uint PROCESS_ALL_ACCESS = 0x001F0FFF;
        const uint MEM_COMMIT = 0x00001000;
        const uint MEM_RESERVE = 0x00002000;
        const uint PAGE_READWRITE = 0x04;

        // --- ARAYÜZ ELEMANLARI ---
        RichTextBox txtEditor = new RichTextBox();
        Button btnInject = new Button();
        Button btnExec = new Button();
        Label lblStatus = new Label();

        public MainForm() {
            this.Text = "Stealth Yerli Executor v2.0";
            this.Size = new Size(600, 420);
            this.BackColor = Color.FromArgb(20, 20, 20);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;

            // Metin Editörü (Lua Kodu İçin)
            txtEditor.Location = new Point(10, 10);
            txtEditor.Size = new Size(560, 260);
            txtEditor.BackColor = Color.FromArgb(30, 30, 30);
            txtEditor.ForeColor = Color.LightGreen;
            txtEditor.Font = new Font("Consolas", 10F);
            txtEditor.Text = "-- Kendi Enjektör Sistemimiz Aktif\nprint(\"Hello from Native Stealth!\")";

            // Inject Butonu
            btnInject.Text = "DLL Inject Et";
            btnInject.Location = new Point(10, 290);
            btnInject.Size = new Size(120, 35);
            btnInject.BackColor = Color.FromArgb(45, 45, 45);
            btnInject.ForeColor = Color.White;
            btnInject.FlatStyle = FlatStyle.Flat;
            btnInject.Click += btnInject_Click;

            // Execute Butonu
            btnExec.Text = "Kodu Çalıştır";
            btnExec.Location = new Point(140, 290);
            btnExec.Size = new Size(120, 35);
            btnExec.BackColor = Color.FromArgb(45, 45, 45);
            btnExec.ForeColor = Color.White;
            btnExec.FlatStyle = FlatStyle.Flat;
            btnExec.Click += btnExec_Click;

            // Durum Çubuğu
            lblStatus.Text = "Durum: DLL bekleniyor... (Hile DLL'ini programın yanına koyun)";
            lblStatus.Location = new Point(10, 345);
            lblStatus.Size = new Size(560, 20);
            lblStatus.ForeColor = Color.Orange;

            this.Controls.Add(txtEditor);
            this.Controls.Add(btnInject);
            this.Controls.Add(btnExec);
            this.Controls.Add(lblStatus);
        }

        private void btnInject_Click(object sender, EventArgs e) {
            string dllName = "hile_cekirdegi.dll"; // Enjekte etmek istediğin C++ DLL adını buraya yaz
            string dllPath = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, dllName);

            if (!System.IO.File.Exists(dllPath)) {
                MessageBox.Show($"Enjekte edilecek '{dllName}' dosyası programın yanında bulunamadı!", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            lblStatus.Text = "Durum: Roblox süreci aranıyor...";
            lblStatus.ForeColor = Color.Yellow;

            // Roblox sürecini bul (UWP veya Web sürümüne göre "RobloxPlayerBeta" taranır)
            Process[] processes = Process.GetProcessesByName("RobloxPlayerBeta");
            if (processes.Length == 0) {
                processes = Process.GetProcessesByName("RobloxPlayer");
            }

            if (processes.Length == 0) {
                lblStatus.Text = "Durum: Roblox açık değil!";
                lblStatus.ForeColor = Color.Red;
                MessageBox.Show("Roblox bulunamadı! Lütfen önce oyunu açın.", "Hata", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            Process targetProcess = processes[0];
            lblStatus.Text = $"Durum: Roblox Bulundu (PID: {targetProcess.Id}). Enjekte ediliyor...";

            // Enjeksiyon İşlemini Başlat
            bool success = InjectDLL(targetProcess.Id, dllPath);

            if (success) {
                lblStatus.Text = "Durum: DLL Başarıyla Enjekte Edildi!";
                lblStatus.ForeColor = Color.LimeGreen;
            } else {
                lblStatus.Text = "Durum: Enjeksiyon başarısız oldu.";
                lblStatus.ForeColor = Color.Red;
            }
        }

        private bool InjectDLL(int processId, string dllPath) {
            IntPtr hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
            if (hProcess == IntPtr.Zero) return false;

            uint size = (uint)((dllPath.Length + 1) * Marshal.SizeOf(typeof(char)));
            
            // Hedef süreçte yer aç
            IntPtr allocAddress = VirtualAllocEx(hProcess, IntPtr.Zero, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (allocAddress == IntPtr.Zero) {
                CloseHandle(hProcess);
                return false;
            }

            // DLL yolunu hedef sürecin hafızasına yaz
            byte[] bytes = Encoding.Default.GetBytes(dllPath);
            IntPtr bytesWritten;
            bool writeSuccess = WriteProcessMemory(hProcess, allocAddress, bytes, (uint)bytes.Length, out bytesWritten);

            if (!writeSuccess) {
                CloseHandle(hProcess);
                return false;
            }

            // LoadLibraryA fonksiyonunun adresini alarak DLL'i çalıştıracak thread'i yarat
            IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
            if (loadLibraryAddr == IntPtr.Zero) {
                CloseHandle(hProcess);
                return false;
            }

            IntPtr hThread = CreateRemoteThread(hProcess, IntPtr.Zero, 0, loadLibraryAddr, allocAddress, 0, out _);
            if (hThread == IntPtr.Zero) {
                CloseHandle(hProcess);
                return false;
            }

            // Temizlik
            CloseHandle(hThread);
            CloseHandle(hProcess);
            return true;
        }

        private void btnExec_Click(object sender, EventArgs e) {
            // Tamamen sıfırdan bir Named Pipe veya Windows Mesajlaşma köprüsü kurulduğunda 
            // buradaki kod, oyunun içindeki DLL'e txtEditor.Text içeriğini gönderecektir.
            MessageBox.Show("Kendi C++ DLL'iniz içerisindeki komut dinleyici (Named Pipe) kurulu olduğunda bu buton kodu iletecektir.", "Stealth Sistemi", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}

