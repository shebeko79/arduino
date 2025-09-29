from maix import time, camera, image, video
import socket
import select

UDP_PORT = 5005

cam = camera.Camera(640, 640, image.Format.FMT_YVU420SP)
e = video.Encoder()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', UDP_PORT))
sock.setblocking(False)

print("wait")
read_ready = select.select([sock], [], [sock])
if not [read_ready]:
    exit(1)

data, addr = sock.recvfrom(10)
print(f"Received message from {addr}: {data.decode()}")


def send_frame(frame_bts):
    if len(frame_bts)>65000:
        return

    while len(frame_bts) > 0:
        _, write_ready, _ = select.select([], [sock], [])
        if not write_ready[0]:
            break

        sent = sock.sendto(frame_bts, addr)
        print(f'send_frame()1 {sent=}')
        frame_bts = frame_bts[sent:]


while True:
    img = cam.read()
    frame = e.encode(img)
    bts = frame.to_bytes()
    print(f'{frame.size()=} {frame.type()=} {frame.get_duration()=} {len(bts)=}')
    send_frame(bts)

    read_ready = select.select([sock], [], [], 0)
    if read_ready[0]:
        data, addr = sock.recvfrom(10)
        print(f"Received message from {addr}: {data.decode()}")





