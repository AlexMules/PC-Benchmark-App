# Benchmarking app 
## PC benchmarking app for testing the system's performance and showing detailed info about the hardware specs

This benchmarking application features a modern Python graphical interface (using **`CustomTkinter`** library) and a C++ backend. It extracts detailed hardware information, such as CPU, RAM, and cache memory specifications, and runs precise benchmarks to evaluate data transfer speeds, as well as the execution speed of arithmetic and logical operations. For a comprehensive analysis, users can run all tests sequentially to automatically generate a detailed performance report that is saved as a text file.

## :hammer_and_wrench: Tools and Technologies
* **`C++ (MSVC)`** – Employed for the high-performance backend modules, including hardware-specific instructions (CPUID) and intensive benchmarking algorithms compiled as DLLs for maximum efficiency.
* **`Windows API & WMI`** (Windows Management Instrumentation) – Used to interact with the **operating system** to query hardware details, such as CPU architecture, RAM configurations, and system paging parameters.
* **`Python 3`** – Used for the main application logic, handling the graphical user interface and managing thread execution for the benchmark processes.
* **`CustomTkinter`** – A modern Python framework used to develop the GUI, providing a clean, responsive layout.
* **`ctypes`** – A Python library that facilitates communication between the GUI and the compiled C++ DLLs, enabling the application to execute performance-critical code directly from the Python environment.
