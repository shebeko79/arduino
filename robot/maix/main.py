from maix import camera, image, display, app
import tracker
import states

CAM_SIZE = [640, 480]
cam: camera.Camera = None
disp: display.Display = None


def main_init():
    global cam
    global disp
    cam = camera.Camera(CAM_SIZE[0], CAM_SIZE[1], tracker.get_camera_format())
    disp = display.Display()

    image.load_font("sans", "/maixapp/share/font/sans.ttf", size=32)
    image.set_default_font("sans")

    states.init()
    states.set_state(states.MainState.state_name)


def main_cycle():
    while not app.need_exit():
        st = states.current_state
        if not st:
            break

        img = cam.read()
        disp_img = st.draw_screen(img, [disp.width(), disp.height()])
        disp.show(disp_img)


if __name__ == "__main__":
    main_init();
    main_cycle();
