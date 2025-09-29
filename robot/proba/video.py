from maix import video, image, camera, app, time

#ffmpeg -loglevel quiet -i output.h265 -c:v copy -c:a copy output.mp4 -y

cam = camera.Camera(640, 640, image.Format.FMT_YVU420SP)
e = video.Encoder()
f = open('output.h265', 'wb')

record_ms = 20000
start_ms = time.ticks_ms()
i = 0

while not app.need_exit():
    img = cam.read()
    frame = e.encode(img)
    #print(frame.size())

    #if (i % 2) == 0:
    f.write(frame.to_bytes())

    if time.ticks_ms() - start_ms > record_ms:
        app.set_exit_flag(True)

f.close()
