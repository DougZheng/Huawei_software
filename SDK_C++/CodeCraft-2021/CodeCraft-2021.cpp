#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stack>
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
		int vmIndex;
	};

	std::vector<int> vmIdOrdered;

	std::vector<ServerInfo> serverInfos;
	std::unordered_map<std::string, int> vmTypeToIndex;
	std::vector<VmInfo> vmInfos;
	std::vector<std::vector<Command>> commands;

	std::vector<int> serverInfosHasId;
	std::vector<int> serversUsedHasId;
	std::vector<ServerInfo> serversUsed;

	std::vector<int> vmIdToIndex;
	std::vector<std::pair<int, int>> installId;
	std::vector<std::vector<int>> serverIndexToVmId;

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

		int migrateLim = 5 * vmResNum / 1000;

		std::vector<ServerInfo> serversReserved = serversUsed;
		decltype(serverIndexToVmId) newVmId(serverIndexToVmId.size());

		for (const auto &command : commands) {

			if (++tim % shuffleFreq == 0) {
				// 
			}

			if (command.commandType) { // add

				++vmResNum;

				const VmInfo &vmInfo = vmInfos[command.vmIndex];
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

					newVmId.emplace_back();
				}

				serversUsed[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);
				installId[command.vmId] = selId;
				ansId.emplace_back(selId);

				serversReserved[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);

				assert(selId.first < newVmId.size());
				newVmId[selId.first].emplace_back(command.vmId);

				vmIdToIndex[command.vmId] = command.vmIndex;
			}
			else { // del

				--vmResNum;

				auto insId = installId[command.vmId];
				const auto &vmInfo = vmInfos[vmIdToIndex[command.vmId]];
				serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);

				assert(insId.first < newVmId.size());

				if (std::find(newVmId[insId.first].cbegin(), newVmId[insId.first].cend(), command.vmId) 
					!= newVmId[insId.first].cend()) {

					newVmId[insId.first].erase(
						std::find(newVmId[insId.first].begin(), newVmId[insId.first].end(), command.vmId));
				}
				else {
					assert(insId.first < serverIndexToVmId.size());
					serverIndexToVmId[insId.first].erase(
						std::find(serverIndexToVmId[insId.first].begin(), serverIndexToVmId[insId.first].end(), command.vmId));
				}
			}
		}

		serverIndexToVmId.resize(newVmId.size());

		struct MigrateRecord {
			int vmId;
			std::pair<int, int> fromId, toId;
		};

		std::stack<MigrateRecord> migrateRecords;

		auto doMigrate = [&](int vmId, std::pair<int, int> fromId, std::pair<int, int> toId) {
			migrateRecords.push({vmId, fromId, toId});
			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
			installId[vmId] = toId;
			serversUsed[fromId.first].uninstall(fromId.second, vmInfo.cpuCores, vmInfo.memorySize);
			serversUsed[toId.first].install(toId.second, vmInfo.cpuCores, vmInfo.memorySize);
			serversReserved[toId.first].install(toId.second, vmInfo.cpuCores, vmInfo.memorySize);
		};

		auto undoMigrate = [&]() {
			auto migrateRecord = migrateRecords.top();
			migrateRecords.pop();
			int vmId = migrateRecord.vmId;
			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
			auto fromId = migrateRecord.fromId;
			auto toId = migrateRecord.toId;
			installId[vmId] = fromId;
			serversUsed[fromId.first].install(fromId.second, vmInfo.cpuCores, vmInfo.memorySize);
			serversUsed[toId.first].uninstall(toId.second, vmInfo.cpuCores, vmInfo.memorySize);
			serversReserved[toId.first].uninstall(toId.second, vmInfo.cpuCores, vmInfo.memorySize);
		};

		auto rollbackMigrate = [&](int tim) {
			while (migrateRecords.size() > tim) {
				undoMigrate();
			}
		};

		std::sort(serversUsedHasId.begin(), serversUsedHasId.end(), 
			[&](int x, int y) {
				return newVmId[x].empty() != newVmId[y].empty() ? newVmId[x].empty() > newVmId[y].empty()
					: serverIndexToVmId[x].size() < serverIndexToVmId[y].size();
			});

		int serverIndexLim = serversUsed.size();
		for (int i = 0; i < serversUsedHasId.size(); ++i) {
			int index = serversUsedHasId[i];
			if (!newVmId[index].empty()) {
				serverIndexLim = i;
				break;
			}
		}
	
		std::vector<std::pair<int, int>> readyMigrate, successMigrate, failMigrate;
		int migrateStep = migrateLim;
		while (migrateStep > 0 && migrateLim > 0) {
			
			readyMigrate.clear();
			migrateStep = std::min(migrateStep, migrateLim);
			int migrateCnt = 0;
			for (int i = 0; i < serverIndexLim; ++i) {
				int index = serversUsedHasId[i];
				if (index == -1) continue;
				if (migrateCnt + serverIndexToVmId[index].size() > migrateStep) {
					break;
				}
				serversUsedHasId[i] = -1;
				migrateCnt += serverIndexToVmId[index].size();
				readyMigrate.emplace_back(i, index);
			}

			int migrateSuccess = 0;
			for (const auto &e : readyMigrate) {
				int tim = migrateRecords.size();
				migrateSuccess += serverIndexToVmId[e.second].size();
				bool isSuccess = true;
				for (const auto &vmId : serverIndexToVmId[e.second]) {
					const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
					auto policy = policys[vmInfo.isDouble];
					auto toId = policy(serversReserved, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
					if (toId.first == -1) {
						rollbackMigrate(tim);
						failMigrate.emplace_back(e);
						migrateSuccess -= serverIndexToVmId[e.second].size();
						isSuccess = false;
						break;
					}
					auto fromId = installId[vmId];
					doMigrate(vmId, fromId, toId);
				}
				if (isSuccess) {
					successMigrate.emplace_back(e);
				}
			}

			for (const auto &e : failMigrate) {
				serversUsedHasId[e.first] = e.second;
			}
			failMigrate.clear();

			migrateLim -= migrateSuccess;
			totalMigration += migrateSuccess;
			if (migrateSuccess == 0) {
				migrateStep /= 2;
			}
			
			while (!migrateRecords.empty()) {
				auto migrateRecord = migrateRecords.top();
				migrateRecords.pop();
				int vmId = migrateRecord.vmId;
				auto fromId = migrateRecord.fromId;
				auto toId = migrateRecord.toId;
				serverIndexToVmId[fromId.first].erase(
					std::find(serverIndexToVmId[fromId.first].begin(), serverIndexToVmId[fromId.first].end(), vmId));
				serverIndexToVmId[toId.first].emplace_back(vmId);
				ansMigrate.push_back({vmIdOrdered[vmId], toId.first, toId.second});
			}
			// std::cerr << migrateLim << " " << migrateStep << " " << migrateSuccess << std::endl;
			// break;
		}

		for (const auto &e : successMigrate) {
			serversUsedHasId[e.first] = e.second;
		}

		for (auto &e : serversUsedHasId) {
			assert(e != -1);
		}

		auto mergeVmId = [&](int index) {
			for (const auto &v : newVmId[index]) {
				serverIndexToVmId[index].emplace_back(v);
			}
			newVmId[index].clear();
		};
		for (int i = 0; i < serversUsed.size(); ++i) {
			mergeVmId(i);
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
		vmIdOrdered.clear();
		for (int i = 0; i < dayNum; ++i) {
			int commandNum;
			std::cin >> commandNum;
			commands[i].reserve(commandNum);
			for (int j = 0; j < commandNum; ++j) {
				Command command;
				std::cin >> command;
				commands[i].emplace_back(command);
				vmIdOrdered.emplace_back(command.vmId);
			}
		}

		std::sort(vmIdOrdered.begin(), vmIdOrdered.end());
		vmIdOrdered.erase(std::unique(vmIdOrdered.begin(), vmIdOrdered.end()), vmIdOrdered.end());
		for (auto &cmds : commands) {
			for (auto &cmd : cmds) {
				cmd.vmId = std::lower_bound(vmIdOrdered.cbegin(), vmIdOrdered.cend(), cmd.vmId) - vmIdOrdered.cbegin();
				if (cmd.commandType) {
					cmd.vmIndex = vmTypeToIndex[cmd.vmType];
				}
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

		vmIdToIndex.resize(vmIdOrdered.size());
		installId.resize(vmIdOrdered.size());

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

	auto calTim = [](clock_t timS, clock_t timT) -> double {
		return static_cast<double>(timT - timS) / CLOCKS_PER_SEC * 1000;
	};

	std::cerr << "\n" << in << std::endl;
	std::cerr << "cost: " << ans.first << std::endl;
	std::cerr << "migration: " << ans.second << std::endl;
	std::cerr << "time cost: " << calTim(timS, timT) << "ms" << std::endl;


	std::fclose(stdin);
	std::fclose(stdout);

	return ans;
}

int main() {

	#ifdef DEBUG
	for (double k = 1.50; k <= 1.51; k += 0.05) {
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
