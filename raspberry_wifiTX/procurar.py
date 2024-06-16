import smbus2
import time

def scan_i2c_bus(bus_number=1):
    # Inicializa o barramento I2C
    bus = smbus2.SMBus(bus_number)
    
    devices = []
    # Scanear todos os endereços I2C possíveis (0x03 a 0x77)
    for address in range(3, 128):
        try:
            bus.read_byte(address)
            devices.append(hex(address))
        except OSError:
            pass

    bus.close()
    return devices

def main():
    print("Escaneando o barramento I2C...")
    devices = scan_i2c_bus()
    if devices:
        print(f"Dispositivos I2C encontrados nos endereços: {', '.join(devices)}")
    else:
        print("Nenhum dispositivo I2C encontrado.")

if __name__ == "__main__":
    main()
