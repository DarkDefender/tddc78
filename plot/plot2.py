import numpy as np
import glob
import matplotlib.pyplot as plt
import sys

cores = [1,2,3,4]

x = np.array(cores)

y = []
low = []
high = []

files = sorted(glob.glob('prob*'))

for fn in files:
     data = np.loadtxt(fn)
     mean = np.mean(data)
     y.append( mean )
     low.append( mean - np.amin(data) )
     high.append( np.amax(data) - mean )

y = np.array(y)

plt.figure()
plt.errorbar(x, y, yerr=[low,high])
plt.xlabel("Problem number")
plt.ylabel("Time in seconds")
plt.xlim(0,5)
#plt.show()
plt.savefig(sys.argv[1] + ".eps")
