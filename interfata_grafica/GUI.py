import ctypes
from ctypes import Structure, c_double, c_int, c_size_t
import customtkinter as ctk
import os
import threading

# Load DLL-uri
dll_path = os.path.join(os.path.dirname(__file__), "program_testare_performanta.dll")
dll = ctypes.CDLL(dll_path)

benchmark_dll_path = os.path.join(os.path.dirname(__file__), "Data_transfer_speed.dll")
benchmark_dll = ctypes.CDLL(benchmark_dll_path)

# Setup pentru DLL info
dll.getCPUInfo.restype = ctypes.c_char_p
dll.getCacheInfo.restype = ctypes.c_char_p
dll.getMemPagingInfo.restype = ctypes.c_char_p
dll.getRAMInfo.restype = ctypes.c_char_p


# Setup pentru Benchmark DLL
class BenchmarkResult(Structure):
    _fields_ = [
        ("sequential", c_double),
        ("stride_count", c_int),
        ("strides", c_int * 3),
        ("strided_results", c_double * 3),
        ("random", c_double)
    ]


benchmark_dll.RunBenchmark.argtypes = [c_size_t, ctypes.POINTER(BenchmarkResult)]
benchmark_dll.RunBenchmark.restype = None

# Theme setup
ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

app = ctk.CTk()
app.title("PC Benchmark App")
app.geometry("1000x650")

icon_path = "logo.ico"
try:
    if os.path.exists(icon_path):
        app.iconbitmap(icon_path)
except Exception as e:
    print(f"Error loading icon: {e}")

text_font = ctk.CTkFont(family="Consolas", size=14)

# ========== MAIN MENU FRAME ==========
main_frame = ctk.CTkFrame(app)
main_frame.pack(fill="both", expand=True)

btn_cpu = ctk.CTkButton(main_frame, text="CPU Info", width=200, height=40)
btn_cache = ctk.CTkButton(main_frame, text="Cache Info", width=200, height=40)
btn_paging = ctk.CTkButton(main_frame, text="Paging Info", width=200, height=40)
btn_ram = ctk.CTkButton(main_frame, text="RAM Info", width=200, height=40)
btn_benchmark = ctk.CTkButton(main_frame, text="Test Data Transfer Speed",
                              width=200, height=40, fg_color="#4CAF50")

btn_cpu.pack(pady=10)
btn_cache.pack(pady=10)
btn_paging.pack(pady=10)
btn_ram.pack(pady=10)
btn_benchmark.pack(pady=10)

# ========== INFO FRAME (pentru CPU, Cache, etc) ==========
info_frame = ctk.CTkFrame(app)

info_label = ctk.CTkLabel(info_frame, text="", font=ctk.CTkFont(family="Consolas", size=16))
info_label.pack(pady=(10, 5))

text_widget = ctk.CTkTextbox(info_frame, width=880, height=500, font=text_font)
text_widget.pack(padx=10, pady=10)
text_widget.configure(state="disabled")

btn_back_info = ctk.CTkButton(info_frame, text="Back", width=100)
btn_back_info.pack(pady=10)

# ========== BENCHMARK FRAME ==========
benchmark_frame = ctk.CTkFrame(app)

# Title
benchmark_title = ctk.CTkLabel(benchmark_frame,
                               text="Data Transfer Benchmark",
                               font=ctk.CTkFont(family="Arial", size=20, weight="bold"))
benchmark_title.pack(pady=20)

# Warning label
warning_label = ctk.CTkLabel(benchmark_frame,
                             text="WARNING: Close all other applications and processes during testing!",
                             font=ctk.CTkFont(family="Arial", size=13),
                             text_color="#FF6B6B")
warning_label.pack(pady=10)

duration_label = ctk.CTkLabel(benchmark_frame,
                              text="NOTE: Tests may take several minutes to complete.",
                              font=ctk.CTkFont(family="Arial", size=12),
                              text_color="#FFA500")
duration_label.pack(pady=5)

# Block sizes selection
sizes_label = ctk.CTkLabel(benchmark_frame,
                           text="Select block sizes to test:",
                           font=ctk.CTkFont(family="Arial", size=12))
sizes_label.pack(pady=10)

sizes_frame = ctk.CTkFrame(benchmark_frame)
sizes_frame.pack(pady=5)

checkboxes = {}
sizes_config = [
    (64 / 1024, "64 KB"),      # 64
    (512 / 1024, "512 KB"),    # 512 KB
    (16, "16 MB"),
    (128, "128 MB"),
    (512, "512 MB"),
    (1024, "1024 MB")
]

for i, (size_mb, label) in enumerate(sizes_config):
    var = ctk.BooleanVar(value=True)
    cb = ctk.CTkCheckBox(sizes_frame, text=label, variable=var,
    font=ctk.CTkFont(family="Arial", size=11))
    cb.grid(row=0, column=i, padx=15, pady=5)
    checkboxes[size_mb] = var

btn_run_benchmark = ctk.CTkButton(benchmark_frame,
                                  text="START TEST",
                                  width=200, height=50,
                                  font=ctk.CTkFont(family="Arial", size=16, weight="bold"),
                                  fg_color="#4CAF50",
                                  hover_color="#45a049")
btn_run_benchmark.pack(pady=20)

# Progress bar
progress_bar = ctk.CTkProgressBar(benchmark_frame, width=500, mode="indeterminate")
progress_bar.pack(pady=10)
progress_bar.set(0)

