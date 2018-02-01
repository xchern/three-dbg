import time
import math
import numpy as np
import matplotlib.pyplot as plt

import threedbg
import atexit
threedbg.init()
atexit.register(threedbg.free)

x = np.linspace(-1, 1, 20)
y = np.linspace(-1, 1, 20)
z = np.linspace(-1, 1, 20)

x, y, z = np.meshgrid(x, y, z)
x = x.flatten()
y = y.flatten()
z = z.flatten()

t = 0
while threedbg.working():
    time.sleep(.05)
    t += 0.05
    print(t)

    ps = np.array((x,y,z)).T.astype(np.float32)
    mask = ps[:,2] < math.sin(t)
    ps = ps[mask, :]

    threedbg.Point.clear()
    threedbg.Point.add(ps, ps)
    threedbg.Point.flush()
    threedbg.Line.clear()
    #threedbg.Line.addAABB(ps - .03, ps + .03)
    threedbg.Line.addAABB(np.reshape(ps.min(axis=0), (1, 3)) - .02,
                          np.reshape(ps.max(axis=0), (1, 3)) + .02)
    threedbg.Line.addAxes(0, 0, 0, 1)
    threedbg.Line.flush()

    im = threedbg.display.getImage()