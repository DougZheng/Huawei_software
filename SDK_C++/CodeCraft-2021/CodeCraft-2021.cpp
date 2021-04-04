#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string>
#include <tuple>
#include <utility>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <list>
#include <queue>
#include <ctime>
#include <random>
#include <functional>
#include <climits>
#include <cassert>
#include <thread>
#include <future>
#include <bitset>

// #define DEBUG
#define ON_LINE

#define EPS 1e-7

class Solution {

	static const int CPUN = 1024 / 2;
	static const int MEMN = 1024 / 2;

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

	using return_type = std::tuple<long long, int, std::string>;

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

		// double calUsedRatio(double levelCoef = 300.0, double acceptRange = 1.20) {
		// 	double serverK = cpuTotal / (0.05 + memoryTotal);
		// 	double vmK = (cpuUsed[0] + cpuUsed[1]) / (0.05 + memoryUsed[0] + memoryUsed[1]);
		// 	double ratio = std::max(serverK / vmK, vmK / serverK);
		// 	return -(levelCoef * std::floor(ratio / acceptRange));
		// }

		bool isEmpty(int nodeId = 0) const {
			return cpuUsed[nodeId] == 0 && memoryUsed[nodeId] == 0;
		}

		bool isNearlyFull(int nodeId = 0) const {
			double ratio = 0.95;
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

	static std::vector<ServerInfo> serverInfos;
	static std::unordered_map<std::string, int> vmTypeToIndex;
	static std::vector<VmInfo> vmInfos;
	static std::vector<std::vector<Command>> commands;

	std::string outputBuffer;

	std::vector<ServerInfo> serversUsed[2];

	std::unordered_map<int, int> vmIdToIndex;
	std::unordered_map<int, std::vector<int>> installId;
	std::vector<std::list<int>> serverIndexToVmId[2][2];

	std::vector<std::vector<std::list<int>>> serversIdRes[2][2];
	std::vector<std::vector<std::list<int>>> serversIdUse[2][2];
	std::vector<int> banned[2];
	std::vector<int> vmList;

	int lastMigrateIndex = -1;

	static int dayNum;
	static int previewNum;
	int curDay;
	int serversUsedNum[2] = {};
	int vmResNum = 0;
	int totalMigration = 0;
	int canMigrateTotal = 0;
	long long totalCost = 0;

	int vmCpuSum = 0;
	int vmMemSum = 0;
	int serverCpuSum = 0;
	int serverMemSum = 0;
	int maxCpu = 0;
	int maxMem = 0;

	int tim = 0;
	const int shuffleFreq = 50;

	const double oo = 1e200;
	double levelCoef = 300.0;
	double acceptRange = 1.5;

	inline double calRatioDiff(int vc, int vm, int sc, int sm) {
		if (vm == 0 || sm == 0) return 0;
		return fabs(static_cast<double>(vc) / vm - static_cast<double>(sc) / sm);
	}

	// inline double calF(const ServerInfo &server, const VmInfo &vmInfo) {
	// 	double serverK = static_cast<double>(server.cpuCores[0]) / server.memorySize[0];
	// 	double vmK = static_cast<double>(vmInfo.cpuCores) / vmInfo.memorySize;
	// 	double ratio = std::max(serverK / vmK, vmK / serverK);
	// 	return levelCoef * std::floor(ratio / acceptRange)
	// 		+ serverK * (server.cpuCores[0] - vmInfo.cpuCores)
	// 		+ 1.0 / serverK * (server.memorySize[0] - vmInfo.memorySize);
	// }

	inline double calF(const ServerInfo &server, const VmInfo &vmInfo) {
		if (vmMemSum == 0) {
			return 0;
		}

		double vmRatio = static_cast<double>(vmCpuSum) / vmMemSum;
		double serverRatio = static_cast<double>(serverCpuSum) / serverMemSum;
		double aimRatio = 2 * vmRatio - serverRatio;

		double resCpu[2] = {server.cpuCores[0], server.cpuCores[1]};
		double resMemory[2] = {server.memorySize[0], server.memorySize[1]};

		double costPerCpuAll = (server.serverCost + (dayNum - curDay + 1) * server.powerCost) 
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

		double equMemory[2] = {resMemory[0] * aimRatio, resMemory[1] * aimRatio};
		double equCpu[2] = {std::min(resCpu[0], equMemory[0]), std::min(resCpu[1], equMemory[1])};

		double costPerCpuRes = (server.serverCost + (dayNum - curDay + 1) * server.powerCost) 
			/ (equCpu[0] + equCpu[1]);

		double weightAll = 0.80;

		// if (curDay == 116) std::cerr << server << " " << costPerCpuAll << " " << costPerCpuRes << std::endl;

		return costPerCpuAll * weightAll + costPerCpuRes * (1.0 - weightAll);
	}

	int selectServerPurchase(const VmInfo &vmInfo) {
		int cpuCores = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		int memorySize = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		double fmn = oo;
		int ret = -1;
		// if (curDay == 116) std::cerr << vmInfo << std::endl;
		for (size_t i = 0; i < serverInfos.size(); ++i) {
			const auto &server = serverInfos[i];
			if (server.cpuCores[0] >= cpuCores && server.memorySize[0] >= memorySize) {
				double fval = calF(server, vmInfo);
				if (fval + EPS < fmn) {
					fmn = fval;
					ret = i;
				}
			}
		}
		if (curDay == 116) {
			// std::cerr << "select: " << serverInfos[ret] << " " << fmn << std::endl;
			// calF(serverInfos[ret], vmInfo);
			// std::cerr << std::endl;
		}
		return ret;
	}

	std::pair<int, int> selectServerInstall(const VmInfo &vmInfo) {
		int cpuCores = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		int memorySize = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		int lim = vmInfo.isDouble ? 1 : 2;
		std::pair<int, int> ret{-1, -1};
		auto searchServer = [&](int cpuBase, int cpuLim, int memBase, int memLim) -> std::pair<int, int> {
			cpuLim = std::min(cpuLim, maxCpu);
			memLim = std::min(memLim, maxMem);
			int rev = lim == 2 && std::rand() % 2;
			for (int i = cpuBase; i <= cpuLim; ++i) {
				for (int j = memBase; j <= memLim; ++j) {
					for (int k = 0; k < lim; ++k) {
						int nodeId = rev ? k ^ 1 : k;
						for (const auto &idx : serversIdRes[vmInfo.isDouble][nodeId][i][j]) {
							if (banned[vmInfo.isDouble][idx]) continue;
							return {idx, vmInfo.isDouble ? -1 : nodeId};
						}
					}
				}
			}
			return {-1, -1};
		};
		static const int fragSize = 5;
		ret = searchServer(cpuCores, cpuCores + fragSize, memorySize, memorySize + fragSize);
		if (ret.first != -1) return ret;

		int step = (cpuCores + memorySize) / 2;

		int dLim = std::max((maxCpu + 1 - cpuCores + step - 1) / step, 
			(maxMem + 1 - memorySize + step - 1) / step);
		for (int d = 0; d <= dLim; ++d) {
			for (int f = 0; f < 2; ++f) {
				int cpuBase = f == 0 ? cpuCores + d * step : cpuCores;
				int memoryBase = f == 0 ? memorySize : memorySize + d * step;
				for (int i = cpuBase, j = memoryBase; i <= maxCpu && j <= maxMem; i += step, j += step) {
					ret = searchServer(i, i + step - 1, j, j + step - 1);
					if (ret.first != -1) return ret;
				}
			}
		}
		return {-1, -1};
	}

	inline int getNodeId(int nodeId) {
		return nodeId != -1 ? nodeId : 0;
	}

	void solveOneDay(const std::vector<Command> &commands) {

		std::map<int, int> buyCnt;
		std::vector<std::vector<int>> ansId;
		std::vector<std::vector<int>> ansMigrate;

		auto newServer = [&](const VmInfo &vmInfo) -> std::vector<int> {
			int buyId = selectServerPurchase(vmInfo);
			++buyCnt[buyId];
			std::vector<int> insId{vmInfo.isDouble, 
				int(serversUsed[vmInfo.isDouble].size()), vmInfo.isDouble ? -1 : 0};
			serversUsed[insId[0]].emplace_back(serverInfos[buyId]);
			serverIndexToVmId[insId[0]][0].emplace_back();
			auto &server = serversUsed[insId[0]].back();
			server.serverId = buyId;
			serversIdRes[insId[0]][0][server.cpuCores[0]][server.memorySize[0]].push_front(insId[1]);
			serversIdUse[insId[0]][0][server.cpuUsed[0]][server.memoryUsed[0]].push_front(insId[1]);
			if (!vmInfo.isDouble) {
				serverIndexToVmId[insId[0]][1].emplace_back();
				serversIdRes[insId[0]][1][server.cpuCores[1]][server.memorySize[1]].push_front(insId[1]);
				serversIdUse[insId[0]][1][server.cpuUsed[1]][server.memoryUsed[1]].push_front(insId[1]);
			}
			banned[vmInfo.isDouble].emplace_back(0);
			serverCpuSum += server.cpuTotal;
			serverMemSum += server.memoryTotal;
			maxCpu = std::max(maxCpu, server.cpuTotal / 2);
			maxMem = std::max(maxMem, server.memoryTotal / 2);
			return insId;
		};

		auto install = [&](const std::vector<int> &insId, int vmId) {
			int nodeId = getNodeId(insId[2]);
			auto &server = serversUsed[insId[0]][insId[1]];
			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
			auto &serIdRes = serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
			serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId[1]));
			auto &serIdUse = serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
			serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId[1]));
			server.install(insId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(insId[1]);
			serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(insId[1]);
			serverIndexToVmId[insId[0]][nodeId][insId[1]].emplace_back(vmId);
			installId[vmId] = insId;
			vmCpuSum += vmInfo.cpuCores;
			vmMemSum += vmInfo.memorySize;
		};

		auto uninstall = [&](const std::vector<int> &insId, int vmId) {
			int nodeId = getNodeId(insId[2]);
			auto &server = serversUsed[insId[0]][insId[1]];
			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
			auto &serIdRes = serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
			serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId[1]));
			auto &serIdUse = serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
			serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId[1]));
			server.uninstall(insId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(insId[1]);
			serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(insId[1]);
			auto &serIds = serverIndexToVmId[insId[0]][nodeId][insId[1]];
			serIds.erase(std::find(serIds.begin(), serIds.end(), vmId));
			installId.erase(vmId);
			vmCpuSum -= vmInfo.cpuCores;
			vmMemSum -= vmInfo.memorySize;
		};

		// int migrateLim = vmResNum * 3 / 100;

		// decltype(serversIdRes) serversIdResBak;
		// decltype(serversIdUse) serversIdUseBak;
		// for (int i = 0; i < 2; ++i) {
		// 	for (int j = 0; j < 2; ++j) {
		// 		serversIdResBak[i][j] = serversIdRes[i][j];
		// 		serversIdUseBak[i][j] = serversIdUse[i][j];
		// 	}
		// }

		// double vmRatio = static_cast<double>(vmCpuSum) / vmMemSum + EPS;
		// int memFragSize = 5;
		// int cpuFragSize = vmRatio * memFragSize;

		// struct Node {
		// 	int cpu;
		// 	int mem;
		// 	double prior;
		// 	bool operator < (const Node &o) const {
		// 		return prior > o.prior;
		// 	}
		// };
		// std::vector<Node> cpuMemPairs;

		// auto calMigratePrior = [&](int cpu, int mem) -> double {
		// 	double serverRatio = static_cast<double>(cpu) / mem + EPS;
		// 	double prior = std::max(serverRatio / vmRatio, vmRatio / serverRatio);
		// 	return prior;
		// };
		// for (int i = 0; i <= CPUN; ++i) {
		// 	for (int j = 0; j <= MEMN; ++j) {
		// 		if (!serversIdResBak[1][0][i][j].empty()) {
		// 			cpuMemPairs.push_back({i, j, calMigratePrior(i, j)});
		// 		} 
		// 	}
		// }
		// std::sort(cpuMemPairs.begin(), cpuMemPairs.end()); 

		// auto removeServer = [&](const std::vector<int> &insId) {
		// 	int nodeId = getNodeId(insId[2]);
		// 	const auto &server = serversUsed[insId[0]][insId[1]];
		// 	auto &serIdRes = serversIdResBak[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
		// 	serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId[1]));
		// 	auto &serIdUse = serversIdUseBak[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
		// 	serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId[1]));
		// };

		// auto searchSeverMigrate = [&](const std::vector<int> &fromId, 
		// 	int cpuBase, int cpuStep, int cpuUsedBase, int memUsedBase) -> std::vector<int> {
		// 	int cpuLim = std::min(cpuBase + cpuStep - 1, CPUN);
		// 	for (int i = cpuBase; i <= cpuLim; ++i) {
		// 		int memDown = (vmMemSum * 1ll * (i - cpuUsedBase - cpuFragSize) + vmCpuSum - 1) / vmCpuSum + memUsedBase;
		// 		int memUp = vmMemSum * 1ll * (i - cpuUsedBase) / vmCpuSum + memFragSize + memUsedBase;
		// 		memDown = std::max(memDown, memUsedBase);
		// 		memUp = std::min(memUp, MEMN);
		// 		for (int j = memDown; j <= memUp; ++j) {
		// 			int lim = fromId[0] ? 1 : 2;
		// 			for (int nodeId = 0; nodeId < lim; ++nodeId) {
		// 				auto &serIds = serversIdResBak[fromId[0]][nodeId][i][j];
		// 				auto it = serIds.begin();
		// 				while (it != serIds.end()) {
		// 					std::vector<int> toId{fromId[0], *it, fromId[0] ? -1 : nodeId};
		// 					if (toId == fromId || serversUsed[toId[0]][toId[1]].isEmpty(nodeId)) {
		// 						++it;
		// 						continue;
		// 					}
		// 					removeServer(toId);
		// 					return toId;
		// 				}
		// 			}
		// 		}
		// 	}
		// 	return {-1, -1, -1};
		// };

		// auto selectServerMigrate = [&](const std::vector<int> &fromId, int cpuBase, int memBase) -> std::vector<int> {
		// 	auto ret = searchSeverMigrate(fromId, cpuBase, cpuFragSize, cpuBase, memBase);
		// 	if (ret[0] != -1) return ret;
		// 	int cpuUsedBase = cpuBase;
		// 	int memUsedBase = memBase;
		// 	cpuBase += cpuFragSize;
		// 	memBase += memFragSize;
		// 	int cpuStep = cpuFragSize;
		// 	int memStep = memFragSize;
		// 	for (int i = cpuBase, j = memBase; i <= CPUN && j <= MEMN; i += cpuStep, j += memStep) {
		// 		auto ret = searchSeverMigrate(fromId, i, cpuStep, cpuUsedBase, memUsedBase);
		// 		if (ret[0] != -1) return ret;
		// 	}
		// 	return {-1, -1, -1};
		// };

		// auto doMigrate = [&](const std::vector<int> &fromId, const std::vector<int> &toId) {
		// 	int nodeId = getNodeId(fromId[2]);
		// 	for (const auto &vmId : serverIndexToVmId[fromId[0]][nodeId][fromId[1]]) {
		// 		const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		// 		uninstall(serversUsed[fromId[0]][fromId[1]], fromId, vmInfo);
		// 		install(serversUsed[toId[0]][toId[1]], toId, vmInfo);
		// 		ansMigrate.push_back({vmId, toId[0], toId[1], toId[2]});
		// 		serverIndexToVmId[toId[0]][getNodeId(toId[2])][toId[1]].emplace_back(vmId);
		// 		installId[vmId] = toId;
		// 	}
		// 	serverIndexToVmId[fromId[0]][nodeId][fromId[1]].clear();
		// };

		// for (const auto &e : cpuMemPairs) {
		// 	for (int isDouble = 0; isDouble < 2; ++isDouble) {
		// 		int lim = isDouble ? 1 : 2;
		// 		for (int nodeId = 0; nodeId < lim; ++nodeId) {
		// 			auto &serIds = serversIdResBak[isDouble][nodeId][e.cpu][e.mem];
		// 			auto it = serIds.begin();
		// 			while (it != serIds.end()) {
		// 				std::vector<int> fromId{isDouble, *it, isDouble ? -1 : nodeId};
		// 				int migrateNum = serverIndexToVmId[fromId[0]][nodeId][fromId[1]].size();
		// 				const auto &server = serversUsed[fromId[0]][fromId[1]];
		// 				if (migrateNum > migrateLim || server.isEmpty(nodeId) 
		// 					|| server.cpuCores[nodeId] <= cpuFragSize && server.memorySize[nodeId] <= memFragSize) {
		// 					++it;
		// 					continue;
		// 				}
		// 				auto toId = selectServerMigrate(fromId, server.cpuUsed[nodeId], server.memoryUsed[nodeId]);
		// 				if (toId[0] == -1) break;
		// 				migrateLim -= migrateNum;
		// 				auto tmpIt = it++;
		// 				removeServer(fromId);
						
		// 				// std::cerr << (fromId[0] ? "2" : fromId[2] ? "B" : "A") << ": " << serversUsed[fromId[0]][fromId[1]] 
		// 				// 	<< " -> " << (toId[0] ? "2" : toId[2] ? "B" : "A") << ": " << serversUsed[toId[0]][toId[1]] << "\n";
		// 				// std::cerr << std::endl;
		// 				doMigrate(fromId, toId);
		// 			}
		// 		}
		// 	}
		// }
		
		for (size_t i = 0; i < serversUsed[0].size(); ++i) {
			const auto &server = serversUsed[0][i];
			for (int j = 0; j < 2; ++j) {
				auto &serIdRes = serversIdRes[0][j][server.cpuCores[j]][server.memorySize[j]];
				assert(std::find(serIdRes.begin(), serIdRes.end(), i) != serIdRes.end());
				auto &serIdUse = serversIdUse[0][j][server.cpuUsed[j]][server.memoryUsed[j]];
				assert(std::find(serIdUse.begin(), serIdUse.end(), i) != serIdUse.end());
			}
		}
		for (size_t i = 0; i < serversUsed[1].size(); ++i) {
			const auto &server = serversUsed[1][i];
			auto &serIdRes = serversIdRes[1][0][server.cpuCores[0]][server.memorySize[0]];
			assert(std::find(serIdRes.begin(), serIdRes.end(), i) != serIdRes.end());
			auto &serIdUse = serversIdUse[1][0][server.cpuUsed[0]][server.memoryUsed[0]];
			assert(std::find(serIdUse.begin(), serIdUse.end(), i) != serIdUse.end());
		}

		auto vmMigrate = [&]() {
			if (vmResNum == 0) {
				return;
			}
			double migrateRatio = 0.5;
			int migrateLim = vmResNum * 3 / 100;
			int migrateLim1 = migrateLim * migrateRatio;
			int migrateLim2 = migrateLim - migrateLim1;

			int curIndex = (lastMigrateIndex + 1) % vmList.size();
			while (migrateLim1 > 0 && curIndex != lastMigrateIndex) {
				int vmId = vmList[curIndex];
				if (installId.count(vmId)) {
					auto fromId = installId[vmId];
					if (serversUsed[fromId[0]][fromId[1]].isNearlyFull(getNodeId(fromId[2]))) {
						curIndex = (curIndex + 1) % vmList.size();
						continue;
					}
					const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
					banned[fromId[0]][fromId[1]] = 1;
					auto selId = selectServerInstall(vmInfo);
					if (selId.first != -1) {
						std::vector<int> toId{selId.second == -1, selId.first, selId.second};
						// std::cerr << (fromId[0] ? "2" : fromId[2] ? "B" : "A") << ": " << serversUsed[fromId[0]][fromId[1]] 
						// 	<< " -> " << (toId[0] ? "2" : toId[2] ? "B" : "A") << ": " << serversUsed[toId[0]][toId[1]] << "\n";
						// std::cerr << std::endl;
						uninstall(fromId, vmId);
						install(toId, vmId);
						ansMigrate.push_back({vmId, toId[0], toId[1], toId[2]});
						--migrateLim1;
					}
					banned[fromId[0]][fromId[1]] = 0;
				}
				curIndex = (curIndex + 1) % vmList.size();
			}
			lastMigrateIndex = (curIndex - 1 + vmList.size()) % vmList.size();

			for (int i = serversUsed[0].size() - 1; i >= 0 && migrateLim2 > 0; --i) {
			// for (size_t i = 0; i < serversUsed[0].size() && migrateLim2 > 0; ++i) {
				const auto &server = serversUsed[0][i];
				if (server.isEmpty(0) + server.isEmpty(1) == 1) {
					int nodeId = server.isEmpty(0) ? 1 : 0;
					const auto vmIdList = serverIndexToVmId[0][nodeId][i];
					if (migrateLim2 < vmIdList.size()) continue;
					for (const auto &vmId : vmIdList) {
						const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
						auto fromId = installId[vmId];
						banned[fromId[0]][fromId[1]] = 1;
						auto selId = selectServerInstall(vmInfo);
						if (selId.first != -1) {
							std::vector<int> toId{selId.second == -1, selId.first, selId.second};
							// std::cerr << (fromId[0] ? "2" : fromId[2] ? "B" : "A") << ": " << serversUsed[fromId[0]][fromId[1]] 
							// 	<< " -> " << (toId[0] ? "2" : toId[2] ? "B" : "A") << ": " << serversUsed[toId[0]][toId[1]] << "\n";
							// std::cerr << std::endl;
							uninstall(fromId, vmId);
							install(toId, vmId);
							ansMigrate.push_back({vmId, toId[0], toId[1], toId[2]});
							--migrateLim2;
						}
						banned[fromId[0]][fromId[1]] = 0;
					}
				}
			}
		};
		vmMigrate();

		int newBuyCnt = 0;

		for (const auto &command : commands) {

			if (command.commandType) { // add

				++vmResNum;

				vmIdToIndex[command.vmId] = command.vmIndex;

				const VmInfo &vmInfo = vmInfos[command.vmIndex];
				auto selId = selectServerInstall(vmInfo);

				std::vector<int> insId{selId.second == -1, selId.first, selId.second};

				if (selId.first == -1) {
					insId = newServer(vmInfo);
					++newBuyCnt;
				}
				install(insId, command.vmId);
				ansId.push_back(insId);

				vmList.emplace_back(command.vmId);
			}
			else { // del

				--vmResNum;

				auto insId = installId[command.vmId];
				uninstall(insId, command.vmId);
			}
		}

		// auto calEmptyNum = [&]() -> int {
		// 	int cnt = 0;
		// 	for (int i = 0; i < 2; ++i) {
		// 		for (auto &server : serversUsed[i]) {
		// 			cnt += server.isEmpty(0) && server.isEmpty(1);
		// 		}
		// 	}
		// 	return cnt;
		// };

		// std::cerr << newBuyCnt << " / " << calEmptyNum() << std::endl;

		auto calRatio = [&]() {
			int cpuAdd = 0, memAdd = 0;
			int cntAdd = 0, cntDel = 0;
			for (const auto &cmd : commands) {
				const auto &vmInfo = vmInfos[cmd.vmIndex];
				if (cmd.commandType) {
					++cntAdd;
					cpuAdd += vmInfo.cpuCores;
					memAdd += vmInfo.memorySize;
				}
				else {
					++cntDel;
				}
			}
			double ratioAdd = 1.0 * cpuAdd / (0.05 + memAdd);
			double ratio = 1.0 * vmCpuSum / (0.05 + vmMemSum);

			std::cerr << curDay << ": " << cntAdd << " " << cntDel << " " << ratioAdd << " " << ratio << std::endl; 
		};
		// calRatio();

		for (const auto &it : buyCnt) {
			totalCost += it.second * 1ll * serverInfos[it.first].serverCost;
		}

		for (size_t i = 0; i < 2; ++i) {
			for (size_t j = 0; j < serversUsed[i].size(); ++j) {
				int cnt = serverIndexToVmId[i][0][j].size();
				if (i == 0) {
					cnt += serverIndexToVmId[i][1][j].size();
				}
				if (cnt == 0) continue;
				totalCost += serversUsed[i][j].powerCost;
			}
		}

		outputBuffer += "(purchase, " + std::to_string(buyCnt.size()) + ")\n";
		int preCnt = 0;
		for (auto &it : buyCnt) {
			outputBuffer += "(" + serverInfos[it.first].serverType 
				+ ", " + std::to_string(it.second) + ")\n";
			it.second += preCnt;
			preCnt = it.second;
		}

		// 服务器重编号
		int serversPreNum = serversUsedNum[0] + serversUsedNum[1];
		for (int i = 0; i < 2; ++i) {
			for (int j = serversUsed[i].size() - 1; j >= serversUsedNum[i]; --j) {
				int serverIndex = serversUsed[i][j].serverId;
				serversUsed[i][j].serverId = serversPreNum + (--buyCnt[serverIndex]);
			}
			serversUsedNum[i] = serversUsed[i].size();
		}

		outputBuffer += "(migration, " + std::to_string(ansMigrate.size()) + ")\n";
		for (const auto &vc : ansMigrate) {
			if (vc.back() == -1) {
				outputBuffer += "(" + std::to_string(vc[0]) + ", " 
					+ std::to_string(serversUsed[vc[1]][vc[2]].serverId) + ")\n";
			}
			else {
				outputBuffer += "(" + std::to_string(vc[0]) + ", " 
					+ std::to_string(serversUsed[vc[1]][vc[2]].serverId) + ", " 
					+ (vc[3] ? "B" : "A") + ")\n";
			}
		}

		for (const auto &id : ansId) {
			if (id[2] != -1) {
				outputBuffer += "(" + std::to_string(serversUsed[id[0]][id[1]].serverId) + ", " 
					+ (id[2] ? "B" : "A") + ")\n";
			}
			else {
				outputBuffer += "(" + std::to_string(serversUsed[id[0]][id[1]].serverId) + ")\n";
			}
		}

		auto printUsedRatio = [&]() {
			if (curDay == 500) {
				std::cerr << std::fixed << std::setprecision(3);
				for (auto &server: serversUsed[0]) {
					// std::cerr << server << std::endl;
					auto r0 = server.calUsedRatio(0);
					auto r1 = server.calUsedRatio(1);
					std::cerr << r0.first << "/" << r0.second << " | " << r1.first << "/" << r1.second << std::endl;
				}
				std::cerr << std::string(80, '-') << std::endl;
				for (auto &server : serversUsed[1]) {
					// std::cerr << server << std::endl;
					auto r0 = server.calUsedRatio();
					std::cerr << r0.first << "/" << r0.second << std::endl;
				}
			}
		};
		// printUsedRatio();
		
		#ifdef ON_LINE
		write();
		#endif
	}

