#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <string>
#include <fstream>
#define BUFSIZE 1024

#pragma comment (lib, "ws2_32.lib")
using namespace std;

string forbiddenResponse =
"HTTP/1.1 403 Forbidden\r\n\r\n"
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>403 Forbidden</title>\r\n"
"</head><body>\r\n"
"<h1>403 FORBIDENT</h1>\r\n"
"<p>CAM TRUY CAP</p>\r\n"
"</body></html>\r\n";

/*Hàm xóa If-Modified-Since*/
void Delete_Modified(char* headerRequest)
{
	char *  Modified = strstr(headerRequest, (char*)"If-Modified-Since");
	if (Modified == NULL)
	{
		return;
	}
	int lengthModified = strlen(Modified);

	for (int i = 0; i < lengthModified; i++)
	{
		if (Modified[i] != '\n')
		{
			for (int j = i; j < lengthModified - i; j++)
			{
				Modified[j] = Modified[j + 1];
			}
		}
		else
		{
			for (int j = i; j < lengthModified - i; j++)
			{
				Modified[j] = Modified[j + 1];
			}
			break;
		}
		i--;
	}
}

//Hàm kiểm tra chuỗi sub có xuất hiện trong chuỗi str hay ko
bool FindSubStr(const char *str, char *sub)
{
	const char *p = str;
	int len = strlen(sub);

	if (str == NULL) return false;
	while (*p != NULL)
	{
		if (strlen(p) >= len)
		{
			if (strncmp(p, sub, len) == 0)
			{
				return true;
			}
		}
		else break;
		p++;
	}
	return false;
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
		while (getline(blacklistFile, line))
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
//Hàm lấy chuỗi trong Src, lấy tất cả các kí tự ngay phía sau chuỗi Begin cho đến khi gặp kí tự kết thúc EndCh
char* GetStr(char* Src, char* Begin, char EndCh)
{
	char* p = Src;
	int lenBegin = strlen(Begin);
	char* Res = NULL;

	while (*p != NULL)
	{
		if (strlen(p) >= lenBegin)
		{
			if (strncmp(p, Begin, lenBegin) == 0)
			{
				p += lenBegin;		//di chuyển con trỏ đến vị trí cần sao chép
				char* temp = p;
				while (*temp != EndCh && *temp != ':')		//tìm vị trí xuất hiện kí tự kết thúc
				{
					++temp;
				}
				Res = (char*)malloc(temp - p + 1);
				strncpy(Res, p, temp - p);
				Res[temp - p] = '\0';

				return Res;
			}
		}
		else break;

		++p;
	}
	return NULL;			//không tìm thấy Begin nên ko thể lấy chuỗi
}

//Hàm chuyển đổi chuỗi char* thành string
string ConvertStr(char* URL, int lenURL)
{
	int i = 0;
	string Res;

	while (i < lenURL)
	{
		Res.push_back(URL[i]);
		++i;
	}
	return Res;
}

//Hàm kiểm tra URL đã được caching chưa
bool CheckURLinList(char* URL, int lenURL)
{
	ifstream URLList;
	string str;
	string URL_string = ConvertStr(URL, lenURL);

	URLList.open("WebCache\\\\URLList.txt", ios::in);

	if (URLList.fail())
		cout << "Khong the mo tap tin URLList.txt" << endl;

	while (!URLList.eof())
	{
		//lấy từng chuỗi trên dòng
		getline(URLList, str);
		if (str == URL_string)			//kiểm tra có URL có trong tập tin
		{
			URLList.close();
			return true;
		}
	}

	URLList.close();
	return false;
}

//Tạo tên cho tập tin từ URL
char* CreateFileName(char* &URL, int &lenURL)
{
	if (lenURL > 181)
	{
		URL = (char*)realloc(URL, 181);
		URL[180] = '\0';
		lenURL = 180;
	}

	int len = lenURL + 10 + 4 + 1;
	char* URL_fileName = (char*)malloc(len);
	char str[] = "WebCache\\\\";
	int id = 0;
	int id_url_filename = 0;

	while (id < 10)
	{
		URL_fileName[id_url_filename] = str[id];
		++id;
		++id_url_filename;
	}

	id = 0;
	while (id < lenURL)
	{
		if (URL[id] == '/' || URL[id] == ':' || URL[id] == '*' || URL[id] == '?' || URL[id] == '"' || URL[id] == '<' || URL[id] == '>' || URL[id] == '|')	//thay thế các ký tự ko hợp lệ
			URL_fileName[id_url_filename] = '_';
		else
			URL_fileName[id_url_filename] = URL[id];
		++id;
		++id_url_filename;
	}
	URL_fileName[id_url_filename] = '.';
	++id_url_filename;
	URL_fileName[id_url_filename] = 't';
	++id_url_filename;
	URL_fileName[id_url_filename] = 'x';
	++id_url_filename;
	URL_fileName[id_url_filename] = 't';
	++id_url_filename;
	URL_fileName[id_url_filename] = '\0';

	return URL_fileName;
}

//Tạo file để lưu dữ liệu caching
void WriteCachingFile(char* URL_fileName, char* Str, int szStr)
{
	FILE *CachingFile;
	CachingFile = fopen(URL_fileName, "ab");
	if (!CachingFile) cout << "Khong the mo tap tin de caching" << endl;
	else
	{
		cout << "Da mo tap tin de luu du lieu caching" << endl;

		fwrite(&szStr, sizeof(int), 1, CachingFile);
		fwrite(Str, szStr, 1, CachingFile);

		fclose(CachingFile);
	}
}

string getHost(string clientRequest) {
	string host;
	int startPos, endPos;
	startPos = clientRequest.find("Host:") + 6;
	endPos = startPos;
	while (clientRequest[endPos] != '\n'&&clientRequest[endPos] != '\r'&&clientRequest[endPos] != '\0'&&clientRequest[endPos] != '\t')
	{
		endPos++;
	}
	host = clientRequest.substr(startPos, endPos - startPos);
	return host;
}

/*Thực thi chương trình*/
DWORD WINAPI handleClientRequest(LPVOID fd_client_socket)
{
	SOCKET fd_client = (SOCKET)fd_client_socket;

	char *headerRequest = NULL;
	char *bodyRequest = NULL;
	char *HostName = NULL;
	char *URL = NULL;
	int szHeaderRequest = 0;
	int szBodyRequest = 0;	//Size của body Request

	bool Method = 0;	//GET: 0, POST: 1
	bool flag = 0;	//Kiểm tra khoảng cách giữa headerRequest và bodyRequest
	bool checkURL = 0;	//kiểm tra URL có ở trong URLList hay ko để xử lí caching

	string clientHeader = "";
	string clientBody = "";
	string host;
	hostent* hostIP;

	bool isEndHeader = false;

	bool isGET=false;
	bool isPOST=false;

	//GET Request=Header+/r/n/r/n
	//POST Request=Request=Header+/r/n/r/n+Body 
	//Header=GET/POST+host+URL+WEB info
	//BODY=HTML

	//Get header from client request
	//Char for receive request
	char c;
	int result;
	
	while(1)
	{
		result = recv(fd_client, &c, sizeof(c), 0);
		if (result == 1)
		{
			clientHeader += c;
			if (c != '\r')
			{
				isEndHeader = false;
			}
			else
			{
				result = recv(fd_client, &c, sizeof(c), 0);
				if (c == '\n')
				{
					clientHeader += c;
					if (!isEndHeader)
					{
						isEndHeader = true;
					}
					else
					{
						break;
					}
				}
			}
		}
		else
		{
			break;
		}

	} 
	clientHeader += '\0';
	headerRequest = new char[clientHeader.length()];
	//////////////////////////////////////////
	for (int i = 0; i < clientHeader.length(); i++) {
		headerRequest[i] = clientHeader[i];
	}

	//Check request is GET or POST
	if (clientHeader[0]=='G')
		isGET = true;
	if (clientHeader[0] == 'P')
		isPOST = true;
	if (!isGET && !isPOST) {
		cout << "This proxy supports only GET and POST request" << endl;
		closesocket(fd_client);
		ExitThread(0);
	}

	//Get body request if request method is POST
	if (isPOST)
	{
		char clientBuf[BUFSIZE + 1];
		memset(clientBuf, NULL, BUFSIZE + 1);
		result= result = recv(fd_client, clientBuf, sizeof(clientBuf) - 1, 0);
		while(result>0)
		{
			result = recv(fd_client, clientBuf, sizeof(clientBuf) - 1, 0);
			//if first char in \0, replace
			if (clientBuf[0] == '\0')
			{
				clientBuf[0] = ' ';
			}

			if (result < BUFSIZE)	
			{
				clientBuf[result] = '\0';
				clientBody += clientBuf;
				break;
			}

			clientBody[result] = '\0';	
			clientBody += clientBuf;

			if (result < BUFSIZE)
				break;
		} 
		clientBody += '\0';
	}

	bodyRequest = new char[clientBody.length()];
	//////////////////////////////////////////
	for (int i = 0; i < clientBody.length(); i++) {
		bodyRequest[i] = clientBody[i];
	}


	//Get host from request
	host = getHost(clientHeader);
	//Check blacklist
	if (isInBlackList(host)) {
		cout << "This is black website" << endl;
		send(fd_client, forbiddenResponse.c_str(), forbiddenResponse.length(), 0);
		closesocket(fd_client);
		ExitThread(0);
	}

	//get IP
	hostIP = gethostbyname(host.c_str());

	//Lấy URL từ headerRequest
	URL = GetStr(headerRequest, (char*)"GET http://", ' ');
	int lenURL = strlen(URL);

	/*Kiểm tra If-modifier-Since nếu có*/
	bool checkModified = FindSubStr(headerRequest, (char*)"If-Modified-Since");

	/*Kiểm tra URL này đã được caching chưa*/
	checkURL = CheckURLinList(URL, lenURL);

	if (checkURL == false)		//Chưa được caching
	{
		if (checkURL == false)
		{
			Delete_Modified(headerRequest);
			szHeaderRequest = strlen(headerRequest);
		}

		/*Khởi tạo cấu trúc địa chỉ cho Server*/
		SOCKADDR_IN IPServer;
		INT PortServer = 80;	//Cổng

		IPServer.sin_family = AF_INET;	//Họ địa chỉ Internet
		IPServer.sin_addr.s_addr = (*(DWORD*)hostIP->h_addr_list[0]);	//IP của server
		IPServer.sin_port = htons(PortServer);

		/*Tạo Socket để kết nối tới Server qua mạng Internet tại port 80*/
		SOCKET Server;
		Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		result = connect(Server, (SOCKADDR*)&IPServer, sizeof(IPServer));

		if (result != 0)
		{
			cout << "Loi ket noi den Server tu ProxyServer\n";
			/*Xóa headerRequest, bodyRequest*/
			if (headerRequest != NULL)
			{
				free(headerRequest);
			}
			if (bodyRequest != NULL)
			{
				free(bodyRequest);
			}

			closesocket(Server);	//Đóng socket của Server
			closesocket(fd_client);	//Đóng sockett của Browser
			ExitThread(0);
		}
		else
		{
			send(Server, headerRequest, szHeaderRequest, 0);	//Gửi header Request lên Web Server
			send(Server, bodyRequest, szBodyRequest, 0);	//Gửi body Request lên Web Server

															/*Nhận header từ Server*/
			char *headerResponse = NULL;
			char *bodyResponse = NULL;
			int sizeResponse = 0;

			//Nhận Header Respone từ Web Server 
			do
			{
				result = recv(Server, &c, sizeof(c), 0);
				cout << c;

				if (result == 1)
				{
					sizeResponse++;
					headerResponse = (char*)realloc(headerResponse, sizeResponse);
					headerResponse[sizeResponse - 1] = c;

					if (c != '\r')
					{
						flag = 0;
					}
					else
					{
						result = recv(Server, &c, sizeof(c), 0);
						cout << c;

						if (c == '\n')
						{
							sizeResponse++;
							headerResponse = (char*)realloc(headerResponse, sizeResponse);
							headerResponse[sizeResponse - 1] = c;
							if (flag == 0)
							{
								flag = 1;
							}
							else
							{
								break;
							}
						}
					}
				}
				else
				{
					break;
				}
			} while (1);

			sizeResponse++;
			headerResponse = (char*)realloc(headerResponse, sizeResponse);
			headerResponse[sizeResponse - 1] = '\0';
			sizeResponse--;

			send(fd_client, headerResponse, sizeResponse, 0);

			///////////////////////////////////////////////////////////////////////////////////
			/*Lấy Cache đưa lên Browser*/
			bool NotFound = FindSubStr(headerResponse, (char*)"304 Not Modified");

			if (NotFound == true)
			{
				char* URL_fileName = CreateFileName(URL, lenURL);		//Tạo tên cho tập tin dựa vào URL
				FILE *CachingFile;
				CachingFile = fopen(URL_fileName, "rb");
				int i = 0;
				int szStr;
				char* Str;

				if (!CachingFile) cout << "Khong the mo tap tin de lay du lieu caching" << endl;
				else
				{
					cout << "Da mo tap tin de lay du lieu caching" << endl;

					while (1)
					{
						i = fread(&szStr, sizeof(int), 1, CachingFile);
						if (i == 0) break;
						Str = (char*)malloc(szStr);
						fread(Str, szStr, 1, CachingFile);
						send(fd_client, Str, szStr, 0);
						free(Str);
						Str = NULL;
					}
					fclose(CachingFile);
				}


				if (headerRequest != NULL)
				{
					free(headerRequest);
				}
				if (bodyRequest != NULL)
				{
					free(bodyRequest);
				}
				if (URL != NULL)
				{
					free(URL);
				}
				
				closesocket(fd_client);
				ExitThread(0);
			}
			//////////////////////////////////////////////////////////////////////////////////

			//Kiểm tra gói tin nếu là 200 OK thì lưu vào WebCache

			char* StatusLine = GetStr(headerResponse, (char*)"HTTP/1.", '\r');
			bool Check200OK = FindSubStr(StatusLine, (char*)"200 OK");
			char* URL_fileName = URL_fileName = CreateFileName(URL, lenURL);		//Tạo tên cho tập tin dựa vào URL

																					////////////////////////////////////////////////////////////
			if (checkURL == true)
			{
				FILE *CachingFile;
				CachingFile = fopen(URL_fileName, "wb");
			}
			////////////////////////////////////////////////////////////

			if (Check200OK == true)
			{
				WriteCachingFile(URL_fileName, headerResponse, sizeResponse);			//Mở file để caching headerResponse

				fstream URLList;			//Mở file URLList.txt để cập nhật URL đã được caching
				URLList.open("WebCache\\\\URLList.txt", ios::app);
				if (URLList.fail())
					cout << "Khong the mo tap tin URLList.txt" << endl;
				URLList << URL << endl;
				URLList.close();
			}

			char HTMLResponse[513] = { 0 };
			do
			{
				result = recv(Server, HTMLResponse, sizeof(HTMLResponse) - 1, 0);	//Đọc 512 Bytes

																					//Loại bỏ 1 ký tự bị thừa từ HEADER
				if (HTMLResponse[0] == '\0')
				{
					HTMLResponse[0] = ' ';
				}

				if (result < 512)	//Kết thúc HTML			
				{
					HTMLResponse[result] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
					cout << HTMLResponse;
					send(fd_client, HTMLResponse, result, 0);

					if (Check200OK == true && checkURL == false)
					{
						WriteCachingFile(URL_fileName, HTMLResponse, result);			//Mở file để caching headerResponse
					}
					break;
				}

				HTMLResponse[result] = '\0';	//Ngắt chuỗi nếu RESPONSE nhận < 512 BYTES hoặc cuối cùng để in chuỗi
				cout << HTMLResponse;
				send(fd_client, HTMLResponse, result, 0);
				if (Check200OK == true)
				{
					WriteCachingFile(URL_fileName, HTMLResponse, result);			//Mở file để caching headerResponse
				}
				memset(HTMLResponse, 0, sizeof(HTMLResponse));	//xóa nội dung của HTMLResponse trước khi nhận			
			} while (result > 0);

			if (headerResponse != NULL)
			{
				free(headerResponse);
			}
			if (bodyResponse != NULL)
			{
				free(bodyResponse);
			}
			if (URL_fileName != NULL)
			{
				free(URL_fileName);
			}
		}
		closesocket(Server);
	}
	else if (checkModified == false && checkURL == true)
	{
		char* URL_fileName = CreateFileName(URL, lenURL);		//Tạo tên cho tập tin dựa vào URL
		FILE *CachingFile;
		CachingFile = fopen(URL_fileName, "rb");
		int i = 0;
		int szStr;
		char* Str;

		if (!CachingFile) cout << "Khong the mo tap tin de lay du lieu caching" << endl;
		else
		{
			cout << "Da mo tap tin de lay du lieu caching" << endl;

			while (1)
			{
				i = fread(&szStr, sizeof(int), 1, CachingFile);
				if (i == 0) break;
				Str = (char*)malloc(szStr);
				fread(Str, szStr, 1, CachingFile);
				send(fd_client, Str, szStr, 0);
				free(Str);
				Str = NULL;
			}
			fclose(CachingFile);
		}
	}

	if (headerRequest != NULL)
	{
		free(headerRequest);
	}
	if (bodyRequest != NULL)
	{
		free(bodyRequest);
	}
	if (URL != NULL)
	{
		free(URL);
	}
	
	closesocket(fd_client);
	ExitThread(0);
}


int main()
{
	//Variable for winsock 
	WSADATA  wsa;
	//Socket 
	SOCKET fd_proxy;
	SOCKADDR_IN addr_proxy;
	INT port = 8888;
	cout << "HTTP proxy server Project" << endl << endl;
	cout << "Initialising winsock...." << endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Error occur" << endl;
		cout << "exiting.........." << endl;
		return 0;
	}
	cout << "Initialised" << endl;

	//Create socket for proxy
	cout << "Creating socket............" << endl;
	fd_proxy = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd_proxy == INVALID_SOCKET) {
		cout << "Could not create socket!" << endl;
		cout << "exiting......" << endl;
		return 0;
	}
	cout << "Socket created!" << endl;

	//Info
	addr_proxy.sin_family = AF_INET;	
	addr_proxy.sin_addr.s_addr = INADDR_ANY;	
	addr_proxy.sin_port = htons(port);

	cout << "Binding........" << endl;
	if (bind(fd_proxy, (SOCKADDR*)&addr_proxy, sizeof(addr_proxy)) == SOCKET_ERROR) {
		cout << "Fail to bind!" << endl;
		cout << "exiting......" << endl;
		return 0;
	}
	cout << "Bind successful!" << endl;

	if (listen(fd_proxy, SOMAXCONN) != 0) {
		cout << "Could not listen new connection!" << endl;
		cout << "exiting.........." << endl;
		return 0;
	}
	while (true)
	{
		cout << "Listening connection from client......" << endl;
		SOCKET fd_client;	//Socket của Browser
		SOCKADDR_IN addr_client;
		INT BrowserAddrLen = sizeof(addr_client);
		fd_client = accept(fd_proxy, (SOCKADDR*)&addr_client, &BrowserAddrLen);
		if (fd_client == -1)
		{
			cout << "Could not accept client request!";
		}
		else
		{
			CloseHandle(CreateThread(NULL, 0, handleClientRequest, (LPVOID)fd_client, 0, NULL));	//Thực thi xong chương trình là xóa Thread ngay
		}
	} 
		
	closesocket(fd_proxy);
	WSACleanup();
	return 0;
}