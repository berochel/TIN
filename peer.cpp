#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <net/if.h>
#include <netdb.h>

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <mutex>
#include "ClassDefinitions.h"

// Qt headers
#include "widget.h"
#include <QApplication>

bool IS_LOGGED_IN;
int LOGIN_ID = 0;
bool IS_PEER_OR_SEEDER;
int listenSocket;
int trackerSocket;
const int MAX_CONNECTIONS = 3;
string command_string = "";
string fileUploadPath;
string fileUploadPathGroup;
string fileDownloadPath;
string fileDownloadName;
struct sockaddr_in6 trackerAddress, peerAddress;
addrinfo *tracker, *peer_own;
std::mutex m;

map<int, file_properties> downloadedFiles;
map<int, string> fileBitVectors;		  //file ID,bit vector
map<int, vector<peer>> currentSeederList; //file ID,peer

socklen_t addr_size = sizeof(struct sockaddr_in6);

using namespace std; //DOWNLOAD FORMAT = d <FILEID> <PIECE RANGE START> <PIECE RANGE END>

void sendPiece(string ip, int port, string filePath, int startPiece, int endPiece, sockaddr_in6 pAddress, int socketStatus)
{
	ifstream fp(filePath, ios::in | ios::binary);
	struct stat sb;
	unsigned long long rc = stat(filePath.c_str(), &sb);
	rc = (rc == 0 ? sb.st_size : 0ll);
	int chunks = (int)::ceil(rc / (PIECE_SIZE));

	//cout << "No of 512 KB chunks in file " << chunks << endl;
	//cout << "Total file size(bytes) " << rc << endl;

	size_t CHUNK_SIZE = PIECE_SIZE;
	if (rc < CHUNK_SIZE)
		CHUNK_SIZE = rc;
	if (startPiece != 0)
		rc = rc - startPiece * PIECE_SIZE;
	char *buff = new char[CHUNK_SIZE];
	int rer = 0;
	fp.seekg((startPiece)*PIECE_SIZE, ios::beg);

	while (fp.tellg() <= endPiece * PIECE_SIZE && fp.read(buff, CHUNK_SIZE))
	{
		rc -= CHUNK_SIZE;

		int z = sendto(socketStatus, buff, CHUNK_SIZE, 0, (struct sockaddr *)&pAddress, addr_size);
		if (z < 0)
		{
			cout<<"sendto failed, errno: "<<errno;
		}
		rer += z;
		if (rc == 0)
			break;
		if (rc < CHUNK_SIZE)
			CHUNK_SIZE = rc;
	}
	sendto(socketStatus, buff, 0, 0, (struct sockaddr *)&pAddress, addr_size);
	cout << fp.tellg() << endl;
	fp.close();
	cout << "Sent " << rer << " bytes to " << ip << ":" << port << endl;
	IS_PEER_OR_SEEDER=false;
}

void getPiece(int seedSocket, string filePath, int fileid, int pieceLocation, sockaddr_in6 seedAddress)
{
	ofstream fp(filePath, ios::out | ios::binary | ios::app);
	fp.seekp(PIECE_SIZE * pieceLocation, ios::beg);
	int i = 0;
	int n, currPiece = pieceLocation;
	string prevBitVec = fileBitVectors[fileid];
	do
	{
		char *buff = new char[PIECE_SIZE];
		memset(buff, 0, PIECE_SIZE);
		n = recvfrom(seedSocket, buff, PIECE_SIZE, 0, (struct sockaddr *)&seedAddress, &addr_size);
		fp.write(buff, n);
		i += n;
		if (n != 0 && currPiece < prevBitVec.length())
			prevBitVec[currPiece++] = '1';
		//cout << "Wrote " << n << " bytes\n";
	} while (n > 0);
	cout << "Wrote " << i << " bytes\n";
	//cout << filePath;
	//prevBitVec[pieceLocation] = '1';
	fileBitVectors[fileid] = prevBitVec;
	fp.close();
	//cout << fileBitVectors[fileid] << " \n";
}

bool getFrequency(string x)
{
	int no1s = 0;
	for (int i = 0; i < x.length(); i++)
	{
		if (x[i] == '1')
			no1s++;
	}
	if (no1s == x.length())
		return false;
	return true;
}

