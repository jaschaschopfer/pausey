# Pausey
**IM 4 – Physical Computing**

Pausey is a small desktop companion designed to promote healthy movement habits in the office. Three times per day—at 10:00, 14:00, and 16:00—it invites you to take a short break for one of three mini-games. These games are simple, physical activities detected via sensors. After completing a game, Pausey awards points and reports them to a central leaderboard.

### How to Use Pausey
1. **Leave the device powered on and connected to Wi-Fi.** It will synchronize time automatically via SNTP.
2. **At the scheduled times, Pausey will beep and show a message on its OLED display.**
3. **You start and stop the game by pressing the button.** Each game has its own behavior:

#### Push-Ups (10:00)
- Detected via a VL6180X distance sensor.
- When you lower yourself below 10 cm and then rise again, it counts as one push-up.
- You hear a beep after each successful rep.
- You can do this for up to 5 minutes or until you press the button again to stop.
- You earn 1 point per push-up, up to 10 points.

#### Stretch (14:00)
- Detected using a vibration sensor that reacts to body movement.
- After pressing the button, the timer runs for up to 2 minutes.
- You only hear a final beep when time is up or if you don't move >5 seconds.
- Points are awarded based on how long you stretch:
  - Less than 60 seconds: 0 points
  - 60–89 seconds: 5 points
  - 90–119 seconds: 7.5 points
  - Full 2 minutes: 10 points

#### Dance (16:00)
- A buzzer beeps to a 60 BPM rhythm.
- You press the button at first to start and then whenever you want to stop.
- The LED ring lights up in direction patterns (↑↓←→).
- You are encouraged to follow the beat and move accordingly.
- No sensors verify your movements; it’s about participation.
- The longer you dance (up to 2 minutes), the more points you earn (up to 10).

After each session, your score is displayed and uploaded to a central database via HTTP.

---

## Step by Step Instructions

### Hardware Requirements

#### Components

* **ESP32-C6-N8 Dev Board** – 1 ×
* **OLED Display** (128 × 64, SSD1306, *I²C*) – 1 ×
* **VL6180X Distance Sensor** – 1 ×
* **SW-18010P Vibration Sensor** – 1 ×
* **WS2812B LED Ring** (12 LEDs) – 1 ×
* **KY-012 Piezo Buzzer** – 1 ×
* **Tactile Button** – 1 ×
* **Breadboard & Jumper Wires** – 1 ×
* **5 V USB Power Supply** – 1 ×

#### Wiring Overview

| Component              | Pin on ESP32                 |
| ---------------------- | ---------------------------- |
| **OLED (I²C)**         | SDA → GPIO21<br>SCL → GPIO22 |
| **VL6180X (I²C)**      | SDA → GPIO21<br>SCL → GPIO22 |
| **Button**             | GPIO0                        |
| **Piezo Buzzer**       | GPIO2                        |
| **Vibration Sensor**   | GPIO19                       |
| **LED Ring (WS2812B)** | GPIO18                       |

> *Pin numbers can be adjusted according to your circuit design if needed.*

### Software Setup

