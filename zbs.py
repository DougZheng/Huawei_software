import numpy as np 
import pandas as pd 
import matplotlib.pyplot as plt 
import seaborn as sns 

cpuCores1 = []
memorySize1 = []
serverCost1 = []
powerCost1 = []

vmCpu1 = []
vmMemory1 = []

with open("data/training-1.txt", "r") as fin:
    print("xxx")
    n = int(fin.readline())
    for i in range(n):
        ss = fin.readline().strip().split(',')
        ss = [s.strip('() ') for s in ss]
        cpuCores1.append(int(ss[1]))
        memorySize1.append(int(ss[2]))
        serverCost1.append(int(ss[3]))
        powerCost1.append(int(ss[4]))
    m = int(fin.readline())
    for i in range(m):
        ss = fin.readline().strip().split(',')
        ss = [s.strip('() ') for s in ss]
        vmCpu1.append(int(ss[1]))
        vmMemory1.append(int(ss[2]))

cpuCores2 = []
memorySize2 = []
serverCost2 = []
powerCost2 = []

with open("data/training-2.txt", "r") as fin:
    print("yyy")
    n = int(fin.readline())
    for i in range(n):
        ss = fin.readline().strip().split(',')
        ss = [s.strip('() ') for s in ss]
        cpuCores2.append(int(ss[1]))
        memorySize2.append(int(ss[2]))
        serverCost2.append(int(ss[3]))
        powerCost2.append(int(ss[4]))

# plt.figure(figsize = (18, 9))
plt.title("serverCost vs. powerCost")
plt.xlabel("serverCost")
plt.ylabel("powerCost")
# sns.scatterplot(x = cpuCores1, y = memorySize1)
sns.scatterplot(x = vmCpu1, y = vmMemory1)

plt.show()
plt.savefig('./pic.png')