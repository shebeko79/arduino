import math

import track_utils
import tracker
import mover
import pan_tilt
from track_utils import CAM_SIZE


class BaseAlgo:
    def process(self):
        pass

    def start(self):
        pass

    def stop(self):
        mover.stop()


current_algo: BaseAlgo = None


def process():
    if current_algo:
        current_algo.process()


def set_algo(algo:BaseAlgo):
    global current_algo

    if current_algo:
        current_algo.stop()

    current_algo = algo

    if algo:
        algo.start()


class MoveToAlgo(BaseAlgo):

    def __init__(self, tr: tracker.TrackObject, max_fit=0.75):
        self.tr = tr
        self.fit = max_fit
        self.ax = 0.0
        self.ay = 0.0
        self.cur_fit = 0.0

        sz = self.tr.size()
        f = max(sz[0]/CAM_SIZE[0], sz[1]/CAM_SIZE[1])
        f *= 2
        if f < max_fit:
            self.fit = f

    def start(self):
        self.tr.start_track()

    def stop(self):
        super().stop()
        self.tr.stop_track()

    def process(self):
        try:
            self.find_position()
            self.move_camera()
            self.move_robot()
        except Exception as e:
            print(e)

    def find_position(self):
        if not self.tr.is_locked():
            return

        ct = self.tr.center()
        sz = self.tr.size()

        self.cur_fit = max(sz[0]/CAM_SIZE[0], sz[1]/CAM_SIZE[1])

        x = ct[0]-CAM_SIZE[0]/2
        y = ct[1]-CAM_SIZE[1]/2

        lax = math.atan2(x, track_utils.FOCAL_DIST)
        lay = math.atan2(y, track_utils.FOCAL_DIST)

        cam_ax = pan_tilt.get_pan_angle()
        cam_ay = pan_tilt.get_tilt_angle()

        self.ax = lax + cam_ax
        self.ay = lay + cam_ay

        print(f"img_pos=({x};{y}) img_ang=({lax/math.pi*180:.2f};{lay/math.pi*180:.2f}) cam_ang=({cam_ax/math.pi*180:.2f};{cam_ay/math.pi*180:.2f}) V=({self.ax/math.pi*180:.2f};{self.ay/math.pi*180:.2f})")

    def move_camera(self):
        if not self.tr.is_locked():
            return

        pan_tilt.set_pan_angle(self.ax)

        y = pan_tilt.angle2tilt(self.ay)
        if y > pan_tilt.Tilt.MAX_DOWN:
            y = pan_tilt.Tilt.MAX_DOWN
        elif y < pan_tilt.Tilt.FRONT-(pan_tilt.Tilt.MAX_DOWN-pan_tilt.Tilt.FRONT):
            y = pan_tilt.Tilt.FRONT-(pan_tilt.Tilt.MAX_DOWN-pan_tilt.Tilt.FRONT)

        pan_tilt.set_tilt(y)

        pass

    def move_robot(self):
        if not self.tr.is_locked():
            mover.stop()
            return

        turn = self.ax/math.pi*4

        d = self.cur_fit/self.fit
        if d > 1.05:
            speed = -1.0
        elif d < 0.96:
            speed = 1.0
        else:
            speed = 0

        speed = speed*math.cos(self.ax)

        print(f"V=({self.ax/math.pi*180:.2f};{self.ay/math.pi*180:.2f}) turn={turn:.2f} speed={speed:.2f} d={d:2f}")

        mover.move(speed, turn)

