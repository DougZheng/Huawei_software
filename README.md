# Huawei_software
2021华为软件精英挑战赛

## 前言

无意中看到赛题，觉得很有意思，就匆忙在报名截止前几天上了车，也因此没来得及找队友。最终成绩是初赛排名 12 ，复赛排名 12 ，个人感觉还有很多 idea 因为时间问题没实现，所以如果有下次一定得好好找队友一起打。 

## 思路

### 输入输出

输入这一块，写一个 split 函数就能比较方便地处理了。

```cpp
static std::vector<std::string> split(const std::string &s, const std::string &delimiters = " ,()") {
    std::vector<std::string> tokens;
    std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    std::string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (pos != std::string::npos || lastPos != std::string::npos) {
        tokens.emplace_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
    return tokens;
}
```

输出可以考虑缓存下来，一次性输出（在初赛阶段可以使用离线做法，可以选择性输出其中一种方案的结果）。

```cpp
std::string outputBuffer;
void write() {
    std::cout << outputBuffer;
    std::cout.flush();
    outputBuffer.clear();
}
```

服务器、虚拟机以及调度命令的结构体，参考了初赛开源的 baseline ，不同点是用数组下标区分 AB 节点，方便索引。

### 简化问题

虚拟机有且仅有两种部署方式：单节点和双节点，考虑到混合部署可能带来的碎片化问题，我采用的方式是分别处理，即一部分服务器只部署单节点虚拟机，另一部分只部署双节点虚拟机，可以一定程度上简化问题。

这带来一个可能的问题，比如双节点部署的服务器缺少时仍有大量空闲的单节点部署的服务器存在，此时直接购买新服务器可能造成浪费。在实际调试中发现，数据集并未出现这种情况。即便有此种情况，在我的代码中也预留了服务器类别的切换操作（详见代码中 switchServer 函数），但实际提升并不大。

### 数据结构

初赛阶段，使用一维数组直接存放所有服务器，每次选择服务器都需要遍历整个数组才可以得到目标服务器，效率较低。进入复赛阶段，由于迁移次数的增加，时间开销也会线性增加。注意到 cpu 核数和内存的最大值是 1024 ，也就是单节点最大值为 512 ，可以使用数组下标来组织所有服务器，定义如下：

```cpp
std::vector<std::vector<std::list<int>>> serversIdRes[2][2];
std::vector<std::vector<std::list<int>>> serversIdUse[2][2];
```

其中 `[2][2]` 的第一维代表服务器类别（0 即单节点部署，1 即双节点部署），第二维代表具体哪个节点（单节点部署则代表两个节点情况，双节点部署只考虑 0 节点即可）。

`serversIdRes[isDouble][nodeId][cpuRes][memRes]`  即表示 cpu 核数剩余 `cpuRes` 且内存剩余 `memRes` 的服务器的下标列表。

`serversIdUse[isDouble][nodeId][cpuUse][memUse]`  即表示 cpu 核数使用 `cpuUse` 且内存使用 `memUse` 的服务器的下标列表。

这样组织的好处是，可以按照某种策略寻找目标服务器，并即时返回。

这也带来一个可能的问题，最坏情况下每次查找都是 512 × 512 ，效率可能还是不够高。由于我的服务器购买策略里倾向于购买小容量的服务器，一定程度上缓解了这个问题。另外，在代码中还没有实现的一个重要优化，可以让查找时间降一个阶，即 $\sqrt{512} × \sqrt{512}$ ，详见优化思路的最后一点。

### 服务器购买策略

每次服务器不够时，就新购买一台。

策略相对比较简单，根据目前的虚拟机 cpu 核数与内存的比值，来评估新服务器的价值，这可能需要一定的泛化，否则实际也只会购买特定的几种比例相对均衡的服务器。这一点在我的代码中尚未考虑并解决，一些比例失衡的服务器性价比反而更高。

### 虚拟机迁移策略

实践发现，迁移策略显得并不是那么重要（至少在复赛阶段）。由于复赛阶段迁移次数较多，迁移策略足够简单也可以有不错的效果，前提是能够快速进行迁移。代码中的迁移策略可以简单理解为两重循环（cpu 核数、内存）直接寻找第一个符合的服务器。在不出现大量撤销请求的数据集里（复赛训练数据集 2 ，利用率稳定达到 98% ，甚至能达到 99% 以上）。

主要的迁移策略就是第一阶段里遍历存量虚拟机，若所在服务器未达到几乎满载，则迁移。第二阶段，此时可能存在大量单节点部署的服务器，其中一个节点满载，另一个空载，针对这种服务器进行迁移。第三阶段，对比例严重失衡的服务器进行迁移调整。

### 优化思路

