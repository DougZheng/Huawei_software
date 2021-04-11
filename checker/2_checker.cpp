#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <ctime>
#include <random>
#include <cassert>

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

    bool isOverload() {
        return cpuCores[0] < 0 || cpuCores[1] < 0 || memorySize[0] < 0 || memorySize[1] < 0;
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


std::unordered_map<std::string, int> serverTypeToIndex;
std::vector<ServerInfo> serverInfos;

std::unordered_map<std::string, int> vmTypeToIndex;
std::vector<VmInfo> vmInfos;

std::vector<std::vector<Command>> commands;


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
        serverTypeToIndex[serverInfo.serverType] = i;
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
    int K; std::cin >> K;
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

std::vector<ServerInfo> serversUsed;
std::unordered_map<int, int> vmIdToIndex;

std::unordered_map<int, std::pair<int, int>> installId;
std::map<int, std::set<int>> serverIndexToVmId;

long long totalCost;
int totalMigration;
int totalVm;

void init() {

    serverTypeToIndex.clear();
    serverInfos.clear();
    vmTypeToIndex.clear();
    vmInfos.clear();
    commands.clear();

    serversUsed.clear();
    vmIdToIndex.clear();
    installId.clear();
    serverIndexToVmId.clear();

    totalCost = 0;
    totalMigration = 0;
    totalVm = 0;
}

long long calCost() {
    long long cost = 0;
    for (size_t i = 0; i < serversUsed.size(); ++i) {
        if (serverIndexToVmId[i].empty()) continue;
        cost += serversUsed[i].powerCost;
    }
    return cost;
}

std::pair<long long, int> judge(std::string inFile, std::string outFile) {

    int migrateExNum = 1;

    std::ifstream in(inFile);
    std::ifstream out(outFile);
    
    init();

    std::freopen(inFile.c_str(), "r", stdin);
    read();
    std::fclose(stdin);

    std::string ss[3];
    for (size_t day = 0; day < commands.size(); ++day) {

        out >> ss[0] >> ss[1];
        assert(ss[0] == "(purchase,");
        ss[1].pop_back();
        int purchaseNum = std::stoi(ss[1]);
        for (int i = 0; i < purchaseNum; ++i) {
            out >> ss[0] >> ss[1];
            ss[0].pop_back();
            ss[1].pop_back();
            std::string serverType = ss[0].substr(1);
            int serverNum = std::stoi(ss[1]);
            assert(serverNum > 0 && serverNum <= 100000);
            totalCost += serverNum * 1ll * serverInfos[serverTypeToIndex[serverType]].serverCost;
            while (serverNum--) {
                serversUsed.emplace_back(serverInfos[serverTypeToIndex[serverType]]);
                serversUsed.back().serverId = serversUsed.size() - 1;
            }
            assert(serversUsed.size() <= 100000u);
        }

        out >> ss[0] >> ss[1];
        assert(ss[0] == "(migration,");
        ss[1].pop_back();
        int migrateNum = std::stoi(ss[1]);
        int migrateLim = totalVm * 3 / 100;
        if (migrateNum > migrateLim) {
            --migrateExNum;
        }
        assert(migrateExNum >= 0);
        totalMigration += migrateNum;
        for (int i = 0; i < migrateNum; ++i) {
            out >> ss[0] >> ss[1];
            ss[0].pop_back();
            int vmId = std::stoi(ss[0].substr(1));
            std::pair<int, int> insId(-1, -1);
            char ch = ss[1].back();
            ss[1].pop_back();
            insId.first = std::stoi(ss[1]);
            if (ch == ',') {
                out >> ss[2];
                insId.second = ss[2][0] - 'A';
                assert(insId.second == 0 || insId.second == 1);
            }
            assert(installId.count(vmId));
            auto oldInsId = installId[vmId];
            auto vmInfo = vmInfos[vmIdToIndex[vmId]];
            assert((oldInsId.second == -1) == (insId.second == -1));
            assert(insId.first < serversUsed.size());
            if (insId.second != -1) {
                serversUsed[oldInsId.first].uninstall(oldInsId.second, vmInfo.cpuCores, vmInfo.memorySize);
                serversUsed[insId.first].install(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
                assert(!serversUsed[insId.first].isOverload());
            }
            else {
                serversUsed[oldInsId.first].uninstall(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                serversUsed[oldInsId.first].uninstall(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                serversUsed[insId.first].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                serversUsed[insId.first].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                assert(!serversUsed[insId.first].isOverload());
            }
            installId[vmId] = insId;
            serverIndexToVmId[oldInsId.first].erase(vmId);
            serverIndexToVmId[insId.first].emplace(vmId);
        }

        auto &cmds = commands[day];
        for (auto &cmd : cmds) {
            if (cmd.commandType) {
                std::pair<int, int> insId(-1, -1);
                out >> ss[0];
                if (ss[0].back() == ',') {
                    out >> ss[1];
                    insId.second = ss[1][0] - 'A';
                    assert(insId.second == 0 || insId.second == 1);
                }
                ss[0].pop_back();
                insId.first = std::stoi(ss[0].substr(1));
                assert(insId.first < serversUsed.size());
                auto vmInfo = vmInfos[cmd.vmType];
                assert(vmInfo.isDouble == (insId.second == -1));
                if (insId.second != -1) {
                    serversUsed[insId.first].install(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
                    assert(!serversUsed[insId.first].isOverload());
                }
                else {
                    serversUsed[insId.first].install(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                    serversUsed[insId.first].install(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                    assert(!serversUsed[insId.first].isOverload());
                }
                installId[cmd.vmId] = insId;
                vmIdToIndex[cmd.vmId] = cmd.vmType;
                serverIndexToVmId[insId.first].emplace(cmd.vmId);

                ++totalVm;
            }
            else {
                auto insId = installId[cmd.vmId];
                auto vmInfo = vmInfos[vmIdToIndex[cmd.vmId]];
                if (insId.second != -1) {
                    serversUsed[insId.first].uninstall(insId.second, vmInfo.cpuCores, vmInfo.memorySize);
                }
                else {
                    serversUsed[insId.first].uninstall(0, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                    serversUsed[insId.first].uninstall(1, vmInfo.cpuCores / 2, vmInfo.memorySize / 2);
                }
                installId.erase(cmd.vmId);
                serverIndexToVmId[insId.first].erase(cmd.vmId);

                --totalVm;
            }
        }

        totalCost += calCost();
    }

    return std::make_pair(totalCost, totalMigration);
}

int main() {

    auto ans1 = judge("../2_data/training-1.txt", "../2_data/training-1.out");
    auto ans2 = judge("../2_data/training-2.txt", "../2_data/training-2.out");
    std::cerr << "\ndataset1:" << std::endl;
    std::cerr << "cost: " << ans1.first << std::endl;
    std::cerr << "migration: " << ans1.second << std::endl;
    std::cerr << "\ndataset2:" << std::endl;
    std::cerr << "cost: " << ans2.first << std::endl;
    std::cerr << "migration: " << ans2.second << std::endl;
    std::cerr << "\ntotal cost: " << ans1.first + ans2.first << std::endl;
    std::cerr << "total migration: " << ans1.second + ans2.second << std::endl;
    return 0;
}