import threedbg
import atexit
threedbg.init()
atexit.register(threedbg.wait)

import numpy as np


x = np.linspace(0, 1, 20)
y = np.linspace(0, 1, 20)
z = np.linspace(0, 1, 20)

x, y, z = np.meshgrid(x, y, z)
x = x.flatten()
y = y.flatten()
z = z.flatten()

t = 0
while threedbg.working():
    t += 0.0001
    ps = np.array((x,y,z)).T.astype(np.float32)
    threedbg.Point.clear()
    threedbg.Point.add(ps, ps)
    threedbg.Point.flush()
