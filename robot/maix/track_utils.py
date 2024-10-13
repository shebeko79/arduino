
CAM_SIZE = [640, 480]
BASE_PATH = "/root/robot"


def make_rect(pt1, pt2):
    x1 = min(pt1[0], pt2[0])
    y1 = min(pt1[1], pt2[1])
    x2 = max(pt1[0], pt2[0])
    y2 = max(pt1[1], pt2[1])
    return [x1, y1, x2, y2]
