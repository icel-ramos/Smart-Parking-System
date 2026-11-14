# Park Easy

**Park Easy** is a real-time smart parking control and telemetry web dashboard designed to interface with an **ESP32** hardware prototype via **Firebase Realtime Database**. It provides attendants and administrators with live bay occupancy tracking, automated barrier gate controls, configurable vehicle pricing, and compliance tracking.

---

## Table of Contents

- [System Overview](#system-overview)
- [Features & Modules](#features--modules)
- [Tech Stack](#tech-stack)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Roadmap](#roadmap)
- [License](#license)

---

## System Overview

* **Real-Time Sensor Telemetry:** Tracks 4 ground-floor parking bays (`1A-01` to `1A-04`) using infrared/ultrasonic beam pathways.
* **Dynamic Capacity Warnings:** Automatically changes capacity status from normal (`< 75%`) to **Yellow Warning** (`75%` / `3 of 4` full) and **Red Alert** (`100%` / `4 of 4` full).
* **Custom Vector Icon Engine:** Uses a unified `24x24` stroke SVG icon system (`data-icon`) and shared navigation/notification state inside `global.js`.

---

## Features & Modules

| Module | File | Description |
| :--- | :--- | :--- |
| **Authentication** | `Codes/index.html` | Operator and administrator login portal with password visibility toggle. |
| **Dashboard** | `Codes/dashboard.html` | Ground floor occupancy bar, **2-Wheeler** vs. **4-Wheeler** stats, and an interactive SVG line chart. |
| **Parking Map** | `Codes/parkingMap.html` | Live 4-bay visual canvas (`1:2.79` aspect ratio), centered vehicle SVG icon, 1-second live duration timer, and manual force override. |
| **Infrastructure & Hardware** | `Codes/infra&Hardware.html` | **Entry/Exit Gate** mode switching (`Automatic` vs. `Manual` Open/Close), **Slot LED** controls (`Green`/`Red`), and hardware health table. |
| **Billing & Pricing** | `Codes/bill&Pricing.html` | Revenue KPI cards, editable Philippine Peso (`PHP`) rate table, searchable transaction log, and **CSV Export**. |
| **Compliance & Settings** | `Codes/comply&Setting.html` | Facility configuration, permit upload & expiry tracking, **Users & Roles** (`Admin`, `Operator`, `Cashier`), and alert toggles. |

---

## Tech Stack

* **Frontend:** HTML5, CSS3, Vanilla JavaScript (ES6+)
* **Database:** Firebase Realtime Database (RTDB)
* **Authentication:** Firebase Authentication (Email & Password)
* **Hardware Prototype:** ESP32 Microcontroller, IR/Ultrasonic Sensors, Servo Gate Barriers, Slot Indicator LEDs

---

## Project Structure

    park-easy/
    ├── Codes/
    │   ├── index.html              # Login portal
    │   ├── index.css               # Login portal styles
    │   ├── global.css              # Shared layout, sidebar, header & dropdown styles
    │   ├── global.js               # Custom SVG icon engine, notifications & account menu
    │   ├── dashboard.html          # Executive summary & interactive SVG chart
    │   ├── dashboard.css           # Dashboard styles
    │   ├── parkingMap.html         # 4-bay live sensor map & manual override
    │   ├── parkingMap.css          # Parking canvas & telemetry styles
    │   ├── infra&Hardware.html     # Gate barriers, slot LEDs & hardware table
    │   ├── infra&Hardware.css      # Infrastructure control styles
    │   ├── bill&Pricing.html       # Rate matrix, KPIs & transaction log
    │   ├── bill&Pricing.css        # Billing & pricing styles
    │   ├── comply&Setting.html     # Facility settings, permits & user roles
    │   └── comply&Setting.css      # Compliance & settings styles
    ├── .gitignore                  # Ignored OS, IDE, Firebase & ESP32 build files
    ├── LICENSE                     # MIT License
    └── README.md                   # Project documentation

---

## Getting Started

### 1. Clone the Repository

    git clone https://github.com/your-username/park-easy.git
    cd park-easy

### 2. Run Locally

Open `Codes/index.html` in your web browser or launch it using the **Live Server** extension in VS Code.

### 3. Connect to Firebase

1. Create a project in the [Firebase Console](https://console.firebase.google.com/).
2. Enable **Authentication** (*Email/Password*) and **Realtime Database** (*Singapore `asia-southeast1`*).
3. Import the initial `slots`, `gates`, and `rates` JSON structure into your Realtime Database.
4. Paste your `firebaseConfig` keys into `Codes/global.js`.

---

## Roadmap

- [x] Responsive multi-page dashboard UI and collapsible sidebar drawer
- [x] Custom `24x24` stroke SVG icon engine (`global.js`) replacing all emojis
- [x] 4-slot live parking map with 1-second duration timer and 75%/100% capacity warnings
- [x] Interactive SVG graphical summary chart on the main dashboard
- [ ] Connect Firebase Authentication and Realtime Database listeners
- [ ] Flash ESP32 firmware to sync sensor readings and gate servos with Firebase

---

## License

Distributed under the **MIT License**. See the `LICENSE` file for details.