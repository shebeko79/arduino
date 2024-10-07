import math

from maix import nn, image

MODEL_PATH = "/root/models/nanotrack.mud"

objects = []


class TrackObject:

    def __init__(self, img: image.Image, rc):
        self.model = nn.NanoTrack(MODEL_PATH)
        self.model.init(img, rc[0], rc[1], rc[2] - rc[0], rc[3] - rc[1])
        self.rc = rc
        self.img_size = [img.width(), img.height()]
        self.r = self.model.track(img)

    def track(self, img: image.Image):
        self.r = self.model.track(img)

    def is_locked(self):
        return self.r.score > 0.93

    def center(self):
        return [self.r.x+self.r.w/2, self.r.y+self.r.h/2]

    def size(self):
        return [self.r.w, self.r.h]


def add_tracker(img, rc):
    tr = TrackObject(img, rc)
    objects.append(tr)


def get_camera_format():
    model = nn.NanoTrack(MODEL_PATH)
    return model.input_format()


def draw_trackers(img: image.Image):
    iw = img.width()
    ih = img.height()
    hi_cl = image.Color.from_rgb(255, 0, 0)
    gray_cl = image.Color.from_rgb(127, 127, 127)

    for i in range(0, len(objects)):
        o: TrackObject = objects[i]
        r = o.r

        x = int(r.x * iw / o.img_size[0])
        y = int(r.y * ih / o.img_size[1])
        w = int(r.w * iw / o.img_size[0])
        h = int(r.h * ih / o.img_size[1])

        if o.is_locked():
            cl = hi_cl
        else:
            cl = gray_cl

        img.draw_rect(x, y, w, h, cl)

        cap = f"{i}"
        font_size = image.string_size(cap)
        img.draw_string(x, y - font_size[1] - 2, cap, cl)


def hit_test(pt):
    sel = None
    sel_d = 0

    for o in objects:
        r = o.r
        if r.x < pt[0] < r.x + r.w and r.y < pt[1] < r.y + r.h:
            c = o.center()
            dx = c[0] - pt[0]
            dy = c[1] - pt[1]
            d = math.sqrt(dx * dx + dy * dy)

            if not sel or d < sel_d:
                sel = o
                sel_d = d

    return sel


def track(img: image.Image):
    for o in objects:
        o.track(img)
