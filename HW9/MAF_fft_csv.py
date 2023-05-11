import csv
import matplotlib.pyplot as plt
import numpy as np

# unpack data here
t = [] # column 0
data = [] # column 1

with open('sigC.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        data.append(float(row[1])) # second column
dt = ( t[-1] / len(data) )
        
        
# smooth data here
numAvg = 400
smoothData = []
for i in range(numAvg):
    smoothData.append(0)
for i in range(len(data)-numAvg):
    moveAvg = 0
    for j in range(numAvg):
        moveAvg = moveAvg + data[i+j]
    moveAvg = moveAvg / numAvg
    smoothData.append(moveAvg)



# fft data here
ds = np.arange(0,t[-1],dt) # time vector
y = data # the data to make the fft from
n = len(y) # length of the signal
k = np.arange(n)
T = n*dt
frq = k/T # two sides frequency range
frq = frq[range(int(n/2))] # one side frequency range
Y = np.fft.fft(y)/n # fft computing and normalization
Y = Y[range(int(n/2))]

ts = np.arange(0,t[-1],dt) # time vector
ySmooth = smoothData # the data to make the fft from
n = len(y) # length of the signal
k = np.arange(n)
T = n*dt
frq = k/T # two sides frequency range
frq = frq[range(int(n/2))] # one side frequency range
YSmooth = np.fft.fft(ySmooth)/n # fft computing and normalization
YSmooth = YSmooth[range(int(n/2))]


# plot data here
fig, (ax1, ax2) = plt.subplots(2, 1)
ax1.plot(t,y,'k')
ax1.plot(t,ySmooth,'r')
ax1.set_xlabel('Time')
ax1.set_ylabel('Amplitude')
ax2.loglog(frq,abs(Y),'k') # plotting the fft
ax2.loglog(frq,abs(YSmooth),'r')
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
fig.suptitle("# Moving Average = "+str(numAvg))
plt.show()