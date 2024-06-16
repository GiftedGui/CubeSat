import os

def check_gpio_usage():
    gpio_path = "/sys/class/gpio"
    if os.path.exists(gpio_path):
        for gpio_pin in os.listdir(gpio_path):
            if gpio_pin.startswith("gpio"):
                with open(os.path.join(gpio_path, gpio_pin, "direction"), 'r') as f:
                    direction = f.read().strip()
                print(f"GPIO pin {gpio_pin} is set to {direction}")

if __name__ == "__main__":
    check_gpio_usage()
