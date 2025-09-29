from maix import camera, image, video, app
import time
import subprocess

cam = camera.Camera(640, 480, image.Format.FMT_YVU420SP)
e = video.Encoder()

ffmpeg = subprocess.Popen([
    'ffmpeg',
    '-re',
    '-f', 'hevc',
    '-i', '-',  # stdin
    '-c', 'copy',
    '-f', 'rtp',
    'rtp://192.168.33.7:1234'
], stdin=subprocess.PIPE)

while not app.need_exit():
    img = cam.read()
    frame = e.encode(img)
    ffmpeg.stdin.write(frame.to_bytes())
