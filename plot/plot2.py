import numpy as np
import glob
import matplotlib.pyplot as plt
import sys

cores = [1,2,3,4]

x = np.array(cores)

y = []
e = []

files = sorted(glob.glob('prob*'))

for fn in files:
     data = np.loadtxt(fn)
     y.append( np.mean(data) )
     e.append( np.std(data) )


y = np.array(y)
e = np.array(e)

plt.figure()
plt.errorbar(x, y, yerr=e)
plt.xlabel("Problem number")
plt.ylabel("Time in seconds")
plt.xlim(0,5)
#plt.show()
plt.savefig(sys.argv[1] + ".eps")
