#!/usr/bin/env python3
import time
import board
import busio
import adafruit_as7341
import json
import socket
import argparse
import sys
from datetime import datetime

# ------------------------- Инициализация датчика -------------------------
i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_as7341.AS7341(i2c)
sensor.led_current = 1
sensor.led = False

GAIN_TABLE = {
    0.5: adafruit_as7341.Gain.GAIN_0_5X,
    1: adafruit_as7341.Gain.GAIN_1X,
    2: adafruit_as7341.Gain.GAIN_2X,
    4: adafruit_as7341.Gain.GAIN_4X,
    8: adafruit_as7341.Gain.GAIN_8X,
    16: adafruit_as7341.Gain.GAIN_16X,
    32: adafruit_as7341.Gain.GAIN_32X,
    64: adafruit_as7341.Gain.GAIN_64X,
    128: adafruit_as7341.Gain.GAIN_128X,
    256: adafruit_as7341.Gain.GAIN_256X,
    512: adafruit_as7341.Gain.GAIN_512X,
}

def read_spectrum():
    return {
        "415nm":    sensor.channel_415nm,
        "445nm":    sensor.channel_445nm,
        "480nm":    sensor.channel_480nm,
        "515nm":    sensor.channel_515nm,
        "555nm":    sensor.channel_555nm,
        "590nm":    sensor.channel_590nm,
        "630nm":    sensor.channel_630nm,
        "680nm":    sensor.channel_680nm,
        "912nm":    sensor.channel_nir,
        "clear":    sensor.channel_clear,
        "timestamp": datetime.now().isoformat()
    }

def main():
    parser = argparse.ArgumentParser(description="AS7341 streaming sender")
    parser.add_argument("--integration", type=int, required=True, help="Integration time in ms")
    parser.add_argument("--gain", type=float, required=True, choices=GAIN_TABLE.keys(), help="Gain value")
    parser.add_argument("--freq", type=float, required=True, help="Frequency (Hz)")
    parser.add_argument("--port", type=int, default=12345, help="UDP port to send to (default 12345)")
    args = parser.parse_args()

    # Настройка датчика
    sensor.integration_time = args.integration
    sensor.gain = GAIN_TABLE[args.gain]

    period = 1.0 / args.freq
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = ('127.0.0.1', args.port)

    print(f" Streaming: integration={args.integration} ms, gain={args.gain}, freq={args.freq} Hz, port={args.port}")
    print(" Press Ctrl+C to stop")

    try:
        while True:
            start = time.time()
            data = read_spectrum()
            data["integration_ms"] = args.integration
            data["gain"] = args.gain
            message = json.dumps(data)
            sock.sendto(message.encode('utf-8'), dest)
            elapsed = time.time() - start
            sleep = period - elapsed
            if sleep > 0:
                time.sleep(sleep)
    except KeyboardInterrupt:
        print("\n Stopped")
        sock.close()

if __name__ == "__main__":
    main()