import numpy as np
import glob
import matplotlib.pyplot as plt
import sys

cores = [1,2,4,8,16]

x = np.array(cores)

y = []
low = []
high = []

files = sorted(glob.glob('number*'))

for fn in files:
     data = np.loadtxt(fn)
     mean = np.mean(data)
     y.append( mean )
     low.append( mean - np.amin(data) )
     high.append( np.amax(data) - mean )

y = np.array(y)

plt.figure()
plt.errorbar(x, y, yerr=[low,high])
plt.xlabel("Number of cores")
plt.ylabel("Time in seconds")
plt.xlim(0,18)
#plt.show()
plt.savefig(sys.argv[1] + ".eps")
