# 📊 Benchmarking app
## PC benchmarking app for testing the system's performance and showing detailed info about the hardware specs

This benchmarking application features a modern Python graphical interface (using **`CustomTkinter`** library) and a C++ backend. It extracts detailed hardware information, such as CPU, RAM, and cache memory specifications, and runs precise benchmarks to evaluate data transfer speeds, as well as the execution speed of arithmetic and logical operations. For a comprehensive analysis, users can run all tests sequentially to automatically generate a detailed performance report that is saved as a text file.<br><br>

## :hammer_and_wrench: Tools and Technologies
* **`C++ (MSVC)`** – Employed for the high-performance backend modules, including hardware-specific instructions (CPUID) and intensive benchmarking algorithms compiled as DLLs for maximum efficiency.
* **`Windows API & WMI`** (Windows Management Instrumentation) – Used to interact with the **operating system** to query hardware details, such as CPU architecture, RAM configurations, and system paging parameters.
* **`Python 3`** – Used for the main application logic, handling the graphical user interface and managing thread execution for the benchmark processes.
* **`CustomTkinter`** – A modern Python framework used to develop the GUI, providing a clean, responsive layout.
* **`ctypes`** – A Python library that facilitates communication between the GUI and the compiled C++ DLLs, enabling the application to execute performance-critical code directly from the Python environment.<br><br>

## ⚙️ C++ modules
* **`System_info.cpp`** - This module extracts detailed hardware information, including CPU specifications (vendor, model, frequency, cache hierarchy) and memory details (RAM capacity, channel configuration). It uses the CPUID instruction for CPU details and Windows API/WMI calls for memory-related information.
* **`Data_transfer_speed.cpp`** - The module provides benchmarks to evaluate data transfer speeds by performing sequential, strided, and random memory access tests. It uses dynamic allocation, cache-flushing techniques to ensure accuracy, and a `PrecisionTimer` class for high-resolution measurement.
* **`Integer_operations.cpp`** – Designed to evaluate the performance of integer arithmetic and logical operations, this module applies a sequence of 13 distinct operations to randomized data buffers. It employs synchronization barriers and "warm-up" runs to ensure stable, repeatable results measured in Giga Operations per Second (**GOps**).
* **`Floating_point_operations.cpp`** – This module assesses the processor's floating-point performance (FPU) by computing the Mandelbrot fractal. It executes complex arithmetic operations on `double` precision numbers and reports performance in Giga Floating Point Operations per Second (**GFLOPS**).<br>

To ensure efficient integration between the C++ modules and the GUI, the source code was compiled into **`Dynamic-Link Libraries`** (DLLs). <br><br>

## 🖼️ GUI
The app features a modern and intuitive graphical user interface developed using the `customtkinter` framework, which provides a clean design and support for system themes. The interface is structured to ensure a pleasant user experience while providing efficient access to system analysis and benchmarking tools.
Key features of the interface include:
* **Structured Layout**: The main dashboard is organized into three distinct areas:
    - Hardware Information (Left Panel): Provides quick access to detailed system reports (CPU specs, cache memory, RAM configuration).
    - Performance Benchmarks (Right Panel): Allows the user to select and initiate specific tests for data transfer speeds, integer or floating-point arithmetic operations.
    - Footer: A dedicated button enables the execution of all performance benchmarks sequentially, automatically generating a comprehensive report saved to a timestamped text file. <br>
<div align="center">
  <img width="600" height="797" alt="image" src="https://github.com/user-attachments/assets/38256ac1-1cc4-4a57-826b-2bf931785472" />
</div><br><br>

* **Advanced Interaction Logic**:
    - Asynchronous Execution (Multithreading): All benchmark tests are executed in separate threads. This ensures the GUI remains responsive, preventing the application from freezing during intensive computational tasks.
    - User Feedback: The application provides real-time status updates through animated progress bars and label changes (e.g., displaying "RUNNING..." during active benchmarks).
    - Test Customization: Within the benchmark frames, users can select specific memory block sizes (e.g., 64 KB to 1 GB) using intuitive checkboxes before starting a test.
    - Guided Navigation: The interface uses a hierarchical structure where individual test frames include safety warnings to close background applications, notes on estimated duration, and "Back" buttons for seamless navigation to the main menu.
    - Input Protection: To prevent system instability or conflicting processes, menu buttons are programmatically disabled while tests are in progress. <br>

<div align="center">
  <img width="600" height="757" alt="image" src="https://github.com/user-attachments/assets/a7633618-b125-4e6b-b6e4-63c725f02132" /><br><br>
  <img width="600" height="654" alt="image" src="https://github.com/user-attachments/assets/c43321d5-f651-4dd1-9024-a8caac74841a" /> <br><br>
  <img width="600" height="755" alt="image" src="https://github.com/user-attachments/assets/48f07128-f630-4e37-aeb6-98ee3e9fe280" /> <br><br>
  <img width="600" height="700" alt="image" src="https://github.com/user-attachments/assets/2e60e644-dee6-4e6d-9501-4ba4185d6027" />
</div><br><br>

## Comparative Performance Analysis
The app is designed to enable the objective evaluation and comparison of performance across different hardware configurations. By executing the same suite of benchmarks on various systems, users can identify specific hardware limitations and gain a clear understanding of how different components impact overall system speed.<br><br>

## 📥 Installation and Usage Guide
