由于时间问题，还有比较多的优化方向没有来得及去仔细考虑并实现，在这里说一下我的理解吧。

- 服务器购买策略很重要，直接决定了成本下限，复赛里的可以预先查看未来一段时间的请求情况，这一条件在我的代码中暂未使用，但观察训练数据可以发现，数据集 1 存在很多次的突发新增虚拟机请求并很快撤销，这类情况可以适当考虑用硬件成本低、能耗较高的服务器来应对。

- 迁移策略并不显得很重要，实践发现简单的策略反而更有效，主要是要做好迁移时间的优化。

- 数据结构，复杂的数据结构在这里不太现实，使用简单的数组和链表组织能获得更好的效果。比如二维数组 + 链表组织所有服务器。

- 多线程，考虑到评测机也只是双核，这个优化可能并不是很重要，需要考虑下线程开销和编码复杂度。

- 迁移时间的优化，可以考虑将 cpu 核数 × 内存这个二维数组切割成若干部分，如按根号规模切割。因为选择服务器使用 first fit 策略，所以没有服务器的那些块是没必要遍历的。

  具体地，原方案是 512 × 512 的二维数组，可以考虑切割成若干大小为 $\sqrt{512} × \sqrt{512}$  的块，每个块维护服务器数量（这很容易动态维护）。这样，在查找目标服务器时，一共也只有 $\sqrt{512} × \sqrt{512}$  个这样的块需要查找，一旦发现块内服务器数量为 0 则可以直接跳过，否则进入块内查找，时间也是 $\sqrt{512} × \sqrt{512}$  。这样，在选择目标服务器时，查找的次数上限也就是 512 左右，而不是 512 × 512 。

  个人认为这个优化应当是可以大量节省运行时间的，不过本人并未在代码中实现，如果需要，也欢迎找我作进一步的讨论。

### 参考代码

