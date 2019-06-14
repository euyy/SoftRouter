#pragma once
//#include "common.h"
#include <vector>
#include <string>
#include <WinSock2.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

class UDPSetting {
public:
	SOCKET RecvSocket;			//����socket
	sockaddr_in RecvAddr;		//���յ�ַ
	sockaddr_in SenderAddr;		//���͵�ַ
	int SenderAddrSize;			//��ַ����
	SOCKET SendSocket;			//����socket
public:

	int Server(int p)
	{
		WSADATA wsaData;			//��ʼ��
		int Port = p;				//������������ַ
		char RecvBuf[1024] = { 0 };	//�������ݵĻ�����
		int BufLen = 1024;			//��������С
		SenderAddrSize = sizeof(SenderAddr);
		//��ʼ��Socket
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		//��ȡ������ַ
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

		//�����������ݱ���socket
		RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		//��socket���ƶ��˿ں�0.0.0.0��
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(Port);
		RecvAddr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(lv_sa.sin_addr));
		bind(RecvSocket, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
	}

	int Client(int p)
	{
		WSADATA wsaData;			//��ʼ��
		int Port = p;				//������������ַ

		//��ʼ��Socket
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		
		//��ȡ������ַ
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

		//����Socket����
		SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		//���÷�������ַ
		RecvAddr.sin_family = AF_INET;
		RecvAddr.sin_port = htons(Port);
		RecvAddr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(lv_sa.sin_addr));

	}
};