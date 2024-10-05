from maix import image
import tracker


class BaseState:
    state_name = "base"

    def __init__(self):
        self.buttons = {}

    def draw_screen(self, cam_img: image.Image, out_size):
        img = cam_img.copy()
        img.resize(out_size[0], out_size[1])

        self.draw_buttons(img)
        tracker.draw_trackers(img)

        return img

    def enter(self):
        pass

    def leave(self):
        pass

    def click_button(self, btn_cap):
        set_state(btn_cap)

    def draw_buttons(self, img: image.Image):
        indent = 8
        enabled_cl = image.Color.from_rgb(255, 255, 255)
        disabled_cl = image.Color.from_rgb(127, 127, 127)

        x = 0
        for cap in self.buttons:
            cl = enabled_cl
            if not self.buttons[cap]:
                cl = disabled_cl

            font_size = image.string_size(cap)
            rect_w = font_size[0] + indent * 2
            rect_h = font_size[1] + indent * 2

            img.draw_rect(x, img.height() - rect_h, rect_w, rect_h, cl, 4)
            img.draw_string(x + (rect_w - font_size[0]) // 2, img.height() - rect_h + (rect_h - font_size[1]) // 2,
                            cap, cl, 1)

            x += rect_w + indent


current_state: BaseState = None
states = {}


def add_state(state):
    states[state.state_name] = state


def set_state(state_name):
    global current_state
    global states

    if not (state_name in states):
        print(f"state {state_name} doesn't exist")
        return

    new_state = states[state_name]

    if current_state:
        current_state.leave()

    current_state = new_state

    new_state.enter()


class MainState(BaseState):
    state_name = "main"

    def enter(self):
        self.buttons = {"Add": True, "Track": len(tracker.objects) > 0}

    def leave(self):
        pass


def init():
    add_state(MainState())
