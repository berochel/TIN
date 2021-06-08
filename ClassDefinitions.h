#include <string>
#include <iostream>
#include <set>
#include <fstream>
#include <thread>

#include <string.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#define BUFFER_SIZE 4096

using namespace std;


size_t PIECE_SIZE = 1024;
struct User
{
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

struct Peer
{
	Peer()
	{
		ip = "";
		port = 0;
		userID = "";
	}

    Peer(const string &ip, int port, const string &userId) : ip(ip), port(port), userID(userId) {}


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

struct Group
{
    Group(const string &name, const string &adminUserId, const set<string> &members) : name(name),
                                                                                       adminUserID(adminUserId),
                                                                                       members(members) {}
	string name;
	string adminUserID;
	set<string> members;
};

struct GroupPendingRequest
{
	GroupPendingRequest()
	{
		grpname = "";
		adminname = "";
	}

    GroupPendingRequest(const string &grpname, const string &adminname, const set<string> &pendingId) : grpname(
            grpname), adminname(adminname), pendingID(pendingId) {}


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
struct FileProperties
{
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

    FileProperties(int id, const string &name, const string &path, const string &groupName, int pieces,
                   const set<Peer> &seederList, const string &hash) : id(id), name(name), path(path),
                                                                      groupName(groupName), pieces(pieces),
                                                                      seederList(seederList), hash(hash) {}

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
