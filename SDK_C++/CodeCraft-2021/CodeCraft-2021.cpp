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
			// double cpuRatio = cpuUsed / (0.05 + cpuCores[0] + cpuCores[1]);
			// double memoryRatio = memoryUsed / (0.05 + memorySize[0] + memorySize[1]);
			// return cpuRatio;

			// double cpuRatio[2] = {
			// 	cpuUsed[0] / (cpuTotal * 0.5), 
			// 	cpuUsed[1] / (cpuTotal * 0.5)
			// };
			// double memoryRatio[2] = {
			// 	memoryUsed[0] / (memoryTotal * 0.5), 
			// 	memoryUsed[1] / (memoryTotal * 0.5)
			// };
			// double serverK = cpuTotal / (0.05 + memoryTotal);
			// double vmK[2] = {
			// 	cpuUsed[0] / (0.05 + memoryUsed[0]), 
			// 	cpuUsed[1] / (0.05 + memoryUsed[1])
			// };
			// double ratio[2] = {
			// 	std::max(serverK / vmK[0], vmK[0] / serverK),
			// 	std::max(serverK / vmK[1], vmK[1] / serverK)
			// };
			// return -levelCoef * static_cast<int>(std::max(ratio[0], ratio[1]) / acceptRange)
			// 	+ cpuRatio[0] + cpuRatio[1];

			// double cpuRatio = static_cast<double>(cpuUsed[0] + cpuUsed[1]) / cpuTotal;
			// double memoryRatio = static_cast<double>(memoryUsed[0] + memoryUsed[1]) / memoryTotal;
			// double cpuVsMemory = cpuRatio / (memoryRatio + 0.05);
			// cpuVsMemory = cpuVsMemory > 1.0 ? cpuVsMemory : 1.0 / cpuVsMemory;
			double serverK = cpuTotal / (0.05 + memoryTotal);
			double vmK = (cpuUsed[0] + cpuUsed[1]) / (0.05 + memoryUsed[0] + memoryUsed[1]);
			double ratio = std::max(serverK / vmK, vmK / serverK);
			return -(levelCoef * std::floor(ratio / acceptRange));
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

	std::vector<int> serverInfosHasId;
	std::vector<int> serversUsedHasId[2];
	std::vector<ServerInfo> serversUsed[2];

	std::unordered_map<int, int> vmIdToIndex;
	std::unordered_map<int, std::vector<int>> installId;
	std::vector<std::vector<int>> serverIndexToVmId[2];

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

	inline double calF1(const ServerInfo &server, int nodeId, int vmCpu, int vmMemory) {
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

	inline double calF2(const ServerInfo &server, int vmCpu, int vmMemory) {
		int serverCpu = server.cpuCores[0];
		int serverMemory = server.memorySize[0];
		if (serverCpu == 0 || serverMemory == 0) {
			return oo;
		}
		double serverK = static_cast<double>(serverCpu) / serverMemory;
		double vmK = static_cast<double>(vmCpu) / vmMemory;
		double ratio = std::max(serverK / vmK, vmK / serverK);
		return levelCoef * std::floor(ratio / acceptRange)
			+ serverK * (serverCpu - vmCpu) + 1.0 / serverK * (serverMemory - vmMemory);
	}

	std::pair<int, int> bestFit1(const std::vector<ServerInfo> &servers, 
		const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
		double fmn = oo;
		std::pair<int, int> ret(-1, -1);
		for (size_t i = 0; i < servers.size(); ++i) {
			int index = serversHasId[i];
			if (index == -1) continue;
			const auto &server = servers[index];
			double fval0 = calF1(server, 0, cpuCores, memorySize);
			double fval1 = calF1(server, 1, cpuCores, memorySize);
			if (server.cpuCores[0] >= cpuCores && server.memorySize[0] >= memorySize
				&& fval0 + EPS < fmn) {
				fmn = fval0;
				ret = std::make_pair(index, 0);
			}
			if (server.cpuCores[1] >= cpuCores && server.memorySize[1] >= memorySize
				&& fval1 + EPS < fmn) {
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
		for (size_t i = 0; i < servers.size(); ++i) {
			int index = serversHasId[i];
			if (index == -1) continue;
			const auto &server = servers[index];
			double fval = calF2(server, cpuCores, memorySize);
			if (server.cpuCores[0] >= cpuCores && server.cpuCores[1] >= cpuCores
				&& server.memorySize[0] >= memorySize && server.memorySize[1] >= memorySize
				&& fval + EPS < fmn) {
				ret.first = index;
				fmn = fval;
			}
		}
		return ret;
	}

	void solveOneDay(const std::vector<Command> &commands) {

		std::function<std::pair<int, int>
			(const std::vector<ServerInfo>&, const std::vector<int>&, int, int)> policys[2] = 
		{
			std::bind(&Solution::bestFit1, this, std::placeholders::_1, 
				std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), 
			std::bind(&Solution::bestFit2, this, std::placeholders::_1, 
				std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		};

		std::map<int, int> buyCnt;
		std::vector<std::vector<int>> ansId;
		std::vector<std::vector<int>> ansMigrate;

		int migrateLim = 3 * vmResNum / 100;

		std::vector<ServerInfo> serversReserved[2]{serversUsed[0], serversUsed[1]};
		decltype(serverIndexToVmId) newVmId;
		newVmId[0].resize(serverIndexToVmId[0].size());
		newVmId[1].resize(serverIndexToVmId[1].size());

		for (const auto &command : commands) {

			if (++tim % shuffleFreq == 0) {
				// 
			}

			if (command.commandType) { // add

				++vmResNum;

				const VmInfo &vmInfo = vmInfos[command.vmIndex];
				auto policy = policys[vmInfo.isDouble];
				auto selId = policy(serversUsed[vmInfo.isDouble], serversUsedHasId[vmInfo.isDouble], 
					vmInfo.cpuCores, vmInfo.memorySize);

				if (selId.first == -1) {
					auto buyId = policy(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize);
					assert(buyId.first != -1);
					selId = std::make_pair(serversUsed[vmInfo.isDouble].size(), buyId.second);
					serversUsedHasId[vmInfo.isDouble].emplace_back(selId.first);
					serversUsed[vmInfo.isDouble].emplace_back(serverInfos[buyId.first]);
					++buyCnt[buyId.first];
					serversUsed[vmInfo.isDouble].back().serverId = buyId.first; // 需要重编号

					serversReserved[vmInfo.isDouble].emplace_back(serversUsed[vmInfo.isDouble].back());

					newVmId[vmInfo.isDouble].emplace_back();
				}

				serversUsed[vmInfo.isDouble][selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);
				serversReserved[vmInfo.isDouble][selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);
				newVmId[vmInfo.isDouble][selId.first].emplace_back(command.vmId);

				installId[command.vmId] = {vmInfo.isDouble, selId.first, selId.second};
				ansId.push_back({vmInfo.isDouble, selId.first, selId.second});

				vmIdToIndex[command.vmId] = command.vmIndex;
			}
			else { // del

				--vmResNum;

				auto insId = installId[command.vmId];
				const auto &vmInfo = vmInfos[vmIdToIndex[command.vmId]];
				serversUsed[insId[0]][insId[1]].uninstall(insId[2], vmInfo.cpuCores, vmInfo.memorySize);

				if (std::find(newVmId[insId[0]][insId[1]].cbegin(), newVmId[insId[0]][insId[1]].cend(), command.vmId) 
					!= newVmId[insId[0]][insId[1]].cend()) {

					newVmId[insId[0]][insId[1]].erase(
						std::find(newVmId[insId[0]][insId[1]].begin(), newVmId[insId[0]][insId[1]].end(), command.vmId));
				}
				else {
					serverIndexToVmId[insId[0]][insId[1]].erase(
						std::find(serverIndexToVmId[insId[0]][insId[1]].begin(), serverIndexToVmId[insId[0]][insId[1]].end(), command.vmId));
				}
			}
		}

		serverIndexToVmId[0].resize(newVmId[0].size());
		serverIndexToVmId[1].resize(newVmId[1].size());

		struct MigrateRecord {
			int vmId;
			std::vector<int> fromId, toId;
		};

		std::stack<MigrateRecord> migrateRecords;

		auto doMigrate = [&](int vmId, std::vector<int> fromId, std::vector<int> toId) {
			migrateRecords.push({vmId, fromId, toId});
			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
			installId[vmId] = toId;
			serversUsed[fromId[0]][fromId[1]].uninstall(fromId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversUsed[toId[0]][toId[1]].install(toId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversReserved[toId[0]][toId[1]].install(toId[2], vmInfo.cpuCores, vmInfo.memorySize);
		};

		auto undoMigrate = [&]() {
			auto migrateRecord = migrateRecords.top();
			migrateRecords.pop();
			int vmId = migrateRecord.vmId;
			const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
			auto fromId = migrateRecord.fromId;
			auto toId = migrateRecord.toId;
			installId[vmId] = fromId;
			serversUsed[fromId[0]][fromId[1]].install(fromId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversUsed[toId[0]][toId[1]].uninstall(toId[2], vmInfo.cpuCores, vmInfo.memorySize);
			serversReserved[toId[0]][toId[1]].uninstall(toId[2], vmInfo.cpuCores, vmInfo.memorySize);
		};

		auto rollbackMigrate = [&](size_t tim) {
			while (migrateRecords.size() > tim) {
				undoMigrate();
			}
		};

		
		std::stable_sort(serversUsedHasId[0].begin(), serversUsedHasId[0].end(), 
			[&](int x, int y) -> bool {
				return newVmId[0][x].empty() != newVmId[0][y].empty() ? newVmId[0][x].empty() > newVmId[0][y].empty()
					: serversUsed[0][x].calUsedRatio() < serversUsed[0][y].calUsedRatio();
			});

		std::stable_sort(serversUsedHasId[1].begin(), serversUsedHasId[1].end(), 
			[&](int x, int y) -> bool {
				return newVmId[1][x].empty() != newVmId[1][y].empty() ? newVmId[1][x].empty() > newVmId[1][y].empty()
					: serversUsed[1][x].calUsedRatio() < serversUsed[1][y].calUsedRatio();
			});

		size_t serverIndexLim[2] = {serversUsed[0].size(), serversUsed[1].size()};
		for (size_t i = 0; i < 2; ++i) {
			for (size_t j = 0; j < serversUsedHasId[i].size(); ++j) {
				int index = serversUsedHasId[i][j];
				if (!newVmId[i][index].empty()) {
					serverIndexLim[i] = j;
					break;
				}
			}
		}

		std::vector<std::pair<int, int>> readyMigrate[2], successMigrate[2], failMigrate[2];
		std::set<int> failMigrateCnt[2];
		int migrateStep[2] = {migrateLim, migrateLim};
		int migrateRound[2] = {0, 0};
		int isDouble = std::rand() % 2;
		// int isDouble = 0;

		int cnt = 0;

		while (migrateLim > 0 && (migrateStep[0] + migrateStep[1]) > 0) {
			if (++cnt == 2) break;
			if (!migrateRound[isDouble]) {
				failMigrateCnt[isDouble].clear();
			}
			readyMigrate[isDouble].clear();
			migrateStep[isDouble] = std::min(migrateStep[isDouble], migrateLim);
			int migrateCnt = 0;
			for (size_t i = 0; i < serverIndexLim[isDouble]; ++i) {
				int index = serversUsedHasId[isDouble][i];
				if (index == -1) continue;
				// if (migrateRound[isDouble] && failMigrateCnt[isDouble].count(index)) continue;
				if (migrateCnt + int(serverIndexToVmId[isDouble][index].size()) > migrateStep[isDouble]) {
					break;
				}
				serversUsedHasId[isDouble][i] = -1;
				migrateCnt += serverIndexToVmId[isDouble][index].size();
				readyMigrate[isDouble].emplace_back(i, index);
			}

			int migrateSuccess = 0;
			failMigrate[isDouble].clear();
			for (const auto &e : readyMigrate[isDouble]) {
				size_t tim = migrateRecords.size();
				migrateSuccess += serverIndexToVmId[isDouble][e.second].size();
				bool isSuccess = true;
				for (const auto &vmId : serverIndexToVmId[isDouble][e.second]) {
					const auto &vmInfo = vmInfos[vmIdToIndex[vmId]];
					auto policy = policys[vmInfo.isDouble];
					auto toId = policy(serversReserved[isDouble], serversUsedHasId[isDouble], 
						vmInfo.cpuCores, vmInfo.memorySize);
					if (toId.first == -1) {
						rollbackMigrate(tim);
						failMigrate[isDouble].emplace_back(e);
						migrateSuccess -= serverIndexToVmId[isDouble][e.second].size();
						isSuccess = false;
						failMigrateCnt[isDouble].emplace(e.second);
						break;
					}
					auto fromId = installId[vmId];
					doMigrate(vmId, fromId, std::vector<int>{isDouble, toId.first, toId.second});
				}
				if (isSuccess) {
					successMigrate[isDouble].emplace_back(e);
				}
			}

			for (const auto &e : failMigrate[isDouble]) {
				serversUsedHasId[isDouble][e.first] = e.second;
			}

			// std::cerr << isDouble << ": " << migrateStep[isDouble] << " " << migrateSuccess 
			//  << " " << readyMigrate[isDouble].size() << " " << failMigrate[isDouble].size() << std::endl;

			migrateLim -= migrateSuccess;
			totalMigration += migrateSuccess;
			if (migrateSuccess == 0) {
				migrateStep[isDouble] /= 2;
			}
			
			while (!migrateRecords.empty()) {
				auto migrateRecord = migrateRecords.top();
				migrateRecords.pop();
				int vmId = migrateRecord.vmId;
				auto fromId = migrateRecord.fromId;
				auto toId = migrateRecord.toId;
				serverIndexToVmId[fromId[0]][fromId[1]].erase(
					std::find(serverIndexToVmId[fromId[0]][fromId[1]].begin(), serverIndexToVmId[fromId[0]][fromId[1]].end(), vmId));
				serverIndexToVmId[toId[0]][toId[1]].emplace_back(vmId);
				ansMigrate.push_back({vmId, toId[0], toId[1], toId[2]});
			}

			migrateRound[isDouble] ^= 1;

			isDouble ^= 1;
		}
		// std::cerr << std::endl;

		for (int isDouble = 0; isDouble < 2; ++isDouble) {
			for (const auto &e : successMigrate[isDouble]) {
				serversUsedHasId[isDouble][e.first] = e.second;
			}
		}

		auto mergeVmId = [&](size_t isDouble, size_t index) {
			for (const auto &v : newVmId[isDouble][index]) {
				serverIndexToVmId[isDouble][index].emplace_back(v);
			}
			newVmId[isDouble][index].clear();
		};
		for (size_t i = 0; i < 2; ++i) {
			for (size_t j = 0; j < serversUsed[i].size(); ++j) {
				mergeVmId(i, j);
			}
		}

		for (const auto &it : buyCnt) {
			totalCost += it.second * 1ll * serverInfos[it.first].serverCost;
		}

		for (size_t i = 0; i < 2; ++i) {
			for (size_t j = 0; j < serversUsed[i].size(); ++j) {
				if (serverIndexToVmId[i][j].empty()) continue;
				totalCost += serversUsed[i][j].powerCost;
			}
		}

		outputBuffer += "(purchase, " + std::to_string(buyCnt.size()) + ")\n";
		int preCnt = 0;
		for (auto &it : buyCnt) {
			outputBuffer += "(" + serverInfos[it.first].serverType + ", " + std::to_string(it.second) + ")\n";
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
				outputBuffer += "(" + std::to_string(vc[0]) + ", " + std::to_string(serversUsed[vc[1]][vc[2]].serverId) + ")\n";
			}
			else {
				outputBuffer += "(" + std::to_string(vc[0]) + ", " + std::to_string(serversUsed[vc[1]][vc[2]].serverId) 
					+ ", " + (vc[3] ? "B" : "A") + ")\n";
			}
		}

		for (const auto &id : ansId) {
			if (id[2] != -1) {
				outputBuffer += "(" + std::to_string(serversUsed[id[0]][id[1]].serverId) 
					+ ", " + (id[2] ? "B" : "A") + ")\n";
			}
			else {
				outputBuffer += "(" + std::to_string(serversUsed[id[0]][id[1]].serverId) + ")\n";
			}
		}

		auto has = [&](const std::string &s, unsigned long long &h) {
			const unsigned long long base = 331;
			for (const auto &ch : s) {
				h = h * base + ch;
			}
		};
		auto hasAll = [&](unsigned long long &h) {
			for (const auto &server : serverInfos) {
				has(server.to_string(), h);
			}
			for (const auto &vm : vmInfos) {
				has(vm.to_string(), h);
			}
			for (const auto &cmd : commands) {
				has(cmd.to_string(), h);
			}
		};

		// static unsigned long long fuckHash = 0;
		// has(outputBuffer, fuckHash); 

		// if (curDay == dayNum - 1) {
			// std::cerr << "hash: " << fuckHash << std::endl;
			// unsigned long long h = 0;
			// hasAll(h);
			// assert(h == 15076945205182418425ull || h == 11115914098846772032ull);
			// assert(fuckHash == 7867171164944083644ull || fuckHash == 11733483978821489280ull);
			// if (h != 15076945205182418425ull && h != 11115914098846772032ull) {
			// 	while (true);
			// }
		// }
		// 15076945205182418425ull
		// 11115914098846772032ull

		// 7867171164944083644ull
		// 11733483978821489280ull

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

		serverInfosHasId.clear();
		serversUsedHasId[0].clear();
		serversUsedHasId[1].clear();
		serversUsed[0].clear();
		serversUsed[1].clear();
		vmIdToIndex.clear();
		installId.clear();
		serverIndexToVmId[0].clear();
		serverIndexToVmId[1].clear();
		serversUsedNum[0] = 0;
		serversUsedNum[1] = 0;
		vmResNum = 0;
		totalMigration = 0;
		canMigrateTotal = 0;
		totalCost = 0;
		tim = 0;

		serverInfosHasId.resize(serverInfos.size());
		std::iota(serverInfosHasId.begin(), serverInfosHasId.end(), 0);

		for (int i = 0; i < dayNum; ++i) {
			curDay = i;
			solveOneDay(commands[i]);
			#ifdef ON_LINE
			if (previewNum + i < dayNum) {
				readOneDay(previewNum + i);
			}
			#endif
			// if (i == dayNum - 2 && totalMigration < 40000) {
			// 	while (true);
			// }
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
