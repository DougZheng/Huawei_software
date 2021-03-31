import numpy as np 
import pandas as pd 
import matplotlib.pyplot as plt 
import seaborn as sns 

n = 513
serverRes = []
serverUse = []

with open("./bin/tmp.out", "r") as fin:
    for i in range(n):
        a = list(map(int, fin.readline().split()))
        serverRes.append(a)
    fin.readline()
    for i in range(n):
        a = list(map(int, fin.readline().split()))
        serverUse.append(a)
serverRes = serverUse
serverRes = np.array(serverRes)
sns.heatmap(data = serverRes[:88, :88], vmax = 10)

plt.show()
plt.savefig('./pic2.png')