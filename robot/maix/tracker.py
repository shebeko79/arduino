import math

from maix import nn, image

import track_utils

NANO_MODEL_PATH = "/root/models/nanotrack.mud"

room_model = nn.YOLO11(model="/root/models/yolo11n_320_room.mud")
room_objects = []

nanotrack_objects = []


class TrackObject:

    def is_locked(self):
        return False

    def center(self):
        return [0, 0]

    def size(self):
        return [0, 0]


class NanoTrackObject(TrackObject):

    def __init__(self, img: image.Image, rc):
        self.model = nn.NanoTrack(NANO_MODEL_PATH)
        self.model.init(img, rc[0], rc[1], rc[2] - rc[0], rc[3] - rc[1])
        self.rc = rc
        self.img_size = [img.width(), img.height()]
        self.r = self.model.track(img)

    def track(self, img: image.Image):
        self.r = self.model.track(img)

    def is_locked(self):
        return self.r.score > 0.93

    def center(self):
        return [self.r.x + self.r.w / 2, self.r.y + self.r.h / 2]

    def size(self):
        return [self.r.w, self.r.h]


def add_nanotracker(img, rc):
    rc[0] = int(rc[0] * track_utils.CAM_SIZE[0])
    rc[1] = int(rc[1] * track_utils.CAM_SIZE[1])
    rc[2] = int(rc[2] * track_utils.CAM_SIZE[0])
    rc[3] = int(rc[3] * track_utils.CAM_SIZE[1])

    tr = NanoTrackObject(img, rc)
    nanotrack_objects.append(tr)


def get_camera_format():
    return image.Format.FMT_RGB888


def draw_trackers(img: image.Image):
    iw = img.width()
    ih = img.height()
    hi_cl = image.Color.from_rgb(255, 0, 0)
    gray_cl = image.Color.from_rgb(127, 127, 127)

    for i in range(0, len(nanotrack_objects)):
        o: TrackObject = nanotrack_objects[i]
        r = o.r

        x = int(o.rc[0] * iw / o.img_size[0])
        y = int(o.rc[1] * ih / o.img_size[1])
        w = int((o.rc[2] - o.rc[0]) * iw / o.img_size[0])
        h = int((o.rc[3] - o.rc[1]) * ih / o.img_size[1])

        if o.is_locked():
            cl = hi_cl
        else:
            cl = gray_cl

        img.draw_rect(x, y, w, h, cl, 2)

        cap = f"{i}"
        font_size = image.string_size(cap)
        img.draw_string(x, y - font_size[1] - 2, cap, cl)

    for o in room_objects:
        x = int(o.x * iw / room_model.input_width())
        y = int(o.y * ih / room_model.input_height())
        w = int(o.w * iw / room_model.input_width())
        h = int(o.h * ih / room_model.input_height())

        if o.score > 0.8:
            cl = hi_cl
        else:
            cl = gray_cl

        img.draw_rect(x, y, w, h, cl, 2)
        cap = room_model.labels[o.class_id]
        print(f'{room_model.labels[o.class_id]}: {o.score:.2f}')

        font_size = image.string_size(cap)
        img.draw_string(x, y - font_size[1] - 2, cap, cl)


def hit_test(pt):
    sel = None
    sel_d = 0

    pt[0] = int(pt[0] * track_utils.CAM_SIZE[0])
    pt[1] = int(pt[1] * track_utils.CAM_SIZE[1])

    for o in nanotrack_objects:
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


def count():
    return len(nanotrack_objects)


def nanotrack_count():
    return len(nanotrack_objects)


def remove_lastnanotrack():
    if len(nanotrack_objects) > 0:
        nanotrack_objects.objects.pop()


def trackers():
    return nanotrack_objects



def track(img: image.Image):
    global room_objects

    if len(nanotrack_objects) > 0:
        nano_img = img.to_format(image.Format.FMT_BGR888)
        for o in nanotrack_objects:
            o.track(nano_img)

    room_img = img
    if room_img.width() != room_model.input_width() or room_img.height() != room_model.input_height():
        room_img = img.resize(room_model.input_width(), room_model.input_height())

    room_objects = room_model.detect(room_img, conf_th=0.5, iou_th=0.45)
    print(f'{room_objects=}')
