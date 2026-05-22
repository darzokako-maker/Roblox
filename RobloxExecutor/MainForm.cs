using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RobloxExecutor {
    public class MainForm : Form {
        // --- ARAYÜZ BİLEŞENLERİ ---
        RichTextBox txtEditor = new RichTextBox();
        ListBox lstLogs = new ListBox();
        Button btnExecute = new Button();
        Button btnClearLogs = new Button();
        Label lblStatus = new Label();

        public MainForm() {
            // Form Pencere Ayarları
            this.Text = "Stealth Komut Yürütme Konsolu v4.0";
            this.Size = new Size(620, 520);
            this.BackColor = Color.FromArgb(16, 16, 16);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;

            // Kod / Komut Giriş Editörü
            txtEditor.Location = new Point(10, 10);
            txtEditor.Size = new Size(580, 220);
            txtEditor.BackColor = Color.FromArgb(26, 26, 26);
            txtEditor.ForeColor = Color.LightGreen;
            txtEditor.Font = new Font("Consolas", 10F);
            txtEditor.Text = "-- Yürütülecek görev listesini girin\nLOG \"Sistem testi baslatildi\"\nWAIT 1000\nSET_PARAM \"VERI_AKISI\" 1\nLOG \"Gorev tamamlandi\"";

            // Günlük Kayıtları (Log Output)
            lstLogs.Location = new Point(10, 240);
            lstLogs.Size = new Size(580, 140);
            lstLogs.BackColor = Color.FromArgb(10, 10, 10);
            lstLogs.ForeColor = Color.Cyan;
            lstLogs.Font = new Font("Consolas", 9F);

            // Kod Yürütme Butonu
            btnExecute.Text = "Kodu Yürüt (Execute)";
            btnExecute.Location = new Point(10, 395);
            btnExecute.Size = new Size(150, 35);
            btnExecute.BackColor = Color.FromArgb(40, 40, 40);
            btnExecute.ForeColor = Color.White;
            btnExecute.FlatStyle = FlatStyle.Flat;
            btnExecute.Click += BtnExecute_Click;

            // Günlük Temizleme Butonu
            btnClearLogs.Text = "Günlüğü Temizle";
            btnClearLogs.Location = new Point(170, 395);
            btnClearLogs.Size = new Size(130, 35);
            btnClearLogs.BackColor = Color.FromArgb(40, 40, 40);
            btnClearLogs.ForeColor = Color.White;
            btnClearLogs.FlatStyle = FlatStyle.Flat;
            btnClearLogs.Click += (s, e) => { lstLogs.Items.Clear(); };

            // Alt Durum Bilgisi
            lblStatus.Text = "Yürütme Katmanı Hazır. Komut bekleniyor.";
            lblStatus.Location = new Point(10, 445);
            lblStatus.Size = new Size(580, 20);
            lblStatus.ForeColor = Color.White;

            // Bileşenleri Forma Ekleme
            this.Controls.Add(txtEditor);
            this.Controls.Add(lstLogs);
            this.Controls.Add(btnExecute);
            this.Controls.Add(btnClearLogs);
            this.Controls.Add(lblStatus);

            LogEkle("Yürütme motoru (Execution Engine) başlatıldı.");
        }

        private void LogEkle(string mesaj) {
            string zaman = DateTime.Now.ToString("HH:mm:ss");
            lstLogs.Items.Add($"[{zaman}] {mesaj}");
            lstLogs.TopIndex = lstLogs.Items.Count - 1; // Listeyi otomatik aşağı kaydır
        }

        private async void BtnExecute_Click(object sender, EventArgs e) {
            string rawCode = txtEditor.Text;

            if (string.IsNullOrWhiteSpace(rawCode)) {
                LogEkle("Hata: Yürütülecek kaynak kod bulunamadı.");
                return;
            }

            // Arayüzün kilitlenmesini önlemek için butonu geçici olarak devre dışı bırakıyoruz
            btnExecute.Enabled = false;
            lblStatus.Text = "Durum: Kod yürütülüyor...";
            lblStatus.ForeColor = Color.Yellow;

            LogEkle("Kod ayrıştırma katmanı (Parser) tetiklendi.");
            
            // Satırları diziye bölüyoruz
            string[] lines = rawCode.Split(new[] { "\r\n", "\r", "\n" }, StringSplitOptions.None);

            // Kod yürütme işlemini arka plandaki ayrı bir iş parçacığına (Task) devrediyoruz
            bool sonuc = await Task.Run(() => YurutmeMotoru(lines));

            if (sonuc) {
                lblStatus.Text = "Durum: Yürütme başarıyla tamamlandı.";
                lblStatus.ForeColor = Color.LimeGreen;
                LogEkle("Yürütme işlemi bitti.");
            } else {
                lblStatus.Text = "Durum: Yürütme sırasında hata oluştu.";
                lblStatus.ForeColor = Color.Red;
            }

            btnExecute.Enabled = true;
        }

        // --- ARKA PLAN KOD YÜRÜTME MOTORU ---
        private bool YurutmeMotoru(string[] kodSatirlari) {
            try {
                int satirNumarasi = 0;

                foreach (string satir in kodSatirlari) {
                    satirNumarasi++;
                    string islenmisSatir = satir.Trim();

                    // Boş satırları veya yorum satırlarını geç
                    if (string.IsNullOrEmpty(islenmisSatir) || islenmisSatir.StartsWith("--") || islenmisSatir.StartsWith("//")) {
                        continue;
                    }

                    // UI Thread'e güvenli erişim sağlayarak log basıyoruz (Invoke)
                    this.Invoke((MethodInvoker)delegate {
                        LogEkle($"Satır {satirNumarasi} işleniyor: {islenmisSatir}");
                    });

                    // Temel Tokenizer/Lexer mantığı: Komutu ve parametreleri ayırıyoruz
                    string[] parcalar = islenmisSatir.Split(new[] { ' ' }, 2);
                    string anaKomut = parcalar[0].ToUpper();
                    string parametreler = parcalar.Length > 1 ? parcalar[1] : string.Empty;

                    // Komut Dağıtım Katmanı (Dispatcher)
                    switch (anaKomut) {
                        case "LOG":
                            // Ekrana sadece log basan temsili komut
                            System.Threading.Thread.Sleep(200); // Kısa işlem gecikmesi
                            break;

                        case "WAIT":
                            // Belirtilen milisaniye kadar yürütmeyi durduran komut
                            if (int.TryParse(parametreler, out int ms)) {
                                System.Threading.Thread.Sleep(ms);
                            }
                            break;

                        case "SET_PARAM":
                            // Yapılandırma parametrelerini güncelleyen temsili komut
                            System.Threading.Thread.Sleep(300);
                            break;

                        default:
                            this.Invoke((MethodInvoker)delegate {
                                LogEkle($"Bilinmeyen Komut Pas Geçildi (Satır {satirNumarasi}): {anaKomut}");
                            });
                            break;
                    }
                }
                return true;
            }
            catch (Exception ex) {
                this.Invoke((MethodInvoker)delegate {
                    LogEkle($"Yürütme Hatası: {ex.Message}");
                });
                return false;
            }
        }
    }
}