public:

	void write() {

		std::cout << outputBuffer;
		std::cout.flush();
		outputBuffer.clear();
	}

	static void read() {

		serverInfos.clear();
		vmInfos.clear();
		vmTypeToIndex.clear();
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

		#ifndef ON_LINE
		std::cin >> dayNum >> previewNum;
		std::cin.ignore();
		commands.resize(dayNum);
		for (int i = 0; i < dayNum; ++i) {
			int commandNum;
			std::cin >> commandNum;
			std::cin.ignore();
			commands[i].reserve(commandNum);
			for (int j = 0; j < commandNum; ++j) {
				Command command;
				std::cin >> command;
				commands[i].emplace_back(command);
				if (commands[i].back().commandType) {
					commands[i].back().vmIndex = vmTypeToIndex[commands[i].back().vmType];
				}
			}
		}
		#endif
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

	void solve(std::promise<return_type> &promiseAns) {

		std::srand(20210331);

		#ifdef ON_LINE
		std::cin >> dayNum;
		std::cin >> previewNum;
		std::cin.ignore();
		commands.resize(dayNum);
		for (int i = 0; i < previewNum; ++i) {
			readOneDay(i);
		}
		#endif

		outputBuffer.clear();

		vmList.clear();

		serversUsed[0].clear();
		serversUsed[1].clear();
		vmIdToIndex.clear();
		installId.clear();
		serverIndexToVmId[0][0].clear();
		serverIndexToVmId[0][1].clear();
		serverIndexToVmId[1][0].clear();
		serversUsedNum[0] = 0;
		serversUsedNum[1] = 0;
		vmResNum = 0;
		totalMigration = 0;
		canMigrateTotal = 0;
		totalCost = 0;
		tim = 0;

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
			#ifdef ON_LINE
			if (previewNum + i < dayNum) {
				readOneDay(previewNum + i);
			}
			#endif
		}
		promiseAns.set_value(std::make_tuple(totalCost, totalMigration, outputBuffer));
	}

	void setAcceptRange(double acceptRange) {
		this->acceptRange = acceptRange;
	}
};

