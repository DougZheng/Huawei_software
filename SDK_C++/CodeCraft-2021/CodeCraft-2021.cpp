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
#include <climits>
#include <cassert>

// #define DEBUG

struct ServerInfo {

	friend std::istream &operator>>(std::istream &is, ServerInfo &serverInfo);
	friend std::ostream &operator<<(std::ostream &os, const ServerInfo &serverInfo);

	std::string serverType;
	int cpuCores[2];
	int memorySize[2];
	int serverCost;
	int powerCost;
	int serverId = -1;

	void install(int nodeId, int cpu, int memory) {
		cpuCores[nodeId] -= cpu;
		memorySize[nodeId] -= memory;
	}

	void uninstall(int nodeId, int cpu, int memory) {
		cpuCores[nodeId] += cpu;
		memorySize[nodeId] += memory;
	}

	int calPriors() const {
		return serverCost / (cpuCores[0] + cpuCores[1]); 
	}
};

struct VmInfo {

	friend std::istream &operator>>(std::istream &is, VmInfo &vmInfo);
	friend std::ostream &operator<<(std::ostream &os, const VmInfo &vmInfo);

	std::string vmType;
	int cpuCores;
	int memorySize;
	int isDouble;
};

struct Command {

	friend std::istream &operator>>(std::istream &is, Command &command);
	friend std::ostream &operator<<(std::ostream &os, const Command &command);

	int commandType;
	int vmType;
	int vmId;
};


std::vector<ServerInfo> serverInfos;

std::unordered_map<std::string, int> vmTypeToIndex;
std::vector<VmInfo> vmInfos;

std::vector<std::vector<Command>> commands;

std::vector<ServerInfo> serversUsed;
std::unordered_map<int, int> vmIdToIndex;

std::unordered_map<int, std::pair<int, int>> installId;
std::map<int, std::set<int>> serverIndexToVmId;

int serversUsedNum;
int vmResNum;

int migrateTotal = 0;

std::vector<int> serverInfosHasId;
std::vector<int> serversUsedHasId;
std::mt19937 rnd(time(0));
int tim;
constexpr int shuffleFreq = 50;

std::istream &operator>>(std::istream &is, ServerInfo &serverInfo) {
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

std::ostream &operator<<(std::ostream &os, const ServerInfo &serverInfo) {
	os << "(" << serverInfo.serverType << ", " 
		<< serverInfo.cpuCores[0] << "+" << serverInfo.cpuCores[1] << ", "
		<< serverInfo.memorySize[0] << "+" << serverInfo.memorySize[1] << ", "
		<< serverInfo.serverCost << ", " << serverInfo.powerCost << ")" << std::endl;
	return os;
}

std::istream &operator>>(std::istream &is, VmInfo &vmInfo) {
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

std::ostream &operator<<(std::ostream &os, const VmInfo &vmInfo) {
	os << "(" << vmInfo.vmType << ", "
		<< vmInfo.cpuCores << ", " << vmInfo.memorySize << ", "
		<< vmInfo.isDouble << ")" << std::endl;
	return os;
}

std::istream &operator>>(std::istream &is, Command &command) {
	std::string commandType, vmType, vmId;
	is >> commandType;
	command.commandType = 0;
	if (commandType[1] == 'a') {
		command.commandType = 1;
		is >> vmType;
		vmType.pop_back();
		command.vmType = vmTypeToIndex[vmType];
	}
	is >> vmId;
	vmId.pop_back();
	command.vmId = std::stoi(vmId);
	return is;
}

std::ostream &operator<<(std::ostream &os, const Command &command) {
	os << "(" << (command.commandType ? "add" : "del") << ", "
		<< (command.commandType ? std::to_string(command.vmType) + ", " : "")
		<< command.vmId << ")" << std::endl;
	return os;
}

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

std::pair<int, int> firstFit1(const std::vector<ServerInfo> &servers, 
	const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
	for (int i = 0; i < servers.size(); ++i) {
		int index = serversHasId[i];
		const auto &server = servers[index];
		if (server.cpuCores[0] >= cpuCores && server.memorySize[0] >= memorySize) {
			return std::make_pair(index, 0);
		}
		else if (server.cpuCores[1] >= cpuCores && server.memorySize[1] >= memorySize) {
			return std::make_pair(index, 1);
		}
	}
	return std::make_pair(-1, -1);
}

int firstFit2(const std::vector<ServerInfo> &servers, 
	const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
	cpuCores /= 2;
	memorySize /= 2;
	for (int i = 0; i < servers.size(); ++i) {
		int index = serversHasId[i];
		const auto &server = servers[index];
		if (server.cpuCores[0] >= cpuCores && server.cpuCores[1] >= cpuCores
			&& server.memorySize[0] >= memorySize && server.memorySize[1] >= memorySize) {
			return index;	
		}
	}
	return -1;
}


constexpr double oo = 1e200;
constexpr double acceptRange = 1.5;
inline double calF(int serverCpu, int serverMemory, int vmCpu, int vmMemory) {
	if (serverCpu == 0 || serverMemory == 0) {
		return oo;
	}
	double serverK = static_cast<double>(serverCpu) / serverMemory;
	double vmK = static_cast<double>(vmCpu) / vmMemory;
	double ratio = serverK > vmK ? serverK / vmK : vmK / serverK;
	return 500.0 * static_cast<int>(ratio / acceptRange) + (serverCpu - vmCpu);
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
		// std::cout << i << " " << index << " " << fval0 << " " << fval1 << std::endl;
	}
	return ret;
}

int bestFit2(const std::vector<ServerInfo> &servers, 
	const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
	cpuCores /= 2;
	memorySize /= 2;
	double fmn = oo, ret = -1;
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
			ret = index; 	
			fmn = fval;
		}
	}
	return ret;
}

