// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <net/if.h>
#include <netdb.h>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <thread>

#include "ClassDefinitions.h"

using namespace std;

vector<user> USER_DB; //uid,pass
vector<group> GROUPS; //grp id,owner
//map<string, set<string>> PENDING_REQUESTS;		  //user id,list of groups
//vector<pair<string, string>> UNACCEPTED_REQUESTS; //grp id,usr id
map<string, peer> peerList;
map<string, group_pending_request> groupPendingRequests;
map<string, vector<file_properties>> filesInGroup;
map<int, file_properties> fileIndex;
vector<string> connectedClients;
//map<string, user> peerDetailsList;
int FILE_ID = 1;

void handlePeerCommunication(string ip, int p, int socketStatus, struct sockaddr_in6 peerAddress, vector<string> cmds)
{
	socklen_t addr_size = sizeof(struct sockaddr_in6);

	//char *buffer; //[4096] = {0};
	bool IS_LOGGED_IN = false;
	string LOGIN_ID = cmds.back();
	cmds.pop_back();
	if (LOGIN_ID != "0")
		IS_LOGGED_IN = true;
	string t, msg = "";

	if (cmds[0] == "10")
	{
		user u(cmds[1], cmds[2]);
		bool flag = false;
		for (int i = 0; i < USER_DB.size(); i++)
		{
			if (USER_DB[i].userID == cmds[1])
			{
				msg = "User already exists";
				flag = true;
				break;
			}
		}
		if (!flag)
		{
			msg = "User added";
			USER_DB.push_back(u);
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "11")
	{
		peer client(ip, p, cmds[1]);
		int i;
		for (i = 0; i < USER_DB.size(); i++)
		{
			if (USER_DB[i].userID == cmds[1] && USER_DB[i].password == cmds[2])
				break;
		}
		if (i == USER_DB.size())
		{
			msg = "User does not exist/Wrong password";
		}
		else
		{
			if (peerList.count(cmds[1]) == 0)
			{
				LOGIN_ID = cmds[1];
				peerList[LOGIN_ID] = client;
				IS_LOGGED_IN = true;
				msg = "Logging in with user ID " + cmds[1];
			}
			else
			{
				msg = "Already logged in another instance";
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "20")
	{
		group grp(cmds[1], LOGIN_ID);
		set<string> mems; //add admin to group
		mems.insert(LOGIN_ID);
		grp.members = mems;
		int i;
		bool flag = false;
		for (i = 0; i < GROUPS.size(); i++)
		{
			if (GROUPS[i].name == cmds[1])
			{
				flag = true;
				break;
			}
		}
		if (flag)
		{
			msg = "Group already exists";
		}
		else
		{
			GROUPS.push_back(grp);

			msg = "Group " + grp.name + " created with admin " + grp.adminUserID;
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "21")
	{
		int i;
		bool flag = false;
		for (i = 0; i < GROUPS.size(); i++)
		{
			if (GROUPS[i].name == cmds[1])
			{

				flag = true;
				break;
			}
		}
		if (!flag)
		{
			msg = "Group not found";
		}
		else
		{
			set<string> existingMembers = GROUPS[i].members;
			if (existingMembers.find(LOGIN_ID) != existingMembers.end())
				msg = LOGIN_ID + " is already a part of group " + GROUPS[i].name;
			else
			{
				set<string> xxx = groupPendingRequests[cmds[1]].pendingID;
				xxx.insert(LOGIN_ID);
				group_pending_request gg(cmds[1], GROUPS[i].adminUserID, xxx);
				groupPendingRequests[cmds[1]] = gg;
				msg = "Request sent";
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "29")
	{
		string grpName = cmds[1];
		int i;
		for (i = 0; i < GROUPS.size(); i++)
		{
			if (GROUPS[i].name == grpName)
				break;
		}
		if (i == GROUPS.size())
		{
			msg = "Group not found";
		}
		else
		{
			set<string> grpmembers = GROUPS[i].members;
			if (grpmembers.find(LOGIN_ID) == grpmembers.end())
				msg = "User " + LOGIN_ID + " not present";
			else
			{
				grpmembers.erase(LOGIN_ID);
				GROUPS[i].members = grpmembers;
				msg = "User " + LOGIN_ID + " removed from group " + GROUPS[i].name;
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "42")
	{
		group_pending_request grpp = groupPendingRequests[cmds[1]];
		if (grpp.grpname == "")
			msg = "Group not found/No pending requests";
		else
		{
			string pp = "";
			for (auto i = grpp.pendingID.begin(); i != grpp.pendingID.end(); ++i)
				pp += (*i + " ");
			if (pp == "")
				msg = "Group not found/No pending requests";
			else
				msg = "For group " + grpp.grpname + " pending requests are: " + pp;
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "43")
	{
		group_pending_request grpp = groupPendingRequests[cmds[1]];
		if (grpp.grpname == "")
			msg = "Group not found/No pending requests";
		else if (grpp.adminname != LOGIN_ID)
			msg = LOGIN_ID + " is not the admin for group " + grpp.grpname;
		else
		{
			set<string> pendID = grpp.pendingID;
			if (pendID.find(cmds[2]) == pendID.end())
				msg = "Group join request for " + cmds[2] + " not found with group " + grpp.grpname;
			else
			{
				pendID.erase(cmds[2]);
				grpp.pendingID = pendID;
				groupPendingRequests[cmds[1]] = grpp;
				msg = "For group " + grpp.grpname + ", join request approved for " + cmds[2];
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "22")
	{
		msg = "All groups in the network: ";
		for (int i = 0; i < GROUPS.size(); i++)
			msg += ("\n" + GROUPS[i].name);
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "32")
	{
		vector<file_properties> grpFiles = filesInGroup[cmds[1]];
		if (grpFiles.size() == 0)
			msg = "No files present in group " + cmds[1];
		else
		{
			msg = "Files present ";
			for (int i = 0; i < grpFiles.size(); i++)
			{
				msg += ("\n" + grpFiles[i].path);
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "30")
	{
		int i, totPiece = 0;
		string totalHash = "";
		char *piece = new char[PIECE_SIZE], hash[20]; //make unsigned for SHA1
		ifstream ifs(cmds[1], ios::binary);
		while (ifs.read((char *)piece, PIECE_SIZE) || ifs.gcount())
		{
			//SHA1(piece, strlen((char *)piece), hash);
			totalHash += string((char *)hash);
			//cout<<ifs.tellg()<<endl;
			totPiece++;
			memset(piece, 0, PIECE_SIZE);
		}
		cout<<"HASH done Pieces="<<totPiece<<"\n";
		for (i = 0; i < GROUPS.size(); i++)
		{
			if (GROUPS[i].name == cmds[2])
				break;
		}
		if (i == GROUPS.size())
			msg = "Group " + cmds[2] + " not found";
		else
		{
			vector<file_properties> vv = filesInGroup[cmds[2]];
			peer currentPeer = peerList[LOGIN_ID];
			set<peer> peerSet;
			peerSet.insert(currentPeer);
			file_properties fp(FILE_ID++, cmds[1], cmds[1], cmds[2], totPiece, totalHash, peerSet);
			for (i = 0; i < vv.size(); i++)
			{
				if (vv[i].path == cmds[1])
					break;
			}
			if (i < vv.size())
				msg = "File " + cmds[1] + " already exists in group " + cmds[2];
			else
			{
				vv.push_back(fp);
				filesInGroup[cmds[2]] = vv;
				fileIndex[FILE_ID - 1] = fp;
				msg = to_string(FILE_ID - 1) + " ID File " + cmds[1] + " added to group " + cmds[2];
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "31") // DL FILE
	{
		int i;
		for (i = 0; i < GROUPS.size(); i++)
		{
			if (GROUPS[i].name == cmds[1])
				break;
		}
		if (i == GROUPS.size())
			msg = "Group " + cmds[1] + " not found";
		else
		{
			vector<file_properties> files = filesInGroup[cmds[1]];
			for (i = 0; i < files.size(); i++)
			{
				if (files[i].path.compare(files[i].path.length() - cmds[2].length(), cmds[2].length(), cmds[2]) == 0)
					break;
			}
			if (i == files.size())
				msg = "File " + cmds[2] + " not found in group " + cmds[1];
			else
			{
				set<peer> seeds = fileIndex[files[i].id].seederList; //files[i].seederList;
				if (seeds.empty())
					msg = "No seeds are currently present";
				else
				{
					msg = to_string(files[i].id) + " "; //add file id to beginning
					for (auto i = seeds.begin(); i != seeds.end(); ++i)
						msg += ((*i).ip + ":" + to_string((*i).port) + " ");
				}
			}
		}
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "19")
	{
		msg = "Logged out. Bye!";
		peerList.erase(LOGIN_ID);
		auto pos = find(connectedClients.begin(), connectedClients.end(), ip + ":" + to_string(p));
		if (pos != connectedClients.end())
			connectedClients.erase(pos);
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "35")
	{
		//Show downloads in peer side, nothing to do here
		//send(acc, msg.c_str(), msg.length(), 0);
	}
	else if (cmds[0] == "39")
	{
		//stop share - remove from seeder list
		msg = "Not implemented";
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else if (cmds[0] == "50") //add as seeder
	{
		file_properties vv = fileIndex[stoi(cmds[1])];
		peer currentPeer = peerList[LOGIN_ID];
		set<peer> peerSet = vv.seederList;
		peerSet.insert(currentPeer);
		vv.seederList = peerSet;
		fileIndex[stoi(cmds[1])] = vv;
		msg = currentPeer.ip + ":" + to_string(currentPeer.port) + " added as seeder for file ID " + cmds[1];
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
		{
			printf("Sendto failed!");
		}
	}
	else
	{
		msg = "Unknown value";
		if (sendto(socketStatus, msg.c_str(), msg.length(), 0, (struct sockaddr *)&peerAddress, addr_size) < 0 )
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
		char *buffer = new char[4096];
		memset(buffer, 0, 4096);
		if ( recvfrom(socketStatus,buffer, 4096, 0, (struct sockaddr *)&peerAddress, &addr_size) < 0 )
		{
			printf("Recv failed %d\n", errno);
			return -1;	
		}

		inet_ntop(AF_INET6, &(peerAddress.sin6_addr), ip, INET6_ADDRSTRLEN);
		int port = ntohs(peerAddress.sin6_port);
		string fullAddress = string(ip) + ":" + to_string(port);
		
		string cmdRecvd = string(buffer);
		string t, msg = "";
		stringstream x(cmdRecvd);
		vector<string> cmds;

		while (getline(x, t, ' '))
		{
			cmds.push_back(t);
		}

		if (cmds[0] == "sync")
		{
			//ip = cmds[1].c_str();
			fullAddress = string(ip) + ":" + to_string(port);
			sendto(socketStatus, fullAddress.c_str(), fullAddress.length(), 0, (struct sockaddr *)&peerAddress, addr_size );
			printf("Connection established with IP: %s and PORT: %d\n", ip, port);
		}
		else
		{
			thread launchPeer(handlePeerCommunication, string(ip), port ,socketStatus, peerAddress, cmds); //, string(ip), port,descriptor
			launchPeer.detach();
		}

		if (find(connectedClients.begin(), connectedClients.end(), fullAddress) != connectedClients.end())
			continue;
		else
			connectedClients.push_back(fullAddress);
		
	}
	return 0; 
}