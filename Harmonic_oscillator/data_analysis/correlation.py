import numpy as np
import matplotlib.pyplot as plt

data = open("../results/output/mean_y2/bh_1_omega_1/mean_y2_N_10.txt","r")
y2 = np.loadtxt(data, unpack = True)
data.close()

print(np.mean(y2))
print(np.std(y2))

plt.figure(1)
plt.plot(np.arange(0,len(y2)), y2, '.')
plt.plot(np.arange(0,len(y2)), np.mean(y2)*np.ones(len(y2)), color='red')
plt.show()


y2 = y2[0:1000]

N=len(y2)
C = np.zeros(N)
y2mean = np.mean(y2)
std = np.std(y2)

for k in range(0,N,1):
	for i in range(1,N-k,1):
		C[k] = C[k] + (y2[i]-y2mean)*(y2[i+k]-y2mean)
	C[k] = C[k] / ((N-k)*std**2)

plt.figure(2)
plt.plot(np.arange(0,1000,1), C[0:1000], '.')
plt.show()