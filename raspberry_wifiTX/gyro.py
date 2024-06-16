import time
import board
import busio
import adafruit_lsm6ds.lsm6ds33

# Inicialização dos pinos I2C: GPIO 2 (SDA) e GPIO 3 (SCL) são definidos como padrão em board.SCL e board.SDA
i2c = busio.I2C(board.SCL, board.SDA)

# Inicialização do LSM6DS3 no endereço 0x6A
sensor = adafruit_lsm6ds.lsm6ds33.LSM6DS33(i2c, address=0x6A)

def read_lsm6ds3():
    # Leitura dos dados do acelerômetro e giroscópio
    accel_x, accel_y, accel_z = sensor.acceleration
    gyro_x, gyro_y, gyro_z = sensor.gyro
    temp = sensor.temperature
    
    # Exibição dos dados no console
    print(f"Accelerometer: X={accel_x:.2f} Y={accel_y:.2f} Z={accel_z:.2f} m/s^2")
    print(f"Gyroscope: X={gyro_x:.2f} Y={gyro_y:.2f} Z={gyro_z:.2f} rad/s")
    print(f"Temperature: {temp:.2f} C")

def main():
    while True:
        print("\nReading LSM6DS3 sensor data...")
        read_lsm6ds3()
        time.sleep(0.5)

if __name__ == "__main__":
    main()
