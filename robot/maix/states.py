from maix import image

import algos
import tracker
import mover


class Button:
    def __init__(self, caption, state_name=None, enabled=True):
        self.caption = caption
        self.state_name = state_name
        self.enabled = enabled


class BaseState:
    state_name = "base"

    rectangle_cap = "Select rectangle"
    rectangle_color = image.Color.from_rgb(255, 0, 0)

    def __init__(self):
        self.buttons = []
        self.accept_rectangle = False
        self.accept_click = False

    def draw_screen(self, cam_img: image.Image, out_size):
        img = cam_img.resize(out_size[0], out_size[1])

        if self.accept_rectangle:
            rectangle_cap_size = image.string_size(self.rectangle_cap);
            img.draw_string(2, img.height() - rectangle_cap_size[1] * 2, self.rectangle_cap,
                            self.rectangle_color, 1.5)
        else:
            tracker.draw_trackers(img)

        self.draw_buttons(img)

        return img

    def enter(self):
        self.buttons = []

    def leave(self):
        pass

    def on_click(self, pt):
        pass

    def on_rectangle(self, cam_img: image.Image, rc):
        pass

    def on_click_button(self, btn: Button):
        if btn.state_name:
            set_state(btn.state_name)

    def hit_test(self, pt):
        for btn in self.buttons:
            if not btn.enabled:
                continue

            rc = btn.rc
            if not rc:
                continue

            if rc[0] < pt[0] < rc[2] and rc[1] < pt[1] < rc[3]:
                return btn
        return None

    def button_by_name(self, caption):
        for btn in self.buttons:
            if btn.caption == caption:
                return btn

        return None

    def draw_buttons(self, img: image.Image):
        indent = 8
        enabled_cl = image.Color.from_rgb(255, 255, 255)
        disabled_cl = image.Color.from_rgb(127, 127, 127)

        x = 0
        for btn in self.buttons:
            cl = enabled_cl
            if not btn.enabled:
                cl = disabled_cl

            font_size = image.string_size(btn.caption, font="sans")
            rect_w = font_size[0] + indent * 2
            rect_h = font_size[1] + indent * 2

            btn.rc = [x, img.height() - rect_h, x + rect_w, img.height()]

            img.draw_rect(x, img.height() - rect_h, rect_w, rect_h, cl, 4)
            img.draw_string(x + (rect_w - font_size[0]) // 2, img.height() - rect_h + (rect_h - font_size[1]) // 2,
                            btn.caption, cl, 1, font="sans")

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
    print(f"new state: {state_name}")


class MainState(BaseState):
    state_name = "main"

    def enter(self):
        self.buttons = [Button("Points", PointsState.state_name),
                        Button("Track", TrackInitState.state_name, len(tracker.objects) > 0),
                        Button("Move", MoveState.state_name)]


class PointsState(BaseState):
    state_name = "points"

    def enter(self):
        self.buttons = [Button("Add", AddPointState.state_name),
                        Button("DelLast", DeleteLastPointState.state_name, len(tracker.objects) > 0),
                        Button("Back", MainState.state_name)]


class AddPointState(BaseState):
    state_name = "add_point"

    def __init__(self):
        super().__init__()
        self.accept_rectangle = True

    def enter(self):
        self.buttons = []

    def on_rectangle(self, cam_img: image.Image, rc):
        if rc[2]-rc[0] == 0 or rc[3] - rc[1] == 0:
            return

        tracker.add_tracker(cam_img, rc)
        set_state(PointsState.state_name)


class DeleteLastPointState(BaseState):
    state_name = "delete_last_point"

    def enter(self):
        if len(tracker.objects) > 0:
            tracker.objects.pop()
        set_state(PointsState.state_name)


class TrackInitState(BaseState):
    state_name = "track_init"

    def __init__(self):
        super().__init__()
        self.accept_click = True

    @staticmethod
    def move_to(tr):
        alg = algos.MoveToAlgo(tr)
        algos.set_algo(alg)
        set_state(TrackState.state_name)

    def enter(self):
        self.buttons = [Button("Back", MainState.state_name)]
        if len(tracker.objects) == 1:
            self.move_to(tracker.objects[0])

    def on_click(self, pt):
        tr = tracker.hit_test(pt)
        if tr:
            self.move_to(tr)


class TrackState(BaseState):
    state_name = "track"

    def __init__(self):
        super().__init__()

    def enter(self):
        self.buttons = [Button("Stop", MainState.state_name)]

    def leave(self):
        algos.set_algo(None)


class MoveState(BaseState):
    state_name = "move"

    def __init__(self):
        super().__init__()

    def enter(self):
        self.buttons = [Button("Forward"), Button("<<<"), Button(">>>"), Button("Reverse"), Button("Stop"),
                        Button("Back", MainState.state_name)]

    def leave(self):
        mover.stop()

    def on_click_button(self, btn: Button):
        if btn.state_name:
            set_state(btn.state_name)
        elif btn.caption == "<<<":
            mover.move(0, -1)
        elif btn.caption == ">>>":
            mover.move(0, 1)
        elif btn.caption == "Forward":
            mover.move(1, 0)
        elif btn.caption == "Reverse":
            mover.move(-1, 0)
        elif btn.caption == "Stop":
            mover.stop()


def init():
    add_state(MainState())
    add_state(PointsState())
    add_state(AddPointState())
    add_state(DeleteLastPointState())
    add_state(TrackInitState())
    add_state(TrackState())
    add_state(MoveState())
