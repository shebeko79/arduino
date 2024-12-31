from maix import app, uart, time

DEVICE_NAME = "/dev/ttyS0"
serial: uart.UART = None
CMD_STR = "cmd:rccar:drive:{};{}\r"


def init():
    global serial

    serial = uart.UART(DEVICE_NAME, 115200)


def constrain(v):
    if v > 1:
        return 1
    elif v < -1:
        return -1
    return v


def move(y, x):
    y = constrain(y)
    x = constrain(x)

    cmd = CMD_STR.format(y, x)
    #print(cmd)
    serial.write_str(cmd)


def stop():
    cmd = CMD_STR.format(0, 0)
    serial.write_str(cmd)


def process():
    data = serial.read()
    if data:
        pass
    #    print(data)
