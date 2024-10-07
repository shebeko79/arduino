import track_utils
import tracker
import mover
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

        sz = self.tr.size()
        f = max(sz[0]/CAM_SIZE[0], sz[1]/CAM_SIZE[1])
        f *=2
        if f < max_fit:
            self.fit = f

    def process(self):
        if not self.tr.is_locked():
            mover.stop()
            return

        ct = self.tr.center()
        sz = self.tr.size()

        x = (ct[0]-CAM_SIZE[0]/2)/CAM_SIZE[0]
        f = max(sz[0]/CAM_SIZE[0], sz[1]/CAM_SIZE[1])

        d = f/self.fit
        if d > 1.05:
            y = -1.0
        elif d < 0.96:
            y = 1.0
        else:
            y = 0

        print(f"x={x:.2f} y={y:.2f} d={d:2f} f={f:2f} ct={ct} sz={sz}")

        mover.move(y, x)
