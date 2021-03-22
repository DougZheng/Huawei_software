#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <random>
#include <functional>
#include <climits>
#include <cassert>

#define DEBUG

class Solution {

	struct ServerInfo {

		friend std::istream &operator>>(std::istream &is, ServerInfo &serverInfo) {
			std::string serverType, cpuCores, memorySize, serverCost, powerCost;
			is >> serverType >> cpuCores >> memorySize >> serverCost >> powerCost;
			serverType.pop_back();
			cpuCores.pop_back();
			memorySize.pop_back();
			serverCost.pop_back();
			powerCost.pop_back();
			serverInfo.serverType = serverType.substr(1);
			serverInfo.cpuCores[0] = serverInfo.cpuCores[1] = std::stoi(cpuCores) / 2;
			serverInfo.memorySize[0] = serverInfo.memorySize[1] = std::stoi(memorySize) / 2;
			serverInfo.serverCost = std::stoi(serverCost);
			serverInfo.powerCost = std::stoi(powerCost);
			return is;
		};

		friend std::ostream &operator<<(std::ostream &os, const ServerInfo &serverInfo) {
			os << "(" << serverInfo.serverType << ", " 
				<< serverInfo.cpuCores[0] << "+" << serverInfo.cpuCores[1] << ", "
				<< serverInfo.memorySize[0] << "+" << serverInfo.memorySize[1] << ", "
				<< serverInfo.serverCost << ", " << serverInfo.powerCost << ")" << std::endl;
			return os;
		}

		std::string serverType;
		int cpuCores[2];
		int memorySize[2];
		int serverCost;
		int powerCost;
		int serverId = -1;

		int cpuUsed = 0;
		int memoryUsed = 0;

		void install(int nodeId, int cpu, int memory) {
			if (nodeId == -1) {
				install(0, cpu / 2, memory / 2);
				install(1, cpu / 2, memory / 2);
				return;
			}
			cpuCores[nodeId] -= cpu;
			memorySize[nodeId] -= memory;

			cpuUsed += cpu;
			memoryUsed += memory;
		}

		void uninstall(int nodeId, int cpu, int memory) {
			if (nodeId == -1) {
				uninstall(0, cpu / 2, memory / 2);
				uninstall(1, cpu / 2, memory / 2);
				return;
			}
			cpuCores[nodeId] += cpu;
			memorySize[nodeId] += memory;

			cpuUsed -= cpu;
			memoryUsed -= memory;
		}

		int calPriors() const {
			return serverCost / (cpuCores[0] + cpuCores[1]); 
		}

		double calUsedRatio() {
			double cpuRatio = cpuUsed / (0.05 + cpuCores[0] + cpuCores[1]);
			double memoryRatio = memoryUsed / (0.05 + memorySize[0] + memorySize[1]);
			return (cpuRatio + memoryRatio) * 0.5;
		}
	};

	struct VmInfo {

		friend std::istream &operator>>(std::istream &is, VmInfo &vmInfo) {
			std::string vmType, cpuCores, memorySize, isDouble;
			is >> vmType >> cpuCores >> memorySize >> isDouble;
			vmType.pop_back();
			cpuCores.pop_back();
			memorySize.pop_back();
			isDouble.pop_back();
			vmInfo.vmType = vmType.substr(1);
			vmInfo.cpuCores = std::stoi(cpuCores);
			vmInfo.memorySize = std::stoi(memorySize);
			vmInfo.isDouble = std::stoi(isDouble);
			return is;
		}

		friend std::ostream &operator<<(std::ostream &os, const VmInfo &vmInfo) {
			os << "(" << vmInfo.vmType << ", "
				<< vmInfo.cpuCores << ", " << vmInfo.memorySize << ", "
				<< vmInfo.isDouble << ")" << std::endl;
			return os;
		}

		std::string vmType;
		int cpuCores;
		int memorySize;
		int isDouble;
	};

	struct Command {

		friend std::istream &operator>>(std::istream &is, Command &command) {
			std::string commandType, vmType, vmId;
			is >> commandType;
			command.commandType = 0;
			if (commandType[1] == 'a') {
				command.commandType = 1;
				is >> vmType;
				vmType.pop_back();
				command.vmType = vmType;
			}
			is >> vmId;
			vmId.pop_back();
			command.vmId = std::stoi(vmId);
			return is;
		}

