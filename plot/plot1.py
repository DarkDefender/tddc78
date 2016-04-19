import numpy as np
import glob
import matplotlib.pyplot as plt

cores = [1,2,4,8,16]

x = np.array(cores)

y = []
e = []

files = sorted(glob.glob('number*'))

for fn in files:
     data = np.loadtxt(fn)
     y.append( np.mean(data) )
     e.append( np.std(data) )


y = np.array(y)
e = np.array(e)

plt.figure()
plt.errorbar(x, y, yerr=e)
plt.xlabel("Number of cores")
plt.ylabel("Time in seconds")
plt.xlim(0,18)
#plt.show()
plt.savefig("test1.png")
