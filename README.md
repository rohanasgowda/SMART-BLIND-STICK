# Smart Blind Stick for Visually Impaired Individuals

## 📌 Overview
The Smart Blind Stick is an assistive device designed to help visually impaired individuals navigate safely. It integrates multiple sensors to detect obstacles, measure depth, detect falls, and send GPS location alerts to family members via SMS using Twilio. The stick provides real-time feedback through a buzzer.

---

## 🧩 Components Used
- Raspberry Pi Pico  
- GPS Module  
- Ultrasonic Sensors (x2)  
- Gyroscope Sensor  
- Buzzer  
- Push Button  
- Resistors (as needed)  
- Breadboard / PCB  
- 9V Battery  

---

## ⚙️ Features
1. **Obstacle Detection:** Ultrasonic sensor detects nearby obstacles and alerts the user through buzzer.  
2. **Depth Detection:** Another ultrasonic sensor measures the depth of drop-offs or steps.  
3. **Fall Detection with Delay:** Gyroscope detects if the user falls. After a **10-second delay**, the GPS module sends the user’s location to family members via Twilio SMS, giving the user a chance to cancel false alarms.  
4. **Manual Location Alert:** Pressing the button sends the user’s location immediately via Twilio SMS.  
5. **GPS Fallback via IP:** If GPS fails to provide location, the system fetches approximate location using IP-based geolocation and sends it via Twilio SMS.  
6. **Buzzer Alerts:** Different beep patterns indicate depth detection and obstacle detection.
   
---

## ✅ Applications
- Navigation assistance for visually impaired individuals  
- Safety during walking or mobility  
- Prototype for wearable assistive technology

---
