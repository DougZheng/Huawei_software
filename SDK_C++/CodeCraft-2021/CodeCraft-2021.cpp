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

		// bool canInstall(const VmInfo &vmInfo) const {
		// 	int vmCpu = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		// 	int vmMem = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		// 	return cpuCores[0] >= vmCpu && memorySize[0] >= vmMem;
		// }

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
		if (vmResNum[vmInfo.isDouble] < 100) {
			aimRatio = 1.0;
		}
		else if (vmResNum[vmInfo.isDouble] < 500) {
			aimRatio = std::max(aimRatio, 0.75);
			aimRatio = std::min(aimRatio, 1.25);
		}
		// if (vmResNum[vmInfo.isDouble] < 10000) {
		// 	aimRatio += 0.08 - std::rand() % 17 * 0.01;
		// }

		double resCpu[2] = {
			static_cast<double>(server.cpuCores[0]), 
			static_cast<double>(server.cpuCores[1])
		};
		double resMemory[2] = {
			static_cast<double>(server.memorySize[0]), 
			static_cast<double>(server.memorySize[1])
		};

		int dayNumUsed = dayNum - curDay + 1;
		// if (curDay == 254 || curDay == 285) {
		// 	dayNumUsed /= 10;
		// }

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

		double weightAll = 0.80;

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
							if (banned[idx]) continue;
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
		// return searchServer(cpuCores, CPUN, memorySize, MEMN);

		// int step = (cpuCores + memorySize) / 2;
		int step = std::max(cpuCores, memorySize);
		int cpuStep = step;
		int memStep = step;

		int dLim = std::max((maxCpu + 1 - cpuCores + cpuStep - 1) / cpuStep, 
			(maxMem + 1 - memorySize + memStep - 1) / memStep);
		for (int d = 0; d <= dLim; ++d) {
			for (int f = 0; f < 2; ++f) {
				int cpuBase = f == 0 ? cpuCores + d * cpuStep : cpuCores;
				int memoryBase = f == 0 ? memorySize : memorySize + d * memStep;
				for (int i = cpuBase, j = memoryBase; i <= maxCpu && j <= maxMem; i += cpuStep, j += memStep) {
					ret = searchServer(i, i + cpuStep - 1, j, j + memStep - 1);
					if (ret.first != -1) return ret;
				}
			}
		}
		return {-1, -1};
	}

	void solveOneDay(const std::vector<Command> &commands) {

		std::map<int, int> buyCnt;
		std::vector<std::pair<int, int>> ansId;
		std::vector<std::vector<int>> ansMigrate;

		auto printUsedRatio = [&]() {
			if (curDay % 80 == 0) {
				std::cerr << "\n\n" << curDay << ": " << std::endl;
				std::cerr << std::fixed << std::setprecision(3);
				for (auto &server: serversUsed) {
					if (server.isDouble) continue;
					// std::cerr << server << std::endl;
					auto r0 = server.calUsedRatio(0);
					auto r1 = server.calUsedRatio(1);
					std::cerr << r0.first << "/" << r0.second << " | " << r1.first << "/" << r1.second << std::endl;
				}
				std::cerr << std::string(80, '-') << std::endl;
				for (auto &server : serversUsed) {
					if (!server.isDouble) continue;
					// std::cerr << server << std::endl;
					auto r0 = server.calUsedRatio();
					std::cerr << r0.first << "/" << r0.second << std::endl;
				}
			}
		};
		// printUsedRatio()

		auto newServer = [&](const VmInfo &vmInfo) -> std::pair<int, int> {
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
		};

		auto install = [&](const std::pair<int, int> &insId, int vmId) {
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
		};

		auto uninstall = [&](const std::pair<int, int> &insId, int vmId) {
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
		};

		auto doMigrate = [&](const std::pair<int, int> &fromId, int vmId) -> bool {
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
		};

		auto vmMigrate = [&](int &migrateRes1, int &migrateRes2, double serverUsedRatio) {
			if (vmResNum[0] + vmResNum[1] == 0) {
				return;
			}
			// std::cerr << curDay << ": " << serverUsedRatio << "   " << migrateRes1 << " " << migrateRes2 << " | ";

			int curIndex = (lastMigrateIndex + 1) % vmList.size();
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
				curIndex = (curIndex + 1) % vmList.size();
			}
			lastMigrateIndex = (curIndex - 1 + vmList.size()) % vmList.size();

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

			// std::cerr << migrateRes1 << " " << migrateRes2 << std::endl;
		};

		double serverUsedRatio = 0.98;
		double migrateRatio = 0.5;
		int migrateLim = (vmResNum[0] + vmResNum[1]) * 3 / 100;

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
			// double ratio = static_cast<double>(migrateUse1) / (migrateUse1 + migrateUse2);
			// migrateRatio = std::min(0.5, ratio);
			// serverUsedRatio = std::max(0.95, serverUsedRatio - 0.01);
		}

		// auto switchServer = [&](int idx) {
		// 	auto &server = serversUsed[idx];
		// 	int lim = server.isDouble ? 1 : 2;
		// 	for (int nodeId = 0; nodeId < lim; ++nodeId) {
		// 		auto &serIdRes = serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
		// 		serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), idx));
		// 		auto &serIdUse = serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
		// 		serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), idx));
		// 	}
		// 	server.isDouble ^= 1;
		// 	lim = server.isDouble ? 1 : 2;
		// 	for (int nodeId = 0; nodeId < lim; ++nodeId) {
		// 		serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(idx);
		// 		serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(idx);
		// 	}
		// };

		// auto pickEmptyServer = [&](int isDouble) -> int {
		// 	for (const auto &idx : serversIdUse[isDouble][0][0][0]) {
		// 		if (serversUsed[idx].isEmpty(1)) {
		// 			return idx;
		// 		}
		// 	}
		// 	return -1;
		// };
		
		auto calEmptyServer1 = [&]() -> int {
			int cnt = 0;
			for (const auto &idx : serversIdUse[0][0][0][0]) {
				cnt += serversUsed[idx].isEmpty(1);
			}
			return cnt;
		};

		auto calEmptyServer2 = [&]() -> int {
			return serversIdUse[1][0][0][0].size();
		};

		for (const auto &command : commands) {

			if (command.commandType) { // add

				vmIdToIndex[command.vmId] = command.vmIndex;

				const VmInfo &vmInfo = vmInfos[command.vmIndex];
				auto insId = selectServerInstall(vmInfo);

				if (insId.first == -1) {
					// // if (!vmInfo.isDouble && calEmptyServer2() > 0) {
					// // 	std::cerr << curDay << " <2> " << calEmptyServer2() << std::endl;
					// // }
					// // else if (vmInfo.isDouble && calEmptyServer1() > 0) {
					// // 	std::cerr << curDay << " <1> " << calEmptyServer1() << std::endl;
					// // }
					// int idx = pickEmptyServer(vmInfo.isDouble ^ 1);
					// // if (idx != -1) {
					// // 	std::cerr << "idx " << idx << std::endl;
					// // }
					// if (idx != -1 && serversUsed[idx].canInstall(vmInfo)) {
					// 	switchServer(idx);
					// 	insId = std::make_pair(idx, vmInfo.isDouble ? -1 : 0);
					// 	// std::cerr << curDay << ": " << "switch " << idx << std::endl;
					// }
					// else {
					// 	insId = newServer(vmInfo);
					// }
					insId = newServer(vmInfo);
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

		auto printVSRatio = [&]() {
			double vmRatio[2] = {
				vmCpuSum[0] / (EPS + vmMemSum[0]), 
				vmCpuSum[1] / (EPS + vmMemSum[1])
			};
			double serverRatio[2] = {
				serverCpuSum[0] / (EPS + serverMemSum[0]), 
				serverCpuSum[1] / (EPS + serverMemSum[1])
			};
			std::cerr << vmRatio[0] << " " << serverRatio[0] << std::endl;
			// std::cerr << vmRatio[1] << " " << serverRatio[1] << std::endl;
		};
		// printVSRatio();

		auto printUsedRatio = [&]() {
			if (curDay % 80 == 0) {
				std::cerr << "\n\n" << curDay << ": " << std::endl;
				std::cerr << std::fixed << std::setprecision(3);
				for (auto &server: serversUsed) {
					if (server.isDouble) continue;
					// std::cerr << server << std::endl;
					auto r0 = server.calUsedRatio(0);
					auto r1 = server.calUsedRatio(1);
					std::cerr << r0.first << "/" << r0.second << " | " << r1.first << "/" << r1.second << std::endl;
				}
				std::cerr << std::string(80, '-') << std::endl;
				for (auto &server : serversUsed) {
					if (!server.isDouble) continue;
					// std::cerr << server << std::endl;
					auto r0 = server.calUsedRatio();
					std::cerr << r0.first << "/" << r0.second << std::endl;
				}
			}
		};
		// printUsedRatio();

		auto checkServersId = [&]() {
			for (size_t i = 0; i < serversUsed.size(); ++i) {
				const auto &server = serversUsed[i];
				int lim = server.isDouble ? 1 : 2;
				for (int nodeId = 0; nodeId < lim; ++nodeId) {
					auto &serIdRes = serversIdRes[server.isDouble][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
					assert(std::find(serIdRes.begin(), serIdRes.end(), i) != serIdRes.end());
					auto &serIdUse = serversIdUse[server.isDouble][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
					assert(std::find(serIdUse.begin(), serIdUse.end(), i) != serIdUse.end());
				}
			}
		};
		// checkServersId();

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
// TODO: switch of server.isDouble