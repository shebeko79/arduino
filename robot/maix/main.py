from maix import camera, image, display, app, touchscreen
import tracker
import states

CAM_SIZE = [640, 480]
cam: camera.Camera = None
disp: display.Display = None
touch: touchscreen.TouchScreen = None

prev_touched = False
start_point = None


def main_init():
    global cam
    global disp
    global touch

    cam = camera.Camera(CAM_SIZE[0], CAM_SIZE[1], tracker.get_camera_format())
    disp = display.Display()
    touch = touchscreen.TouchScreen()

    image.load_font("sans", "/maixapp/share/font/sans.ttf", size=32)

    states.init()
    states.set_state(states.MainState.state_name)


def main_cycle():
    global prev_touched
    global start_point

    while not app.need_exit():
        st = states.current_state
        if not st:
            break

        img = cam.read()

        tracker.track(img)

        touch_status = touch.read()
        touched = touch_status[2]
        touch_pt = [touch_status[0], touch_status[1]]

        disp_img = st.draw_screen(img, [disp.width(), disp.height()])
        if start_point:
            disp_img.draw_rect(start_point[0], start_point[1], touch_pt[0] - start_point[0],
                               touch_pt[1] - start_point[1],
                               states.BaseState.rectangle_color, 3)

        disp.show(disp_img)

        if touched and not prev_touched:
            btn = st.hit_test(touch_pt)
            if btn:
                st.on_click_button(btn)
            else:
                if st.accept_click:
                    st.on_click(touch_pt)

                if st.accept_rectangle:
                    start_point = touch_pt

        if not touched and prev_touched:
            if st.accept_rectangle and start_point:
                x1 = int(start_point[0] * CAM_SIZE[0] / disp.width())
                y1 = int(start_point[1] * CAM_SIZE[1] / disp.height())
                x2 = int(touch_pt[0] * CAM_SIZE[0] / disp.width())
                y2 = int(touch_pt[1] * CAM_SIZE[1] / disp.height())

                st.on_rectangle(img, [x1, y1, x2, y2])
                start_point = None

        prev_touched = touched


if __name__ == "__main__":
    main_init();
    main_cycle();
