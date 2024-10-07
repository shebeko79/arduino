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
        return -1;
    return v;


def move(y, x):
    rw = lw = y
    lw += x
    rw -= x

    lw = constrain(lw)
    rw = constrain(rw)

    lw = int(lw * 255)
    rw = int(rw * 255)

    cmd = CMD_STR.format(lw, rw)
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
