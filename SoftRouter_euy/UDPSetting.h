#pragma once
//#include "common.h"
#include <vector>
#include <string>
#include <WinSock2.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

class UDPSetting {
public:
	SOCKET RecvSocket;			//接收socket
	sockaddr_in RecvAddr;		//接收地址
	sockaddr_in SenderAddr;		//发送地址
	int SenderAddrSize;			//地址长度
	SOCKET SendSocket;			//发送socket
public:

	int Server(int p)
	{
		WSADATA wsaData;			//初始化
		int Port = p;				//服务器监听地址
		char RecvBuf[1024] = { 0 };	//发送数据的缓冲区
		int BufLen = 1024;			//缓冲区大小
		SenderAddrSize = sizeof(SenderAddr);
		//初始化Socket
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		//获取主机地址
		char lv_name[50];
		gethostname(lv_name, 50);
		hostent * lv_pHostent;
		lv_pHostent = (hostent *)malloc(sizeof(hostent));
		if (NULL == (lv_pHostent = gethostbyname(lv_name)))
		{
			printf("get Hosrname Fail \n");
			exit(-1);
		}
		SOCKADDR_IN lv_sa;
		memcpy(&lv_sa.sin_addr.S_un.S_addr, lv_pHostent->h_addr_list[0], lv_pHostent->h_length);

		//创建接收数据报的socket
		RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		//将socket与制定端口和0.0.0.0绑定
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(Port);
		RecvAddr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(lv_sa.sin_addr));
		bind(RecvSocket, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
	}

	int Client(int p)
	{
		WSADATA wsaData;			//初始化
		int Port = p;				//服务器监听地址

		//初始化Socket
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		
		//获取主机地址
		char lv_name[50];
		gethostname(lv_name, 50);
		hostent * lv_pHostent;
		lv_pHostent = (hostent *)malloc(sizeof(hostent));
		if (NULL == (lv_pHostent = gethostbyname(lv_name)))
		{
			printf("get Hosrname Fail \n");
			exit(-1);
		}
		SOCKADDR_IN lv_sa;
		memcpy(&lv_sa.sin_addr.S_un.S_addr, lv_pHostent->h_addr_list[0], lv_pHostent->h_length);

		//创建Socket对象
		SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		//设置服务器地址
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(Port);
		RecvAddr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(lv_sa.sin_addr));

	}
};