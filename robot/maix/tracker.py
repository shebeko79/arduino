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


def add_tracker(img, rc):
    tr = TrackObject(img, rc)
    objects.append(tr)


def get_camera_format():
    model = nn.NanoTrack(MODEL_PATH)
    return model.input_format()


def draw_trackers(img: image.Image):
    for i in range(0,len(objects)):
        o = object[i]
        r = o.r;

        img.draw_rect(r.x, r.y, r.w, r.h, image.Color.from_rgb(255, 0, 0), 4)

        cap = f"{i}"
        font_size = image.string_size(cap)
        img.draw_string(r.x, r.y - font_size[1] - 2, cap, image.Color.from_rgb(255, 0, 0), 1.5)
