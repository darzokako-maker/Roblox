import customtkinter as ctk
import subprocess
import os

class StealthUI(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("YAHYA PRIVATE STEALTH")
        self.geometry("600x400")
        ctk.set_appearance_mode("dark")

        self.label = ctk.CTkLabel(self, text="STEALTH EXECUTOR v18.2", font=("Consolas", 20, "bold"))
        self.label.pack(pady=20)

        self.textbox = ctk.CTkTextbox(self, width=500, height=200)
        self.textbox.pack(pady=10)

        self.inject_btn = ctk.CTkButton(self, text="INJECT ENGINE", fg_color="green", command=self.run_engine)
        self.inject_btn.pack(side="left", padx=50)

        self.exec_btn = ctk.CTkButton(self, text="EXECUTE SCRIPT", command=self.execute)
        self.exec_btn.pack(side="right", padx=50)

    def run_engine(self):
        if os.path.exists("stealth_engine.exe"):
            subprocess.Popen(["stealth_engine.exe"])
        else:
            print("Hata: stealth_engine.exe bulunamadı.")

    def execute(self):
        with open("script.lua", "w") as f:
            f.write(self.textbox.get("1.0", "end"))
        print("Script gönderildi.")

if __name__ == "__main__":
    app = StealthUI()
    app.mainloop()
  
