
# OSimX 2.0 — Operating System Simulator

### Visualize. Simulate. Understand.

![C++](https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-6-41CD52?style=flat-square&logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?style=flat-square&logo=cmake&logoColor=white)
![SQLite](https://img.shields.io/badge/SQLite-Database-003B57?style=flat-square&logo=sqlite&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-Desktop-0078D4?style=flat-square)

**OSimX 2.0** is an interactive desktop application designed to simplify Operating System concepts through hands-on simulations, algorithm visualizations, and performance analysis.

Built with **C++17 and Qt 6**, OSimX allows students to explore CPU Scheduling, Memory Management, Disk Scheduling, Deadlock Management, and File System operations in a unified learning environment.

The application features a modern graphical interface, user authentication, database-backed simulation history, and interactive algorithm comparisons.

Designed for **students, educators, and OS enthusiasts**, OSimX bridges the gap between theoretical knowledge and practical understanding.

---

## ✨ Key Features

- **Interactive OS Simulations** — Explore CPU Scheduling, Memory Management, Disk Scheduling, Deadlock Management, and File System operations.
- **Algorithm Visualization** — Understand execution through Gantt charts, memory representations, and disk movement graphs.
- **Performance Analysis** — Examine scheduling metrics, page faults, disk head movement, and simulation results.
- **Modern Desktop Interface** — Clean, intuitive graphical interface built using Qt 6 Widgets.
- **User Authentication** — Integrated login and signup with database-backed account management.
- **Simulation History** — Save, retrieve, and restore previous experiments.
- **Algorithm Comparison** — Compare simulation results to understand algorithm behavior and efficiency.
- **Database Integration** — SQLite by default, with optional MySQL support.
- **Automated Testing** — C++ regression tests for core simulation functionality.

---

## 🧠 Operating System Modules

OSimX 2.0 provides five interactive simulation modules covering essential Operating System concepts.

| Module | Supported Algorithms / Features |
|--------|--------------------------------|
| **⚙️ CPU Scheduling** | FCFS, SJF, SRTF, Round Robin, Priority Scheduling |
| **🧩 Memory Management** | First Fit, Best Fit, Worst Fit, FIFO, LRU, Optimal Page Replacement |
| **💽 Disk Scheduling** | FCFS, SSTF, SCAN, C-SCAN, LOOK, C-LOOK |
| **🔒 Deadlock Management** | Banker's Algorithm, Deadlock Detection, Resource Request Analysis |
| **📁 File System** | File & Directory Operations, Permissions, Contiguous, Linked & Indexed Allocation |

Each module allows users to configure simulation inputs, execute algorithms, visualize results, and explore fundamental OS concepts.

Supported simulations can also be saved and restored through the application's database-backed history system.

---

## 🛠️ Technology Stack

OSimX 2.0 is built using C++ and Qt, with a modular architecture for simulation, visualization, and data persistence.

| Component | Technology |
|-----------|------------|
| **Programming Language** | C++17 |
| **GUI Framework** | Qt 6 Widgets |
| **Database** | SQLite (Default), MySQL (Optional) |
| **Database Interface** | Qt SQL |
| **Build System** | CMake 3.16+ |
| **Compiler** | MinGW-w64 (Windows), GCC (Linux) |
| **Build Tool** | Ninja |
| **Testing** | CTest & C++ Regression Tests |
| **Development Tools** | VS Code, Git & GitHub |
| **Platform** | Windows Desktop, Linux Source Builds |

---

## 🚀 Installation & Setup

### Prerequisites

To build OSimX 2.0 from source, ensure you have:

- C++17-compatible compiler (MinGW-w64)
- Qt 6 (Widgets and SQL modules)
- CMake 3.16+
- Ninja Build System
- Git

### Clone the Repository

```bash
git clone https://github.com/Dipanshu0001-OP/OSimX.git
cd OSimX
```

### Build and Run (Windows)

OSimX includes a Windows build script for automated compilation, testing, and execution.

```powershell
.\BUILD_AND_RUN_WINDOWS.bat
```

> **Note:** The script requires a compatible Qt 6 and MinGW installation. Configure `OSIMX_QT_ROOT` and `OSIMX_MINGW_ROOT` if your installation paths differ from the script defaults.

### Manual Build

Alternatively, use CMake and Ninja from a terminal configured with Qt 6 and a compatible C++ compiler:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
.\build\osimx.exe
```

### Download Prebuilt Application

A packaged Windows version will be available from [GitHub Releases](https://github.com/Dipanshu0001-OP/OSimX/releases) once published.

The packaged application can be launched using `osimx.exe` without installing a C++ compiler or configuring a separate SQLite server.

For database configuration details, see [DATABASE_SETUP.md](DATABASE_SETUP.md).

---

## 📁 Project Structure

OSimX 2.0 follows a modular architecture that separates simulation algorithms, user interfaces, database services, and application logic.

```text
OSimX/
├── app/                  # Application initialization
├── database/             # Authentication & data persistence
├── models/               # Simulation data models
├── modules/              # Operating System simulation engines
│   ├── cpu_scheduling/
│   ├── memory_management/
│   ├── disk_scheduling/
│   ├── deadlock/
│   └── file_system/
├── ui/                   # Qt graphical interfaces
│   ├── authentication/
│   ├── common/
│   └── main/
├── resources/            # Logos, icons & assets
├── tests/                # Automated regression tests
├── main.cpp              # Application entry point
├── CMakeLists.txt         # Build configuration
└── BUILD_AND_RUN_WINDOWS.bat
```

---

## 👥 Team & Acknowledgments

### Team Monolith

OSimX 2.0 is developed by a dedicated team of Computer Science students at **Graphic Era Deemed to be University**, combining expertise in Operating Systems, C++ development, database management, and user interface design.

| Team Member | Role |
|-------------|------|
| **Dipanshu Tandon** | Team Lead & Core Developer |
| **Saloni Sharma** | UI & Backend Developer |
| **Srishti** | Database Developer |
| **Yash Variyani** | Algorithms Developer |

### 🎓 Project Mentor

We express our sincere gratitude to **Prof. Dr. Puneet Sharma** for his valuable guidance, encouragement, and continuous support throughout the development of OSimX.

### 🏫 Institution

**Graphic Era Deemed to be University**  
Department of Computer Science & Engineering  
Academic Year: 2026–2027

We also thank the faculty members of the Department of Computer Science & Engineering, particularly those associated with **Operating Systems (TCS-502)** and **DBMS (TCS-503)**, for providing the academic foundation and support that contributed to this project.

---

## 📜 License

OSimX is distributed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

## 📬 Contact & Support

For questions, suggestions, or collaboration:

- **Project Lead:** Dipanshu Tandon
- **Email:** dipanshutandon123@gmail.com
- **GitHub:** [Dipanshu0001-OP](https://github.com/Dipanshu0001-OP)
- **Repository:** [OSimX](https://github.com/Dipanshu0001-OP/OSimX)

If you find OSimX helpful, consider giving the repository a ⭐!

---

<p align="center">
  Made with ❤️ by <b>Team Monolith</b><br>
  Graphic Era Deemed to be University<br>
  Academic Year 2026–2027
</p>