		friend std::ostream &operator<<(std::ostream &os, const Command &command) {
			os << "(" << (command.commandType ? "add" : "del") << ", "
				<< (command.commandType ? command.vmType + ", " : "")
				<< command.vmId << ")" << std::endl;
			return os;
		}

		int commandType;
		std::string vmType;
		int vmId;
	};

	std::vector<ServerInfo> serverInfos;
	std::unordered_map<std::string, int> vmTypeToIndex;
	std::vector<VmInfo> vmInfos;
	std::vector<std::vector<Command>> commands;

	std::vector<int> serverInfosHasId;
	std::vector<int> serversUsedHasId;
	std::vector<ServerInfo> serversUsed;

	std::unordered_map<int, int> vmIdToIndex;
	std::unordered_map<int, std::pair<int, int>> installId;
	std::map<int, std::set<int>> serverIndexToVmId;

	int serversUsedNum = 0;
	int vmResNum = 0;
	int totalMigration = 0;
	int canMigrateTotal = 0;
	long long totalCost = 0;

	int tim = 0;
	const int shuffleFreq = 50;

	const double oo = 1e200;
	double levelCoef = 500.0;
	double acceptRange = 1.5;

	std::function<std::pair<int, int>
		(const std::vector<ServerInfo>&, const std::vector<int>&, int, int)> policys[2] = 
	{
		std::bind(&Solution::bestFit1, this, std::placeholders::_1, 
			std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), 
		std::bind(&Solution::bestFit2, this, std::placeholders::_1, 
			std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	};


	inline double calF(int serverCpu, int serverMemory, int vmCpu, int vmMemory) {
		if (serverCpu == 0 || serverMemory == 0) {
			return oo;
		}
		double serverK = static_cast<double>(serverCpu) / serverMemory;
		double vmK = static_cast<double>(vmCpu) / vmMemory;
		double ratio = serverK > vmK ? serverK / vmK : vmK / serverK;
		return levelCoef * static_cast<int>(ratio / acceptRange) + (serverCpu - vmCpu);
	}

	std::pair<int, int> bestFit1(const std::vector<ServerInfo> &servers, 
		const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
		double fmn = oo;
		std::pair<int, int> ret(-1, -1);
		for (int i = 0; i < servers.size(); ++i) {
			int index = serversHasId[i];
			if (index == -1) continue;
			const auto &server = servers[index];
			double fval0 = calF(server.cpuCores[0], server.memorySize[0], cpuCores, memorySize);
			double fval1 = calF(server.cpuCores[1], server.memorySize[1], cpuCores, memorySize);
			if (server.cpuCores[0] >= cpuCores && server.memorySize[0] >= memorySize
				&& fval0 < fmn) {
				fmn = fval0;
				ret = std::make_pair(index, 0);
			}
			if (server.cpuCores[1] >= cpuCores && server.memorySize[1] >= memorySize
				&& fval1 < fmn) {
				fmn = fval1;
				ret = std::make_pair(index, 1);
			}
		}
		return ret;
	}

	std::pair<int, int> bestFit2(const std::vector<ServerInfo> &servers, 
		const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
		cpuCores /= 2;
		memorySize /= 2;
		double fmn = oo;
		std::pair<int, int> ret(-1, -1);
		for (int i = 0; i < servers.size(); ++i) {
			int index = serversHasId[i];
			if (index == -1) continue;
			const auto &server = servers[index];
			double fval = calF(std::min(server.cpuCores[0], server.cpuCores[1]), 
				std::min(server.memorySize[0], server.memorySize[1]), 
				cpuCores, memorySize);
			if (server.cpuCores[0] >= cpuCores && server.cpuCores[1] >= cpuCores
				&& server.memorySize[0] >= memorySize && server.memorySize[1] >= memorySize
				&& fval < fmn) {
				ret.first = index;
				fmn = fval;
			}
		}
		return ret;
	}

	void solveOneDay(const std::vector<Command> &commands) {

		std::map<int, int> buyCnt;
		std::vector<std::pair<int, int>> ansId;
		std::vector<std::vector<int>> ansMigrate;

		int migrateNum = 5 * vmResNum / 1000;

		std::vector<ServerInfo> serversReserved = serversUsed;
		decltype(serverIndexToVmId) newVmId;

		for (const auto &command : commands) {

			if (++tim % shuffleFreq == 0) {
				// 
			}

			if (command.commandType) { // add

				++vmResNum;

				const VmInfo &vmInfo = vmInfos[vmTypeToIndex[command.vmType]];
				auto policy = policys[vmInfo.isDouble];
				auto selId = policy(serversUsed, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);

				if (selId.first == -1) {
					auto buyId = policy(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize);
					assert(buyId.first != -1);
					selId = std::make_pair(serversUsed.size(), buyId.second);
					serversUsedHasId.emplace_back(selId.first);
					serversUsed.emplace_back(serverInfos[buyId.first]);
					++buyCnt[buyId.first];
					serversUsed.back().serverId = buyId.first; // 需要重编号

					serversReserved.emplace_back(serversUsed.back());
				}

				serversUsed[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);
				installId[command.vmId] = selId;
				ansId.emplace_back(selId);

				serversReserved[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);
				newVmId[selId.first].emplace(command.vmId);

				vmIdToIndex[command.vmId] = vmTypeToIndex[command.vmType];
			}
			else { // del

				--vmResNum;

				auto insId = installId[command.vmId];
				const auto &vmInfo = vmInfos[vmIdToIndex[command.vmId]];
				serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);

				if (newVmId[insId.first].count(command.vmId)) {
					newVmId[insId.first].erase(command.vmId);
				}
				else {
					serverIndexToVmId[insId.first].erase(command.vmId);
				}
			}
		}

		auto mergeVmId = [&](int index) {
			for (const auto &v : newVmId[index]) {
				serverIndexToVmId[index].emplace(v);
			}
			newVmId[index].clear();
		};

		std::vector<std::pair<int, int>> migrateIndex;
		std::vector<int> migrateVmId;

		// std::sort(serversUsedHasId.begin(), serversUsedHasId.end(), 
		// 	[](int x, int y) {
		// 		// return serversUsed[x].powerCost > serversUsed[y].powerCost;
		// 		// return serversUsed[x].cpuUsed > serversUsed[y].cpuUsed;
		// 		return serversUsed[x].calUsedRatio() > serversUsed[y].calUsedRatio();
		// 	});

		for (int i = 0; i < serversUsedHasId.size(); ++i) {
			int index = serversUsedHasId[i];
			if (migrateIndex.size() < migrateNum 
				&& serverIndexToVmId[index].size() == 1 && newVmId[index].size() == 0) {
				migrateIndex.emplace_back(i, index);
				migrateVmId.emplace_back(*serverIndexToVmId[index].begin());
				serversUsedHasId[i] = -1;
			}
			mergeVmId(index);
		}

		int migrateCnt = 0;
		for (const auto &e : migrateIndex) {

			int vmId = *serverIndexToVmId[e.second].begin();
			auto insId = installId[vmId];
			auto vmInfo = vmInfos[vmIdToIndex[vmId]];
			auto policy = policys[vmInfo.isDouble];

			auto selId = policy(serversReserved, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
			if (selId.first == -1) continue; 

			installId[vmId] = selId;

			serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
			serversUsed[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);

			serversReserved[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);

			ansMigrate.push_back({vmId, selId.first, selId.second});
			serverIndexToVmId[insId.first].erase(vmId);
			serverIndexToVmId[selId.first].emplace(vmId);

			++migrateCnt;
		}

		totalMigration += migrateCnt;
		canMigrateTotal += migrateIndex.size();

		for (const auto &e : migrateIndex) {
			serversUsedHasId[e.first] = e.second;
		}

		std::cout << "(purchase, " << buyCnt.size() << ")" << std::endl;
		int preCnt = 0;
		for (auto &it : buyCnt) {

			totalCost += it.second * 1ll * serverInfos[it.first].serverCost;

			std::cout << "(" << serverInfos[it.first].serverType << ", " << it.second << ")" << std::endl;
			it.second += preCnt;
			preCnt = it.second;
		}

		// 服务器重编号
		for (int i = serversUsed.size() - 1; i >= serversUsedNum; --i) {
			int serverIndex = serversUsed[i].serverId;
			serversUsed[i].serverId = serversUsedNum + (--buyCnt[serverIndex]);
		}
		serversUsedNum = serversUsed.size();

		std::cout << "(migration, " << ansMigrate.size() << ")" << std::endl;
		for (const auto &vc : ansMigrate) {
			if (vc.back() == -1) {
				std::cout << "(" << vc[0] << ", " << serversUsed[vc[1]].serverId << ")" << std::endl;
			}
			else {
				std::cout << "(" << vc[0] << ", " << serversUsed[vc[1]].serverId << 
					", " << (vc[2] ? "B" : "A") << ")" << std::endl;
			}
		}

		for (const auto &id : ansId) {
			if (id.second != -1) {
				std::cout << "(" << serversUsed[id.first].serverId 
					<< ", " << (id.second ? "B" : "A") << ")" << std::endl;
			}
			else {
				std::cout << "(" << serversUsed[id.first].serverId << ")" << std::endl;
			}
		}

		for (int i = 0; i < serversUsed.size(); ++i) {
			if (serverIndexToVmId[i].empty()) continue;
			totalCost += serversUsed[i].powerCost;
		}
	}

public:

	void read() {

		int serverNum;
		std::cin >> serverNum;
		serverInfos.reserve(serverNum);
		for (int i = 0; i < serverNum; ++i) {
			ServerInfo serverInfo;
			std::cin >> serverInfo;
			serverInfos.emplace_back(serverInfo);	
		}

		int vmNum;
		std::cin >> vmNum;
		vmInfos.reserve(vmNum);
		for (int i = 0; i < vmNum; ++i) {
			VmInfo vmInfo;
			std::cin >> vmInfo;
			vmTypeToIndex[vmInfo.vmType] = i;
			vmInfos.emplace_back(vmInfo);
		}

		int dayNum;
		std::cin >> dayNum;
		commands.resize(dayNum);
		for (int i = 0; i < dayNum; ++i) {
			int commandNum;
			std::cin >> commandNum;
			commands[i].reserve(commandNum);
			for (int j = 0; j < commandNum; ++j) {
				Command command;
				std::cin >> command;
				commands[i].emplace_back(command);
			}
		}
	}

	std::pair<long long, int> solve() {
			
		serverInfosHasId.clear();
		serversUsedHasId.clear();
		serversUsed.clear();
		vmIdToIndex.clear();
		installId.clear();
		serverIndexToVmId.clear();
		serversUsedNum = 0;
		vmResNum = 0;
		totalMigration = 0;
		canMigrateTotal = 0;
		totalCost = 0;
		tim = 0;

		serverInfosHasId.resize(serverInfos.size());
		std::iota(serverInfosHasId.begin(), serverInfosHasId.end(), 0);
		// std::sort(serverInfosHasId.begin(), serverInfosHasId.end(), 
		// 	[](int x, int y) {
		// 		return serverInfos[x].calPriors() < serverInfos[y].calPriors();
		// 	});

		for (const auto &cmds : commands) {
			solveOneDay(cmds);
		}
		return std::make_pair(totalCost, totalMigration);
	}

	void setAcceptRange(double acceptRange) {
		this->acceptRange = acceptRange;
	}
};

std::pair<long long, int> solve(std::string in = "", std::string out = "", double acceptRange = 1.5) {

	clock_t timS = clock();
	
	assert(std::freopen(in.c_str(), "r", stdin) != nullptr);
	assert(std::freopen(out.c_str(), "w", stdout) != nullptr);

	Solution solution;
	solution.read();
	
	solution.setAcceptRange(acceptRange);

	auto ans = solution.solve();
	clock_t timT = clock();

	auto calTim = [timS, timT]() -> double {
		return static_cast<double>(timT - timS) / CLOCKS_PER_SEC * 1000;
	};

	std::cerr << "\n" << in << std::endl;
	std::cerr << "cost: " << ans.first << std::endl;
	std::cerr << "migration: " << ans.second << std::endl;
	std::cerr << "time cost: " << calTim() << "ms" << std::endl;

	std::fclose(stdin);
	std::fclose(stdout);

	return ans;
}

int main() {

	#ifdef DEBUG
	for (double k = 1.00; k <= 1.3; k += 0.05) {
		std::cerr << "\nk = " << k << std::endl;
		auto ans1 = solve("../../data/training-1.txt", "../../data/training-1.out", k);
		auto ans2 = solve("../../data/training-2.txt", "../../data/training-2.out", k);
		std::cerr << "\n" << "summary:" << std::endl;
		std::cerr << "total cost: " << ans1.first + ans2.first << std::endl;
		std::cerr << "total migration: " << ans1.second + ans2.second << std::endl;
	}
	#endif

	#ifndef DEBUG
	Solution solution;
	solution.read();
	solution.solve();
	#endif

	return 0;
}
