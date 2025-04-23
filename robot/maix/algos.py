import math

import track_utils
import tracker
import mover
import pan_tilt
from track_utils import CAM_SIZE


class BaseAlgo:

    def __init__(self):
        self.stopped = False

    def process(self):
        pass

    def start(self):
        self.stopped = False
        pass

    def stop(self):
        self.stopped = True
        mover.stop()

    def is_stopped(self):
        return self.stopped


current_algo: BaseAlgo = None


def process():
    if current_algo:
        current_algo.process()


def set_algo(algo: BaseAlgo):
    global current_algo

    if current_algo:
        current_algo.stop()

    current_algo = algo

    if algo:
        algo.start()


def get_fit(tr):
    sz = tr.size()
    return max(sz[0]/CAM_SIZE[0], sz[1]/CAM_SIZE[1])


class MoveToAlgo(BaseAlgo):

    def __init__(self, tr: tracker.TrackObject, fit, do_pan, do_move, continues):
        self.tr = tr
        self.fit = fit
        self.ax = 0.0
        self.ay = 0.0
        self.cur_fit = 0.0
        self.do_pan = do_pan
        self.do_move = do_move
        self.continues = continues

        if self.fit is None:
            self.fit = get_fit(self.tr)

    def start(self):
        super().start()
        self.tr.start_track()

    def stop(self):
        super().stop()
        self.tr.stop_track()

    def process(self):
        try:
            if self.is_stopped():
                return

            self.find_position()

            if self.do_pan:
                self.move_camera()

            if self.do_move:
                self.move_robot()
        except Exception as e:
            print(e)

    def find_position(self):
        if not self.tr.is_locked():
            return

        ct = self.tr.center()

        self.cur_fit = get_fit(self.tr)

        x = ct[0]-CAM_SIZE[0]/2
        y = ct[1]-CAM_SIZE[1]/2

        lax = math.atan2(x, track_utils.FOCAL_DIST)
        lay = math.atan2(y, track_utils.FOCAL_DIST)

        cam_ax = pan_tilt.get_pan_angle()
        cam_ay = pan_tilt.get_tilt_angle()

        self.ax = lax + cam_ax
        self.ay = lay + cam_ay

        #print(f"img_pos=({x};{y}) img_ang=({lax/math.pi*180:.2f};{lay/math.pi*180:.2f}) cam_ang=({cam_ax/math.pi*180:.2f};{cam_ay/math.pi*180:.2f}) V=({self.ax/math.pi*180:.2f};{self.ay/math.pi*180:.2f})")

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
        if d < 0.8:
            speed = 1.0
        elif d < 1.0:
            speed = 0.5
        else:
            speed = 0

        speed = speed*math.cos(self.ax)

        print(f"V=({self.ax/math.pi*180:.2f};{self.ay/math.pi*180:.2f}) turn={turn:.2f} speed={speed:.2f} d={d:2f} cur_fit={self.cur_fit} target_fit={self.fit}")

        mover.move(speed, turn)

        if speed == 0 and not self.continues:
            self.stop()

