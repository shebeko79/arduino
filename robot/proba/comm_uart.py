
from maix import app, uart, pinmap, time
import struct

# ports = uart.list_devices()

# pinmap.set_pin_function("A16", "UART0_TX")
# pinmap.set_pin_function("A17", "UART0_RX")
device = "/dev/ttyS0"

serial0 = uart.UART(device, 115200)

cmd = "cmd:rccar:drive:{};{}\r".format(255,255)
serial0.write_str(cmd)

while not app.need_exit():
    data = serial0.read()
    if data:
        print(data)

    time.sleep_ms(1000) # sleep 1ms to make CPU free
    serial0.write_str(cmd)



