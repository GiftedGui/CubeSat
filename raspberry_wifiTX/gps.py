import serial
import time
from datetime import datetime

# Tentar usar a porta /dev/serial0
try:
    uart = serial.Serial("/dev/serial0", baudrate=9600, timeout=1)
except Exception as e:
    print(f"Failed to open /dev/serial0: {e}")
    print("Trying /dev/ttyAMA0...")
    try:
        uart = serial.Serial("/dev/ttyAMA0", baudrate=9600, timeout=1)
    except Exception as e:
        print(f"Failed to open /dev/ttyAMA0: {e}")
        exit(1)

def convert_to_degrees(value, direction):
    degrees = int(value[:2])
    minutes = float(value[2:])
    decimal_degrees = degrees + (minutes / 60)
    if direction in ['S', 'W']:
        decimal_degrees *= -1
    return decimal_degrees

def parse_gprmc(data):
    parts = data.split(',')
    time_str = parts[1]
    lat = convert_to_degrees(parts[3], parts[4])
    lon = convert_to_degrees(parts[5], parts[6])
    speed_knots = float(parts[7])
    speed_mps = speed_knots * 0.514444  # Convertendo para m/s
    course = parts[8]
    date_str = parts[9]
    return time_str, lat, lon, speed_mps, course, date_str

def parse_gpgga(data):
    parts = data.split(',')
    time_str = parts[1]
    lat = convert_to_degrees(parts[2], parts[3])
    lon = convert_to_degrees(parts[4], parts[5])
    fix_quality = parts[6]
    num_satellites = parts[7]
    hdop = parts[8]
    altitude = parts[9]
    return time_str, lat, lon, fix_quality, num_satellites, hdop, altitude

def parse_gpgsa(data):
    parts = data.split(',')
    fix_type = parts[2]
    pdop = parts[15]
    hdop = parts[16]
    vdop = parts[17].split('*')[0]
    return fix_type, pdop, hdop, vdop

def read_gps():
    last_uart_data = ""  # Armazena os últimos dados lidos da UART
    while True:
        try:
            if uart.in_waiting > 0:
                gps_data = uart.readline().decode('ascii', errors='replace')
                if gps_data.startswith('$GPRMC'):
                    print("------------------\n")  # Imprime a linha de separação apenas antes de imprimir os dados de tempo
                    last_uart_data = gps_data
                    time_str, lat, lon, speed, course, date_str = parse_gprmc(gps_data)
                    timestamp = datetime.strptime(time_str, '%H%M%S.%f').strftime('%Y-%m-%d %H:%M:%S')
                    print(f"Time: {timestamp}, Date: {date_str}")
                    print(f"Latitude: {lat:.6f}, Longitude: {lon:.6f}")
                    print(f"Speed Over Ground: {speed:.2f} m/s, Course Over Ground: {course} degrees")
                elif gps_data.startswith('$GPGGA'):
                    time_str, lat, lon, fix_quality, num_satellites, hdop, altitude = parse_gpgga(gps_data)
                    print(f"Fix Quality: {fix_quality}, Number of Satellites: {num_satellites}, HDOP: {hdop}")
                    print(f"Altitude: {altitude} meters")
                elif gps_data.startswith('$GPGSA'):
                    fix_type, pdop, hdop, vdop = parse_gpgsa(gps_data)
                    fix_status = "No Fix" if fix_type == "1" else "2D Fix" if fix_type == "2" else "3D Fix"
                    print(f"Fix Status: {fix_status}, PDOP: {pdop}, HDOP: {hdop}, VDOP: {vdop}")
        except Exception as e:
            print(f"An error occurred: {e}")

def main():
    print("Reading GPS data...")
    read_gps()

if __name__ == "__main__":
    main()
