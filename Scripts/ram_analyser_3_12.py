import json
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import os
import sys

def find_default_jsonl():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    default_path = os.path.join(script_dir, 'memory_usage.jsonl')
    return default_path if os.path.isfile(default_path) else None

def parse_jsonl_and_plot(file_path, output_image=None):
    records = []
    with open(file_path, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                record = json.loads(line)
                ts_ms = record.get('timestamp_ms')
                used_mb = record.get('used_mb')
                if ts_ms is None or used_mb is None:
                    continue
                dt = datetime.fromtimestamp(ts_ms / 1000.0)
                records.append((ts_ms, used_mb, dt))
            except json.JSONDecodeError as e:
                print(f"Ошибка JSON в строке {line_num}: {e}")

    if not records:
        print("Нет данных для построения графика.")
        return

    records.sort(key=lambda x: x[0])
    timestamps = [rec[2] for rec in records]
    used_mb = [rec[1] for rec in records]

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(timestamps, used_mb, marker='o', linestyle='-', linewidth=1.5,
            markersize=3, color='tab:red')
    ax.set_xlabel('Время')
    ax.set_ylabel('Использовано RAM (МБ)')
    ax.set_title('Динамика использования оперативной памяти')
    ax.grid(True, linestyle='--', alpha=0.7)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
    ax.xaxis.set_major_locator(mdates.AutoDateLocator())
    fig.autofmt_xdate(rotation=45)

    if timestamps:
        last_ts = timestamps[-1]
        last_mb = used_mb[-1]
        ax.annotate(f'{last_mb:.1f} МБ',
                    xy=(last_ts, last_mb),
                    xytext=(5, 5), textcoords='offset points',
                    fontsize=9,
                    bbox=dict(boxstyle='round,pad=0.3', fc='yellow', alpha=0.7))

    if output_image:
        plt.savefig(output_image, dpi=150, bbox_inches='tight')
        print(f"График сохранён в {output_image}")
    else:
        plt.show()

if __name__ == '__main__':
    file_path = find_default_jsonl()
    if file_path is None:
        print("Ошибка: файл memory_usage.jsonl не найден в папке скрипта.")
        sys.exit(1)
    parse_jsonl_and_plot(file_path)
