import math
from enum import IntEnum
from scservo_sdk import *
from maix import pinmap, time


class ScServo:

    ADDR_SCS_ID = 5
    ADDR_SCS_BAUD_RATE = 6
    ADDR_SCS_TORQUE_ENABLE = 40
    ADDR_SCS_GOAL_POSITION = 42
    ADDR_SCS_GOAL_SPEED = 46
    ADDR_SCS_LOCK = 48
    ADDR_SCS_PRESENT_POSITION = 56

    SCS_MOVING_STATUS_THRESHOLD = 40

    def __init__(self):
        self.portHandler = PortHandler('/dev/ttyS2')
        self.packetHandler = PacketHandler(1)

        pinmap.set_pin_function("A28", "UART2_TX")
        pinmap.set_pin_function("A29", "UART2_RX")

        if not self.portHandler.openPort():
            raise Exception("Failed to open the port")

        if not self.portHandler.setBaudRate(500000):
            raise Exception("Failed to change the baudrate")

    def raise_tx(self, result):
        if result != COMM_SUCCESS:
            raise Exception(self.packetHandler.getTxRxResult(result))

    def raise_rx(self, result):
        if result != 0:
            raise Exception(self.packetHandler.getRxPacketError(result))

    def set_torque(self, scs_id, enable=True):
        if enable:
            trq = 1
        else:
            trq = 0

        tx_res, rx_res = self.packetHandler.write1ByteTxRx(self.portHandler, scs_id, self.ADDR_SCS_TORQUE_ENABLE, trq)
        self.raise_tx(tx_res)
        self.raise_rx(rx_res)

    def set_pos(self, scs_id, pos):
        pos = int(pos)
        tx_res, rx_res = self.packetHandler.write2ByteTxRx(self.portHandler, scs_id, self.ADDR_SCS_GOAL_POSITION, pos)
        self.raise_tx(tx_res)
        self.raise_rx(rx_res)

    def set_speed(self, scs_id, speed):
        tx_res, rx_res = self.packetHandler.write2ByteTxRx(self.portHandler, scs_id, self.ADDR_SCS_GOAL_SPEED, speed)
        self.raise_tx(tx_res)
        self.raise_rx(rx_res)

    def get_pos_speed(self, scs_id):
        pos_speed, tx_res, rx_res = self.packetHandler.read4ByteTxRx(self.portHandler, scs_id, self.ADDR_SCS_PRESENT_POSITION)
        self.raise_tx(tx_res)
        self.raise_rx(rx_res)

        position = SCS_LOWORD(pos_speed)
        speed = SCS_HIWORD(pos_speed)
        return [position, speed]

    def change_id(self, scs_id, new_id):
        tx_res = self.packetHandler.write1ByteTxOnly(self.portHandler, scs_id, self.ADDR_SCS_LOCK, 0)
        self.raise_tx(tx_res)

        tx_res = self.packetHandler.write1ByteTxOnly(self.portHandler, scs_id, self.ADDR_SCS_ID, new_id)
        self.raise_tx(tx_res)

        tx_res = self.packetHandler.write1ByteTxOnly(self.portHandler, new_id, self.ADDR_SCS_LOCK, 1)
        self.raise_tx(tx_res)

    def ping(self, scs_id):
        r = self.packetHandler.ping(self.portHandler, scs_id)
        print(r)
        scs_model_number, scs_comm_result, scs_error = r
        if scs_comm_result != COMM_SUCCESS:
            print("%s" % self.packetHandler.getTxRxResult(scs_comm_result))
        elif scs_error != 0:
            print("%s" % self.packetHandler.getRxPacketError(scs_error))
        else:
            print("[ID:%03d] ping Succeeded. SCServo model number : %d" % (scs_id, scs_model_number))

    def sync_move_to(self, scs_id, goal_pos):
        goal_pos = int(goal_pos)
        self.set_pos(scs_id, goal_pos)

        while True:
            pos, speed = self.get_pos_speed(scs_id)
            #print(f"{scs_id}: goal={goal_pos} current={pos} speed={speed}")
            if not abs(goal_pos - pos) > self.SCS_MOVING_STATUS_THRESHOLD:
                break


class Tilt(IntEnum):
    BACKWARD_MAX_DOWN = 100
    BACKWARD = 220
    UP = 512
    FRONT = 800
    MAX_DOWN = 1000
    MIN = BACKWARD_MAX_DOWN
    MAX = MAX_DOWN


class Pan(IntEnum):
    MIN = 0
    CENTER = 512
    MAX = 1023


PAN_ID = 1
TILT_ID = 2
RAD_TO_SERVO = (Tilt.FRONT - Tilt.BACKWARD) / math.pi

srv: ScServo = None


def set_pan(pos):
    if pos < Pan.MIN:
        pos = Pan.MIN
    elif pos > Pan.MAX:
        pos = Pan.MAX

    srv.set_pos(PAN_ID, pos)


def set_tilt(pos):
    if pos < Tilt.MIN:
        pos = Tilt.MIN
    elif pos > Tilt.MAX:
        pos = Tilt.MAX

    srv.set_pos(TILT_ID, pos)


def get_pan():
    pos, speed = srv.get_pos_speed(PAN_ID)
    return pos


def get_tilt():
    pos, speed = srv.get_pos_speed(TILT_ID)
    return pos


def pan2angle(pos):
    return (pos-Pan.CENTER)/RAD_TO_SERVO


def angle2pan(angle):
    return angle*RAD_TO_SERVO+Pan.CENTER


def tilt2angle(pos):
    return (pos-Tilt.FRONT)/RAD_TO_SERVO


def angle2tilt(angle):
    return angle*RAD_TO_SERVO+Tilt.FRONT


def set_pan_angle(angle):
    set_pan(angle2pan(angle))


def set_tilt_angle(angle):
    set_tilt(angle2tilt(angle))


def get_pan_angle():
    return pan2angle(get_pan())


def get_tilt_angle():
    return tilt2angle(get_tilt())


def init():
    global srv
    srv = ScServo()

    srv.set_speed(PAN_ID, 500)
    srv.set_speed(TILT_ID, 500)

    set_pan(Pan.CENTER)
    set_tilt(Tilt.FRONT)
