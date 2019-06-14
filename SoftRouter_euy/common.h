#pragma once
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <Windows.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#define INFIDIST	255			//距离无限远，即不可达
#define BUF_SIZE	1024		//UDP通信时最大缓冲区大小
#define AdministratorPort	52000	//主进程的端口号
#define TimeOut			5000		//互斥量的超时时间	

/*

0	7	255	255	10
7	0	1	255	8
255	1	0	2	255
255	255	2	0	2
10	8	255	2	0


52001	52002	52003	52004	52005


*/