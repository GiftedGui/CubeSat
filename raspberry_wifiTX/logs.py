import time
import board
import busio
import adafruit_lsm6ds.lsm6ds33
import serial
import os
import sys
from datetime import datetime

# Inicialização dos pinos I2C: GPIO 2 (SDA) e GPIO 3 (SCL) são definidos como padrão em board.SCL e board.SDA
i2c = busio.I2C(board.SCL, board.SDA)

# Inicialização do LSM6DS33 no endereço 0x6A
sensor = adafruit_lsm6ds.lsm6ds33.LSM6DS33(i2c, address=0x6A)

# Tentar usar a porta /dev/serial0 para o GPS
def init_uart():
    try:
        return serial.Serial("/dev/serial0", baudrate=9600, timeout=1)
    except Exception as e:
        print(f"Failed to open /dev/serial0: {e}")
        print("Trying /dev/ttyAMA0...")
        try:
            return serial.Serial("/dev/ttyAMA0", baudrate=9600, timeout=1)
        except Exception as e:
            print(f"Failed to open /dev/ttyAMA0: {e}")
            exit(1)

uart = init_uart()

def read_lsm6ds33():
    # Leitura dos dados do acelerômetro e giroscópio
    accel_x, accel_y, accel_z = sensor.acceleration
    gyro_x, gyro_y, gyro_z = sensor.gyro
    temp = sensor.temperature

    # Retorna os valores separados
    return accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, temp

def convert_to_degrees(value, direction):
    if not value:
        return 0.0
    degrees = int(value[:2])
    minutes = float(value[2:])
    decimal_degrees = degrees + (minutes / 60)
    if direction in ['S', 'W']:
        decimal_degrees *= -1
    return decimal_degrees

def parse_gprmc(data):
    parts = data.split(',')
    time_str = parts[1] if parts[1] else "000000.000"
    lat = convert_to_degrees(parts[3], parts[4]) if parts[3] and parts[4] else 0.0
    lon = convert_to_degrees(parts[5], parts[6]) if parts[5] and parts[6] else 0.0
    speed_knots = float(parts[7]) if parts[7] else 0.0
    speed_mps = speed_knots * 0.514444  # Convertendo para m/s
    course = parts[8] if parts[8] else "0.0"
    date_str = parts[9] if parts[9] else "000000"
    return time_str, lat, lon, speed_mps, course, date_str

def parse_gpgga(data):
    parts = data.split(',')
    time_str = parts[1] if parts[1] else "000000.000"
    lat = convert_to_degrees(parts[2], parts[3]) if parts[2] and parts[3] else 0.0
    lon = convert_to_degrees(parts[4], parts[5]) if parts[4] and parts[5] else 0.0
    altitude = float(parts[9]) if parts[9] else 0.0
    num_satellites = int(parts[7]) if parts[7] else 0
    return time_str, lat, lon, altitude, num_satellites

def read_gps():
    gps_data = ""
    while uart.in_waiting > 0:
        gps_data = uart.readline().decode('ascii', errors='replace')
        if gps_data.startswith('$GPRMC'):
            time_str, lat, lon, speed, course, date_str = parse_gprmc(gps_data)
            return lat, lon, speed, time_str
        elif gps_data.startswith('$GPGGA'):
            time_str, lat, lon, altitude, num_satellites = parse_gpgga(gps_data)
            return lat, lon, altitude, num_satellites
    # Retorna valores padrão se não houver dados GPS válidos
    return 0.0, 0.0, 0.0, 0

def write_to_file(data_string):
    with open("RECECAO.txt", "a") as file:
        file.write(data_string + "\n")

def restart_program():
    print("Restarting program...")
    os.execv(sys.executable, [sys.executable] + sys.argv)

def main():
    try:
        while True:
            print("\nReading LSM6DS33 sensor data...")
            accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, temp = read_lsm6ds33()
            print("\nReading GPS data...")
            lat, lon, altitude, num_satellites = read_gps()
            data_string = f"{accel_x:.2f}, {accel_y:.2f}, {accel_z:.2f}, {gyro_x:.2f}, {gyro_y:.2f}, {gyro_z:.2f}, {temp:.2f}, {lat:.6f}, {lon:.6f}, {altitude:.2f}, {num_satellites}, 0, 0"
            write_to_file(data_string)
            time.sleep(0.5)
    except Exception as e:
        print(f"An error occurred: {e}")
        restart_program()

if __name__ == "__main__":
    main()
