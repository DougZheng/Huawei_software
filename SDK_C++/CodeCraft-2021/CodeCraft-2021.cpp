#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
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

int serversUsedNum;

std::vector<int> serverInfosHasId;
std::vector<int> serversUsedHasId;
std::mt19937 rnd(time(0));
int tim;
int shuffleFreq = 50;

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

std::pair<int, int> bestFit1(const std::vector<ServerInfo> &servers, 
	const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
	auto cal = [](int cpuCores0, int cpuCores1, int cpuCores) {
		return std::max(cpuCores0, cpuCores1) - cpuCores;
	};
	int fmn = INT_MAX;
	std::pair<int, int> ret(-1, -1);
	for (int i = 0; i < servers.size(); ++i) {
		int index = serversHasId[i];
		const auto &server = servers[index];
		int fval0 = cal(server.cpuCores[0], server.cpuCores[1], cpuCores);
		int fval1 = cal(server.cpuCores[0], server.cpuCores[1], cpuCores);
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

int bestFit2(const std::vector<ServerInfo> &servers, 
	const std::vector<int> &serversHasId, int cpuCores, int memorySize) {
	cpuCores /= 2;
	memorySize /= 2;
	auto cal = [](int cpuCores0, int cpuCores1, int cpuCores) {
		return std::max(cpuCores0, cpuCores1) - cpuCores;
	};
	int fmn = INT_MAX, ret = -1;
	for (int i = 0; i < servers.size(); ++i) {
		int index = serversHasId[i];
		const auto &server = servers[index];
		int fval = cal(server.cpuCores[0], server.cpuCores[1], cpuCores);
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
	for (const auto &command : commands) {

		if (++tim % shuffleFreq == 0) {
			std::shuffle(serversUsedHasId.begin(), serversUsedHasId.end(), rnd);
			std::shuffle(serverInfosHasId.begin(), serverInfosHasId.end(), rnd);
		}

		if (command.commandType) { // add
			const VmInfo &vmInfo = vmInfos[command.vmType];
			if (!vmInfo.isDouble) {
				auto selId = firstFit1(serversUsed, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
				if (selId.first == -1) {
					auto buyId = firstFit1(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize);
					assert(buyId.first != -1);
					selId = std::make_pair(serversUsed.size(), buyId.second);
					serversUsedHasId.emplace_back(selId.first);
					serversUsed.emplace_back(serverInfos[buyId.first]);
					++buyCnt[buyId.first];
					serversUsed.back().serverId = buyId.first; // 需要重编号
				}
				serversUsed[selId.first].install(selId.second, vmInfo.cpuCores, vmInfo.memorySize);
				installId[command.vmId] = selId;
				ansId.emplace_back(selId);
			}
			else {
				auto selId = firstFit2(serversUsed, serversUsedHasId, vmInfo.cpuCores, vmInfo.memorySize);
				if (selId == -1) {
					auto buyId = firstFit2(serverInfos, serverInfosHasId, vmInfo.cpuCores, vmInfo.memorySize);
					assert(buyId != -1);
					selId = serversUsed.size();
					serversUsedHasId.emplace_back(selId);
					serversUsed.emplace_back(serverInfos[buyId]);
					++buyCnt[buyId];
					serversUsed.back().serverId = buyId; // 需要重编号
				}
				serversUsed[selId].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				serversUsed[selId].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				installId[command.vmId] = std::make_pair(selId, -1);
				ansId.emplace_back(std::make_pair(selId, -1));
			}
			vmIdToIndex[command.vmId] = command.vmType;
		}
		else { // del
			auto insId = installId[command.vmId];
			auto vmInfo = vmInfos[vmIdToIndex[command.vmId]];
			if (insId.second != -1) {
				serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
			}
			else {
				serversUsed[insId.first].uninstall(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
				serversUsed[insId.first].uninstall(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);	
			}
		}
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

	std::cout << "(migration, 0)" << std::endl;
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
	std::fill(serverInfosHasId.begin(), serverInfosHasId.end(), 0);

	for (const auto &cmd : commands) {
		solve(cmd);
	}

	return 0;
}
