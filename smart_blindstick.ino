import time
import machine
import network
import urequests
import math
from ubinascii import b2a_base64
from machine import Pin, I2C

# Twilio Configurations
TWILIO_ACCOUNT_SID = "TWILIO_ACCOUNT_SID"
TWILIO_AUTH_TOKEN = "TWILIO_AUTH_TOKEN"
TWILIO_PHONE = "TWILIO_PHONE"
TO_PHONE = "TO_PHONE"

# Wi-Fi Credentials
SSID = "Rohan"
PASSWORD = "abcd"

# Pins
TRIG1 = Pin(2, Pin.OUT)
ECHO1 = Pin(3, Pin.IN)
TRIG2 = Pin(4, Pin.OUT)
ECHO2 = Pin(5, Pin.IN)
buzzer = Pin(16, Pin.OUT)
button = Pin(15, Pin.IN, Pin.PULL_UP)

# I2C and UART setup
i2c = I2C(0, scl=Pin(9), sda=Pin(8))
uart = machine.UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))

# Global for previous acceleration magnitude
prev_magnitude = 0

# MPU6050 Class
class MPU6050:
    def __init__(self, i2c, address=0x68):
        self.addr = address
        self.i2c = i2c
        self.i2c.writeto_mem(self.addr, 0x6B, b'\x00')  # Wake sensor

    def read_raw_data(self, reg):
        high = self.i2c.readfrom_mem(self.addr, reg, 1)[0]
        low = self.i2c.readfrom_mem(self.addr, reg + 1, 1)[0]
        val = (high << 8) | low
        return val - 65536 if val > 32768 else val

    def get_acceleration(self):
        ax = self.read_raw_data(0x3B) / 16384.0
        ay = self.read_raw_data(0x3D) / 16384.0
        az = self.read_raw_data(0x3F) / 16384.0
        return {"AcX": ax, "AcY": ay, "AcZ": az}

# Connect to Wi-Fi
def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(SSID, PASSWORD)
    print("Connecting to Wi-Fi...")
    while not wlan.isconnected():
        time.sleep(1)
    print("Connected:", wlan.ifconfig())

# Send SMS via Twilio
def send_sms(msg):
    url = f"https://api.twilio.com/2010-04-01/Accounts/{TWILIO_ACCOUNT_SID}/Messages.json"
    headers = {
        "Authorization": "Basic " + b2a_base64(f"{TWILIO_ACCOUNT_SID}:{TWILIO_AUTH_TOKEN}".encode()).decode().strip(),
        "Content-Type": "application/x-www-form-urlencoded"
    }
    data = f"From={TWILIO_PHONE}&To={TO_PHONE}&Body={msg}".encode()
    try:
        response = urequests.post(url, headers=headers, data=data)
        print("SMS Status:", response.status_code)
        response.close()
    except Exception as e:
        print("SMS failed:", e)

# Get GPS coordinates
def get_gps_location():
    if uart.any():
        try:
            line = uart.readline().decode().strip()
            if "$GPGGA" in line:
                return parse_gpgga(line)
        except Exception as e:
            print("GPS read error:", e)
    return None, None

# Fallback to IP geolocation
def get_ip_location():
    try:
        response = urequests.get("http://ip-api.com/json")
        data = response.json()
        response.close()
        return data["lat"], data["lon"]
    except Exception as e:
        print("IP location error:", e)
        return None, None

# Parse GPS
def parse_gpgga(data):
    try:
        parts = data.split(',')
        if parts[2] and parts[4]:
            lat = convert_to_degrees(parts[2], parts[3])
            lon = convert_to_degrees(parts[4], parts[5])
            return lat, lon
    except:
        pass
    return None, None

def convert_to_degrees(value, direction):
    value = float(value)
    degrees = int(value / 100)
    minutes = value - (degrees * 100)
    coord = degrees + (minutes / 60)
    return -coord if direction in ['S', 'W'] else coord

# Send location
def send_location():
    lat, lon = get_gps_location()
    if not lat:
        lat, lon = get_ip_location()
    if lat and lon:
        send_sms(f"Fall Detected! Location: https://www.google.com/maps?q={lat},{lon}")
    else:
        send_sms("Fall Detected! Location unavailable.")

# Distance reading
def read_distance(trig, echo):
    trig.low()
    time.sleep_us(2)
    trig.high()
    time.sleep_us(10)
    trig.low()
    while echo.value() == 0:
        pass
    start = time.ticks_us()
    while echo.value() == 1:
        pass
    end = time.ticks_us()
    return (time.ticks_diff(end, start) * 0.0343) / 2

# Fall detection
def detect_fall(mpu):
    global prev_magnitude
    try:
        accel = mpu.get_acceleration()
        mag = math.sqrt(accel["AcX"]**2 + accel["AcY"]**2 + accel["AcZ"]**2)
        delta = abs(mag - prev_magnitude)
        prev_magnitude = mag

        print("Accel magnitude:", mag, "Delta:", delta)

        if delta > 1.2:  # adjust as needed
            print("Fall detected!")
            buzzer.high()
            time.sleep(1)
            buzzer.low()

            print("Waiting 10s for user to cancel...")
            for _ in range(100):  # 10 seconds in 0.1s steps
                if button.value() == 0:
                    print("User canceled SMS.")
                    buzzer.high()
                    time.sleep(0.2)
                    buzzer.low()
                    return False
                time.sleep(0.1)

            return True
    except Exception as e:
        print("Fall detection error:", e)
    return False

# Main function
def main():
    connect_wifi()
    mpu = MPU6050(i2c)

    while True:
        # Obstacle detection
        d1 = read_distance(TRIG1, ECHO1)
        if d1 < 30:
            buzzer.high()
            time.sleep(0.2)
            buzzer.low()

        # Pit detection
        d2 = read_distance(TRIG2, ECHO2)
        if d2 < 10:
            buzzer.high()
            time.sleep(0.5)
            buzzer.low()

        # Emergency button
        if button.value() == 0:
            print("Emergency button pressed.")
            send_location()
            time.sleep(2)

        # Fall detection
        if detect_fall(mpu):
            send_location()

        time.sleep(0.1)

# Run
try:
    main()
except KeyboardInterrupt:
    print("Stopped.")

