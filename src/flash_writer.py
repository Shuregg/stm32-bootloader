import serial
import time
import sys
import os

def init_serial_port(
    _port="COM12", 
    _baudrate=115200, 
    _bytesize=serial.EIGHTBITS, 
    _timeout=2, 
    _stopbits=serial.STOPBITS_ONE
):
    """Инициализация COM-порта"""
    try:
        serialPort = serial.Serial(
            port = _port,
            baudrate = _baudrate,
            bytesize = _bytesize,
            timeout = _timeout,
            stopbits = _stopbits
        )
        return serialPort
    except serial.SerialException as e:
        print(f"Ошибка открытия COM-порта: {e}")
        return None

def load_firmware(filename):
    """Загрузка прошивки из файла"""
    try:
        with open(filename, 'rb') as f:
            firmware = f.read()

        file_size = os.path.getsize(filename)
        print("Размер файла в байтах:", file_size)

        return firmware, file_size
    except FileNotFoundError:
        print(f"Ошибка: Файл {filename} не найден")
        return None
    except Exception as e:
        print(f"Ошибка чтения файла: {e}")
        return None

def prepare_uart(serialPort):
    # Очищаем буферы порта
    serialPort.reset_input_buffer()
    serialPort.reset_output_buffer()

    # Даем время устройству подготовиться
    time.sleep(0.5)

def check_resp_uart(serialPort):
    resp = serialPort.read(1)
    good_resp = 10
    good_resp_b = good_resp.to_bytes(1)

    # if (resp != good_resp_b):
    #     raise Exception(f"Ответ устройства ({resp.hex()}) не равен {good_resp_b.hex()}!")
    return resp

def write_uart(serialPort, firmware, total_bytes, chunk_size=1):
    bytes_sent = 0
    for i in range(0, total_bytes, chunk_size):
        chunk = firmware[i:i+chunk_size]
        bytes_written = serialPort.write(chunk)
        bytes_sent += bytes_written

        # Пауза для предотвращения переполнения буфера
        time.sleep(0.005)

        resp = bytes(0)

        if (bytes_sent % 256 == 0):
            resp = check_resp_uart(serialPort)

        # Выводим прогресс
        progress = (bytes_sent * 100) // total_bytes
        print(f"\rПрогресс: {progress}% ({bytes_sent}/{total_bytes} байт). Последний ответ: {resp.hex()}", end='')

    print()
    return bytes_sent

def send_firmware_size(serialPort, firmware_size):
    print(f"Отправляем размер прошивки ({hex(firmware_size)})")

    word_size = 4
    chunk_size = 1
    prepare_uart(serialPort)

    firmware_size_b = firmware_size.to_bytes(word_size, 'big')

    print(f'Перевод размера прошивки в байты ({word_size}): 0x{firmware_size_b.hex()}')

    write_uart(serialPort, firmware_size_b, word_size, chunk_size)

    print("Ожидаем ответ устройсва")
    check_resp_uart(serialPort)



def upload_firmware(serialPort, firmware, total_bytes):
    """Загрузка прошивки через COM-порт"""
    try:
        send_firmware_size(serialPort, total_bytes)

        print(f"Начинаем загрузку прошивки ({total_bytes} байт)...")

        prepare_uart(serialPort)

        # Отправляем данные порциями
        chunk_size = 1

        bytes_sent = write_uart(serialPort, firmware, total_bytes, chunk_size)

        print(f"\nОтправлено {bytes_sent} байт из {total_bytes}")

        return bytes_sent == total_bytes

    except serial.SerialException as e:
        print(f"\nОшибка при отправке данных: {e}")
        return False
    except Exception as e:
        print(f"\nНеожиданная ошибка: {e}")
        return False

def main():
    """Основная функция"""
    print("Программа загрузки прошивки")
    print("-" * 40)

    # Инициализация COM-порта
    serialPort = init_serial_port(
        "COM15", # "/dev/ttyACM0",
        115200, 
        serial.EIGHTBITS, 
        2, 
        serial.STOPBITS_ONE
    )

    if not serialPort:
        return
    
    print(f"COM-порт открыт: {serialPort.port}")

    # Загрузка прошивки
    filepath = input("Введите путь до .bin файла\n> ")
    print(filepath[-4:])

    if (filepath[-4:] != ".bin"):
        print("Расширение файла должно быть .bin")

    firmware, size = load_firmware(filepath)
    if not firmware:
        serialPort.close()
        return

    # Загрузка прошивки на устройство
    success = upload_firmware(serialPort, firmware, size)

    # Закрываем порт
    serialPort.close()

    if success:
        print("Загрузка прошивки успешно завершена!")
    else:
        print("Ошибка при загрузке прошивки!")
        sys.exit(1)

if __name__ == "__main__":
    main()
