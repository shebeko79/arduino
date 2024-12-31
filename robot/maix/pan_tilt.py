from scservo_sdk import *
from maix import pinmap, time


portHandler = PortHandler('/dev/ttyS2')
packetHandler = PacketHandler(1)

# Control table address
ADDR_SCS_ID = 5
ADDR_SCS_BAUD_RATE = 6
ADDR_SCS_TORQUE_ENABLE = 40
ADDR_SCS_GOAL_ACC = 41
ADDR_SCS_GOAL_POSITION = 42
ADDR_SCS_GOAL_SPEED = 46
ADDR_SCS_LOCK = 48
ADDR_SCS_PRESENT_POSITION = 56


SCS_MINIMUM_POSITION_VALUE = 100
SCS_MAXIMUM_POSITION_VALUE = 4000
SCS_MOVING_STATUS_THRESHOLD = 40
SCS_MOVING_SPEED = 0
SCS_MOVING_ACC = 0

PAN_ID = 1
TILT_ID = 2


def raise_tx(result):
    if result != COMM_SUCCESS:
        raise Exception(packetHandler.getTxRxResult(result))


def raise_rx(result):
    if result != 0:
        raise Exception(packetHandler.getRxPacketError(result))


def setup_acc(scs_id, acc=0):
    tx_res, rx_res = packetHandler.write1ByteTxRx(portHandler, scs_id, ADDR_SCS_GOAL_ACC, acc)
    raise_tx(tx_res)
    raise_rx(rx_res)


def set_torque(scs_id, enable=True):
    if enable:
        trq = 1
    else:
        trq = 0

    tx_res, rx_res = packetHandler.write1ByteTxRx(portHandler, scs_id, ADDR_SCS_TORQUE_ENABLE, trq)
    raise_tx(tx_res)
    raise_rx(rx_res)


def set_pos(scs_id, pos):
    tx_res, rx_res = packetHandler.write2ByteTxRx(portHandler, scs_id, ADDR_SCS_GOAL_POSITION, pos)
    raise_tx(tx_res)
    raise_rx(rx_res)


def set_speed(scs_id, speed):
    tx_res, rx_res = packetHandler.write2ByteTxRx(portHandler, scs_id, ADDR_SCS_GOAL_SPEED, speed)
    raise_tx(tx_res)
    raise_rx(rx_res)


def get_pos_speed(scs_id):
    pos_speed, tx_res, rx_res = packetHandler.read4ByteTxRx(portHandler, scs_id, ADDR_SCS_PRESENT_POSITION)
    raise_tx(tx_res)
    raise_rx(rx_res)

    position = SCS_LOWORD(pos_speed)
    speed = SCS_HIWORD(pos_speed)
    return [position, speed]


def change_id(scs_id, new_id):
    tx_res = packetHandler.write1ByteTxOnly(portHandler, scs_id, ADDR_SCS_LOCK, 0)
    raise_tx(tx_res)

    tx_res = packetHandler.write1ByteTxOnly(portHandler, scs_id, ADDR_SCS_ID, new_id)
    raise_tx(tx_res)

    tx_res = packetHandler.write1ByteTxOnly(portHandler, new_id, ADDR_SCS_LOCK, 1)
    raise_tx(tx_res)

def move_to(scs_id, goal_pos):
    set_pos(scs_id, goal_pos)

    while True:
        pos, speed = get_pos_speed(scs_id)

        print(f"{scs_id}: goal={goal_pos} current={pos} speed={speed}")

        if not abs(goal_pos - pos) > SCS_MOVING_STATUS_THRESHOLD:
            break

    time.sleep(3);


def shake_pan():
    scs_id = PAN_ID
    for i in range(0, 1):
        move_to(scs_id, 0)
        move_to(scs_id, 512)
        move_to(scs_id, 1023)
        move_to(scs_id, 512)


def shake_tilt():
    scs_id = TILT_ID
    for i in range(0, 1):
        move_to(scs_id, 1000)  # down
        move_to(scs_id, 800)  # normal
        move_to(scs_id, 512)  # up
        move_to(scs_id, 220)  # upside-normal
        move_to(scs_id, 100)  # down other side
        move_to(scs_id, 220)
        move_to(scs_id, 512)
        move_to(scs_id, 800)

def shake_all():
    scs_id = PAN_ID
    move_to(scs_id, 0)
    shake_tilt()
    move_to(scs_id, 512)
    shake_tilt()
    move_to(scs_id, 1023)
    shake_tilt()
    move_to(scs_id, 512)
    shake_tilt()

def ping(scs_id):
    r = packetHandler.ping(portHandler, scs_id)
    print(r)
    scs_model_number, scs_comm_result, scs_error = r
    if scs_comm_result != COMM_SUCCESS:
        print("%s" % packetHandler.getTxRxResult(scs_comm_result))
    elif scs_error != 0:
        print("%s" % packetHandler.getRxPacketError(scs_error))
    else:
        print("[ID:%03d] ping Succeeded. SCServo model number : %d" % (scs_id, scs_model_number))

if __name__ == "__main__":

    pinmap.set_pin_function("A28", "UART2_TX")
    pinmap.set_pin_function("A29", "UART2_RX")

    if not portHandler.openPort():
        print("Failed to open the port")
        quit()

    if not portHandler.setBaudRate(500000):
        raise Exception("Failed to change the baudrate")

    #ping(PAN_ID)
    #ping(TILT_ID)

    set_speed(PAN_ID, 500)
    set_speed(TILT_ID, 500)

    #shake_pan()
    #set_torque(PAN_ID, False)

    #shake_tilt()
    #move_to(TILT_ID, 800);
    #set_torque(TILT_ID, False)
    
    shake_all();
    set_torque(PAN_ID, False)
    set_torque(TILT_ID, False)

    portHandler.closePort()

