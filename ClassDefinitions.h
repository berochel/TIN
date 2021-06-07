#include <string>
#include <iostream>
#include <set>
#include <fstream>
#include <thread>

#include <string.h>
#include <netinet/in.h>
#include <openssl/sha.h>

using namespace std;

size_t PIECE_SIZE = 1024;
class User
{
public:
	User() {}
	User(string userID, string password)
	{
		this->userID = userID;
		this->password = password;
		this->isLoggedIn = true;
	}
	string userID;
	string password;
	bool isLoggedIn;
};

class Peer
{
public:
	Peer()
	{
		ip = "";
		port = 0;
		userID = "";
	}
	Peer(string ip, int port, string userID)
	{
		this->ip = ip;
		this->port = port;
		this->userID = userID;
	}


	bool operator<(const Peer &rhs) const
	{
		return port < rhs.port;
	}

	Peer(const Peer &p)
	{
		ip = p.ip;
		port = p.port;
		userID = p.userID;
	}
	string ip;
	int port;
	string userID;
};

class Group
{
public:
	Group(string name, string adminUserID)
	{
		this->name = name;
		this->adminUserID = adminUserID;
	}
	string name;
	string adminUserID;
	set<string> members;
};

class GroupPendingRequest
{
public:
	GroupPendingRequest()
	{
		grpname = "";
		adminname = "";
	}
	GroupPendingRequest(string gName, string admin, set<string> members)
	{
		grpname = gName;
		adminname = admin;
		pendingID = members;
	}
	GroupPendingRequest(const GroupPendingRequest &g)
	{
		grpname = g.grpname;
		adminname = g.adminname;
		pendingID = g.pendingID;
	}
	string grpname;
	string adminname;
	set<string> pendingID;
};
class FileProperties
{
public:
	int id;
	string name;
	string path;
	string groupName;
	int pieces;
	set<Peer> seederList;
	string hash;

	FileProperties()
	{
		id = -1;
		name = "";
		path = "";
		groupName = "";
		pieces = 0;
		hash = "";
	}
	FileProperties(int i, string n, string p, string grp, int pi, string hh, set<Peer> seedList)
	{
		id = i;
		name = n;
		path = p;
		groupName = grp;
		pieces = pi;
		hash = hh;
		seederList = seedList;
	}
	FileProperties(const FileProperties &f)
	{
		id = f.id;
		name = f.name;
		path = f.path;
		groupName = f.groupName;
		pieces = f.pieces;
		hash = f.hash;
		seederList = f.seederList;
	}
};