1. **Install Arduino IDE**
   Download the latest 2.x release: [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

2. **Add ESP32 Board Support**

   1. *File* → *Preferences*
   2. In **Additional Board URLs** add:

      ```
      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
      ```
   3. *Tools* → *Board* → *Boards Manager* → search **ESP32** → install **“esp32 by Espressif Systems”**

3. **Select Board Settings**

   * **Board:** ESP32-C6 Dev Module
   * **CPU Frequency:** 80 MHz
   * **Flash Size:** 2 MB

4. **Install Required Libraries**
   *Sketch* → *Include Library* → *Manage Libraries…*

   * Adafruit SSD1306
   * Adafruit GFX
   * Adafruit VL6180X
   * Adafruit NeoPixel
   * Ticker
   * *(built-in with ESP32 core)* WiFi, HTTPClient, time.h

### Uploading the Firmware & Configuring Backend

1. **Clone/Download the Project**
   Make sure all source files are inside a folder named **Pausey**. Either keep one big `.ino` or split into multiple tabs.

2. **Connect the ESP32**
   Plug in via USB → *Tools* → *Port* → select the active serial port.

3. **Flash the Code**
   Click **Upload**.
   Open *Serial Monitor* @ 115200 baud to watch logs (Wi-Fi, SNTP sync, etc.).

> *Adjust WIFI_SSID, WIFI_PASSWORD and DEVICE_ID in your .ino file.*
> *Adjust details of the http POST Request according to your database in your .ino file.*

### Database table Setup

1. Set up a PHP server (e.g., XAMPP).
2. Create MySQL table:

   ```sql
   CREATE TABLE scores (
     userId  VARCHAR(50),
     gameId  VARCHAR(10),
     score   INT,
     timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
   );
   ```
3. Place `load.php` on the server to handle HTTP POST requests.
4. Add the config.php file according to your database.

### Testing

1. After uploading the .ino file to the ESP32 and successfully connecting to the WiFi, the OLED should display the **Idle face**.
2. At **10:00, 14:00, 16:00** it invites you to play.
3. Press the **button** to start and end the game; points show afterward and are POSTed to your load.php

---

## Flow Chart

![Flow Chart](https://github.com/user-attachments/assets/c024f52e-1a27-49d9-ab86-f19cc79370f1)

---

## Component Plan

![Komponentenplan_Pausey](https://github.com/user-attachments/assets/99598ff2-6931-4e48-9b13-a455bc1808f3)

---

## Wiring Diagram

![Steckplan_Pausey](https://github.com/user-attachments/assets/8727f94e-d115-4b47-a694-1cf9a27f6ff6)

---

## Pictures and Screenshots
# INSERT SCREENSHOTS HERE!
![Vordere Seite](https://github.com/user-attachments/assets/45197f57-99f1-420f-a755-74ab10a6ef22)
![Seitenansicht](https://github.com/user-attachments/assets/9ceab8ff-9787-4f34-a57b-e7e449bc8c4d)
![Hintere Seite](https://github.com/user-attachments/assets/2f565c47-e655-4922-a6da-121a1170c520)

---

## Report on the implementation process

### Development Process

During the development of Pausey, we followed an iterative and agile approach. We began with ideation and testing, proceeded with parallel development of the microcontroller firmware and frontend components, and concluded with integration and design refinements. Each phase helped us validate assumptions, improve usability, and align the project with the needs of our target group.

### Discarded Approaches

While exploring different gameplay and interaction possibilities, we tested several concepts that we eventually decided to discard:

- **Location-based NFC tag system**: We initially considered placing NFC tags in various office locations to encourage movement. However, we abandoned this idea due to the risk of the tags being lost, thrown away, or removed.

- **Social check-ins via RFID/NFC**: Another idea was to use RFID/NFC tags to "scan" colleagues and prove real-world social interaction. We discarded this because many people work alone, and we found that Pausey was especially helpful in such contexts—serving as a reminder when no human colleague is present.

- **Leaderboard login system**: We considered implementing an individual login system for the leaderboard. However, feedback from interviews, particularly with people over 50, indicated a strong preference for simplicity. Users expressed that they already deal with too much digital information and appreciated not having to manage another account or app.

## Design Decisions

The concept of Pausey was shaped early through moodboards and discussions. We envisioned it as a small, calm creature from the forest—something users would perceive as gentle and non-intrusive. The color palette and overall design reflect this inspiration. The user interface was intentionally kept simple, with minimal interaction required. This decision was made to ensure accessibility and comfort, especially for older users who prefer low-complexity devices.

## Inspirations

Although we didn’t initially aim for it, someone remarked that Pausey reminded them of a Tamagotchi. We appreciated this observation, as our device is indeed intended to act like a small pet or creature that the user develops a light bond with. However, unlike a Tamagotchi, Pausey is not needy or annoying—it simply exists quietly and gently nudges the user toward healthy movement.

## Failures and Replanning

Not everything went smoothly during the build:

- One of the speaker modules from our hardware kits was defective. We spent considerable time debugging the code before realizing the hardware was the issue. This emphasized the importance of verifying hardware early and regularly.

- Due to the crowded breadboard setup, components such as the LED ring became unstable or loose. We had to invest time into finding a layout that kept connections secure and reliable.

## Reflection

The development of Pausey offered numerous learning opportunities and unexpected challenges. It combined programming, electronics, design, and user testing into a single compact product. Below are some reflections on what we learned, what we struggled with, and what could be improved.

### Challenges

- The AI occasionally modified parts of the code that should have remained unchanged. To mitigate this, we had to write very precise prompts and maintain a clear understanding of our architecture.
  
- Understanding microcontroller logic was new for us. Concepts like blocking vs non-blocking code, pin initialization, and state machines took time to grasp.

### Learning Effect

- We learned how to link the ESP32's logic with a backend via HTTP and how these components interact to form a full system.

- We became confident working with modular hardware and solving real-world issues through rapid prototyping.

### Known Bugs

- The vibration sensor used for the stretch exercise is not always sensitive enough to detect subtle movement changes. A gyroscope would have provided more reliable data.

- If the device is powered on after one or more scheduled game times have already passed, it still plays them in sequence instead of skipping to the current time. This could be improved in future versions.

## Planning

We worked in a step-by-step, agile fashion:

1. **Microcontroller setup** and **frontend (HTML/CSS)** were developed in parallel.
2. We then created the **database** to store leaderboard scores.
3. Firmware development followed, focusing on a clean and modular `.ino` structure.
4. Finally, we implemented **PHP server scripts** (config, load, unload) and polished the **JavaScript and CSS** for frontend presentation.

## Task Distribution

- **HTML/CSS**: Saurabh  
- **Microcontroller hardware setup**: Jascha  
- **INO file (Arduino firmware)**: Worked on together  
- **PHP + Database**: Worked on together  

## Tools Used

We were encouraged to use AI tools during development, and we did so extensively:

- **ChatGPT** (used in 4 focused threads):
  - **Planning**: GPT-3.5 (for broad ideation and structuring)
  - **Coding**: GPT-o4-mini-high (for writing and testing Arduino code)
  - **Debugging**: GPT-o4-mini-high (helped resolve issues and optimize logic)
  - **Explaining**: GPT-4o (for deeper understanding of sensor behavior and architectural advice)

---

## Video-Documentation
# EMBED VIDEO HERE