void startDownload(int fileid, string fileName, string filePath)
{
	vector<peer> peerlist = currentSeederList[fileid];
	int reuseAddress = 1;
	int maxConns = (MAX_CONNECTIONS < peerlist.size() ? MAX_CONNECTIONS : peerlist.size());

	ifstream fp(fileName, ios::in | ios::binary);
	struct stat sb;
	unsigned long long rc = stat(fileName.c_str(), &sb);
	rc = (rc == 0 ? sb.st_size : 0ll);
	int chunks = (int)::ceil(rc / (PIECE_SIZE)) + 1;
	fp.close();
	cout << "CHUNKS: " << chunks << endl;
	size_t CHUNK_SIZE = PIECE_SIZE;
	if (rc < CHUNK_SIZE)
		CHUNK_SIZE = rc;

	int startPiece = 0, endPiece = chunks / maxConns;
	string bv(chunks, '0');
	fileBitVectors[fileid] = bv;
	ofstream output(filePath, ios::out | ios::binary | ios::app);
	for (int i = 0; i < maxConns; i++)
	{
		struct sockaddr_in6 seedAddress;
		seedAddress.sin6_family = AF_INET6;
		seedAddress.sin6_port = htons(peerlist[i].port);
		inet_pton(AF_INET6, peerlist[i].ip.c_str(), &(seedAddress.sin6_addr));

		int seedSocket = socket(AF_INET6, SOCK_DGRAM, 0);
		if (seedSocket < 0)
		{
			cout << "Unable to download. Socket creation error \n";
			return;
		}

		//d <FILEID> <PIECE RANGE START> <PIECE RANGE END>
		string sss = "32 " + to_string(fileid) + " " + to_string(startPiece) + " " + to_string(endPiece);
		cout << "Send DL request to seed " << peerlist[i].ip << ":" << peerlist[i].port << " for file ID " << fileid << endl;
		sendto(seedSocket, sss.c_str(), sss.length(), 0, (struct sockaddr *)&seedAddress, addr_size);

		thread writeToFile(getPiece, seedSocket, filePath, fileid, startPiece, seedAddress);
		writeToFile.join();
		startPiece = endPiece + 1;
		endPiece += (chunks / maxConns);
		if (startPiece >= chunks)
			i = maxConns;
		if (endPiece > chunks)
			endPiece = chunks;
		if (i == maxConns - 2)
			endPiece = chunks;
	}
	int y = 1;

	while (getFrequency(fileBitVectors[fileid]))
		;
	//char *buffer = new char[PIECE_SIZE];
	//int valread = read(listenSocket, buffer, 4096);
	//cout << string(buffer) << endl;
	cout << "Download of file " + fileName + " complete\n";
	//mark as seeder
	string ss = "50 " + to_string(fileid);
	char *buffer = new char[4096];
	memset(buffer, 0, 4096);
	sendto(trackerSocket, ss.c_str(), ss.length(), 0, (struct sockaddr *)&trackerAddress, addr_size);
	m.lock();
	recvfrom(trackerSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&trackerAddress, &addr_size);
	m.unlock();
	cout << string(buffer) << endl;

	int totPiece = 0;
	ifstream ifs(fileName, ios::binary);
	string totalHash = "";
	char *piece = new char[PIECE_SIZE], hash[20]; //change array to unsigned for SHA1

	while (ifs.read((char *)piece, PIECE_SIZE) || ifs.gcount())
	{
		//SHA1(piece, strlen((char *)piece), hash);
		totalHash += string((char *)hash);
		totPiece++;
		memset(piece, 0, PIECE_SIZE);
	}

	file_properties f(fileid, fileName, fileName, fileUploadPathGroup, totPiece, totalHash, set<peer>());
	downloadedFiles[fileid] = f;

}

