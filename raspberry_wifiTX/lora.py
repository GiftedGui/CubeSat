import spidev
import RPi.GPIO as GPIO
import time

# Define o número do GPIO para o CS (Chip Select)
CS_PIN = 7

# Frequência desejada em MHz (866 MHz)
FREQUENCY_MHZ = 866

# Inicializa o GPIO
GPIO.setmode(GPIO.BOARD)
GPIO.setup(CS_PIN, GPIO.OUT)

# Inicializa o objeto SPI
spi = spidev.SpiDev()
spi.open(0, 0)  # Abre o barramento SPI 0, dispositivo 0

# Função para configurar a frequência do LoRa
def set_lora_frequency(frequency_mhz):
    # Cálculo do valor de frequência baseado na fórmula fornecida pelo fabricante
    frequency_value = int((frequency_mhz * (2 ** 19)) / 32) 

    # Envia o comando para configurar a frequência via SPI
    spi.writebytes([0x6C, (frequency_value >> 16) & 0xFF, (frequency_value >> 8) & 0xFF, frequency_value & 0xFF])

# Função para enviar uma string via LoRa SPI
def send_string_lora_spi(string):
    GPIO.output(CS_PIN, GPIO.LOW)  # Habilita o dispositivo LoRa
    spi.writebytes(list(string.encode()))  # Envia a string como bytes via SPI
    GPIO.output(CS_PIN, GPIO.HIGH)  # Desabilita o dispositivo LoRa
    time.sleep(0.1)  # Breve pausa para permitir que o dispositivo processe os dados

# Configura a frequência do LoRa para 866 MHz
set_lora_frequency(FREQUENCY_MHZ)

# Loop principal
try:
    while True:
        print("Enviando mensagem...")
        send_string_lora_spi("Hello, LoRa!")
        time.sleep(1)  # Aguarda 1 segundo entre cada envio
except KeyboardInterrupt:
    # Interrompe o loop quando Ctrl+C é pressionado
    pass
finally:
    # Limpa o GPIO e fecha o barramento SPI
    GPIO.cleanup()
    spi.close()
