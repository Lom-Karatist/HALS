#!/usr/bin/env python3
import time
import board
import busio
import adafruit_as7341
import json
import socket
import argparse
import sys
import threading
import select
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

# Глобальные параметры (вместо current_integration_time)
current_atime = 10      # начальное значение, соответствующее 50 мс (например)
current_astep = 20000   # подобрать так, чтобы давало нужную экспозицию
current_gain = 0.5
current_freq = 1.0
params_lock = threading.Lock()

def apply_parameters():
    """Применить текущие настройки к датчику."""
    with params_lock:
        atime = current_atime
        astep = current_astep
        gain = current_gain
    try:
        sensor.stop()
    except: pass
    try:
        sensor.enable = False
    except: pass

    sensor.atime = atime          # установка ATIME
    sensor.astep = astep          # установка ASTEP
    sensor.gain = GAIN_TABLE[gain]

    time.sleep(0.02)
    try:
        sensor.start()
    except: pass
    try:
        sensor.enable = True
    except: pass
    print(f"[APPLY] atime={atime}, astep={astep}, gain={gain}", flush=True)

def udp_command_server(port=12346):
    global current_atime, current_astep, current_gain, current_freq
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', port))
    sock.setblocking(False)
    print(f"[UDP commands] Listening on port {port}", flush=True)
    while True:
        try:
            readable, _, _ = select.select([sock], [], [], 0.5)
            if readable:
                data, addr = sock.recvfrom(1024)
                try:
                    cmd = json.loads(data.decode())
                    cmd_type = cmd.get("type")
                    value = cmd.get("value")
                    if cmd_type == "set_integration_time":
                        # Ожидаем словарь {"atime": int, "astep": int}
                        if isinstance(value, dict) and "atime" in value and "astep" in value:
                            with params_lock:
                                current_atime = value["atime"]
                                current_astep = value["astep"]
                            apply_parameters()
                            print(f"[CMD] integration: atime={current_atime}, astep={current_astep}", flush=True)
                    elif cmd_type == "set_gain":
                        with params_lock:
                            current_gain = float(value)
                        apply_parameters()
                        print(f"[CMD] gain -> {value}", flush=True)
                    elif cmd_type == "set_freq":
                        with params_lock:
                            current_freq = float(value)
                        print(f"[CMD] frequency -> {value} Hz", flush=True)
                except Exception as e:
                    print(f"[CMD] Error: {e}", flush=True)
        except Exception:
            pass

def read_spectrum():
    with params_lock:
        atime = current_atime
        astep = current_astep
        gain = current_gain
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
        "timestamp": datetime.now().isoformat(),
        "atime": atime,
        "astep": astep,
        "gain": gain
    }

def main():
    global current_atime, current_astep, current_gain, current_freq
    
    parser = argparse.ArgumentParser(description="AS7341 streaming sender")
    parser.add_argument("--atime", type=int, required=True, help="ATIME register (0-65534)")
    parser.add_argument("--astep", type=int, required=True, help="ASTEP register (0-65535)")
    parser.add_argument("--gain", type=float, required=True, choices=GAIN_TABLE.keys(), help="Gain value")
    parser.add_argument("--freq", type=float, required=True, help="Frequency (Hz)")
    parser.add_argument("--port", type=int, default=12345, help="UDP port for data (default 12345)")
    parser.add_argument("--cmd-port", type=int, default=12346, help="UDP port for commands (default 12346)")
    
    args = parser.parse_args()
    with params_lock:
        current_atime = args.atime
        current_astep = args.astep
        current_gain = args.gain
        current_freq = args.freq
    apply_parameters()

    # Запускаем поток для приёма команд
    cmd_thread = threading.Thread(target=udp_command_server, args=(args.cmd_port,), daemon=True)
    cmd_thread.start()

    period = 1.0 / args.freq
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = ('127.0.0.1', args.port)

    print(f"Streaming: atime={args.atime}, astep={args.astep}, gain={args.gain}, freq={args.freq} Hz, data port={args.port}, cmd port={args.cmd_port}")
    print("Press Ctrl+C to stop")

    try:
        while True:
            start = time.time()

            # Читаем данные с датчика
            data = read_spectrum()

            message = json.dumps(data)
            sock.sendto(message.encode('utf-8'), dest)

            elapsed = time.time() - start
            sleep = period - elapsed
            if sleep > 0:
                time.sleep(sleep)
    except KeyboardInterrupt:
        print("\nStopped")
        sock.close()

if __name__ == "__main__":
    main()
