from maix import camera, display, image, nn, app, time

cam = camera.Camera(640, 640)
disp = display.Display()

orig_img = cam.read()
disp.show(orig_img)

#time.sleep_ms(100)

#img = cam.read()
img = orig_img.copy()
img.sub(orig_img)
disp.show(img)

rest_img = orig_img.copy()
rest_img.add(img)

orig_img.save("orig.jpg")
img.save("sub.jpg")
rest_img.save("rest.jpg")
