from ulab import numpy as np # to get access to ulab numpy functions
import time

dt = 1.0/100.0 # 10Hz
t = np.arange(0.0, 10.24, dt) # 10s

y = 1*np.sin(2 * np.pi * 1 * t) + 2*np.sin(2 * np.pi * 2 * t) + 3*np.sin(2 * np.pi * 3 * t)

n = len(y) # length of the signal
Y = np.fft.fft(y) # fft computing and normalization

for i in y:
    print((i,))
    time.sleep(0.1)
