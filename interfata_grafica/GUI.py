import ctypes
import customtkinter as ctk
import os


dll = ctypes.CDLL(
    r"D:\Alex\UTCN\An 3\Semestrul 1\Structura_sistemelor_de_calcul\Proiect\program_testare_performanta"
    r"\x64\Release\program_testare_performanta.dll")


dll.getCPUInfo.restype = ctypes.c_char_p
dll.getCacheInfo.restype = ctypes.c_char_p
dll.getMemPagingInfo.restype = ctypes.c_char_p
dll.getRAMInfo.restype = ctypes.c_char_p


ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

root = ctk.CTk()
root.title("PC Benchmark App")
root.geometry("1000x650")

icon_path = "logo.ico"
try:
    if os.path.exists(icon_path):
        root.iconbitmap(icon_path)
    else:
        print(f"Icon file not found at: {icon_path}")
except Exception as e:
    print(f"Error loading icon: {e}")

text_font = ctk.CTkFont(family="Consolas", size=14)

main_frame = ctk.CTkFrame(root)
main_frame.pack(fill="both", expand=True)

btn_cpu = ctk.CTkButton(main_frame, text="CPU Info")
btn_cache = ctk.CTkButton(main_frame, text="Cache Info")
btn_paging = ctk.CTkButton(main_frame, text="Paging Info")
btn_ram = ctk.CTkButton(main_frame, text="RAM Info")

btn_cpu.pack(pady=10)
btn_cache.pack(pady=10)
btn_paging.pack(pady=10)
btn_ram.pack(pady=10)


info_frame = ctk.CTkFrame(root)


info_label = ctk.CTkLabel(info_frame, text="", font=ctk.CTkFont(family="Consolas", size=16))
info_label.pack(pady=(10, 5))


text_widget = ctk.CTkTextbox(info_frame, width=880, height=500, font=text_font)
text_widget.pack(padx=10, pady=10)
text_widget.configure(state="disabled")


btn_back = ctk.CTkButton(info_frame, text="Back", width=100)
btn_back.pack(pady=10)



def show_info(dll_func, label_text):
    main_frame.pack_forget()
    info_frame.pack(fill="both", expand=True)

    info_label.configure(text=label_text)

    text_widget.configure(state="normal")
    text_widget.delete("0.0", "end")
    text_widget.insert("0.0", dll_func().decode())
    text_widget.configure(state="disabled")


def go_back():
    info_frame.pack_forget()
    main_frame.pack(fill="both", expand=True)



btn_cpu.configure(command=lambda: show_info(dll.getCPUInfo, "CPU Information"))
btn_cache.configure(command=lambda: show_info(dll.getCacheInfo, "Cache Memory Information"))
btn_paging.configure(command=lambda: show_info(dll.getMemPagingInfo, "Memory Paging Information"))
btn_ram.configure(command=lambda: show_info(dll.getRAMInfo, "RAM Information"))
btn_back.configure(command=go_back)

root.mainloop()