void receiveData()
{
	struct sockaddr_in6 recAddress;
	char ip[INET6_ADDRSTRLEN];
	int port = ntohs(recAddress.sin6_port);
	char buffer[4096] = {};

	while(true)
	{
		memset(buffer, 0, 4096);
		m.lock();
		int valread = recvfrom(trackerSocket, buffer, sizeof(buffer), 0,(struct sockaddr *)&recAddress, &addr_size);
		m.unlock();

		if (valread > 0)
			cout << string(buffer) << endl;

		if (string(buffer).substr(0, 3) == "Log")
		{
			IS_LOGGED_IN = true;

		}
		else if (string(buffer).substr(0, 3) == "30 ")
		{
			string fileDetails = string(buffer);
			fileDetails.erase(0, 3);
			fileDetails = fileDetails.substr(0, fileDetails.find(" "));

			int totPiece = 0;
			ifstream ifs(fileUploadPath, ios::binary);
			string totalHash = "";
			char *piece = new char[PIECE_SIZE], hash[20]; //change array to unsigned for SHA1

			while (ifs.read((char *)piece, PIECE_SIZE) || ifs.gcount())
			{
				//SHA1(piece, strlen((char *)piece), hash);
				totalHash += string((char *)hash);
				totPiece++;
				memset(piece, 0, PIECE_SIZE);
			}

			file_properties f(stoi(fileDetails), fileUploadPath, fileUploadPath, fileUploadPathGroup, totPiece, totalHash, set<peer>());
			downloadedFiles[stoi(fileDetails)] = f;
			if (!IS_PEER_OR_SEEDER)
			{
				IS_PEER_OR_SEEDER = true;
				// thread startListenOnPeer(listenForConnections);
				// startListenOnPeer.detach();
				// listenForConnections();
			}
		}
		else if (string(buffer).substr(0, 3)=="32 ")
		{
			char ip[INET_ADDRSTRLEN];
			memset(ip, 0, INET_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(recAddress.sin6_addr), ip, INET_ADDRSTRLEN);
			int port = ntohs(recAddress.sin6_port);
			string fullAddress = string(ip) + ":" + to_string(port);
			printf("connection established with peer IP : %s and PORT : %d\n", ip, port);
			stringstream x(buffer);
			string t;
			vector<string> argsFromPeer;
			while (getline(x, t, ' '))
				argsFromPeer.push_back(t);

			printf("Peer %s:%d requested for %s with piece range from %s-%s\n", ip, port, downloadedFiles[stoi(argsFromPeer[1])].path.c_str(), argsFromPeer[2].c_str(), argsFromPeer[3].c_str());
			thread sendDataToPeer(sendPiece, string(ip), port, downloadedFiles[stoi(argsFromPeer[1])].path, stoi(argsFromPeer[2]), stoi(argsFromPeer[3]), recAddress, trackerSocket);
			sendDataToPeer.detach();
			// sendPiece(string(ip), port, downloadedFiles[stoi(argsFromPeer[1])].path, stoi(argsFromPeer[2]), stoi(argsFromPeer[3]), recAddress, trackerSocket);
		}
		else if (string(buffer).substr(0, 3)=="31 ")
		{
			string b = string(buffer);
			b.erase(0, 3);
			stringstream x(b);
			string t;
			cout<<"bufor: "<<b<<endl;
			vector<peer> peerList;
			int i = 0, fileid = 0;
			while (getline(x, t, ' ') )
			{
				if (i == 0)
				{
					fileid = stoi(t);
				}
				else
				{
					peer ppp(t.substr(0, t.find_last_of(":")), stoi(t.substr(t.find_last_of(":") + 1)), "");
					peerList.push_back(peer(ppp));
					cout << "Added seed " << ppp.ip << ":" << ppp.port << endl;
				}
				i++;
			}
			currentSeederList[fileid] = peerList;
			//START DOWNLOAD
			thread startdl(startDownload, fileid, fileDownloadName, fileDownloadPath);
			startdl.detach();
			// startDownload( fileid, fileDownloadName, fileDownloadPath);
		}
	}
}

void runGUIApplication(int argc, char **argv, int tracker_socket, int loginID, socklen_t addr_size)
{
	// Qt init
	QApplication a(argc, argv);
    	Widget w(tracker_socket, loginID, addr_size);
    	w.show();
    	a.exec();
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "Parameters not provided.Exiting...\n";
		return -1;
	}
	
	IS_LOGGED_IN = false;
	IS_PEER_OR_SEEDER = false;
	fileUploadPath = "";
	fileUploadPathGroup = "";
	fileDownloadPath = "";
	fileDownloadName = "";
	
	int reuseAddress = 1;
	ifstream trackInfo(argv[2]);
	string ix, px;
	trackInfo >> ix >> px;
	trackInfo.close();

	socklen_t addr_size = sizeof(struct sockaddr_in6);

	trackerAddress.sin6_family = AF_INET6;
	trackerAddress.sin6_port = htons(stoi(px));
	inet_pton(AF_INET6, ix.c_str(), &(trackerAddress.sin6_addr));


	trackerSocket = socket(AF_INET6, SOCK_DGRAM, 0);
	int listenSocketOptions = setsockopt(trackerSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress));
	setsockopt(trackerSocket, SOL_SOCKET, SO_REUSEPORT, &reuseAddress, sizeof(reuseAddress));

	if (trackerSocket < 0 || listenSocketOptions < 0)
	{
		cout << "Socket creation error \n";
		return -1;
	}

	char ip[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &(trackerAddress.sin6_addr), ip, INET6_ADDRSTRLEN);
	int port = ntohs(trackerAddress.sin6_port);
	//printf("Connecting to: %s, Port: %d\n", ip, port);

	char buffer[4096] = {0};
	string syncActual = "sync " + string(argv[1]) + " ";
	sendto(trackerSocket, syncActual.c_str(), syncActual.length(), 0, (struct sockaddr *)&trackerAddress, addr_size);

	buffer[4096] = {0};
	m.lock();
	recvfrom(trackerSocket,buffer, sizeof(buffer), 0, (struct sockaddr *)&trackerAddress, &addr_size);
	m.unlock();
	string s_temp(buffer);
	int temp = stoi(s_temp.substr(s_temp.find_last_of(":") + 1));

	peerAddress.sin6_family = AF_INET6;
	inet_pton(AF_INET6, argv[1], &(peerAddress.sin6_addr));
	peerAddress.sin6_port = htons(temp);

	thread read(receiveData);
	read.detach();

	memset(buffer, 0, 4096);	
    	runGUIApplication(argc, argv, trackerSocket, LOGIN_ID, addr_size);

	return 0;
}
