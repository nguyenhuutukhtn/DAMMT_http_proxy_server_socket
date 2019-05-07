#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <fstream>
#include<string>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;

string baseCacheDirectory = "./cache/";
string hostIpCacheInfoDirectory = "./hostIpMapping.txt";
string blockedHostInfoDirectory = "./blockedHostInfo.txt";
string fileNotFoundResponse =
"HTTP/1.0 404 Not Found\r\n"
"<html><head><title>404 Not Found</head></title>\r\n"
"<body><p>404 Not Found: The requested resource could not be found!</p></body></html>\r\n";

string internalServerErrorResponse =
"HTTP/1.0 500 Internal Server Error\r\n"
"<html><head><title>500 Internal Server Error</head></title><body><p>\r\n"
"These messages in the exact format as shown above should be sent back to the client if any of the above error occurs. "
"</p></body></html>\r\n";

string badErrorResponse =
"HTTP/1.0 400 Bad Request\r\n"
"<html><head><title>400 Bad Request</head></title><body><p>\r\n"
"These messages in the exact format as shown above should be sent back to the client if any of the above error occurs. "
"</p></body></html>\r\n";

string forbiddenResponse =
"HTTP/1.1 403 Forbidden\r\n\r\n"
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>403 Forbidden</title>\r\n"
"</head><body>\r\n"
"<h1>403 HAHAHA</h1>\r\n"
"<p>WEB 18+ CAM TRUY CAP</p>\r\n"
"</body></html>\r\n";

string getHost(string clientRequest) {
	string host;
	int startPos, endPos;
	startPos = clientRequest.find("Host:") + 6;
	endPos = startPos;
	while (clientRequest[endPos]!='\n'&&clientRequest[endPos]!='\r'&&clientRequest[endPos] != '\0'&&clientRequest[endPos] != '\t')
	{
		endPos++;
	}
	host = clientRequest.substr(startPos, endPos-startPos);
	return host;
}
bool isInBlackList(string host) {
	ifstream blacklistFile;
	string line;

	size_t pos;

	blacklistFile.open("blacklist.conf");
	//char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below    
	//printf("Current dir: %s", dir);

	//blackListFile.open("blacklist.conf", ios_base::in);
	if (blacklistFile.fail()) {
		cout << "fail to open blacklist.conf" << endl;
		return false;
	}
	else
	{
		cout << "\nopen success" << endl;
		while (getline(blacklistFile,line))
		{
			/*cout << line << endl;
			cout << line.length() << endl;*/
		pos = line.find(host);
		if (pos != string::npos) {
		return true;
		}
		}
	}



	return false;
}
//function for handle client request (not in thread)
DWORD WINAPI handleClientRequest(LPVOID param) {

	SOCKET fd_client = (SOCKET)param;

	SOCKET fd_web_server;
	SOCKADDR_IN web_server_addr;
	//buffer for read client request
	char* clientRequest;
	string headerRequest;
	clientRequest = new char[65535];
	memset(clientRequest, 0, 65535);
	string host;
	hostent* hostIP;
	//read client request


	int result = recv(fd_client, clientRequest, 65535, 0);
	string clientRequestString = clientRequest;
	if (clientRequestString.substr(0, 3).compare("GET") != 0)
	{
		closesocket(fd_client);
		//cout << "close thread" << endl;
		ExitThread(0);
	}
	else
	{
		cout << clientRequestString;
		host = getHost(clientRequestString);
		cout << "domain is:" + host << endl;
		cout << host.length() << endl;
		if (isInBlackList(host)) {
			cout << "This is black website" << endl;
			send(fd_client, forbiddenResponse.c_str(), forbiddenResponse.length(), 0);
			closesocket(fd_client);
			ExitThread(0);
		}
		else
		{
			hostIP = gethostbyname(host.c_str());
			if (!hostIP) {
				cout << "fail to get IP" << endl;
				closesocket(fd_client);
				ExitThread(0);
			}
			else
			{
				web_server_addr.sin_family = AF_INET;
				web_server_addr.sin_addr.s_addr = *hostIP->h_addr_list[0];
				web_server_addr.sin_port = htons(80);

				fd_web_server= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (connect(fd_web_server, (SOCKADDR*)&hostIP, sizeof(hostIP))==0) 
				{
					cout << "connection error";
					closesocket(fd_client);
					ExitThread(0);
				}
				else
				{
					/*send(fd_web_server, clientRequest, sizeof(clientRequest), 0);
					recv(fd_web_server, clientRequest, sizeof(clientRequest), 0);
					cout << fd_web_server << endl;*/

				}
			}

		}
	}
	ExitThread(0);
}
int main() {
	//bool a = isInBlackList("kenh14.vn");
	SOCKADDR_IN proxy_addr, client_addr;
	//socklen_t client_sock_len = sizeof(client_addr);
	SOCKET fd_proxy, fd_client;
	int timeOut;
	char buf[2048];
	string ip = "127.0.0.1";
	int opt = 1, portNo, pid;
	/*if (argc != 3) {
	fprintf(stderr, "usage: %s <port> <timeout>\n", argv[0]);
	exit(1);
	}*/
	portNo = 8888;
	timeOut = 5000;

	cout << "Http proxy socket project" << endl;
	cout << "Author: Nguyen Huu Tu" << endl;

	//printHeart();

	cout << "Handle GET request from client" << endl;
	//Initialising Winsock
	WORD wVersion = MAKEWORD(2, 2);
	WSADATA wsa;

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("\nFailed. Error Code : %d", WSAGetLastError());
		return 0;
	}

	printf("\nInitialised.");

	//Creating a socket
	cout << "\n\nCreating socket" << endl;
	if ((fd_proxy = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 3000;

	if (setsockopt(fd_proxy, SOL_SOCKET, SO_RCVTIMEO,
		reinterpret_cast<char*>(&tv), sizeof(timeval)))
		return 1;

	//
	//setsockopt(fd_proxy, SOL_SOCKET, SO_REUSEADDR, (char*)opt, sizeof(int));

	proxy_addr.sin_family = AF_INET;
	proxy_addr.sin_addr.s_addr = INADDR_ANY;
	proxy_addr.sin_port = htons(portNo);

	// connect
	/*if (connect(proxy_addr, (SOCKADDR*)&addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
	cout << "Can't connect to server!" << endl;
	WSACleanup();
	closesocket(clientFTP);
	_getch();
	return 0;
	}*/
	cout << "\n Binding to server.....\n";

	if (bind(fd_proxy, (SOCKADDR*)&proxy_addr, sizeof(proxy_addr)) == SOCKET_ERROR)
	{
		printf("\ncould not bind server, error : %d\n", GetLastError());
		return false;
	}
	else
	{
		cout << "bind server successful" << endl;
	}
	cout << "\n Listening new connection\n";
	if (listen(fd_proxy, SOMAXCONN) <0)
	{
		cout << ("Could not listen to new connections");
		return false;
	}

	while (true)
	{
		INT clientAddrLen = sizeof(client_addr);
		/* Block till we have an open connection in the queue */
		fd_client = accept(fd_proxy, (SOCKADDR*)&client_addr, &clientAddrLen);
		//printf("client no. %d connected\n", fd_client);
		if (fd_client == -1) {
			perror("Error accepting connection with client\n");
			exit(1);
		}
		else {
			//cout << "accept success" << endl;
		}
		if (fd_client > 0)
		{
			//Handle request in new thread
			CloseHandle(CreateThread(NULL, 0, handleClientRequest, (LPVOID)fd_client, 0, NULL));
		}
		else {

		}
	}
	return 0;
}