import spidev
import time
import RPi.GPIO as GPIO

# Initialize SPI
spi = spidev.SpiDev()
spi.open(0, 0)  # Open SPI bus 0, device 0
spi.max_speed_hz = 1000000  # Set SPI speed (adjust as necessary)

# Configure chip select (CS) pin
CS_PIN = 8  # GPIO 8 (BCM numbering for GPIO 8 is GPIO 8)
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(CS_PIN, GPIO.OUT)

def read_chip_id():
    # Enable chip select
    GPIO.output(CS_PIN, GPIO.LOW)
    time.sleep(0.001)  # Small delay to ensure proper communication

    # Send command to read chip ID (typically 0xD0 for BME680)
    # The first byte is the register address, the second byte is a dummy byte to receive data
    chip_id_cmd = [0xD0, 0x00]
    response = spi.xfer2(chip_id_cmd)
    
    # Disable chip select
    GPIO.output(CS_PIN, GPIO.HIGH)

    return response[1]  # The second byte of the response should contain the chip ID

# Read and check the chip ID
chip_id = read_chip_id()
if chip_id != 0xFF:
    print("SPI device detected! Chip ID:", hex(chip_id))
else:
    print("No SPI device detected or invalid chip ID.")

# Close SPI
spi.close()
