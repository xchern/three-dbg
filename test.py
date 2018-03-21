import pytest

import time
import math
import numpy as np
import matplotlib.pyplot as plt

import threedbg
import atexit
threedbg.init()
atexit.register(threedbg.free)

x = np.linspace(-1, 1, 5)
y = np.linspace(-1, 1, 5)
z = np.linspace(-1, 1, 5)

x, y, z = np.meshgrid(x, y, z)
x = x.flatten()
y = y.flatten()
z = z.flatten()
ps = np.array((x,y,z)).T

def test_show_point():
    threedbg.Point.clear()
    threedbg.Point.add(ps, ps)
    threedbg.Point.flush()
    time.sleep(2)

def test_show_line():
    threedbg.Line.clear()
    threedbg.Line.addAABB(np.reshape(ps.min(axis=0), (1, 3)) - .02,
                          np.reshape(ps.max(axis=0), (1, 3)) + .02)
    threedbg.Line.flush()
    time.sleep(1)

    threedbg.Line.clear()
    threedbg.Line.addAxes(0, 0, 0, 2)
    threedbg.Line.flush()
    time.sleep(1)

    threedbg.Line.clear()
    threedbg.Line.addAABB(ps - .1, ps + .1)
    threedbg.Line.flush()
    time.sleep(1)

def test_snapshot():
    im = threedbg.display.getImage()
    #plt.imshow(im[::-1,:,:])
    #plt.show()

def test_camera():
    state = threedbg.camera.dump()
    threedbg.camera.load(state)

def test_key():
    print('press any character key')
    while True:
        c = threedbg.display.getChar()
        if c != '\0':
            break
    print('press any it is', c)