void solve(const std::vector<Command> &commands) {
	std::map<int, int> buyCnt;
	std::vector<std::pair<int, int>> ansId;
	std::vector<std::vector<int>> ansMigrate;

	int migrateNum = 5 * vmResNum / 1000;

	std::vector<ServerInfo> serversReserved = serversUsed;
	decltype(serverIndexToVmId) newVmId;

	for (const auto &command : commands) {

		if (++tim % shuffleFreq == 0) {
			// std::shuffle(serversUsedHasId.begin(), serversUsedHasId.end(), rnd);
			// std::shuffle(serverInfosHasId.begin(), serverInfosHasId.end(), rnd);
			// std::sort(serversUsedHasId.begin(), serversUsedHasId.end(), 
			// 	[](int x, int y) {
			// 		int fx = std::min(serversUsed[x].cpuCores[0], serversUsed[x].cpuCores[1]);
			// 		int fy = std::min(serversUsed[y].cpuCores[0], serversUsed[y].cpuCores[1]);
			// 		return fx < fy;
			// 	});
		}
		// if (tim == 2) exit(1);

		if (command.commandType) { // add

			++vmResNum;

			const VmInfo &vmInfo = vmInfos[command.vmType];
			if (!vmInfo.isDouble) {
				auto selId = bestFit1(serversUsed, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
				if (selId.first == -1) {
					auto buyId = bestFit1(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize);
					// for (double ratio = 1.1; buyId.first == -1; ratio += 0.1) {
					// 	buyId = bestFit1(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize, ratio);
					// }
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

				// serverIndexToVmId[selId.first].emplace(command.vmId);
				newVmId[selId.first].emplace(command.vmId);
			}
			else {
				auto selId = bestFit2(serversUsed, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
				if (selId == -1) {
					auto buyId = bestFit2(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize);
					// for (double ratio = 1.1; buyId == -1; ratio += 0.1) {
					// 	buyId = bestFit2(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize, ratio);
					// }
					assert(buyId != -1);
					selId = serversUsed.size();
					serversUsedHasId.emplace_back(selId);
					serversUsed.emplace_back(serverInfos[buyId]);
					++buyCnt[buyId];
					serversUsed.back().serverId = buyId; // 需要重编号

					serversReserved.emplace_back(serversUsed.back());
				}
				serversUsed[selId].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				serversUsed[selId].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				installId[command.vmId] = std::make_pair(selId, -1);
				ansId.emplace_back(std::make_pair(selId, -1));

				serversReserved[selId].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				serversReserved[selId].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);

				// serverIndexToVmId[selId].emplace(command.vmId);
				newVmId[selId].emplace(command.vmId);
			}
			
			vmIdToIndex[command.vmId] = command.vmType;
		}
		else { // del

			--vmResNum;

			auto insId = installId[command.vmId];
			auto vmInfo = vmInfos[vmIdToIndex[command.vmId]];
			if (insId.second != -1) {
				serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
			}
			else {
				serversUsed[insId.first].uninstall(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				serversUsed[insId.first].uninstall(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);	
			}

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
	for (int i = 0; i < serversUsedHasId.size(); ++i) {
		int index = serversUsedHasId[i];
		if (serverIndexToVmId[index].size() == 1 && newVmId[index].size() == 0) {
			migrateIndex.emplace_back(i, index);
			migrateVmId.emplace_back(*serverIndexToVmId[index].begin());
			serversUsedHasId[i] = -1;
			if (migrateIndex.size() >= migrateNum) break;
		}
		mergeVmId(index);
	}

	// for (int i = 0; i < migrateIndex.size(); ++i) {
	// 	auto &e = migrateIndex[i];
	// 	if (serverIndexToVmId[e.second].size() == 1 
	// 		&& migrateVmId[i] == *serverIndexToVmId[e.second].begin()) {
	// 		serversUsedHasId[e.first] = -1;
	// 		readyMigrate.emplace_back(e);
	// 	}
	// }

	int migrateCnt = 0;
	for (const auto &e : migrateIndex) {

		int vmId = *serverIndexToVmId[e.second].begin();
		auto insId = installId[vmId];
		auto vmInfo = vmInfos[vmIdToIndex[vmId]];

		if (insId.second != -1) {

			auto selId = bestFit1(serversReserved, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
			if (selId.first == -1) continue; 

			installId[vmId] = selId;

			serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
			serversUsed[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);

			serversReserved[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);

			ansMigrate.push_back({vmId, selId.first, selId.second});
			serverIndexToVmId[e.second].clear();

			++migrateCnt;
		}
		else {

			auto selId = bestFit2(serversReserved, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
			if (selId == -1) continue;

			installId[vmId] = std::make_pair(selId, -1);

			serversUsed[insId.first].uninstall(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
			serversUsed[insId.first].uninstall(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
			serversUsed[selId].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
			serversUsed[selId].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);

			serversReserved[selId].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
			serversReserved[selId].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);

			ansMigrate.push_back({vmId, selId});
			serverIndexToVmId[e.second].clear();

			++migrateCnt;
		}
	}

	migrateTotal += migrateCnt;

	for (const auto &e : migrateIndex) {
		serversUsedHasId[e.first] = e.second;
	}

	std::cout << "(purchase, " << buyCnt.size() << ")" << std::endl;
	int preCnt = 0;
	for (auto &it : buyCnt) {
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
		if (vc.size() == 2) {
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
}

int main() {

	#ifdef DEBUG
	std::string inPath = "../../data/training-1.txt";
	std::string outPath = "../../data/training-1.out";
	assert(std::freopen(inPath.c_str(), "r", stdin) != nullptr);
	assert(std::freopen(outPath.c_str(), "w", stdout) != nullptr);
	#endif

	read();

	serverInfosHasId.resize(serverInfos.size());
	std::iota(serverInfosHasId.begin(), serverInfosHasId.end(), 0);
	std::sort(serverInfosHasId.begin(), serverInfosHasId.end(), 
		[](int x, int y) {
			return serverInfos[x].calPriors() < serverInfos[y].calPriors();
		});

	for (const auto &cmd : commands) {
		solve(cmd);
	}

	#ifdef DEBUG
	std::cerr << "server number: " << serversUsed.size() << std::endl;
	std::cerr << "migration number: " << migrateTotal << std::endl;
	#endif
	return 0;
}
