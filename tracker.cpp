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

void handlePeerCommunication(string ip, int p, int acc)
{
	cout << "Thread launch\n";
	char *buffer; //[4096] = {0};
	bool IS_LOGGED_IN = false;
	string LOGIN_ID = "";
	while (true)
	{
		buffer = new char[4096];
		memset(buffer, 0, 4096);
		int l = read(acc, buffer, 4096);
		printf("Client %s:%d said %s (Msg length %d)\n", ip.c_str(), p, buffer, l);
		string cmdRecvd = string(buffer);
		//for (int i = 0; i < l; i++)cmdRecvd += buffer[i];
		string t, msg = "";
		//delete[] buffer;cout<<"W1";
		stringstream x(cmdRecvd);
		//cout << "W1\n";
		vector<string> cmds;
		while (getline(x, t, ' '))
		{
			cmds.push_back(t);
		}
		//cout << cmds[0]<<"W1\n";
		if (cmds[0] == "a")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "b")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "c")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "d")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "e")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "f")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "g")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "h")
		{
			msg = "All groups in the network: ";
			for (int i = 0; i < GROUPS.size(); i++)
				msg += ("\n" + GROUPS[i].name);
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "i")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "j")
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "k") // DL FILE
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
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "l")
		{
			msg = "Logged out. Bye!";
			peerList.erase(LOGIN_ID);
			auto pos = find(connectedClients.begin(), connectedClients.end(), ip + ":" + to_string(p));
			if (pos != connectedClients.end())
				connectedClients.erase(pos);
			send(acc, msg.c_str(), msg.length(), 0);
			break;
		}
		else if (cmds[0] == "m")
		{
			//Show downloads in peer side, nothing to do here
			//send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "n")
		{
			//stop share - remove from seeder list
			msg = "Not implemented";
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else if (cmds[0] == "o") //add as seeder
		{
			file_properties vv = fileIndex[stoi(cmds[1])];
			peer currentPeer = peerList[LOGIN_ID];
			set<peer> peerSet = vv.seederList;
			peerSet.insert(currentPeer);
			vv.seederList = peerSet;
			fileIndex[stoi(cmds[1])] = vv;
			msg = currentPeer.ip + ":" + to_string(currentPeer.port) + " added as seeder for file ID " + cmds[1];
			send(acc, msg.c_str(), msg.length(), 0);
		}
		else
		{
			msg = "Unknown value";
			send(acc, msg.c_str(), msg.length(), 0);
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "Parameters not provided.Exiting...\n";
		return -1;
	}

	ifstream trackInfo(argv[1]);
	string ix, px;
	trackInfo >> ix >> px;
	trackInfo.close();

	struct sockaddr_in6 peerAddress;

	struct in6_addr buf;
	inet_pton(AF_INET6,ix.c_str(),&buf);

	addrinfo hint = {};
	hint.ai_flags = AI_NUMERICHOST;
	hint.ai_family = AF_INET6;
	hint.ai_socktype = SOCK_STREAM;
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

	int socketStatus = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	if (socketStatus < 0)
	{
		cout << "Socket creation error \n";
		return -1;
	}

	int bindStatus = ::bind(socketStatus, res->ai_addr, res->ai_addrlen);

	if (bindStatus < 0)
	{
		printf("Bind failed with status: %d\n", bindStatus);
		return -1;
	}
	int listenStatus = listen(socketStatus, 3);
	if (listenStatus < 0)
	{
		cout << ("Listen Failed \n");
		return -1;
	}

	
	int port = ntohs(((struct sockaddr_in6*)res->ai_addr)->sin6_port);

	printf("Listen started on %s, Port: %d\n", ix.c_str(), port);



	socklen_t addr_size = sizeof(struct sockaddr_in6);
	while (true)
	{
		int acc = accept(socketStatus, (struct sockaddr *)&peerAddress, &addr_size);
		char ip[res->ai_addrlen];

		inet_ntop(AF_INET6, &(peerAddress.sin6_addr), ip, INET6_ADDRSTRLEN);
		port = ntohs(peerAddress.sin6_port);
		string fullAddress = string(ip) + ":" + to_string(port);
		
		string temp = "You are connected to " + ix + ":" + px + " with IP " + string(ip) + ":" + to_string(port);
		send(acc, temp.c_str(), temp.length(), 0);

		char *buffer = new char[4096];
		memset(buffer, 0, 4096);
		int l = read(acc, buffer, 4096);
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
			printf("Connection established with IP: %s and PORT: %d\n", ip, port);
		}

		if (find(connectedClients.begin(), connectedClients.end(), fullAddress) != connectedClients.end())
			continue;
		else
			connectedClients.push_back(fullAddress);
		thread launchPeer(handlePeerCommunication, string(ip), port, acc); //, string(ip), port,descriptor
		launchPeer.detach();
	}
	return 0; //compile with g++ -std=c++17 -o tracker tracker.cpp
}