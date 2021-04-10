import numpy as np 
import pandas as pd 
import matplotlib.pyplot as plt 
import seaborn as sns 

ratio1 = []
ratio2 = []

with open("./bin/tmp1.out", "r") as fin:
    while True:
        try:
            r1, r2 = map(float, fin.readline().strip().split())
            ratio1.append(r1)
            ratio2.append(r2)
        except:
            break

plt.plot(ratio1)
plt.plot(ratio2)

plt.show()
plt.savefig('./11_vm_vs_server.png')