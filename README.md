# OSimX - Operating System Simulator

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.0-green.svg)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16-red.svg)](https://cmake.org/)
[![MySQL](https://img.shields.io/badge/MySQL-8.0-orange.svg)](https://www.mysql.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## 📖 Table of Contents

- [About](#about)
- [Modules](#modules)
- [Screenshots](#screenshots)
- [Tech Stack](#tech-stack)
- [Project Structure](#project-structure)
- [Acknowledgement](#Acknowledgement)
- [Mentor](#mentor)
- [Institution](#institution)
- [Team Members](#team-members)
- [Tools and Technologies](#tools&technologies)
- [Conclusion](#conclusion)

---

## 📌 About

**OSimX** is an interactive Operating System Simulator designed to help students understand core OS concepts through hands-on experimentation and visualization.

### The Problem

Operating Systems is one of the most challenging subjects in computer science because its concepts are abstract, algorithm-heavy, and difficult to visualize. Students often struggle to understand how CPU scheduling, memory management, disk operations, and file systems actually work beyond textbook diagrams. They memorize formulas and pass exams, but they never truly see these algorithms in action. Existing simulators are either too simplistic, too complex, or lack persistent storage, forcing students to re-enter data every time.

### The Solution

OSimX bridges this gap by providing an interactive, visual learning environment where students can experiment with OS concepts safely. What sets OSimX apart is its MySQL database integration, which enables persistent storage of simulation inputs, results, and experiment history — allowing students to track learning progress and compare algorithm performance over time.

Built with C++17 on Linux using Qt6 for the graphical interface and MySQL for data persistence, OSimX bridges the gap between theoretical OS concepts and practical understanding through visualization and hands-on experimentation.

---

## 🧠 Modules

OSimX covers **10 core OS modules**, each implemented as an independent simulation engine with database persistence.

### 1. User & Authentication Module

### 2. Process Management Module
- Process creation (PID, arrival time, burst time, priority)
- Process state visualization
- Process Control Block (PCB) simulation
- Parent-child relationships

### 3. CPU Scheduling Module ⭐
- **FCFS** (First Come First Served) — Non-preemptive
- **SJF** (Shortest Job First) — Non-preemptive
- **SRTF** (Shortest Remaining Time First) — Preemptive
- **Round Robin** — With configurable time quantum
- **Priority Scheduling** — Preemptive

**Metrics Calculated:**
- Waiting Time
- Turnaround Time
- Response Time
- CPU Utilization
- Throughput

**Visualization:**
- Interactive Gantt Chart

---

### 4. Memory Management Module

**A. Memory Allocation Algorithms:**
- First Fit
- Best Fit
- Worst Fit

**B. Page Replacement Algorithms:**
- FIFO (First In First Out)
- LRU (Least Recently Used)
- Optimal (Belady's Algorithm)

**Visualization:**
- Memory frames showing allocated/free blocks
- Page fault tracking
- Hit/miss ratio display

---

### 5. File System Module
- Directory tree with root (`/`)
- File operations: create, delete, read, write, rename, move
- Permission system (read/write/execute)
- File metadata (size, timestamps)

**Visualization:**
- Directory tree view
- File properties panel
- Path navigation

---

### 6. Disk Scheduling Module
- FCFS — First Come First Served
- SSTF — Shortest Seek Time First
- SCAN — Elevator algorithm
- C-SCAN — Circular SCAN
- LOOK — SCAN but only until last request
- C-LOOK — Circular LOOK

**Metrics:**
- Total head movement
- Average seek time

**Visualization:**
- Disk head movement graph

---

### 7. Deadlock Module
- **Banker's Algorithm** — Deadlock avoidance
- Resource Allocation Graph — Cycle detection
- Wait-For Graph — Simplified deadlock detection

**Visualization:**
- Allocation matrices
- Safe sequence display
- Resource allocation graph

---

### 8. IPC Module
- Pipes — Unidirectional communication
- Message Queues — Message passing
- Shared Memory — Simulated memory sharing

**Visualization:**
- Data flow between processes
- Message queues

---

### 9. Synchronization Module
- Producer-Consumer — Bounded buffer
- Reader-Writer — Shared resource access
- Dining Philosophers — Resource contention

**Synchronization Tools:**
- Mutex — Mutual exclusion
- Semaphore — Counting and binary

**Visualization:**
- Race condition demonstration
- Resource access visualization

---

### 10. History & Reports Module
- Experiment history viewer
- Algorithm comparison (side-by-side)
- Performance analytics
- Report generation

---

## 📸 Screenshots

###CPU Scheduling with Gantt Chart
<img width="1197" height="823" alt="Schedulers" src="https://github.com/user-attachments/assets/16ab12b7-9eef-4819-b451-c91f312482f2" />

---

## 🛠️ Tech Stack

| Component| Technology | Purpose |
|-----------|------------|---------|
| **Language** | C++17 | Core logic and algorithms |
| **UI Framework** | Qt6 | Graphical user interface |
| **Database** | MySQL 8.0 | Persistent data storage |
| **Build System** | CMake 3.16+ | Project build and compilation |
| **Version Control** | Git & GitHub | Source code management |
| **OS** | Linux (Fedora) | Development environment |

## 📁 Project Structure
<img width="1231" height="1278" alt="image" src="https://github.com/user-attachments/assets/74b98caf-43f8-406d-943a-7d6fdbb602c9" />


## Acknowledgments
We would like to express our sincere gratitude to all those who contributed to the successful completion of OSimX.

---

## 🙏 Project Mentor

We extend our deepest gratitude to our project mentor (Prof.)Dr.Puneet Sharma for their invaluable guidance, continuous support, and constructive feedback throughout the development of this project. Their expertise in Operating Systems and Database Management Systems helped us navigate through complex concepts and implement them effectively.

---

## 🏫 Institution

We are grateful to **Graphic Era Deemed to be University** for providing us with the platform, resources, and environment to work on this project. The university's commitment to academic excellence and innovation has been instrumental in shaping our learning journey.

---

## 📚 Faculty Members

We would like to thank all the faculty members of the **Department of Computer Science & Engineering** for their encouragement and support. Special thanks to:

- **Operating Systems Course Faculty (TCS-502)** - For building our foundation in OS concepts
- **DBMS Course Faculty (TCS-503)** - For helping us understand database design and implementation

---

## 👥 Team Members

We, the members of Team Avengers, thank each other for the collaboration, hard work, and dedication throughout this project:

| # |       Name          | Roll No.  |            Role            |
|---|---------------------|-----------|----------------------------|
| 1 | **Dipanshu Tandon** | 240211490 | Team Lead & Core Developer |
| 2 | **Saloni Sharma**   | 24022818  | UI & Backend Developer     |
| 3 | **Srishti**         | 24022705  | Database Developer         |
| 4 | **Yash Variyani**   | 240211343 | Algorithms Developer       |


Working together on OSimX has been an enriching experience where we learned not just technical skills, but also teamwork, communication, and problem-solving.
---


## 💻 Tools and Technologies

We acknowledge the open-source tools and technologies that made OSimX possible:

| Tool               | Purpose                        |
|--------------------|--------------------------------|
| **C++17**          | Core programming language      |
| **Qt6**            | Graphical User Interface       |
| **MySQL 8.0**      | Database management            |
| **CMake 3.16+**    | Build system                   |
| **Git**            | Version control                |
| **GitHub**         | Code hosting and collaboration |
| **Linux (Fedora)** | Development environment        |
| **VS Code/VIM**    | Code editor                    |

---

## ⭐ Show Your Support

If you found this project helpful, please consider giving it a star on GitHub! ⭐
(https://github.com/Dipanshu0001-OP/osimx)

---

## 📞 Contact

For any questions, feedback, or collaboration:

- **Email:** dipanshutandon123@gmail.com
- **GitHub:** [Dipanshu0001-OP](https://github.com/Dipanshu0001-OP)
- **Project Link:** https://github.com/Dipanshu0001-OP/osimx

---

## 🎯 Conclusion

OSimX is the result of collective effort, learning, and dedication. We hope this project serves as a useful educational tool for students learning Operating Systems. We are committed to continuously improving and expanding OSimX in the future.

---

*Made with ❤️ by the Team Monolith*

**Graphic Era Deemed to be University**
**Academic Year: 2026-2027**
