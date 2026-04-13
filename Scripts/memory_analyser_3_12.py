import json
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import argparse
import os
import sys

def find_default_jsonl():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    default_path = os.path.join(script_dir, 'usb_space.jsonl')
    return default_path if os.path.isfile(default_path) else None

def parse_jsonl_and_plot(file_path, output_image=None):
    records = []  # храним кортежи (timestamp_ms, used_mb, datetime)

    with open(file_path, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            try:
                record = json.loads(line)
                ts_ms = record.get('timestamp_ms')
                total_bytes = record.get('total_bytes')
                available_bytes = record.get('available_bytes')

                if ts_ms is None or total_bytes is None or available_bytes is None:
                    print(f"Пропущена строка {line_num}: отсутствуют обязательные поля")
                    continue

                used_bytes = total_bytes - available_bytes
                used_mb = used_bytes / (1024 * 1024)
                dt = datetime.fromtimestamp(ts_ms / 1000.0)
                records.append((ts_ms, used_mb, dt))

            except json.JSONDecodeError as e:
                print(f"Ошибка JSON в строке {line_num}: {e}")

    if not records:
        print("Нет данных для построения графика.")
        return

    # Сортировка по возрастанию timestamp_ms
    records.sort(key=lambda x: x[0])

    # Распаковка отсортированных данных
    timestamps = [rec[2] for rec in records]
    used_mb = [rec[1] for rec in records]

    # Построение графика
    fig, ax = plt.subplots(figsize=(12, 6))
    ax.plot(timestamps, used_mb, marker='o', linestyle='-', linewidth=1.5,
            markersize=3, color='tab:blue')

    ax.set_xlabel('Время')
    ax.set_ylabel('Занятое место (МБ)')
    ax.set_title('Динамика заполнения USB-накопителя во времени')
    ax.grid(True, linestyle='--', alpha=0.7)

    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
    ax.xaxis.set_major_locator(mdates.AutoDateLocator())
    fig.autofmt_xdate(rotation=45)

    if timestamps:
        last_ts = timestamps[-1]
        last_mb = used_mb[-1]
        ax.annotate(f'{last_mb:.1f} МБ',
                    xy=(last_ts, last_mb),
                    xytext=(5, 5),
                    textcoords='offset points',
                    fontsize=9,
                    bbox=dict(boxstyle='round,pad=0.3', fc='yellow', alpha=0.7))

    if output_image:
        plt.savefig(output_image, dpi=150, bbox_inches='tight')
        print(f"График сохранён в {output_image}")
    else:
        plt.show()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Построить график занятости USB из JSONL-лога.\n'
                    'Данные автоматически сортируются по времени.'
    )
    parser.add_argument('input_file', nargs='?', help='Путь к JSONL файлу (опционально)')
    parser.add_argument('-o', '--output', help='Путь для сохранения графика')
    args = parser.parse_args()

    if args.input_file:
        file_path = args.input_file
        if not os.path.isfile(file_path):
            print(f"Ошибка: файл {file_path} не найден.")
            sys.exit(1)
    else:
        file_path = find_default_jsonl()
        if file_path is None:
            print("Ошибка: файл usb_space.jsonl не найден в папке скрипта.")
            sys.exit(1)
        print(f"Автоматически найден файл: {file_path}")

    parse_jsonl_and_plot(file_path, args.output)
