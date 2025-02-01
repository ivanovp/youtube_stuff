#!/usr/bin/env /home/pi/lcd/bin/python3
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import sh1106
from PIL import ImageFont
import time

def main():
        # Create I2C interface
        serial = i2c(port=1, address=0x3C)

        # Create device
        device = sh1106(serial)

        # Use a truetype font or the default one if not available
        try:
                font = ImageFont.truetype("DejaVuSans.ttf", 20)
        except IOError:
                print ("Loading default...")
                font = ImageFont.load_default()

        # Draw "Hello World" on the OLED display
        cntr = 0
        while True:
                with canvas(device) as draw:
                        draw.text((0, 0), "Hello World!!!", font=font, fill=255)
                        draw.text((0, 20), str(cntr), font=font, fill=255)
                cntr += 1
                time.sleep(1)


if __name__ == "__main__":
    main()
