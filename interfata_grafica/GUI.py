import ctypes
from ctypes import Structure, c_double, c_int, c_size_t, c_char_p
import customtkinter as ctk
import os
import threading
import time
import datetime


dll_path = os.path.join(os.path.dirname(__file__), "System_info.dll")
try:
    dll = ctypes.CDLL(dll_path)
    dll.getCPUInfo.restype = c_char_p
    dll.getCacheInfo.restype = c_char_p
    dll.getMemPagingInfo.restype = c_char_p
    dll.getRAMInfo.restype = c_char_p
except OSError:
    print("Error loading System_info.dll")

benchmark_dll_path = os.path.join(os.path.dirname(__file__), "Data_transfer_speed.dll")
try:
    benchmark_dll = ctypes.CDLL(benchmark_dll_path)

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
except OSError:
    print("Error loading Data_transfer_speed.dll")

integer_dll_path = os.path.join(os.path.dirname(__file__), "Integer_operations.dll")
try:
    integer_dll = ctypes.CDLL(integer_dll_path)

    class IntegerBenchmarkResult(Structure):
        _fields_ = [
            ("gops", c_double),
            ("latency_ns", c_double),
            ("duration", c_double)
        ]

    integer_dll.runIntegerBenchmark.restype = IntegerBenchmarkResult
    integer_dll.runIntegerBenchmark.argtypes = []
except OSError:
    print("Error loading Integer_operations.dll")

fpu_dll_path = os.path.join(os.path.dirname(__file__), "Floating_point_operations.dll")
try:
    fpu_dll = ctypes.CDLL(fpu_dll_path)

    class FPUBenchmarkResult(Structure):
        _fields_ = [
            ("gflops", c_double),
            ("latency_ns", c_double),
            ("duration", c_double)
        ]

    fpu_dll.runMandelbrotBenchmark.restype = FPUBenchmarkResult
    fpu_dll.runMandelbrotBenchmark.argtypes = []
except OSError:
    print("Error loading Floating_point_operations.dll")



ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

app = ctk.CTk()
app.title("PC Benchmark App")
app.geometry("1000x750")

icon_path = "logo.ico"
try:
    if os.path.exists(icon_path):
        app.iconbitmap(icon_path)
except Exception as e:
    print(f"Error loading icon: {e}")

text_font = ctk.CTkFont(family="Consolas", size=14)

main_frame = ctk.CTkFrame(app)
main_frame.pack(fill="both", expand=True)

MAIN_BTN_COLOR = "#2196F3"
MAIN_BTN_HOVER = "#1976D2"

menu_container = ctk.CTkFrame(main_frame, fg_color="transparent")
menu_container.place(relx=0.5, rely=0.45, anchor="center")

left_frame = ctk.CTkFrame(menu_container, fg_color="transparent")
left_frame.pack(side="left", padx=50, pady=20, anchor="n")

ctk.CTkLabel(left_frame, text="Hardware Information", font=("Arial", 20, "bold")).pack(pady=(0, 20))

