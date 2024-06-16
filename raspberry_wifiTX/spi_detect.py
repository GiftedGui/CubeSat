import spidev
import time
import RPi.GPIO as GPIO

# Initialize SPI
spi = spidev.SpiDev()
spi.open(0, 0)  # Open SPI bus 0, device 0

# Set up GPIO pin 8 for chip select (CS) functionality
CS_PIN = 8  # GPIO 8
GPIO.setmode(GPIO.BOARD)
GPIO.setup(CS_PIN, GPIO.OUT)

# Force the use of GPIO 8
GPIO.setwarnings(False)

# Send probe message (example: 0xAA)
probe_data = [0xAA]

# Enable chip select
GPIO.output(CS_PIN, GPIO.LOW)

# Send probe message
spi.xfer(probe_data)

# Disable chip select
GPIO.output(CS_PIN, GPIO.HIGH)

# Wait for response
time.sleep(0.1)  # Adjust delay as needed

# Read chip ID
chip_id = spi.xfer([0x00])  # Send dummy byte to read chip ID
chip_id = chip_id[0]  # Extract chip ID from response

# Check response
if chip_id != 0xFF:
    print("SPI device detected! Chip ID:", hex(chip_id))
else:
    print("No SPI device detected.")

# Close SPI
spi.close()
