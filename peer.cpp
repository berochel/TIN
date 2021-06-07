#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>


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

#define MAX_CONNECTIONS 3

int LOGIN_ID = 0;
bool IS_PEER_OR_SEEDER;

int trackerSocket;

string fileUploadPath;
string fileUploadPathGroup;
string fileDownloadPath;
string fileDownloadName;
struct sockaddr_in6 trackerAddress, peerAddress;
std::mutex m;

map<int, FileProperties> downloadedFiles;
map<int, string> fileBitVectors;		  //file ID,bit vector
map<int, vector<Peer>> currentSeederList; //file ID,Peer

socklen_t addr_size = sizeof(struct sockaddr_in6);

using namespace std; //DOWNLOAD FORMAT = d <FILEID> <PIECE RANGE START> <PIECE RANGE END>

void sendPiece(string ip, int port, const string& filePath, int startPiece, int endPiece, sockaddr_in6 pAddress, int socketStatus)
{
	ifstream fp(filePath, ios::in | ios::binary);
	struct stat sb;
	unsigned long long rc = stat(filePath.c_str(), &sb);
	rc = (rc == 0 ? sb.st_size : 0ll);

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

void getPiece(int seedSocket, const string& filePath, int fileid, int pieceLocation, sockaddr_in6 seedAddress)
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

	} while (n > 0);
	cout << "Wrote " << i << " bytes\n";

	fileBitVectors[fileid] = prevBitVec;
	fp.close();

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

void startDownload(int fileid, const string& fileName, const string& filePath)
{
	vector<Peer> peerlist = currentSeederList[fileid];

	int maxConns = (MAX_CONNECTIONS < peerlist.size() ? MAX_CONNECTIONS : peerlist.size());

	ifstream fp(fileName, ios::in | ios::binary);
	struct stat sb;
	unsigned long long rc = stat(fileName.c_str(), &sb);
	rc = (rc == 0 ? sb.st_size : 0ll);
	int chunks = (int)::ceil(rc / (PIECE_SIZE)) + 1;
	fp.close();
	cout << "CHUNKS: " << chunks << endl;

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

	while (getFrequency(fileBitVectors[fileid]));

	cout << "Download of file " + fileName + " complete\n";

	// mark as seeder
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
	string totalHash;
	char *piece = new char[PIECE_SIZE], hash[20]; //change array to unsigned for SHA1

	while (ifs.read((char *)piece, PIECE_SIZE) || ifs.gcount())
	{
		totalHash += string((char *)hash);
		totPiece++;
		memset(piece, 0, PIECE_SIZE);
	}

	FileProperties f(fileid, fileName, fileName, fileUploadPathGroup, totPiece, totalHash, set<Peer>());
	downloadedFiles[fileid] = f;

}

void receiveData()
{
	struct sockaddr_in6 recAddress;
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

        }
		else if (string(buffer).substr(0, 3) == "30 ")
		{
			string fileDetails = string(buffer);
			fileDetails.erase(0, 3);
			fileDetails = fileDetails.substr(0, fileDetails.find(' '));

			int totPiece = 0;
			ifstream ifs(fileUploadPath, ios::binary);
			string totalHash;
			char *piece = new char[PIECE_SIZE], hash[20]; //change array to unsigned for SHA1

			while (ifs.read((char *)piece, PIECE_SIZE) || ifs.gcount())
			{
				totalHash += string((char *)hash);
				totPiece++;
				memset(piece, 0, PIECE_SIZE);
			}

			FileProperties f(stoi(fileDetails), fileUploadPath, fileUploadPath, fileUploadPathGroup, totPiece, totalHash, set<Peer>());
			downloadedFiles[stoi(fileDetails)] = f;
			if (!IS_PEER_OR_SEEDER)
			{
				IS_PEER_OR_SEEDER = true;
			}
		}
		else if (string(buffer).substr(0, 3)=="32 ")
		{
			char ip[INET_ADDRSTRLEN];
			memset(ip, 0, INET_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(recAddress.sin6_addr), ip, INET_ADDRSTRLEN);
			int port = ntohs(recAddress.sin6_port);
			string fullAddress = string(ip) + ":" + to_string(port);
			cout << "connection established with Peer IP : " << ip << " and PORT : " << port << endl;
			stringstream x(buffer);
			string t;
			vector<string> argsFromPeer;
			while (getline(x, t, ' '))
				argsFromPeer.push_back(t);

			cout <<"Peer " <<ip << ":" << port << " requested for " << downloadedFiles[stoi(argsFromPeer[1])].path <<" with piece range from " << argsFromPeer[2] << "-" << argsFromPeer[3] << endl;
			thread sendDataToPeer(sendPiece, string(ip), port, downloadedFiles[stoi(argsFromPeer[1])].path, stoi(argsFromPeer[2]), stoi(argsFromPeer[3]), recAddress, trackerSocket);
			sendDataToPeer.detach();
		}
		else if (string(buffer).substr(0, 3)=="31 ")
		{
			string b = string(buffer);
			b.erase(0, 3);
			stringstream x(b);
			string t;
			cout<<"buffer: "<<b<<endl;
			vector<Peer> peerList;
			int i = 0, fileid = 0;
			while (getline(x, t, ' ') )
			{
				if (i == 0)
				{
					fileid = stoi(t);
				}
				else
				{
					Peer ppp(t.substr(0, t.find_last_of(':')), stoi(t.substr(t.find_last_of(':') + 1)), "");
					peerList.push_back(Peer(ppp));
					cout << "Added seed " << ppp.ip << ":" << ppp.port << endl;
				}
				i++;
			}
			currentSeederList[fileid] = peerList;
			//START DOWNLOAD
			thread startdl(startDownload, fileid, fileDownloadName, fileDownloadPath);
			startdl.detach();
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
		cout << "Parameters not provided. Exiting...\n";
		return -1;
	}

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

	char buffer[4096] = {0};
	string syncActual = "sync " + string(argv[1]) + " ";
	sendto(trackerSocket, syncActual.c_str(), syncActual.length(), 0, (struct sockaddr *)&trackerAddress, addr_size);

	buffer[4096] = {0};
	m.lock();
	recvfrom(trackerSocket,buffer, sizeof(buffer), 0, (struct sockaddr *)&trackerAddress, &addr_size);
	m.unlock();
	string s_temp(buffer);
	int temp = stoi(s_temp.substr(s_temp.find_last_of(':') + 1));

	peerAddress.sin6_family = AF_INET6;
	inet_pton(AF_INET6, argv[1], &(peerAddress.sin6_addr));
	peerAddress.sin6_port = htons(temp);

	thread read(receiveData);
	read.detach();

	memset(buffer, 0, 4096);	
    	runGUIApplication(argc, argv, trackerSocket, LOGIN_ID, addr_size);

	return 0;
}
