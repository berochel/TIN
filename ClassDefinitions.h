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
class user
{
public:
	user() {}
	user(string userID, string password)
	{
		this->userID = userID;
		this->password = password;
		this->isLoggedIn = true;
	}
	string userID;
	string password;
	bool isLoggedIn;
};

class peer
{
public:
	peer()
	{
		ip = "";
		port = 0;
		userID = "";
	}
	peer(string ip, int port, string userID)
	{
		this->ip = ip;
		this->port = port;
		this->userID = userID;
	}
	/*peer &operator=(const peer &t)
	{
		this->ip = ip;
		this->port = port;
		this->userID = userID;
	}*/
	bool operator<(const peer &rhs) const
	{
		return port < rhs.port;
	}

	peer(const peer &p)
	{
		ip = p.ip;
		port = p.port;
		userID = p.userID;
	}
	string ip;
	int port;
	string userID;
};

class group
{
public:
	group(string name, string adminUserID)
	{
		this->name = name;
		this->adminUserID = adminUserID;
	}
	string name;
	string adminUserID;
	set<string> members;
};

class group_pending_request
{
public:
	group_pending_request()
	{
		grpname = "";
		adminname = "";
	}
	group_pending_request(string gName, string admin, set<string> members)
	{
		grpname = gName;
		adminname = admin;
		pendingID = members;
	}
	group_pending_request(const group_pending_request &g)
	{
		grpname = g.grpname;
		adminname = g.adminname;
		pendingID = g.pendingID;
	}
	string grpname;
	string adminname;
	set<string> pendingID;
};
class file_properties
{
public:
	int id;
	string name;
	string path;
	string groupName;
	int pieces;
	set<peer> seederList;
	string hash;

	file_properties()
	{
		id = -1;
		name = "";
		path = "";
		groupName = "";
		pieces = 0;
		hash = "";
	}
	file_properties(int i, string n, string p, string grp, int pi, string hh, set<peer> seedList)
	{
		id = i;
		name = n;
		path = p;
		groupName = grp;
		pieces = pi;
		hash = hh;
		seederList = seedList;
	}
	file_properties(const file_properties &f)
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

/*class connectedClient

{
public:
	connectedClient()
	{
		ip = "";
		port = 0;
	}
	connectedClient(string i, int p)
	{
		ip = i;
		port = p;
	}
	connectedClient(const connectedClient &c)
	{
		ip = c.ip;
		port = c.port;
	}
	string ip;
	int port;
};*/