std::vector<Solution::ServerInfo> Solution::serverInfos;
std::unordered_map<std::string, int> Solution::vmTypeToIndex;
std::vector<Solution::VmInfo> Solution::vmInfos;
std::vector<std::vector<Solution::Command>> Solution::commands;
int Solution::dayNum;
int Solution::previewNum;

std::pair<long long, int> solveMultithread(std::string in = "", std::string out = "") {

	auto calTim = [](clock_t timS, clock_t timT) -> double {
		return static_cast<double>(timT - timS) / CLOCKS_PER_SEC * 1000;
	};

	clock_t timS = std::clock();

	std::cerr << "\n" << in << std::endl;

	assert(std::freopen(in.c_str(), "r", stdin) != nullptr);
	Solution::read();
	std::fclose(stdin);

	// std::vector<double> acceptRanges{1.00, 1.05, 1.10, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.55, 1.60};
	std::vector<double> acceptRanges{1.25};

	int n = acceptRanges.size();
	std::vector<std::promise<Solution::return_type>> promises;
	std::vector<std::future<Solution::return_type>> futures;
	std::vector<std::thread> threads;
	std::vector<Solution> solutions;
	std::vector<Solution::return_type> ans;

	promises.resize(n);
	solutions.resize(n);
	for (int i = 0; i < n; ++i) {
		futures.emplace_back(promises[i].get_future());
		solutions[i].setAcceptRange(acceptRanges[i]);
		threads.emplace_back(std::thread(&Solution::solve, solutions[i], std::ref(promises[i])));
	}

	auto display = [&](int index) {
		std::cerr << "\nacceptRange: " << acceptRanges[index] << std::endl;
		std::cerr << "cost: " << std::get<0>(ans[index]) << std::endl;
		std::cerr << "migration: " << std::get<1>(ans[index]) << std::endl;
	};
	for (int i = 0; i < n; ++i) {
		ans.emplace_back(futures[i].get());
		threads[i].join();
		display(i);
	}

	clock_t timT = std::clock();

	std::cerr << "\ntime cost: " << calTim(timS, timT) << "ms" << std::endl;

	int mnId = std::min_element(ans.cbegin(), ans.cend()) - ans.cbegin();
	std::cerr << "\noptimal: ";
	display(mnId);

	assert(std::freopen(out.c_str(), "w", stdout) != nullptr);
	std::cout << std::get<2>(ans[mnId]) << std::endl;
	std::fclose(stdout);

	return std::make_pair(std::get<0>(ans[mnId]), std::get<1>(ans[mnId]));
}

int main() {

	#ifdef DEBUG
	auto ans1 = solveMultithread("../../data/training-1.txt", "../../data/training-1.out");
	auto ans2 = solveMultithread("../../data/training-2.txt", "../../data/training-2.out");
	std::cerr << "\n" << "summary:" << std::endl;
	std::cerr << "total cost: " << ans1.first + ans2.first << std::endl;
	std::cerr << "total migration: " << ans1.second + ans2.second << std::endl;
	#endif

	#ifndef DEBUG
	Solution::read();

	double acceptRange = 1.25;
	std::promise<Solution::return_type> promise;
	Solution::return_type ans;
	std::future<Solution::return_type> future(promise.get_future());
	Solution solution;
	solution.setAcceptRange(acceptRange);
	std::thread thread(&Solution::solve, solution, std::ref(promise));

	ans = future.get();
	thread.join();
	#endif

	return 0;
}
