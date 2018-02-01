import time
import numpy as np
import matplotlib.pyplot as plt

import threedbg
import atexit
threedbg.init()
atexit.register(threedbg.wait)

x = np.linspace(0, 1, 10)
y = np.linspace(0, 1, 10)
z = np.linspace(0, 1, 10)

x, y, z = np.meshgrid(x, y, z)
x = x.flatten()
y = y.flatten()
z = z.flatten()

t = 0

ims = []
for i in range(10):
    t += 0.0001
    ps = np.array((x,y,z)).T.astype(np.float32)
    threedbg.Point.clear()
    threedbg.Point.add(ps, ps)
    threedbg.Point.flush()

    im = threedbg.display.getImage()
    ims.append(im)