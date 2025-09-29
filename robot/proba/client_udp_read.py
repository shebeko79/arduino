import subprocess
import numpy as np
import cv2
import threading
import queue
import socket
import select

UDP_IP = "192.168.33.10"
UDP_PORT = 5005


class H265Decoder:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.process = subprocess.Popen(
            [
                "ffmpeg",
                #"-loglevel", "quiet",
                "-fflags", "nobuffer",
                "-flags", "low_delay",
                "-probesize", "32",
                "-analyzeduration", "0",
                "-f", "hevc",
                "-i", "pipe:0",
                "-f", "rawvideo",
                "-pix_fmt", "bgr24",
                "pipe:1"
            ],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            bufsize=0
        )
        self.frame_size = self.width * self.height * 3
        self.queue = queue.Queue()
        self.running = True
        self.thread = threading.Thread(target=self._reader_thread, daemon=True)
        self.thread.start()

    def _reader_thread(self):
        while self.running:
            raw_frame = self.process.stdout.read(self.frame_size)
            print(f'_reader_thread()1 {len(raw_frame)=}')
            if len(raw_frame) != self.frame_size:
                continue
            frame = np.frombuffer(raw_frame, dtype=np.uint8).reshape((self.height, self.width, 3))
            self.queue.put(frame)

    def send(self, h265_bytes):
        try:
            print(f'H265Decoder.send()1 {len(h265_bytes)=}')
            self.process.stdin.write(h265_bytes)
            print('H265Decoder.send()2')
        except BrokenPipeError:
            self.running = False

    def read(self, timeout=1.0):
        try:
            return self.queue.get(timeout=timeout)
        except queue.Empty:
            return None

    def stop(self):
        self.running = False
        self.process.terminate()
        self.thread.join()


if __name__ == "__main__":
    decoder = H265Decoder(640, 640)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setblocking(False)

    try:
        while True:

            while True:
                print(f'send({UDP_IP}:{UDP_PORT})')
                sock.sendto(b'1', (UDP_IP, UDP_PORT))

                ready = select.select([sock], [], [], 5.0)
                if ready[0]:
                    break

            h265_frame, addr = sock.recvfrom(65536)
            decoder.send(h265_frame)

            frame = decoder.read(timeout=0.5)
            if frame is not None:
                cv2.imshow("H.265 Stream", frame)
                if cv2.waitKey(1) == 27:
                    break
    finally:
        decoder.stop()
        cv2.destroyAllWindows()
