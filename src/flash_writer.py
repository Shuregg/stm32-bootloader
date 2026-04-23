import serial
import time
import sys

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
        
        if len(firmware) != 6544:
            print(f"Предупреждение: Размер файла {len(firmware)} байт, ожидается 6544 байт")
            print("Продолжаем загрузку...")
        
        return firmware
    except FileNotFoundError:
        print(f"Ошибка: Файл {filename} не найден")
        return None
    except Exception as e:
        print(f"Ошибка чтения файла: {e}")
        return None

def upload_firmware(serialPort, firmware):
    """Загрузка прошивки через COM-порт"""
    try:
        total_bytes = len(firmware)
        print(f"Начинаем загрузку прошивки ({total_bytes} байт)...")
        
        # Очищаем буферы порта
        serialPort.reset_input_buffer()
        serialPort.reset_output_buffer()
        
        # Даем время устройству подготовиться
        time.sleep(0.5)
        
        # Отправляем команду начала прошивки (опционально)
        # serialPort.write(b'START_FIRMWARE_UPDATE\n')
        # time.sleep(0.1)
        
        # Отправляем данные порциями
        chunk_size = 6544
        bytes_sent = 0
        
        for i in range(0, total_bytes, chunk_size):
            chunk = firmware[i:i+chunk_size]
            bytes_written = serialPort.write(chunk)
            bytes_sent += bytes_written
            
            # Пауза для предотвращения переполнения буфера
            time.sleep(0.01)
            
            # Выводим прогресс
            progress = (bytes_sent * 100) // total_bytes
            print(f"\rПрогресс: {progress}% ({bytes_sent}/{total_bytes} байт)", end='')
        
        print(f"\nОтправлено {bytes_sent} байт из {total_bytes}")
        
        # Отправляем команду завершения прошивки (опционально)
        # serialPort.write(b'END_FIRMWARE_UPDATE\n')
        
        # Читаем ответ от устройства (если ожидается)
        time.sleep(0.5)
        response = serialPort.read(100)
        if response:
            print(f"Ответ устройства: {response[:50]}...")
        
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
        "COM12",
        115200, 
        serial.EIGHTBITS, 
        2, 
        serial.STOPBITS_ONE
    )

    if not serialPort:
        return
    
    print(f"COM-порт открыт: {serialPort.port}")
    
    # Загрузка прошивки
    firmware = load_firmware("./firmware.bin")
    if not firmware:
        serialPort.close()
        return
    
    # Загрузка прошивки на устройство
    success = upload_firmware(serialPort, firmware)
    
    # Закрываем порт
    serialPort.close()
    
    if success:
        print("Загрузка прошивки успешно завершена!")
    else:
        print("Ошибка при загрузке прошивки!")
        sys.exit(1)

if __name__ == "__main__":
    main()