btn_cpu = ctk.CTkButton(left_frame, text="CPU Info", width=250, height=45,
                        fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_cpu.pack(pady=10)

btn_cache = ctk.CTkButton(left_frame, text="Cache Memory Info", width=250, height=45,
                          fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_cache.pack(pady=10)

btn_paging = ctk.CTkButton(left_frame, text="Paging Info", width=250, height=45,
                           fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_paging.pack(pady=10)

btn_ram = ctk.CTkButton(left_frame, text="RAM Info", width=250, height=45,
                        fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_ram.pack(pady=10)

right_frame = ctk.CTkFrame(menu_container, fg_color="transparent")
right_frame.pack(side="right", padx=50, pady=20, anchor="n")

ctk.CTkLabel(right_frame, text="Performance Benchmarks", font=("Arial", 20, "bold")).pack(pady=(0, 20))

btn_benchmark = ctk.CTkButton(right_frame, text="Test Data Transfer Speed",
                              width=250, height=45,
                              fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_benchmark.pack(pady=10)

btn_integer = ctk.CTkButton(right_frame, text="Test Integer Operations",
                            width=250, height=45,
                            fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_integer.pack(pady=10)

btn_fpu = ctk.CTkButton(right_frame, text="Test Floating Point Operations",
                        width=250, height=45,
                        fg_color=MAIN_BTN_COLOR, hover_color=MAIN_BTN_HOVER)
btn_fpu.pack(pady=10)

footer_frame = ctk.CTkFrame(main_frame, fg_color="transparent")
footer_frame.pack(side="bottom", pady=40)

btn_run_all = ctk.CTkButton(footer_frame, text="RUN ALL PERFORMANCE TESTS & SAVE REPORT",
                            width=400, height=50,
                            font=("Arial", 14, "bold"),
                            fg_color="#D32F2F", hover_color="#B71C1C")
btn_run_all.pack()

status_label = ctk.CTkLabel(footer_frame, text="", font=("Arial", 12), text_color="white")
status_label.pack(pady=5)

info_frame = ctk.CTkFrame(app)

info_label = ctk.CTkLabel(info_frame, text="", font=ctk.CTkFont(family="Consolas", size=16))
info_label.pack(pady=(10, 5))

text_widget = ctk.CTkTextbox(info_frame, width=880, height=500, font=text_font)
text_widget.pack(padx=10, pady=10)
text_widget.configure(state="disabled")

btn_back_info = ctk.CTkButton(info_frame, text="Back", width=100)
btn_back_info.pack(pady=10)

benchmark_frame = ctk.CTkFrame(app)

benchmark_title = ctk.CTkLabel(benchmark_frame,
                               text="Data Transfer Benchmark",
                               font=ctk.CTkFont(family="Arial", size=20, weight="bold"))
benchmark_title.pack(pady=20)

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

sizes_label = ctk.CTkLabel(benchmark_frame,
                           text="Select block sizes to test:",
                           font=ctk.CTkFont(family="Arial", size=12))
sizes_label.pack(pady=10)

sizes_frame = ctk.CTkFrame(benchmark_frame)
sizes_frame.pack(pady=5)

checkboxes = {}
sizes_config = [
    (64 / 1024, "64 KB"),
    (512 / 1024, "512 KB"),
    (16, "16 MB"),
    (128, "128 MB"),
    (512, "512 MB"),
    (1024, "1 GB")
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

progress_bar = ctk.CTkProgressBar(benchmark_frame, width=500, mode="indeterminate")
progress_bar.pack(pady=10)
progress_bar.set(0)

results_label = ctk.CTkLabel(benchmark_frame,
                             text="Results:",
                             font=ctk.CTkFont(family="Arial", size=13, weight="bold"))
results_label.pack(pady=5)

benchmark_text = ctk.CTkTextbox(benchmark_frame, width=880, height=300,
                                font=ctk.CTkFont(family="Consolas", size=14))
benchmark_text.pack(padx=10, pady=10)
benchmark_text.configure(state="disabled")

btn_back_benchmark = ctk.CTkButton(benchmark_frame, text="Back to Menu",
                                   width=150, height=35)
btn_back_benchmark.pack(pady=10)

integer_frame = ctk.CTkFrame(app)

integer_title = ctk.CTkLabel(integer_frame,
                             text="Integer Operations Benchmark",
                             font=ctk.CTkFont(family="Arial", size=20, weight="bold"))
integer_title.pack(pady=20)

integer_warning_label = ctk.CTkLabel(integer_frame,
                                     text="WARNING: Close all other applications and processes during testing!",
                                     font=ctk.CTkFont(family="Arial", size=13),
                                     text_color="#FF6B6B")
integer_warning_label.pack(pady=10)

integer_duration_label = ctk.CTkLabel(integer_frame,
                                      text="NOTE: Test may take several minutes to complete.",
                                      font=ctk.CTkFont(family="Arial", size=12),
                                      text_color="#FFA500")
integer_duration_label.pack(pady=5)

integer_info_label = ctk.CTkLabel(integer_frame,
                                  text="This benchmark measures the speed of integer operations:\n"
                                       "ADD, SUB, MUL, XOR, NOT, ROL, AND",
                                  font=ctk.CTkFont(family="Arial", size=12))
integer_info_label.pack(pady=10)

btn_run_integer = ctk.CTkButton(integer_frame,
                                text="START TEST",
                                width=200, height=50,
                                font=ctk.CTkFont(family="Arial", size=16, weight="bold"),
                                fg_color="#4CAF50",  # Verde
                                hover_color="#45a049")
btn_run_integer.pack(pady=20)

integer_progress_bar = ctk.CTkProgressBar(integer_frame, width=500, mode="indeterminate")
integer_progress_bar.pack(pady=10)
integer_progress_bar.set(0)

integer_results_label = ctk.CTkLabel(integer_frame,
                                     text="Results:",
                                     font=ctk.CTkFont(family="Arial", size=13, weight="bold"))
integer_results_label.pack(pady=5)

integer_text = ctk.CTkTextbox(integer_frame, width=880, height=300,
                              font=ctk.CTkFont(family="Consolas", size=14))
integer_text.pack(padx=10, pady=10)
integer_text.configure(state="disabled")

btn_back_integer = ctk.CTkButton(integer_frame, text="Back to Menu",
                                 width=150, height=35)
btn_back_integer.pack(pady=10)

fpu_frame = ctk.CTkFrame(app)

fpu_title = ctk.CTkLabel(fpu_frame,
                         text="Floating Point Operations Benchmark",
                         font=ctk.CTkFont(family="Arial", size=20, weight="bold"))
fpu_title.pack(pady=20)

fpu_warning_label = ctk.CTkLabel(fpu_frame,
                                 text="WARNING: Close all other applications and processes during testing!",
                                 font=ctk.CTkFont(family="Arial", size=13),
                                 text_color="#FF6B6B")
fpu_warning_label.pack(pady=10)

fpu_duration_label = ctk.CTkLabel(fpu_frame,
                                  text="NOTE: Test may take several minutes to complete.",
                                  font=ctk.CTkFont(family="Arial", size=12),
                                  text_color="#FFA500")
fpu_duration_label.pack(pady=5)

fpu_info_label = ctk.CTkLabel(fpu_frame,
                              text="This benchmark measures the speed of floating point operations using\n"
                                   "Mandelbrot fractal calculations",
                              font=ctk.CTkFont(family="Arial", size=12))
fpu_info_label.pack(pady=10)

btn_run_fpu = ctk.CTkButton(fpu_frame,
                            text="START TEST",
                            width=200, height=50,
                            font=ctk.CTkFont(family="Arial", size=16, weight="bold"),
                            fg_color="#4CAF50",  # Verde
                            hover_color="#45a049")
btn_run_fpu.pack(pady=20)

fpu_progress_bar = ctk.CTkProgressBar(fpu_frame, width=500, mode="indeterminate")
fpu_progress_bar.pack(pady=10)
fpu_progress_bar.set(0)

fpu_results_label = ctk.CTkLabel(fpu_frame,
                                 text="Results:",
                                 font=ctk.CTkFont(family="Arial", size=13, weight="bold"))
fpu_results_label.pack(pady=5)

fpu_text = ctk.CTkTextbox(fpu_frame, width=880, height=300,
                          font=ctk.CTkFont(family="Consolas", size=14))
fpu_text.pack(padx=10, pady=10)
fpu_text.configure(state="disabled")

btn_back_fpu = ctk.CTkButton(fpu_frame, text="Back to Menu",
                             width=150, height=35)
btn_back_fpu.pack(pady=10)


is_running = False
is_integer_running = False
is_fpu_running = False
is_full_report_running = False


def toggle_menu_buttons(state):
    btn_cpu.configure(state=state)
    btn_cache.configure(state=state)
    btn_paging.configure(state=state)
    btn_ram.configure(state=state)
    btn_benchmark.configure(state=state)
    btn_integer.configure(state=state)
    btn_fpu.configure(state=state)


def show_info(dll_func, label_text):
    main_frame.pack_forget()
    info_frame.pack(fill="both", expand=True)

    info_label.configure(text=label_text)

    text_widget.configure(state="normal")
    text_widget.delete("0.0", "end")
    try:
        text_widget.insert("0.0", dll_func().decode())
    except Exception as e:
        text_widget.insert("0.0", f"Error retrieving info: {e}")
    text_widget.configure(state="disabled")


def go_back_from_info():
    info_frame.pack_forget()
    main_frame.pack(fill="both", expand=True)


def show_benchmark():
    main_frame.pack_forget()
    benchmark_frame.pack(fill="both", expand=True)

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


def show_integer():
    main_frame.pack_forget()
    integer_frame.pack(fill="both", expand=True)

    integer_text.configure(state="normal")
    integer_text.delete("0.0", "end")
    integer_text.insert("0.0", "Ready to run Integer Operations Benchmark.\n")
    integer_text.insert("end", "Click 'START TEST' to begin...\n")
    integer_text.configure(state="disabled")


def go_back_from_integer():
    global is_integer_running
    if not is_integer_running:
        integer_frame.pack_forget()
        main_frame.pack(fill="both", expand=True)


def show_fpu():
    main_frame.pack_forget()
    fpu_frame.pack(fill="both", expand=True)

    fpu_text.configure(state="normal")
    fpu_text.delete("0.0", "end")
    fpu_text.insert("0.0", "Ready to run Floating Point Operations Benchmark.\n")
    fpu_text.insert("end", "Click 'START TEST' to begin...\n")
    fpu_text.configure(state="disabled")


def go_back_from_fpu():
    global is_fpu_running
    if not is_fpu_running:
        fpu_frame.pack_forget()
        main_frame.pack(fill="both", expand=True)


def log_benchmark(message):
    def _log():
        benchmark_text.configure(state="normal")
        benchmark_text.insert("end", message)
        benchmark_text.see("end")
        benchmark_text.configure(state="disabled")

    app.after(0, _log)


def log_integer(message):
    def _log():
        integer_text.configure(state="normal")
        integer_text.insert("end", message)
        integer_text.see("end")
        integer_text.configure(state="disabled")

    app.after(0, _log)


def log_fpu(message):
    def _log():
        fpu_text.configure(state="normal")
        fpu_text.insert("end", message)
        fpu_text.see("end")
        fpu_text.configure(state="disabled")

    app.after(0, _log)


def clear_benchmark_log():
    benchmark_text.configure(state="normal")
    benchmark_text.delete("0.0", "end")
    benchmark_text.configure(state="disabled")


def clear_integer_log():
    integer_text.configure(state="normal")
    integer_text.delete("0.0", "end")
    integer_text.configure(state="disabled")


def clear_fpu_log():
    fpu_text.configure(state="normal")
    fpu_text.delete("0.0", "end")
    fpu_text.configure(state="disabled")


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
            log_benchmark(f"Block Size: {label}\n")
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


def run_integer_thread():
    global is_integer_running

    try:
        log_integer("Please wait, this may take several minutes...\n\n")

        # Run benchmark
        result = integer_dll.runIntegerBenchmark()

        log_integer("-" * 50 + "\n")
        log_integer("                 RESULTS\n")
        log_integer("-" * 50 + "\n\n")

        log_integer(f"  Performance:  {result.gops:.3f} GOps\n")
        log_integer(f"               (Giga Operations per Second)\n\n")

        log_integer(f"  Latency:      {result.latency_ns:.4f} ns/op\n")
        log_integer(f"               (nanoseconds per operation)\n\n")

        log_integer(f"  Duration:     {result.duration:.1f} seconds\n\n")

        log_integer("=" * 50 + "\n")
        log_integer("BENCHMARK COMPLETED SUCCESSFULLY!\n")
        log_integer("=" * 50 + "\n")

    except Exception as e:
        log_integer(f"\nERROR: {str(e)}\n")

    finally:
        is_integer_running = False
        app.after(0, finish_integer)


def run_fpu_thread():
    global is_fpu_running

    try:
        log_fpu("Please wait, this may take several minutes...\n\n")

        result = fpu_dll.runMandelbrotBenchmark()

        log_fpu("-" * 50 + "\n")
        log_fpu("                 RESULTS\n")
        log_fpu("-" * 50 + "\n\n")

        log_fpu(f"  Score:        {result.gflops:.3f} GFLOPS\n")
        log_fpu(f"               (Giga Floating Point Ops/Sec)\n\n")

        log_fpu(f"  Latency:      {result.latency_ns:.4f} ns/op\n")
        log_fpu(f"               (nanoseconds per operation)\n\n")

        log_fpu(f"  Duration:     {result.duration:.3f} seconds\n\n")

        log_fpu("=" * 50 + "\n")
        log_fpu("BENCHMARK COMPLETED SUCCESSFULLY!\n")
        log_fpu("=" * 50 + "\n")

    except Exception as e:
        log_fpu(f"\nERROR: {str(e)}\n")

    finally:
        is_fpu_running = False
        app.after(0, finish_fpu)


def finish_benchmark():
    progress_bar.stop()
    progress_bar.set(0)
    btn_run_benchmark.configure(state="normal", text="START TEST", fg_color="#4CAF50")
    btn_back_benchmark.configure(state="normal")


def finish_integer():
    integer_progress_bar.stop()
    integer_progress_bar.set(0)
    btn_run_integer.configure(state="normal", text="START TEST", fg_color="#4CAF50")
    btn_back_integer.configure(state="normal")


def finish_fpu():
    fpu_progress_bar.stop()
    fpu_progress_bar.set(0)
    btn_run_fpu.configure(state="normal", text="START TEST", fg_color="#4CAF50")
    btn_back_fpu.configure(state="normal")


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

    btn_run_benchmark.configure(state="disabled", text="RUNNING...", fg_color="#808080")
    btn_back_benchmark.configure(state="disabled")
    progress_bar.set(0)
    progress_bar.start()
    clear_benchmark_log()

    thread = threading.Thread(target=run_benchmark_thread, args=(selected_with_labels,))
    thread.daemon = True
    thread.start()


def start_integer():
    global is_integer_running

    if is_integer_running:
        return

    is_integer_running = True

    btn_run_integer.configure(state="disabled", text="RUNNING...", fg_color="#808080")
    btn_back_integer.configure(state="disabled")
    integer_progress_bar.set(0)
    integer_progress_bar.start()
    clear_integer_log()

    thread = threading.Thread(target=run_integer_thread)
    thread.daemon = True
    thread.start()


def start_fpu():
    global is_fpu_running

    if is_fpu_running:
        return

    is_fpu_running = True

    btn_run_fpu.configure(state="disabled", text="RUNNING...", fg_color="#808080")
    btn_back_fpu.configure(state="disabled")
    fpu_progress_bar.set(0)
    fpu_progress_bar.start()
    clear_fpu_log()

    thread = threading.Thread(target=run_fpu_thread)
    thread.daemon = True
    thread.start()


def run_full_report_thread():
    global is_full_report_running
    report = []
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    def update_status(msg):
        app.after(0, lambda: status_label.configure(text=msg))

    try:
        # hardware information
        update_status("Gathering Hardware Information...")
        report.append("=" * 60)
        report.append(f"PC BENCHMARK REPORT - {timestamp}")
        report.append("=" * 60 + "\n")

        if dll:
            report.append("--- 1. CPU INFORMATION ---")
            report.append(dll.getCPUInfo().decode())
            report.append("\n--- 2. CACHE MEMORY ---")
            report.append(dll.getCacheInfo().decode())
            report.append("\n--- 3. MEMORY PAGING ---")
            report.append(dll.getMemPagingInfo().decode())
            report.append("\n--- 4. RAM INFORMATION ---")
            report.append(dll.getRAMInfo().decode())
            report.append("-" * 60 + "\n")

        # integer operations tests
        update_status("Running Integer Operations Benchmark...")
        report.append("--- 5. INTEGER OPERATIONS BENCHMARK ---")
        int_res = integer_dll.runIntegerBenchmark()
        report.append(f"Performance:  {int_res.gops:.3f} GOps")
        report.append(f"Latency:      {int_res.latency_ns:.4f} ns/op")
        report.append(f"Duration:     {int_res.duration:.2f} s")
        report.append("")

        time.sleep(3)

        # floating point operations tests
        update_status("Running Floating Point Operations Benchmark...")
        report.append("--- 6. FLOATING POINT OPERATIONS BENCHMARK ---")
        fpu_res = fpu_dll.runMandelbrotBenchmark()
        report.append(f"Score:        {fpu_res.gflops:.3f} GFLOPS")
        report.append(f"Latency:      {fpu_res.latency_ns:.4f} ns/op")
        report.append(f"Duration:     {fpu_res.duration:.2f} s")
        report.append("")

        time.sleep(3)

        # data transfer tests
        update_status("Running Data Transfer Tests...")
        report.append("--- 7. DATA TRANSFER SPEED ---")

        all_sizes_to_test = sizes_config
        for size_mb, label in all_sizes_to_test:
            update_status(f"Testing Memory Block Transfer: {label}...")
            report.append(f"[ Block Size: {label} ]")
            res = BenchmarkResult()
            benchmark_dll.RunBenchmark(int(size_mb * 1024 * 1024), ctypes.byref(res))
            report.append(f"  Sequential Access: {res.sequential:.2f} MB/s")
            for i in range(res.stride_count):
                stride_val = res.strides[i]
                speed_val = res.strided_results[i]
                report.append(f"  Strided Access (Stride {stride_val}): {speed_val:.2f} MB/s")
            report.append(f"  Random Access:   {res.random:.2f} MB/s")
            report.append("")

        report.append("=" * 60)
        report.append("END OF REPORT")

        # salvare fisier
        with open("benchmark_report.txt", "w") as f:
            f.write("\n".join(report))

        update_status("Done! Results saved to benchmark_report.txt")

    except Exception as e:
        update_status(f"Error: {str(e)}")
    finally:
        is_full_report_running = False
        app.after(0, lambda: btn_run_all.configure(state="normal", text="RUN ALL PERFORMANCE TESTS & SAVE REPORT",
                                                   fg_color="#D32F2F"))
        app.after(0, lambda: toggle_menu_buttons("normal"))


def start_full_report():
    global is_full_report_running
    if is_full_report_running: return
    if is_running or is_integer_running or is_fpu_running:
        status_label.configure(text="Wait for other tests!", text_color="red")
        return

    is_full_report_running = True
    btn_run_all.configure(state="disabled", text="RUNNING TESTS...", fg_color="gray")
    status_label.configure(text="Starting...", text_color="white")

    toggle_menu_buttons("disabled")

    thread = threading.Thread(target=run_full_report_thread, daemon=True)
    thread.start()


# button commands bindings
btn_cpu.configure(command=lambda: show_info(dll.getCPUInfo, "CPU Information"))
btn_cache.configure(command=lambda: show_info(dll.getCacheInfo, "Cache Memory Information"))
btn_paging.configure(command=lambda: show_info(dll.getMemPagingInfo, "Memory Paging Information"))
btn_ram.configure(command=lambda: show_info(dll.getRAMInfo, "RAM Information"))
btn_benchmark.configure(command=show_benchmark)
btn_integer.configure(command=show_integer)
btn_fpu.configure(command=show_fpu)

btn_back_info.configure(command=go_back_from_info)
btn_back_benchmark.configure(command=go_back_from_benchmark)
btn_back_integer.configure(command=go_back_from_integer)
btn_back_fpu.configure(command=go_back_from_fpu)

btn_run_benchmark.configure(command=start_benchmark)
btn_run_integer.configure(command=start_integer)
btn_run_fpu.configure(command=start_fpu)
btn_run_all.configure(command=start_full_report)

def main():
    app.mainloop()

if __name__ == "__main__":
    main()