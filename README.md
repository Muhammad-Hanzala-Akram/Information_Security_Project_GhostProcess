# 👻 Project SPECTRE/Ghost Process
### Advanced Defense Evasion via Memory-Based Process Hollowing

![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11%2064--bit-blue?style=flat-square)
![Language](https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c)
![Python](https://img.shields.io/badge/C2%20Server-Python%203-3776AB?style=flat-square&logo=python)
![Category](https://img.shields.io/badge/Category-Offensive%20Security%20Research-red?style=flat-square)
![License](https://img.shields.io/badge/License-Academic%20Use%20Only-yellow?style=flat-square)

> ⚠️ **Educational & Research Purposes Only** — All testing was performed in an isolated lab environment. This project is not intended for use on live or production systems.

---

## 📌 Overview

**Project SPECTRE** is an academic offensive security research project that demonstrates **Process Hollowing** — a fileless malware technique used to inject and execute arbitrary code inside a legitimate Windows process.

Modern antivirus (AV) solutions scan files on disk. This technique bypasses that entirely by running malicious code **entirely in memory**, hidden inside a trusted Windows process like `svchost.exe`.

The project also includes a **data exfiltration module** that connects back to a Python-based attacker C2 server — simulating real-world Command & Control behavior.

---

## ⚙️ How It Works

```
[ Loader (C) ]
      │
      ▼
Spawn svchost.exe → SUSPENDED
      │
      ▼
NtUnmapViewOfSection() → Hollow the memory
      │
      ▼
VirtualAllocEx() + WriteProcessMemory() → Inject payload
      │
      ▼
SetThreadContext() + ResumeThread() → Execute payload
      │
      ▼
Payload → connects to Python C2 Server → Exfiltrate data
```

---

## 🗂️ Project Structure

```
spectre-hollow/
├── loader/
│   ├── main.c              # Main process hollowing loader
│   ├── hollow.c            # Core hollowing logic
│   ├── hollow.h            # Header file
│   └── Makefile            # Build configuration
├── payload/
│   ├── payload.c           # Injected payload source
│   └── exfil.c             # Data exfiltration module
├── c2-server/
│   └── server.py           # Python attacker C2 server
├── docs/
│   └── SPECTRE_Documentation.docx
├── README.md
└── .gitignore
```

---

## 🔧 Prerequisites

- Windows 10 / 11 (64-bit) — isolated VM recommended
- MinGW-w64 or MSVC (C compiler)
- Python 3.x (for C2 server)
- Process Hacker 2 (for analysis)
- x64dbg (for debugging)
- Wireshark (for network analysis)

---

## 🚀 Build & Run

### 1. Compile the Loader

```bash
# Using MinGW
gcc -o spectre_loader.exe loader/main.c loader/hollow.c -lntdll

# Using MSVC
cl loader/main.c loader/hollow.c /Fe:spectre_loader.exe
```

### 2. Start the Python C2 Server

```bash
cd c2-server
python server.py --port 4444
```

### 3. Run the Loader (inside isolated VM only)

```bash
spectre_loader.exe
```

---

## 🔑 Key Windows APIs Used

| API | Purpose |
|-----|---------|
| `CreateProcess()` | Spawn target process in suspended state |
| `NtUnmapViewOfSection()` | Hollow out the original executable memory |
| `VirtualAllocEx()` | Allocate RWX memory in remote process |
| `WriteProcessMemory()` | Write payload into allocated region |
| `GetThreadContext()` / `SetThreadContext()` | Redirect instruction pointer to payload |
| `ResumeThread()` | Resume process to execute payload |

---

## 🐍 Python C2 Server

The attacker server (`c2-server/server.py`) listens for incoming TCP connections from the injected payload and receives exfiltrated data.

```python
# Quick start
python server.py --host 0.0.0.0 --port 4444
```

Received data is printed to the terminal and saved to `output.log`.

---

## 🔍 Detection Artifacts

During analysis with Process Hacker, x64dbg, and Wireshark, the following anomalies were identified that EDR tools can use for detection:

| Artifact | Description |
|----------|-------------|
| **RWX Memory Region** | Read-Write-Execute memory inside svchost.exe — highly anomalous |
| **Image Path Mismatch** | In-memory image does not match on-disk executable |
| **Suspicious API Sequence** | `NtUnmapViewOfSection → VirtualAllocEx → WriteProcessMemory` chain |
| **Unusual Network Connection** | svchost.exe connecting to unknown IP on non-standard port |
| **Thread Context Modification** | RIP register manually redirected to non-standard entry point |

---

## 📊 Results Summary

- ✅ Process Hollowing executed successfully on Windows 10 64-bit
- ✅ Payload ran under `svchost.exe` identity — AV remained silent
- ✅ Python C2 server received exfiltrated data over TCP
- ✅ 5 distinct detection artifacts identified for defensive research

---

## ⚠️ Disclaimer

This project was developed strictly for **academic and educational purposes** as part of an Information Security course. It is intended to help security researchers and defenders understand how fileless malware operates so that better detection systems can be built.

- Do **NOT** use this on any system you do not own
- Do **NOT** use this against live or production environments
- All testing was conducted in an **isolated virtual machine**
- The payload used is **non-malicious** and demonstrates concept only

The author holds no responsibility for any misuse of this code.

---

## 👨‍💻 Author

**Muhammad Hanzala Akram**
Roll No: BITF24M006
Category: Information Security / Offensive Research

---

## 📚 References

- [Microsoft Docs — Windows Process Creation](https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes)
- [MITRE ATT&CK — Process Hollowing (T1055.012)](https://attack.mitre.org/techniques/T1055/012/)
- [Windows Internals by Mark Russinovich](https://docs.microsoft.com/en-us/sysinternals/)
- [Process Hacker](https://processhacker.sourceforge.io/)
- [x64dbg Debugger](https://x64dbg.com/)
