// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>

#include <netdb.h>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>


#include "ClassDefinitions.h"

using namespace std;

vector<User> userDB; //uid,pass
vector<Group> groupsDB; //grp id,owner
map<string, Peer> peerList;
map<string, GroupPendingRequest> groupPendingRequests;
map<string, vector<FileProperties>> filesInGroup;
map<int, FileProperties> fileIndex;
vector<string> connectedClients;

int FILE_ID = 1;

void handlePeerCommunication(string ip, int port, int socketStatus, struct sockaddr_in6 peerAddress, vector<string> cmds)
{
	socklen_t addr_size = sizeof(struct sockaddr_in6);

	string LOGIN_ID = cmds.back();
	cmds.pop_back();

	string message;

	if (cmds[0] == "10")
	{
		User user(cmds[1], cmds[2]);
		bool flag = false;
		for (int i = 0; i < userDB.size(); i++)
		{
			if (userDB[i].userID == cmds[1])
			{
                message = "User already exists";
				flag = true;
				break;
			}
		}
		if (!flag)
		{
            message = "User added";
			userDB.push_back(user);
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "11")
	{
		Peer client(ip, port, cmds[1]);
		int i;
		for (i = 0; i < userDB.size(); i++)
		{
			if (userDB[i].userID == cmds[1] && userDB[i].password == cmds[2])
				break;
		}
		if (i == userDB.size())
		{
            message = "User does not exist/Wrong password";
		}
		else
		{
			if (peerList.count(cmds[1]) == 0)
			{
				LOGIN_ID = cmds[1];
				peerList[LOGIN_ID] = client;
                message = "Logging in with User ID " + cmds[1];
			}
			else
			{
                message = "Already logged in another instance";
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "20")
	{
		set<string> groupMembers; //add admin to Group
		groupMembers.insert(LOGIN_ID);
        Group newGroup(cmds[1], LOGIN_ID, groupMembers);
		bool flag = false;
		for (auto & group : groupsDB)
		{
			if (group.name == cmds[1])
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
            message = "Group already exists";
		}
		else
		{
			groupsDB.push_back(newGroup);

            message = "Group " + newGroup.name + " created with admin " + newGroup.adminUserID;
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "21")
	{
		int i;
		bool flag = false;
		for (i = 0; i < groupsDB.size(); i++)
		{
			if (groupsDB[i].name == cmds[1])
			{
				flag = true;
				break;
			}
		}
		if (!flag)
		{
            message = "Group not found";
		}
		else
		{
			set<string> existingMembers = groupsDB[i].members;
			if (existingMembers.find(LOGIN_ID) != existingMembers.end())
                message = LOGIN_ID + " is already a part of Group " + groupsDB[i].name;
			else
			{
				set<string> pendingIds = groupPendingRequests[cmds[1]].pendingID;
				pendingIds.insert(LOGIN_ID);
				GroupPendingRequest gpr(cmds[1], groupsDB[i].adminUserID, pendingIds);
				groupPendingRequests[cmds[1]] = gpr;
                message = "Request sent";
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "29")
	{
		string grpName = cmds[1];
		int i;
		for (i = 0; i < groupsDB.size(); i++)
		{
			if (groupsDB[i].name == grpName)
				break;
		}
		if (i == groupsDB.size())
		{
            message = "Group not found";
		}
		else
		{
			set<string> groupMembers = groupsDB[i].members;
			if (groupMembers.find(LOGIN_ID) == groupMembers.end())
                message = "User " + LOGIN_ID + " not present";
			else
			{
				groupMembers.erase(LOGIN_ID);
                groupsDB[i].members = groupMembers;
                message = "User " + LOGIN_ID + " removed from Group " + groupsDB[i].name;
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "42")
	{
		GroupPendingRequest gpr = groupPendingRequests[cmds[1]];
		if (gpr.grpname == "")
            message = "Group not found/No pending requests";
		else
		{
			string pendingRequests = "";
			for (auto i = gpr.pendingID.begin(); i != gpr.pendingID.end(); ++i)
                pendingRequests += (*i + " ");
			if (pendingRequests == "")
                message = "Group not found/No pending requests";
			else
                message = "For Group " + gpr.grpname + " pending requests are: " + pendingRequests;
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "43")
	{
		GroupPendingRequest gpr = groupPendingRequests[cmds[1]];
		if (gpr.grpname == "")
            message = "Group not found/No pending requests";
		else if (gpr.adminname != LOGIN_ID)
            message = LOGIN_ID + " is not the admin for Group " + gpr.grpname;
		else
		{
			set<string> pendingIds = gpr.pendingID;
			if (pendingIds.find(cmds[2]) == pendingIds.end())
                message = "Group join request for " + cmds[2] + " not found with Group " + gpr.grpname;
			else
			{
				pendingIds.erase(cmds[2]);
                gpr.pendingID = pendingIds;
				groupPendingRequests[cmds[1]] = gpr;
                message = "For Group " + gpr.grpname + ", join request approved for " + cmds[2];
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "22")
	{
        message = "All groups in the network: ";
		for (auto & group : groupsDB)
            message += ("\n" + group.name);
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "32")
	{
		vector<FileProperties> grpFiles = filesInGroup[cmds[1]];
		if (grpFiles.size() == 0)
            message = "No files present in Group " + cmds[1];
		else
		{
            message = "Files present ";
			for (auto & grpFile : grpFiles)
			{
                message += ("\n" + grpFile.path);
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "30")
	{
		int totPiece = 0;
		string totalHash;
		char *piece = new char[PIECE_SIZE], hash[20]; //make unsigned for SHA1
		ifstream ifs(cmds[1], ios::binary);
		while (ifs.read((char *)piece, PIECE_SIZE) || ifs.gcount())
		{
			totalHash += string((char *)hash);
			totPiece++;
			memset(piece, 0, PIECE_SIZE);
		}
		cout<<"HASH done Pieces="<<totPiece<<"\n";
		int i;
		for (i = 0; i < groupsDB.size(); i++)
		{
			if (groupsDB[i].name == cmds[2])
				break;
		}
		if (i == groupsDB.size())
            message = "Group " + cmds[2] + " not found";
		else
		{
			vector<FileProperties> filesProps = filesInGroup[cmds[2]];
			Peer currentPeer = peerList[LOGIN_ID];
			set<Peer> peerSet;
			peerSet.insert(currentPeer);
			FileProperties fp(FILE_ID++, cmds[1], cmds[1], cmds[2], totPiece,  peerSet, totalHash);
			for (i = 0; i < filesProps.size(); i++)
			{
				if (filesProps[i].path == cmds[1])
					break;
			}
			if (i < filesProps.size())
                message = "File " + cmds[1] + " already exists in Group " + cmds[2];
			else
			{
				filesProps.push_back(fp);
				filesInGroup[cmds[2]] = filesProps;
				fileIndex[FILE_ID - 1] = fp;
                message = "30 " + to_string(FILE_ID - 1) + " ID File " + cmds[1] + " added to Group " + cmds[2];
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "31") // DL FILE
	{
		int i;
		for (i = 0; i < groupsDB.size(); i++)
		{
			if (groupsDB[i].name == cmds[1])
				break;
		}
		if (i == groupsDB.size())
            message = "Group " + cmds[1] + " not found";
		else
		{
			vector<FileProperties> files = filesInGroup[cmds[1]];
			for (i = 0; i < files.size(); i++)
			{
				if (files[i].path.compare(files[i].path.length() - cmds[2].length(), cmds[2].length(), cmds[2]) == 0)
					break;
			}
			if (i == files.size())
                message = "File " + cmds[2] + " not found in Group " + cmds[1];
			else
			{
				set<Peer> seeds = fileIndex[files[i].id].seederList; //files[i].seederList;
				if (seeds.empty())
                    message = "No seeds are currently present";
				else
				{
                    message = "31 " + to_string(files[i].id) + " "; //add file id to beginning
					for (auto i = seeds.begin(); i != seeds.end(); ++i)
                        message += ((*i).ip + ":" + to_string((*i).port) + " ");
				}
			}
		}
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "19")
	{
        message = "Logged out. Bye!";
		peerList.erase(LOGIN_ID);
		auto pos = find(connectedClients.begin(), connectedClients.end(), ip + ":" + to_string(port));
		if (pos != connectedClients.end())
			connectedClients.erase(pos);
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "35")
	{
		//Show downloads in Peer side, nothing to do here
		//send(acc, msg.c_str(), msg.length(), 0);
	}
	else if (cmds[0] == "39")
	{
		//stop share - remove from seeder list
		message = "Not implemented";
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "50") //add as seeder
	{
		FileProperties fileInfo = fileIndex[stoi(cmds[1])];
		Peer currentPeer = peerList[LOGIN_ID];
		set<Peer> peerSet = fileInfo.seederList;
		peerSet.insert(currentPeer);
        fileInfo.seederList = peerSet;
		fileIndex[stoi(cmds[1])] = fileInfo;
        message = currentPeer.ip + ":" + to_string(currentPeer.port) + " added as seeder for file ID " + cmds[1];
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else
	{
        message = "Unknown value";
		if (sendto(socketStatus, message.c_str(), message.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}

}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "Parameters not provided.Exiting...\n";
		return -1;
	}

	ifstream trackInfo(argv[1]);
	string ix, px;
	trackInfo >> ix >> px;
	trackInfo.close();

	socklen_t addr_size = sizeof(struct sockaddr_in6);


	struct in6_addr buf;
	inet_pton(AF_INET6,ix.c_str(),&buf);

	addrinfo hint = {};
	hint.ai_flags = AI_NUMERICHOST;
	hint.ai_family = AF_INET6;
	hint.ai_socktype - SOCK_DGRAM;
	hint.ai_protocol = 0;
	hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

	addrinfo *res;
	int err;

	err = getaddrinfo(ix.c_str(), px.c_str(), &hint, &res);
	if (err < 0)
	{
    	cerr << "Can't get address to bind the socket! Error " << err << ". Quitting" << endl;
    	return -1;
	}

	int socketStatus = socket(PF_INET6, SOCK_DGRAM,0);

	if (socketStatus < 0)
	{
		cout << "Socket creation error \n";
		return -1;
	}

	int bindStatus = ::bind(socketStatus, res->ai_addr, res->ai_addrlen);

	if (bindStatus < 0)
	{
		printf("Bind failed with status: %d\n", errno);
		return -1;
	}

	int port = ntohs(((struct sockaddr_in6*)res->ai_addr)->sin6_port);
	printf("Listen started on %s, Port: %d\n", ix.c_str(), port);

	while (true)
	{
		char ip[res->ai_addrlen];
		struct sockaddr_in6 peerAddress = {};
		char *buffer = new char[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		if ( recvfrom(socketStatus,buffer, BUFFER_SIZE, 0, (struct sockaddr *)&peerAddress, &addr_size) < 0 )
		{
			printf("Recv failed %d\n", errno);
			return -1;
		}

		inet_ntop(AF_INET6, &(peerAddress.sin6_addr), ip, INET6_ADDRSTRLEN);
		int port = ntohs(peerAddress.sin6_port);
		string fullAddress = string(ip) + ":" + to_string(port);

		string cmdRecvd = string(buffer);
		string cmdPart, msg;
		stringstream cmdStream(cmdRecvd);
		vector<string> cmds;

		while (getline(cmdStream, cmdPart, ' '))
		{
			cmds.push_back(cmdPart);
		}

		if (cmds[0] == "sync")
		{
			fullAddress = string(ip) + ":" + to_string(port);
			sendto(socketStatus, fullAddress.c_str(), fullAddress.length(), 0, (struct sockaddr *)&peerAddress, addr_size );
			printf("Connection established with IP: %s and PORT: %d\n", ip, port);
		}
		else
		{
			handlePeerCommunication(string(ip), port ,socketStatus, peerAddress, cmds);
		}

		if (find(connectedClients.begin(), connectedClients.end(), fullAddress) != connectedClients.end())
			continue;
		else
			connectedClients.push_back(fullAddress);

	}
	return 0;
}