# Results label
results_label = ctk.CTkLabel(benchmark_frame,
                             text="Results:",
                             font=ctk.CTkFont(family="Arial", size=13, weight="bold"))
results_label.pack(pady=5)

# Results text area
benchmark_text = ctk.CTkTextbox(benchmark_frame, width=880, height=300,
                                font=ctk.CTkFont(family="Consolas", size=14))
benchmark_text.pack(padx=10, pady=10)
benchmark_text.configure(state="disabled")

# Back button
btn_back_benchmark = ctk.CTkButton(benchmark_frame, text="Back to Menu",
                                   width=150, height=35)
btn_back_benchmark.pack(pady=10)

# ========== GLOBAL STATE ==========
is_running = False


# ========== FUNCTIONS ==========

def show_info(dll_func, label_text):
    main_frame.pack_forget()
    info_frame.pack(fill="both", expand=True)

    info_label.configure(text=label_text)

    text_widget.configure(state="normal")
    text_widget.delete("0.0", "end")
    text_widget.insert("0.0", dll_func().decode())
    text_widget.configure(state="disabled")


def go_back_from_info():
    info_frame.pack_forget()
    main_frame.pack(fill="both", expand=True)


def show_benchmark():
    main_frame.pack_forget()
    benchmark_frame.pack(fill="both", expand=True)

    # Initial message
    benchmark_text.configure(state="normal")
    benchmark_text.delete("0.0", "end")
    benchmark_text.insert("0.0", "Ready to run benchmark.\n")
    benchmark_text.insert("end", "Select block sizes and click 'START TEST'...\n")
    benchmark_text.configure(state="disabled")


def go_back_from_benchmark():
    global is_running
    if not is_running:
        benchmark_frame.pack_forget()
        main_frame.pack(fill="both", expand=True)


def log_benchmark(message):
    """Thread-safe logging to benchmark text widget"""

    def _log():
        benchmark_text.configure(state="normal")
        benchmark_text.insert("end", message)
        benchmark_text.see("end")
        benchmark_text.configure(state="disabled")

    app.after(0, _log)


def clear_benchmark_log():
    benchmark_text.configure(state="normal")
    benchmark_text.delete("0.0", "end")
    benchmark_text.configure(state="disabled")


def run_benchmark_thread(selected_sizes_with_labels):
    global is_running

    try:
        log_benchmark("=" * 50 + "\n")
        log_benchmark("     DATA TRANSFER SPEED BENCHMARK STARTED\n")
        log_benchmark("=" * 50 + "\n\n")
        log_benchmark("Initializing benchmark...\n")
        log_benchmark(f"Testing {len(selected_sizes_with_labels)} block size(s)\n\n")

        for size_mb, label in selected_sizes_with_labels:
            log_benchmark(f"{'-' * 50}\n")
            log_benchmark(f"Block Size: {label}\n")  # <-- Folosește label-ul
            log_benchmark(f"{'-' * 50}\n")

            result = BenchmarkResult()
            benchmark_dll.RunBenchmark(int(size_mb * 1024 * 1024), ctypes.byref(result))

            log_benchmark(f"  Sequential access:  {result.sequential:>10.2f} MB/s\n")

            for i in range(result.stride_count):
                stride = result.strides[i]
                speed = result.strided_results[i]
                log_benchmark(f"  Strided access (stride = {stride:>3}): {speed:>10.2f} MB/s\n")

            log_benchmark(f"  Random access:      {result.random:>10.2f} MB/s\n")
            log_benchmark("\n")

        log_benchmark("=" * 50 + "\n")
        log_benchmark("ALL TESTS COMPLETED SUCCESSFULLY!\n")
        log_benchmark("=" * 50 + "\n")

    except Exception as e:
        log_benchmark(f"\nERROR: {str(e)}\n")

    finally:
        is_running = False
        app.after(0, finish_benchmark)


def finish_benchmark():
    progress_bar.stop()
    progress_bar.set(0)
    btn_run_benchmark.configure(state="normal", text="START TEST", fg_color="#4CAF50")
    btn_back_benchmark.configure(state="normal")


def start_benchmark():
    global is_running

    selected_with_labels = [(size_mb, label)
                           for (size_mb, label), var in zip(sizes_config, checkboxes.values())
                           if var.get()]

    if not selected_with_labels:
        log_benchmark("\nWARNING: Please select at least one block size!\n")
        return

    if is_running:
        return

    is_running = True

    # UI updates
    btn_run_benchmark.configure(state="disabled", text="RUNNING...", fg_color="#808080")
    btn_back_benchmark.configure(state="disabled")
    progress_bar.set(0)
    progress_bar.start()
    clear_benchmark_log()

    # Run in separate thread
    thread = threading.Thread(target=run_benchmark_thread, args=(selected_with_labels,))
    thread.daemon = True
    thread.start()


# ========== BUTTON COMMANDS ==========

btn_cpu.configure(command=lambda: show_info(dll.getCPUInfo, "CPU Information"))
btn_cache.configure(command=lambda: show_info(dll.getCacheInfo, "Cache Memory Information"))
btn_paging.configure(command=lambda: show_info(dll.getMemPagingInfo, "Memory Paging Information"))
btn_ram.configure(command=lambda: show_info(dll.getRAMInfo, "RAM Information"))
btn_benchmark.configure(command=show_benchmark)

btn_back_info.configure(command=go_back_from_info)
btn_back_benchmark.configure(command=go_back_from_benchmark)
btn_run_benchmark.configure(command=start_benchmark)

app.mainloop()