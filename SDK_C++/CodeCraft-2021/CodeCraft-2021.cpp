#include <iostream>
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
#include <ctime>
#include <random>
#include <functional>
#include <climits>
#include <cassert>
#include <thread>
#include <future>

// #define DEBUG
#define ON_LINE

#define EPS 1e-3

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

		double calUsedRatio(double levelCoef = 300.0, double acceptRange = 1.20) {
			double serverK = cpuTotal / (0.05 + memoryTotal);
			double vmK = (cpuUsed[0] + cpuUsed[1]) / (0.05 + memoryUsed[0] + memoryUsed[1]);
			double ratio = std::max(serverK / vmK, vmK / serverK);
			return -(levelCoef * std::floor(ratio / acceptRange));
		}

		bool isEmpty(int nodeId = 0) const {
			return cpuUsed[nodeId] == 0 && memoryUsed[nodeId] == 0;
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

	static int dayNum;
	static int previewNum;
	int curDay;
	int serversUsedNum[2] = {};
	int vmResNum = 0;
	int totalMigration = 0;
	int canMigrateTotal = 0;
	long long totalCost = 0;

	int tim = 0;
	const int shuffleFreq = 50;

	const double oo = 1e200;
	double levelCoef = 300.0;
	double acceptRange = 1.5;

	int selectServerPurchase(const VmInfo &vmInfo) {
		int cpuCores = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		int memorySize = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		for (size_t i = 0; i < serverInfos.size(); ++i) {
			const auto &server = serverInfos[i];
			if (server.cpuCores[0] >= cpuCores && server.memorySize[0] >= memorySize) {
				return i;
			}
		}
		return -1;
	}

	inline double calF(const ServerInfo &server, int nodeId, int vmCpu, int vmMemory) {
		int serverCpu = server.cpuCores[nodeId];
		int serverMemory = server.memorySize[nodeId];
		if (serverCpu == 0 || serverMemory == 0) {
			return oo;
		}
		double serverK = static_cast<double>(serverCpu) / serverMemory;
		double vmK = static_cast<double>(vmCpu) / vmMemory;
		double ratio = std::max(serverK / vmK, vmK / serverK);
		return levelCoef * std::floor(ratio / acceptRange)
			+ serverK * (serverCpu - vmCpu) + 1.0 / serverK * (serverMemory - vmMemory);
	}

	std::pair<int, int> selectServerInstall(const VmInfo &vmInfo) {
		int cpuCores = vmInfo.isDouble ? vmInfo.cpuCores / 2 : vmInfo.cpuCores;
		int memorySize = vmInfo.isDouble ? vmInfo.memorySize / 2 : vmInfo.memorySize;
		int lim = vmInfo.isDouble ? 1 : 2;
		std::pair<int, int> ret{-1, -1};
		auto searchServer = [&](int cpuBase, int cpuLim, int memBase, int memLim) -> std::pair<int, int> {
			cpuLim = std::min(cpuLim, CPUN);
			memLim = std::min(memLim, MEMN);
			for (int i = cpuBase; i <= cpuLim; ++i) {
				for (int j = memBase; j <= memLim; ++j) {
					for (int k = 0; k < lim; ++k) {
						for (const auto &idx : serversIdRes[vmInfo.isDouble][k][i][j]) {
							return {idx, vmInfo.isDouble ? -1 : k};
						}
					}
				}
			}
			return {-1, -1};
		};
		return searchServer(cpuCores, CPUN, memorySize, MEMN);
		// for (int i = cpuCores; i <= CPUN; ++i) {
		// 	for (int j = memorySize; j <= MEMN; ++j) {
		// 		for (int k = 0; k < lim; ++k) {
		// 			for (const auto &idx : serversIdRes[vmInfo.isDouble][k][i][j]) {
		// 				if (banned[vmInfo.isDouble][idx]) continue;
		// 				return {idx, vmInfo.isDouble ? -1 : k};
		// 			}
		// 		}
		// 	}
		// }
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
			return insId;
		};

		auto install = [&](ServerInfo &server, const std::vector<int> &insId, const VmInfo &vmInfo) {
			int nodeId = getNodeId(insId[2]);
			auto &serIdRes = serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
			serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId[1]));
			auto &serIdUse = serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
			serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId[1]));
			server.install(insId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(insId[1]);
			serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(insId[1]);
		};

		auto uninstall = [&](ServerInfo &server, const std::vector<int> &insId, const VmInfo &vmInfo) {
			int nodeId = getNodeId(insId[2]);
			auto &serIdRes = serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]];
			serIdRes.erase(std::find(serIdRes.begin(), serIdRes.end(), insId[1]));
			auto &serIdUse = serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]];
			serIdUse.erase(std::find(serIdUse.begin(), serIdUse.end(), insId[1]));
			server.uninstall(insId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversIdRes[insId[0]][nodeId][server.cpuCores[nodeId]][server.memorySize[nodeId]].push_front(insId[1]);
			serversIdUse[insId[0]][nodeId][server.cpuUsed[nodeId]][server.memoryUsed[nodeId]].push_front(insId[1]);
		};

		std::set<std::vector<int>> serverMigrated;
		auto selectServerMigrate = [&](const std::vector<int> &fromId, int cpuBase, int cpuLim, int memBase, int memLim) -> std::vector<int> {
			cpuLim = std::min(cpuLim, CPUN);
			memLim = std::min(memLim, MEMN);
			for (int i = cpuBase; i <= cpuLim; ++i) {
				for (int j = memBase; j <= memLim; ++j) {
					int lim = fromId[0] ? 1 : 2;
					for (int nodeId = 0; nodeId < lim; ++nodeId) {
						for (const auto &idx : serversIdRes[fromId[0]][nodeId][i][j]) {
							if (serversUsed[fromId[0]][idx].isEmpty(nodeId)) continue; 
							std::vector<int> toId{fromId[0], idx, fromId[0] ? -1 : nodeId};
							if (fromId == toId || serverMigrated.count(toId)) continue;
							return toId;
						}
					}
				}
			}
			return {-1, -1, -1};
		};

		struct MigrateRecord {
			std::vector<int> fromId;
			std::vector<int> toId;
		};
		std::vector<MigrateRecord> migrateRecords;

		int migrateLim = vmResNum * 3 / 100;
		
		for (int i = 0; i <= CPUN; ++i) {
			for (int j = 0; j <= MEMN; ++j) {
				if (i == 0 && j == 0) continue;
				for (int isDouble = 0; isDouble < 2; ++isDouble) {
					int lim = isDouble ? 1 : 2;
					for (int nodeId = 0; nodeId < lim; ++nodeId) {
						for (const auto &idx : serversIdUse[isDouble][nodeId][i][j]) {
							std::vector<int> fromId{isDouble, idx, isDouble ? -1 : nodeId};
							if (serverMigrated.count(fromId)) continue;
							auto toId = selectServerMigrate(fromId, i, i + 10, j, j + 10);
							if (toId[0] == -1) break;
							migrateRecords.push_back({fromId, toId});
							serverMigrated.emplace(fromId);
							serverMigrated.emplace(toId);
						}
					}
				}
			}
		}

		auto doMigrate = [&](const std::vector<int> &fromId, const std::vector<int> &toId) {
			int nodeId = getNodeId(fromId[2]);
			for (const auto &vmId : serverIndexToVmId[fromId[0]][nodeId][fromId[1]]) {
				const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
				uninstall(serversUsed[fromId[0]][fromId[1]], fromId, vmInfo);
				install(serversUsed[toId[0]][toId[1]], toId, vmInfo);
				ansMigrate.push_back({vmId, toId[0], toId[1], toId[2]});
				serverIndexToVmId[toId[0]][getNodeId(toId[2])][toId[1]].emplace_back(vmId);
				installId[vmId] = toId;
			}
			serverIndexToVmId[fromId[0]][nodeId][fromId[1]].clear();
		};

		for (const auto &record : migrateRecords) {
			int nodeId = getNodeId(record.fromId[2]);
			int migrateNum = serverIndexToVmId[record.fromId[0]][nodeId][record.fromId[1]].size();
			if (migrateNum > migrateLim) continue;
			migrateLim -= migrateNum;
			totalMigration += migrateNum;
			doMigrate(record.fromId, record.toId);
		}

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

		// struct MigrateRecord {
		// 	int vmId;
		// 	std::vector<int> fromId, toId;
		// };

		// std::stack<MigrateRecord> migrateRecords;

		// auto doMigrate = [&](int vmId, std::vector<int> fromId, std::vector<int> toId) {
		// 	migrateRecords.push({vmId, fromId, toId});
		// 	const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		// 	installId[vmId] = toId;
		// 	uninstall(serversUsed[fromId[0]][fromId[1]], fromId, vmInfo);
		// 	install(serversUsed[toId[0]][toId[1]], toId, vmInfo);
		// };

		// auto undoMigrate = [&]() {
		// 	auto migrateRecord = migrateRecords.top();
		// 	migrateRecords.pop();
		// 	int vmId = migrateRecord.vmId;
		// 	const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		// 	auto fromId = migrateRecord.fromId;
		// 	auto toId = migrateRecord.toId;
		// 	installId[vmId] = fromId;
		// 	install(serversUsed[fromId[0]][fromId[1]], fromId, vmInfo);
		// 	uninstall(serversUsed[toId[0]][toId[1]], toId, vmInfo);
		// };

		// auto rollbackMigrate = [&](size_t tim) {
		// 	while (migrateRecords.size() > tim) {
		// 		undoMigrate();
		// 	}
		// };

		// int migrateLim = 3 * vmResNum / 100;

		// std::vector<int> readyMigrate[2], successMigrate[2], failMigrate[2];
		// std::set<int> failMigrateCnt[2];
		// int migrateStep[2] = {migrateLim, migrateLim};
		// int migrateRound[2] = {0, 0};
		// int isDouble = std::rand() % 2;

		// int cnt = 0;

		// while (migrateLim > 0 && (migrateStep[0] + migrateStep[1]) > 0) {
		// 	if (++cnt == 2) break;
		// 	if (!migrateRound[isDouble]) {
		// 		failMigrateCnt[isDouble].clear();
		// 	}
		// 	readyMigrate[isDouble].clear();
		// 	migrateStep[isDouble] = std::min(migrateStep[isDouble], migrateLim);
		// 	int migrateCnt = 0;
		// 	for (size_t i = 0; i < serversUsed[isDouble].size(); ++i) {
		// 		if (banned[isDouble][i]) continue;
		// 		// if (migrateRound[isDouble] && failMigrateCnt[isDouble].count(index)) continue;
		// 		if (migrateCnt + int(serverIndexToVmId[isDouble][i].size()) > migrateStep[isDouble]) {
		// 			break;
		// 		}
		// 		banned[isDouble][i] = 1;
		// 		migrateCnt += serverIndexToVmId[isDouble][i].size();
		// 		readyMigrate[isDouble].emplace_back(i);
		// 	}

		// 	int migrateSuccess = 0;
		// 	failMigrate[isDouble].clear();
		// 	for (const auto &idx : readyMigrate[isDouble]) {
		// 		size_t tim = migrateRecords.size();
		// 		migrateSuccess += serverIndexToVmId[isDouble][idx].size();
		// 		bool isSuccess = true;
		// 		for (const auto &vmId : serverIndexToVmId[isDouble][idx]) {
		// 			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
		// 			auto toId = selectServerInstall(vmInfo);
		// 			if (toId.first == -1) {
		// 				rollbackMigrate(tim);
		// 				failMigrate[isDouble].emplace_back(idx);
		// 				migrateSuccess -= serverIndexToVmId[isDouble][idx].size();
		// 				isSuccess = false;
		// 				failMigrateCnt[isDouble].emplace(idx);
		// 				break;
		// 			}
		// 			auto fromId = installId[vmId];
		// 			doMigrate(vmId, fromId, std::vector<int>{isDouble, toId.first, toId.second});
		// 		}
		// 		if (isSuccess) {
		// 			successMigrate[isDouble].emplace_back(idx);
		// 		}
		// 	}

		// 	for (const auto &idx : failMigrate[isDouble]) {
		// 		banned[isDouble][idx] = 0;
		// 	}

		// 	// std::cerr << isDouble << ": " << migrateStep[isDouble] << " " << migrateSuccess 
		// 	//  << " " << readyMigrate[isDouble].size() << " " << failMigrate[isDouble].size() << std::endl;

		// 	migrateLim -= migrateSuccess;
		// 	totalMigration += migrateSuccess;
		// 	if (migrateSuccess == 0) {
		// 		migrateStep[isDouble] /= 2;
		// 	}
			
		// 	while (!migrateRecords.empty()) {
		// 		auto migrateRecord = migrateRecords.top();
		// 		migrateRecords.pop();
		// 		int vmId = migrateRecord.vmId;
		// 		auto fromId = migrateRecord.fromId;
		// 		auto toId = migrateRecord.toId;
		// 		serverIndexToVmId[fromId[0]][fromId[1]].erase(
		// 			std::find(serverIndexToVmId[fromId[0]][fromId[1]].begin(), serverIndexToVmId[fromId[0]][fromId[1]].end(), vmId));
		// 		serverIndexToVmId[toId[0]][toId[1]].emplace_back(vmId);
		// 		ansMigrate.push_back({vmId, toId[0], toId[1], toId[2]});
		// 	}

		// 	migrateRound[isDouble] ^= 1;

		// 	isDouble ^= 1;
		// }
		// for (int isDouble = 0; isDouble < 2; ++isDouble) {
		// 	for (const auto &idx : successMigrate[isDouble]) {
		// 		banned[isDouble][idx] = 0;
		// 	}
		// }
		// // std::cerr << std::endl;

		auto printRes = [&]() {
			for (int i = 0; i <= CPUN; ++i) {
				for (int j = 0; j <= MEMN; ++j) {
					std::cerr << serversIdRes[1][0][i][j].size() << " \n"[j == MEMN];
				}
			}
			std::cerr << std::endl;
		};

		auto printUse = [&]() {
			std::vector<std::vector<int>> tp(CPUN + 1, std::vector<int>(MEMN + 1));
			for (int i = 0; i <= CPUN; ++i) {
				for (int j = 0; j <= MEMN; ++j) {
					for (auto &idx : serversIdRes[1][0][i][j]) {
						auto &server = serversUsed[1][idx];
						++tp[server.cpuUsed[0]][server.memoryUsed[0]];
					}
				}
			}
			for (int i = 0; i <= CPUN; ++i) {
				for (int j = 0; j <= MEMN; ++j) {
					std::cerr << tp[i][j] << " \n"[j == MEMN];
				}
			}
			std::cerr << std::endl;
		};

		// if (curDay == 500) {
		// 	printRes();
		// 	printUse();
		// }

		std::sort(serverInfos.begin(), serverInfos.end(), 
			[&](const ServerInfo &server1, const ServerInfo &server2) {
				int cost1 = curDay < 555 ? server1.powerCost : server1.serverCost;
				int cost2 = curDay < 555 ? server2.powerCost : server2.serverCost;
				return cost1 < cost2;
			});

		for (const auto &command : commands) {

			if (command.commandType) { // add

				++vmResNum;

				const VmInfo &vmInfo = vmInfos[command.vmIndex];
				auto selId = selectServerInstall(vmInfo);

				std::vector<int> insId{vmInfo.isDouble, selId.first, selId.second};

				if (selId.first == -1) {
					insId = newServer(vmInfo);
				}
				install(serversUsed[insId[0]][insId[1]], insId, vmInfo);
				serverIndexToVmId[insId[0]][getNodeId(insId[2])][insId[1]].emplace_back(command.vmId);

				installId[command.vmId] = insId;
				ansId.push_back(insId);

				vmIdToIndex[command.vmId] = command.vmIndex;
			}
			else { // del

				--vmResNum;

				auto insId = installId[command.vmId];
				const auto &vmInfo = vmInfos[vmIdToIndex[command.vmId]];
				uninstall(serversUsed[insId[0]][insId[1]], insId, vmInfo);

				auto &serIds = serverIndexToVmId[insId[0]][getNodeId(insId[2])][insId[1]];
				serIds.erase(std::find(serIds.begin(), serIds.end(), command.vmId));
			}
		}

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

		// auto has = [&](const std::string &s, unsigned long long &h) {
		// 	const unsigned long long base = 331;
		// 	for (const auto &ch : s) {
		// 		h = h * base + ch;
		// 	}
		// };
		// auto hasAll = [&](unsigned long long &h) {
		// 	for (const auto &server : serverInfos) {
		// 		has(server.to_string(), h);
		// 	}
		// 	for (const auto &vm : vmInfos) {
		// 		has(vm.to_string(), h);
		// 	}
		// 	for (const auto &cmd : commands) {
		// 		has(cmd.to_string(), h);
		// 	}
		// };
		
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
		std::cin >> dayNum >> previewNum;
		std::cin.ignore();
		commands.resize(dayNum);
		for (int i = 0; i < previewNum; ++i) {
			readOneDay(i);
		}
		#endif

		outputBuffer.clear();

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