```cpp
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <random>
#include <cassert>
#include <chrono>

// #define DEBUG
#define EPS 1e-7

class Solution {

public:

	static std::vector<std::string> split(const std::string &s, const std::string &delimiters = " ,()") {
		std::vector<std::string> tokens;
		std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
		std::string::size_type pos = s.find_first_of(delimiters, lastPos);
		while (pos != std::string::npos || lastPos != std::string::npos) {
			tokens.emplace_back(s.substr(lastPos, pos - lastPos));
			lastPos = s.find_first_not_of(delimiters, pos);
			pos = s.find_first_of(delimiters, lastPos);
		}
		return tokens;
	}

	struct VmInfo;

	struct ServerInfo {

		friend std::istream &operator>>(std::istream &is, ServerInfo &serverInfo) {
			std::string s;
			std::getline(is, s);
			std::vector<std::string> tokens(Solution::split(s));
			serverInfo.serverType = tokens[0];
			serverInfo.cpuCores[0] = serverInfo.cpuCores[1] = std::stoi(tokens[1]) / 2;
			serverInfo.memorySize[0] = serverInfo.memorySize[1] = std::stoi(tokens[2]) / 2;
			serverInfo.serverCost = std::stoi(tokens[3]);
			serverInfo.powerCost = std::stoi(tokens[4]);
			return is;
		};

		std::string to_string() const {
			return "(" + serverType + ", " + std::to_string(cpuCores[0]) + "/" 
				+ std::to_string(cpuTotal / 2) + "+" + std::to_string(cpuCores[1]) + "/"
				+ std::to_string(cpuTotal / 2) + ", " + std::to_string(memorySize[0]) + "/"
				+ std::to_string(memoryTotal / 2) + "+" + std::to_string(memorySize[1]) + "/"
				+ std::to_string(memoryTotal / 2) + ", " + std::to_string(serverCost) + ", "
				+ std::to_string(powerCost) + ")";
		}

		friend std::ostream &operator<<(std::ostream &os, const ServerInfo &serverInfo) {
			os << serverInfo.to_string();
			return os;
		}

		std::string serverType;
		int cpuCores[2];
		int memorySize[2];
		int serverCost;
		int powerCost;
		int serverId = -1;

		int cpuUsed[2] = {};
		int memoryUsed[2] = {};

		int cpuTotal;
		int memoryTotal;

		int isDouble;

		void install(int nodeId, int cpu, int memory) {
			if (nodeId == -1) {
				install(0, cpu / 2, memory / 2);
				install(1, cpu / 2, memory / 2);
				return;
			}
			cpuCores[nodeId] -= cpu;
			memorySize[nodeId] -= memory;

			cpuUsed[nodeId] += cpu;
			memoryUsed[nodeId] += memory;
		}

		void uninstall(int nodeId, int cpu, int memory) {
			if (nodeId == -1) {
				uninstall(0, cpu / 2, memory / 2);
				uninstall(1, cpu / 2, memory / 2);
				return;
			}
			cpuCores[nodeId] += cpu;
			memorySize[nodeId] += memory;

			cpuUsed[nodeId] -= cpu;
			memoryUsed[nodeId] -= memory;
		}

		bool canInstall(const VmInfo &vmInfo) const {
			int vmCpu = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
			int vmMem = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
			return cpuCores[0] >= vmCpu && memorySize[0] >= vmMem;
		}

		bool isEmpty(int nodeId) const {
			return cpuUsed[nodeId] == 0 && memoryUsed[nodeId] == 0;
		}

		bool isNearlyFull(int nodeId, double ratio) const {
			return cpuUsed[nodeId] >= cpuTotal / 2 * ratio
				&& memoryUsed[nodeId] >= memoryTotal / 2 * ratio;
		}

		std::pair<double, double> calUsedRatio(int nodeId = 0) const {
			double cpuRatio = static_cast<double>(cpuUsed[nodeId]) / (cpuTotal / 2);
			double memRatio = static_cast<double>(memoryUsed[nodeId]) / (memoryTotal / 2);
			return {cpuRatio, memRatio};
		}
	};

	struct VmInfo {

		friend std::istream &operator>>(std::istream &is, VmInfo &vmInfo) {
			std::string s;
			std::getline(is, s);
			std::vector<std::string> tokens(Solution::split(s));
			vmInfo.vmType = tokens[0];
			vmInfo.cpuCores = std::stoi(tokens[1]);
			vmInfo.memorySize = std::stoi(tokens[2]);
			vmInfo.isDouble = std::stoi(tokens[3]);
			return is;
		}

		std::string to_string() const {
			return "(" + vmType + ", " + std::to_string(cpuCores) + ", " 
				+ std::to_string(memorySize) + ", " + std::to_string(isDouble) + ")";
		}

		friend std::ostream &operator<<(std::ostream &os, const VmInfo &vmInfo) {
			os << vmInfo.to_string();
			return os;
		}

		std::string vmType;
		int cpuCores;
		int memorySize;
		int isDouble;
	};

	struct Command {

		friend std::istream &operator>>(std::istream &is, Command &command) {
			std::string s;
			std::getline(is, s);
			std::vector<std::string> tokens(Solution::split(s));
			command.commandType = tokens[0] == "add";
			command.vmType = tokens.size() == 3 ? tokens[1] : "";
			command.vmId = std::stoi(tokens.back());
			return is;
		}

		std::string to_string() const { 
			return "(" + (commandType ? std::string("add") : std::string("del")) + ", "
				+ (commandType ? vmType + ", " : "") + std::to_string(vmId) + ")";
		}

		friend std::ostream &operator<<(std::ostream &os, const Command &command) {
			os << command.to_string();
			return os;
		}

		int commandType;
		std::string vmType;
		int vmId;
		int vmIndex;
	};

private:

	static int dayNum;
	static int previewNum;

	static constexpr int CPUN = 1024 / 2;
	static constexpr int MEMN = 1024 / 2;
	static constexpr double oo = 1e200;

	static std::vector<ServerInfo> serverInfos;
	static std::unordered_map<std::string, int> vmTypeToIndex;
	static std::vector<VmInfo> vmInfos;
	static std::vector<std::vector<Command>> commands;

	std::string outputBuffer;

	std::vector<ServerInfo> serversUsed;
	std::unordered_map<int, int> vmIdToIndex;
	std::unordered_map<int, std::pair<int, int>> installId;
	std::vector<std::list<int>> serverIndexToVmId[2][2];

	std::vector<std::vector<std::list<int>>> serversIdRes[2][2];
	std::vector<std::vector<std::list<int>>> serversIdUse[2][2];
	std::vector<int> banned;
	std::vector<int> vmList;

	std::map<int, int> buyCnt;
	std::vector<std::pair<int, int>> ansId;
	std::vector<std::vector<int>> ansMigrate;

	int curDay;

	int serversUsedNum = 0;
	int vmResNum[2] = {};

	int vmCpuSum[2] = {};
	int vmMemSum[2] = {};
	int serverCpuSum[2] = {};
	int serverMemSum[2] = {};

	int maxCpu = 0;
	int maxMem = 0;

	int lastMigrateIndex = -1;

	int totalMigration = 0;
	long long totalCost = 0;

	inline int getNodeId(int nodeId) {
		return nodeId != -1 ? nodeId : 0;
	}
	inline double calF(const ServerInfo &server, const VmInfo &vmInfo) {
		if (vmResNum[vmInfo.isDouble] == 0) {
			return 0;
		}

		double vmRatio = static_cast<double>(vmCpuSum[vmInfo.isDouble]) / vmMemSum[vmInfo.isDouble];
		double serverRatio = static_cast<double>(serverCpuSum[vmInfo.isDouble]) / serverMemSum[vmInfo.isDouble];
		double aimRatio = 2 * vmRatio - serverRatio;
		if (vmResNum[vmInfo.isDouble] < 150) {
			aimRatio = 1.0;
		}
		else if (vmResNum[vmInfo.isDouble] < 1000) {
			aimRatio = std::max(aimRatio, 0.75);
			aimRatio = std::min(aimRatio, 1.25);
		}

		double resCpu[2] = {
			static_cast<double>(server.cpuCores[0]), 
			static_cast<double>(server.cpuCores[1])
		};
		double resMemory[2] = {
			static_cast<double>(server.memorySize[0]), 
			static_cast<double>(server.memorySize[1])
		};

		int dayNumUsed = dayNum - curDay + 1;

		double costPerCpuAll = (server.serverCost + dayNumUsed * server.powerCost) 
			/ std::min(server.cpuCores[0] * 1.0, server.memorySize[0] * aimRatio);

		if (vmInfo.isDouble) {
			resCpu[0] -= vmInfo.cpuCores / 2;
			resCpu[1] -= vmInfo.cpuCores / 2;
			resMemory[0] -= vmInfo.memorySize / 2;
			resMemory[1] -= vmInfo.memorySize / 2;
		}
		else {
			resCpu[0] -= vmInfo.cpuCores;
			resMemory[0] -= vmInfo.memorySize;
		}

		double equMemory[2] = {
			resMemory[0] * aimRatio, 
			resMemory[1] * aimRatio
		};
		double equCpu[2] = {
			std::min(resCpu[0], equMemory[0]), 
			std::min(resCpu[1], equMemory[1])
		};

		double costPerCpuRes = (server.serverCost + dayNumUsed * server.powerCost) 
			/ (equCpu[0] + equCpu[1]);

		double weightAll = 0.95;

		return costPerCpuAll * weightAll + costPerCpuRes * (1.0 - weightAll);
	}

	int selectServerPurchase(const VmInfo &vmInfo) {
		int cpuCores = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		int memorySize = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		double fmn = oo;
		int ret = -1;
		for (size_t i = 0; i < serverInfos.size(); ++i) {
			const auto &server = serverInfos[i];
			if (server.cpuCores[0] >= cpuCores && server.memorySize[0] >= memorySize) {
			// if (server.canInstall(vmInfo)) {
				double fval = calF(server, vmInfo);
				if (fval + EPS < fmn) {
					fmn = fval;
					ret = i;
				}
			}
		}
		return ret;
	}

	std::pair<int, int> searchServer(const VmInfo &vmInfo, int cpuBase, int cpuLim, int memBase, int memLim) {
		int lim = vmInfo.isDouble ? 1 : 2;
		cpuLim = std::min(cpuLim, maxCpu);
		memLim = std::min(memLim, maxMem);
		int rev = lim == 2 && std::rand() % 2;
		for (int i = cpuBase; i <= cpuLim; ++i) {
			for (int j = memBase; j <= memLim; ++j) {
				for (int k = 0; k < lim; ++k) {
					int nodeId = rev ? k ^ 1 : k;
					for (const auto &idx : serversIdRes[vmInfo.isDouble][nodeId][i][j]) {
						if (banned[idx]) continue;
						return {idx, vmInfo.isDouble ? -1 : nodeId};
					}
				}
			}
		}
		return {-1, -1};
	}

	std::pair<int, int> selectServerInstall(const VmInfo &vmInfo) {
		int cpuCores = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		int memorySize = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		std::pair<int, int> ret{-1, -1};

		static const int fragSize = 5;
		ret = searchServer(vmInfo, cpuCores, cpuCores + fragSize, memorySize, memorySize + fragSize);
		if (ret.first != -1) return ret;

		// int step = (cpuCores + memorySize) / 2;
		int step = std::max(cpuCores, memorySize);
		int cpuStep = step;
		int memStep = step;

		int midCpu = cpuCores + 15;
		int midMem = memorySize + 15;
		
		for (int i = cpuCores; i <= midCpu; i += cpuStep) {
			for (int j = memorySize; j <= midMem; j += memStep) {
				ret = searchServer(vmInfo, i, i + cpuStep - 1, j, j + memStep - 1);
				if (ret.first != -1) return ret;
			}
		}
		for (int i = midCpu; i <= maxCpu; i += cpuStep) {
			for (int j = midMem; j <= maxMem; j += memStep) {
				ret = searchServer(vmInfo, i, i + cpuStep - 1, j, j + memStep - 1);
				if (ret.first != -1) return ret;
			}
		}
		for (int i = cpuCores; i <= midCpu; i += cpuStep) {
			for (int j = midMem; j <= maxMem; j += memStep) {
				ret = searchServer(vmInfo, i, i + cpuStep - 1, j, j + memStep - 1);
				if (ret.first != -1) return ret;
			}
		}
		for (int i = midCpu; i <= maxCpu; i += cpuStep) {
			for (int j = memorySize; j <= midMem; j += memStep) {
				ret = searchServer(vmInfo, i, i + cpuStep - 1, j, j + memStep - 1);
				if (ret.first != -1) return ret;
			}
		}
		return {-1, -1};
	}

	std::pair<int, int> newServer(const VmInfo &vmInfo) {
		int buyId = selectServerPurchase(vmInfo);
		++buyCnt[buyId];
		std::pair<int, int> insId{int(serversUsed.size()), vmInfo.isDouble ? -1 : 0};
		serversUsed.emplace_back(serverInfos[buyId]);
		serverIndexToVmId[0][0].emplace_back();
		serverIndexToVmId[0][1].emplace_back();
		serverIndexToVmId[1][0].emplace_back();
		auto &server = serversUsed.back();
		server.isDouble = vmInfo.isDouble;
		server.serverId = buyId;
		serversIdRes[server.isDouble][0][server.cpuCores[0]][server.memorySize[0]].push_front(insId.first);
		serversIdUse[server.isDouble][0][server.cpuUsed[0]][server.memoryUsed[0]].push_front(insId.first);
		if (!vmInfo.isDouble) {
			serversIdRes[server.isDouble][1][server.cpuCores[1]][server.memorySize[1]].push_front(insId.first);
			serversIdUse[server.isDouble][1][server.cpuUsed[1]][server.memoryUsed[1]].push_front(insId.first);
		}
		banned.emplace_back(0);
		serverCpuSum[server.isDouble] += server.cpuTotal;
		serverMemSum[server.isDouble] += server.memoryTotal;
		maxCpu = std::max(maxCpu, server.cpuTotal / 2);
		maxMem = std::max(maxMem, server.memoryTotal / 2);
		return insId;
	}

	void install(const std::pair<int, int> &insId, int vmId) {
		int nodeId = getNodeId(insId.second);
		auto &server = serversUsed[insId.first];
		const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		auto &serIdRes = serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
		serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId.first));
		auto &serIdUse = serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
		serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId.first));
		server.install(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
		serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(insId.first);
		serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(insId.first);
		serverIndexToVmId[server.isDouble][nodeId][insId.first].emplace_back(vmId);
		installId[vmId] = insId;
		vmCpuSum[vmInfo.isDouble] += vmInfo.cpuCores;
		vmMemSum[vmInfo.isDouble] += vmInfo.memorySize;
	}

	void uninstall(const std::pair<int, int> &insId, int vmId) {
		int nodeId = getNodeId(insId.second);
		auto &server = serversUsed[insId.first];
		const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		auto &serIdRes = serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
		serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId.first));
		auto &serIdUse = serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
		serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId.first));
		server.uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
		serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(insId.first);
		serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(insId.first);
		auto &serIds = serverIndexToVmId[server.isDouble][nodeId][insId.first];
		serIds.erase(std::find(serIds.begin(), serIds.end(), vmId));
		installId.erase(vmId);
		vmCpuSum[vmInfo.isDouble] -= vmInfo.cpuCores;
		vmMemSum[vmInfo.isDouble] -= vmInfo.memorySize;
	}

	bool doMigrate(const std::pair<int, int> &fromId, int vmId) {
		const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		banned[fromId.first] = 1;
		auto toId = selectServerInstall(vmInfo);
		if (toId.first == -1) {
			banned[fromId.first] = 0;
			return false;
		}
		uninstall(fromId, vmId);
		install(toId, vmId);
		ansMigrate.push_back({vmId, toId.first, toId.second});
		banned[fromId.first] = 0;
		return true;
	}

	void vmMigrate(int &migrateRes1, int &migrateRes2, double serverUsedRatio) {
		if (vmResNum[0] + vmResNum[1] == 0) {
			return;
		}

		int vmSize = vmList.size();
		int curIndex = (lastMigrateIndex + 1) % vmSize;
		while (migrateRes1 > 0 && curIndex != lastMigrateIndex) {
			int vmId = vmList[curIndex];
			if (installId.count(vmId)) {
				auto fromId = installId[vmId];
				if (serversUsed[fromId.first].isNearlyFull(getNodeId(fromId.second), serverUsedRatio)) {
					curIndex = (curIndex + 1) % vmList.size();
					continue;
				}
				migrateRes1 -= doMigrate(fromId, vmId);
			}
			curIndex = (curIndex + 1) % vmSize;
		}
		lastMigrateIndex = (curIndex - 1 + vmSize) % vmSize;

		for (int i = serversUsed.size() - 1; i >= 0 && migrateRes2 > 0; --i) {
			const auto &server = serversUsed[i];
			if (server.isDouble) continue;
			if (server.isEmpty(0) + server.isEmpty(1) == 1) {
				int nodeId = server.isEmpty(0) ? 1 : 0;
				const auto vmIdList = serverIndexToVmId[0][nodeId][i];
				if (migrateRes2 < int(vmIdList.size())) continue;
				for (const auto &vmId : vmIdList) {
					auto fromId = installId[vmId];
					migrateRes2 -= doMigrate(fromId, vmId);
				}
			}
		}

		double aimRatio[2] = {
			vmMemSum[0] > 0 ? static_cast<double>(vmCpuSum[0]) / vmMemSum[0] : 1.0, 
			vmMemSum[1] > 0 ? static_cast<double>(vmCpuSum[1]) / vmMemSum[1] : 1.0
		};
		aimRatio[0] = std::max(aimRatio[0], 1.0 / aimRatio[0]);
		aimRatio[1] = std::max(aimRatio[1], 1.0 / aimRatio[1]);
		double migrateFactor = 2.0;
		for (size_t i = 0; i < serversUsed.size() && migrateRes2 > 0; ++i) {
			const auto &server = serversUsed[i];
			if (server.isEmpty(0) && server.isEmpty(1)) continue;
			double ratio[2] = {
				server.isEmpty(0) ? 1.0 : static_cast<double>(server.cpuUsed[0]) / server.memoryUsed[0], 
				server.isEmpty(1) ? 1.0 : static_cast<double>(server.cpuUsed[1]) / server.memoryUsed[1]
			};
			ratio[0] = std::max(ratio[0], 1.0 / ratio[0]);
			ratio[1] = std::max(ratio[1], 1.0 / ratio[1]);
			double maxRatio = std::max(ratio[0], ratio[1]);
			if (maxRatio > migrateFactor * aimRatio[server.isDouble]) {
				int lim = server.isDouble ? 1 : 2;
				for (int nodeId = 0; nodeId < lim; ++nodeId) {
					const auto vmIdList = serverIndexToVmId[server.isDouble][nodeId][i];
					if (migrateRes2 < int(vmIdList.size())) continue;
					for (const auto &vmId : vmIdList) {
						auto fromId = installId[vmId];
						migrateRes2 -= doMigrate(fromId, vmId);
					}
				}
			}
		}
	}

	void switchServer(int idx) {
		auto &server = serversUsed[idx];
		int lim = server.isDouble ? 1 : 2;
		for (int nodeId = 0; nodeId < lim; ++nodeId) {
			auto &serIdRes = serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
			serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), idx));
			auto &serIdUse = serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
			serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), idx));
		}
		serverCpuSum[server.isDouble] -= server.cpuTotal;
		serverMemSum[server.isDouble] -= server.memoryTotal;
		server.isDouble ^= 1;
		lim = server.isDouble ? 1 : 2;
		for (int nodeId = 0; nodeId < lim; ++nodeId) {
			serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(idx);
			serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(idx);
		}
		serverCpuSum[server.isDouble] += server.cpuTotal;
		serverMemSum[server.isDouble] += server.memoryTotal;
	}

	int pickEmptyServer(int isDouble) {
		for (const auto &idx : serversIdUse[isDouble][0][0][0]) {
			if (serversUsed[idx].isEmpty(1)) {
				return idx;
			}
		}
		return -1;
	}

	void solveOneDay(const std::vector<Command> &commands) {

		buyCnt.clear();
		ansId.clear();
		ansMigrate.clear();

		double serverUsedRatio = 0.93;
		double migrateRatio = 0.5;
		int migrateLim = (vmResNum[0] + vmResNum[1]) * 3 / 100;
		// if (curDay == dayNum / 2) {
		// 	migrateLim = vmResNum[0] + vmResNum[1];
		// }

		while (true) {
			int migrateLim1 = migrateLim * migrateRatio;
			int migrateLim2 = migrateLim - migrateLim1;
			int migrateRes1 = migrateLim1;
			int migrateRes2 = migrateLim2;
			vmMigrate(migrateRes1, migrateRes2, serverUsedRatio);
			int migrateUse1 = migrateLim1 - migrateRes1;
			int migrateUse2 = migrateLim2 - migrateRes2;
			migrateLim -= migrateUse1 + migrateUse2;
			totalMigration += migrateUse1 + migrateUse2;
			if (migrateUse1 + migrateUse2 == 0 || migrateLim == 0) {
				break;
			}
		}

		// auto printUsedRatio = [&]() {
		// 	if (curDay % 80 == 0) {
		// 		std::cerr << "\n\n" << curDay << ": " << std::endl;
		// 		std::cerr << std::fixed << std::setprecision(3);
		// 		for (auto &server: serversUsed) {
		// 			if (server.isDouble) continue;
		// 			// std::cerr << server << std::endl;
		// 			auto r0 = server.calUsedRatio(0);
		// 			auto r1 = server.calUsedRatio(1);
		// 			std::cerr << r0.first << "/" << r0.second << " | " << r1.first << "/" << r1.second << std::endl;
		// 		}
		// 		std::cerr << std::string(80, '-') << std::endl;
		// 		for (auto &server : serversUsed) {
		// 			if (!server.isDouble) continue;
		// 			// std::cerr << server << std::endl;
		// 			auto r0 = server.calUsedRatio();
		// 			std::cerr << r0.first << "/" << r0.second << std::endl;
		// 		}
		// 	}
		// };
		// // printUsedRatio();

		for (const auto &command : commands) {

			if (command.commandType) { // add

				vmIdToIndex[command.vmId] = command.vmIndex;

				const VmInfo &vmInfo = vmInfos[command.vmIndex];
				auto insId = selectServerInstall(vmInfo);

				if (insId.first == -1) {
					int idx = pickEmptyServer(vmInfo.isDouble ^ 1);
					if (idx != -1 && serversUsed[idx].canInstall(vmInfo)) {
						switchServer(idx);
						insId = std::make_pair(idx, vmInfo.isDouble ? -1 : 0);
					}
					else {
						insId = newServer(vmInfo);
					}
				}
				install(insId, command.vmId);
				ansId.push_back(insId);

				vmList.emplace_back(command.vmId);
				
				++vmResNum[vmInfo.isDouble];
			}
			else { // del

				auto insId = installId[command.vmId];
				uninstall(insId, command.vmId);

				--vmResNum[insId.second == -1];
			}
		}

		#ifdef DEBUG
		for (const auto &it : buyCnt) {
			totalCost += it.second * 1ll * serverInfos[it.first].serverCost;
		}
		for (const auto &server : serversUsed) {
			if (!server.isEmpty(0) || !server.isEmpty(1)) {
				totalCost += server.powerCost;
			}
		}
		#endif

		outputBuffer += "(purchase, " + std::to_string(buyCnt.size()) + ")\n";
		int preCnt = 0;
		for (auto &it : buyCnt) {
			outputBuffer += "(" + serverInfos[it.first].serverType 
				+ ", " + std::to_string(it.second) + ")\n";
			it.second += preCnt;
			preCnt = it.second;
		}

		// 服务器重编号
		int serversPreNum = serversUsedNum;
		for (int i = serversUsed.size() - 1; i >= serversUsedNum; --i) {
			int serverIndex = serversUsed[i].serverId;
			serversUsed[i].serverId = serversPreNum + (--buyCnt[serverIndex]);
		}
		serversUsedNum = serversUsed.size();

		outputBuffer += "(migration, " + std::to_string(ansMigrate.size()) + ")\n";
		for (const auto &vc : ansMigrate) {
			if (vc.back() == -1) {
				outputBuffer += "(" + std::to_string(vc[0]) + ", " 
					+ std::to_string(serversUsed[vc[1]].serverId) + ")\n";
			}
			else {
				outputBuffer += "(" + std::to_string(vc[0]) + ", " 
					+ std::to_string(serversUsed[vc[1]].serverId) + ", " 
					+ (vc[2] ? "B" : "A") + ")\n";
			}
		}

		for (const auto &id : ansId) {
			if (id.second != -1) {
				outputBuffer += "(" + std::to_string(serversUsed[id.first].serverId) + ", " 
					+ (id.second ? "B" : "A") + ")\n";
			}
			else {
				outputBuffer += "(" + std::to_string(serversUsed[id.first].serverId) + ")\n";
			}
		}
		
		write();

		// auto printVSRatio = [&]() {
		// 	std::cerr << std::fixed << std::setprecision(3);
		// 	double vmRatio[2] = {
		// 		vmCpuSum[0] / (EPS + vmMemSum[0]), 
		// 		vmCpuSum[1] / (EPS + vmMemSum[1])
		// 	};
		// 	double serverRatio[2] = {
		// 		serverCpuSum[0] / (EPS + serverMemSum[0]), 
		// 		serverCpuSum[1] / (EPS + serverMemSum[1])
		// 	};
		// 	// std::cerr << vmRatio[0] << " " << serverRatio[0] << std::endl;
		// 	// std::cerr << vmRatio[1] << " " << serverRatio[1] << std::endl;
		// 	// std::cerr << vmCpuSum[0] << " " << serverCpuSum[0] << " " << 1.0 * vmCpuSum[0] / serverCpuSum[0] << " | ";
		// 	// std::cerr << vmMemSum[0] << " " << serverMemSum[0] << " " << 1.0 * vmMemSum[0] / serverMemSum[0] << std::endl;
		// 	std::cerr << vmCpuSum[1] << " " << serverCpuSum[1] << " " << 1.0 * vmCpuSum[1] / serverCpuSum[1] << " | ";
		// 	std::cerr << vmMemSum[1] << " " << serverMemSum[1] << " " << 1.0 * vmMemSum[1] / serverMemSum[1] << std::endl;
		// };
		// // printVSRatio();

		// auto checkServersId = [&]() {
		// 	for (size_t i = 0; i < serversUsed.size(); ++i) {
		// 		const auto &server = serversUsed[i];
		// 		int lim = server.isDouble ? 1 : 2;
		// 		for (int nodeId = 0; nodeId < lim; ++nodeId) {
		// 			auto &serIdRes = serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
		// 			assert(std::find(serIdRes.begin(), serIdRes.end(), i) != serIdRes.end());
		// 			auto &serIdUse = serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
		// 			assert(std::find(serIdUse.begin(), serIdUse.end(), i) != serIdUse.end());
		// 		}
		// 	}
		// };
		// // checkServersId();

	}

public:

	void write() {
		std::cout << outputBuffer;
		std::cout.flush();
		outputBuffer.clear();
	}

	static void read() {

		serverInfos.clear();
		vmTypeToIndex.clear();
		vmInfos.clear();
		commands.clear();

		int serverNum;
		std::cin >> serverNum;
		std::cin.ignore();
		serverInfos.reserve(serverNum);
		for (int i = 0; i < serverNum; ++i) {
			ServerInfo serverInfo;
			std::cin >> serverInfo;
			serverInfo.cpuTotal = serverInfo.cpuCores[0] + serverInfo.cpuCores[1];
			serverInfo.memoryTotal = serverInfo.memorySize[0] + serverInfo.memorySize[1];
			serverInfos.emplace_back(serverInfo);
		}

		int vmNum;
		std::cin >> vmNum;
		std::cin.ignore();
		vmInfos.reserve(vmNum);
		for (int i = 0; i < vmNum; ++i) {
			VmInfo vmInfo;
			std::cin >> vmInfo;
			vmTypeToIndex[vmInfo.vmType] = i;
			vmInfos.emplace_back(vmInfo);
		}
	}

	void readOneDay(int day) {
		int commandNum;
		std::cin >> commandNum;
		std::cin.ignore();
		commands[day].reserve(commandNum);
		for (int i = 0; i < commandNum; ++i) {
			Command command;
			std::cin >> command;
			commands[day].emplace_back(command);
			if (commands[day].back().commandType) {
				commands[day].back().vmIndex = vmTypeToIndex[commands[day].back().vmType];
			}
		}
	}

	void solve() {

		std::cin >> dayNum;
		std::cin >> previewNum;
		std::cin.ignore();
		commands.resize(dayNum);
		for (int i = 0; i < previewNum; ++i) {
			readOneDay(i);
		}

		std::srand(20210331);

		for (size_t i = 0; i < 2; ++i) {
			for (size_t j = 0; j < 2; ++j) {
				if (i == 1 && j == 1) break;
				serversIdRes[i][j].resize(CPUN + 1);
				serversIdUse[i][j].resize(CPUN + 1);
				for (auto &serId : serversIdRes[i][j]) {
					serId.resize(MEMN + 1);
				}
				for (auto &serId : serversIdUse[i][j]) {
					serId.resize(MEMN + 1);
				}
			}
		}

		std::sort(serverInfos.begin(), serverInfos.end(), 
			[](const ServerInfo &a, const ServerInfo &b) {
				return a.powerCost < b.powerCost;
			});

		for (int i = 0; i < dayNum; ++i) {
			curDay = i;
			solveOneDay(commands[i]);
			if (previewNum + i < dayNum) {
				readOneDay(previewNum + i);
			}
		}
	}
};

int Solution::dayNum;
int Solution::previewNum;
std::vector<Solution::ServerInfo> Solution::serverInfos;
std::unordered_map<std::string, int> Solution::vmTypeToIndex;
std::vector<Solution::VmInfo> Solution::vmInfos;
std::vector<std::vector<Solution::Command>> Solution::commands;

int main() {

	Solution::read();

	Solution solution;
	solution.solve();

	return 0;
}
```

