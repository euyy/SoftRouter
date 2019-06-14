#pragma once
#include "common.h"
#include "RouterList.h" 
#include "UDPSetting.h"
#include "Message.h"
#include <Windows.h>
class Router {
public:
	int no;
	int port;
	int convergence;
	int noChange;
	RouterList routerlist;
	vector<int> isReceivedRouterList;
	vector<int> neighborDist;
	vector<int> neighborPort;
	vector<RouterList> historylist;
	UDPSetting udp;
	HANDLE mutex;
	vector<ForwardingMsg> receivePacket;
	vector<ForwardingMsg> lossPacket;
	vector<ForwardingMsg> sendPacket;
public:
	Router(int no, vector<int> neighborDist, vector<int> neighborPort) {
		int routerNumber = neighborPort.size();
		this->no = no;
		this->port = neighborPort[no];
		this->convergence = 0;
		this->noChange = 0;
		this->routerlist.distance = neighborDist;
		this->routerlist.setNextHop(no, routerNumber);
		this->neighborDist = neighborDist;
		this->neighborPort = neighborPort;
		mutex = CreateMutex(NULL, 0, NULL);
		initHistoryList(routerNumber);
		initIsReceivedRouterList(routerNumber);
	}
	void initHistoryList(int routerNum) {

		RouterList router;
		router.setDistance(INFIDIST, routerNum);
		router.setNextHop(no, routerNum);
		for (int i = 0; i < routerNum; i++) {
			historylist.push_back(router);
		}
		historylist[no].distance[no] = 0;
	}

	void initIsReceivedRouterList(int routerNum) {
		int value = 10;
		vector<int> arr(routerNum, value);
		isReceivedRouterList = arr;

	}

	void NetworkSetting() {
		udp.Server(port);
	}

	void SendThread() {
		int routerNum = neighborDist.size();
		while (true) {
			WaitForSingleObject(mutex, INFINITE);

			for (int i = 0; i < routerNum; i++) {
				if (neighborDist[i] == 0) {
					continue;
				}
				else if (neighborDist[i] < INFIDIST) {
					udp.Client(neighborPort[i]);
					Message msg(routerlist, no);
					string mutexName = to_string(i);
					HANDLE mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
					WaitForSingleObject(mtx, TimeOut);
					if (sendto(udp.SendSocket, msg.PacketMessage().c_str(), msg.PacketMessage().size(), 0, (SOCKADDR *)&(udp.RecvAddr), sizeof(udp.RecvAddr)) == SOCKET_ERROR) {
						cout << "send " << WSAGetLastError() << endl;
					}
					ReleaseMutex(mtx);
				}
			}
			ReleaseMutex(mutex);
			Sleep(2000);
		}
	}

	void ReceiveThread() {
		char  buf[BUF_SIZE];
		//receive a message
		while (true) {
			ZeroMemory(buf, BUF_SIZE);
			if (recvfrom(udp.RecvSocket, buf, BUF_SIZE, 0, (SOCKADDR *)&(udp.SenderAddr), &(udp.SenderAddrSize)) > 0) {
				Process(buf);
			}
			else if (recvfrom(udp.RecvSocket, buf, BUF_SIZE, 0, (SOCKADDR *)&(udp.SenderAddr), &(udp.SenderAddrSize)) == SOCKET_ERROR) {
				cout << WSAGetLastError() << endl;
			}
			else {
				cout << "recv Return " << recvfrom(udp.RecvSocket, buf, BUF_SIZE, 0, (SOCKADDR *)&(udp.SenderAddr), &(udp.SenderAddrSize)) << endl;
			}
		}
	}
	//更新路由器的信息
	void update(Message msg) {
		updateHistoryList(msg);
		updateReceivedRouterList(msg);
		updateRouterList(msg);
	}
	//更新路由表
	void updateRouterList(Message msg) {
		//获取根据当前的邻居路由发来的路由表生成该路由的新的路由表
		int routerNum = neighborPort.size();
		RouterList routerlist(no, routerNum);
		routerlist.distance[no] = 0;
		for (int i = 0; i < routerNum; i++) {
			for (int j = 0; j < routerNum; j++) {
				if (historylist[j].distance[i] < routerlist.distance[i]) {
					routerlist.distance[i] = historylist[j].distance[i];
					routerlist.nextHop[i] = historylist[j].nextHop[i];
				}
			}
		}
		//当有三次以上路由表未改变，则认为路由表已经收敛
		if (this->routerlist.distance == routerlist.distance && this->routerlist.nextHop == routerlist.nextHop) {
			noChange++;
			if (noChange >= 3) {
				if (noChange == 3)
					cout << "Router " << no << " has convergenced after " << convergence << " Times" << endl;
			}
			else {
				convergence++;
				PrintRouterList();
			}
		}
		//若新的路由表发生改变，则更新路由表
		else {
			WaitForSingleObject(mutex, INFINITE);
			this->routerlist = routerlist;
			ReleaseMutex(mutex);
			noChange = 0;
			convergence++;
			PrintRouterList();
		}
	}
	//更新存留的邻居路由发来的路由表
	void updateHistoryList(Message msg) {
		int no = msg.routerNo;
		int dist = neighborDist[no];
		msg.routerlist.addToDist(dist);
		historylist[no] = msg.routerlist;
	}
	//若十次里，从未接收到来自某路由的路由表，则将存储邻居路由的路由表的vector中对应项置为不可达
	void updateReceivedRouterList(Message msg) {
		isReceivedRouterList[msg.routerNo] = 11;
		int routerNum = isReceivedRouterList.size();
		RouterList routerlist(no, routerNum);
		for (int i = 0; i < routerNum; i++) {
			if (i == no)
			{
				continue;
			}
			if (isReceivedRouterList[i] == 0) {
				historylist[i] = routerlist;
			}
			else {
				isReceivedRouterList[i]--;
			}
		}
	}
	//根据接收的消息，进行处理
	void Process(char buf[]) {
		Message msg(buf);
		//receive a router list
		if (msg.type == 0) {
			update(msg);
		}
		//receive a packet message
		else if (msg.type == 1) {
			receivePacket.push_back(msg.packet);
			//the packet has arrived the destination
			if (msg.packet.destinationAddr == no) {
				cout << "Router " << no << " has received a message from Router " << msg.routerNo << endl;
			}
			//the packet will be lost
			else if (msg.packet.TTL == 0) {
				lossPacket.push_back(msg.packet);
			}
			//the packet will be sent to next router
			else {
				msg.packet.TTL--;
				sendPacket.push_back(msg.packet);
				string message = msg.PacketMessage();
				int nextRouterNo = routerlist.nextHop[msg.packet.destinationAddr];
				udp.Client(neighborPort[nextRouterNo]);
				string mutexName = to_string(nextRouterNo);
				HANDLE mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
				WaitForSingleObject(mtx, TimeOut);
				if (sendto(udp.SendSocket, message.c_str(), message.size(), 0, (SOCKADDR *)&(udp.RecvAddr), sizeof(udp.RecvAddr)) == SOCKET_ERROR) {
					cout << "send " << WSAGetLastError() << endl;
				}
				ReleaseMutex(mtx);
			}
			PrintForwardingInfo();
		}
		//receive a command
		else if (msg.type == 2) {
			if (msg.cmd.type == 0) {
				PrintRouterList();
			}
			else if (msg.cmd.type == 2) {
				PrintForwardingInfo();
			}
			else if (msg.cmd.type == 1) {
				WaitForSingleObject(mutex, INFINITE);
				if (no == msg.cmd.router1) {
					neighborDist[msg.cmd.router2] = msg.cmd.distance;
				}
				else if (no == msg.cmd.router2) {
					neighborDist[msg.cmd.router1] = msg.cmd.distance;
				}
				ReleaseMutex(mutex);
			}
			else if (msg.cmd.type == 3) {
				Message message(msg.cmd.packet, no);
				sendPacket.push_back(message.packet);
				string msg_str = message.PacketMessage();
				int nextRouterNo = routerlist.nextHop[message.packet.destinationAddr];
				udp.Client(neighborPort[nextRouterNo]);
				string mutexName = to_string(nextRouterNo);
				HANDLE mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
				WaitForSingleObject(mtx, TimeOut);
				if (sendto(udp.SendSocket, msg_str.c_str(), msg_str.size(), 0, (SOCKADDR *)&(udp.RecvAddr), sizeof(udp.RecvAddr)) == SOCKET_ERROR) {
					cout << "send " << WSAGetLastError() << endl;
				}
				ReleaseMutex(mtx);
				PrintForwardingInfo();
			}
		}
		//receive other
		else {

		}
	}
	//打印路由表
	void PrintRouterList() {
		WaitForSingleObject(mutex, INFINITE);
		cout << "********************************Router " << no << " Start Print ********************************" << endl;

		cout << "\t\tDistance\t\tNext-Hop" << endl;
		int cnt = neighborPort.size();
		for (int i = 0; i < cnt; i++) {
			cout << i << "\t\t";
			if (routerlist.distance[i] == INFIDIST) {
				cout << endl;
			}
			else if (routerlist.distance[i] == 0) {
				cout << "   " << 0 << "\t\t\t  ---" << endl;
			}
			else {
				cout << "   " << routerlist.distance[i] << "\t\t\t   " << routerlist.nextHop[i] << endl;
			}
		}
		cout << "********************************Router " << no << "  End  Print ********************************" << endl << endl;
		ReleaseMutex(mutex);
	}
	//打印转发分组的统计信息
	void PrintForwardingInfo() {
		WaitForSingleObject(mutex, INFINITE);
		cout << "********************************Router " << no << " Forwarding Packet Infomation Start Print ********************************" << endl;

		int cnt = receivePacket.size();
		cout << "The number of received forwarding packets is : " << cnt << endl;
		cout << "No\t" << "\tsourceAddr\t" << "\tdestinationAddr\t" << "\tTTL\t" << "data" << endl;
		for (int i = 0; i < cnt; i++) {
			cout << (i + 1) << "\t\t    " << receivePacket[i].sourceAddr << "\t\t\t\t";
			cout << receivePacket[i].destinationAddr << "\t\t " << receivePacket[i].TTL << "\t" << receivePacket[i].data << endl;
		}

		cnt = lossPacket.size();
		cout << endl << "The number of lost forwarding packets is : " << cnt << endl;
		cout << "No\t" << "\tsourceAddr\t" << "\tdestinationAddr\t" << "\tTTL\t" << "data" << endl;
		for (int i = 0; i < cnt; i++) {
			cout << (i + 1) << "\t\t    " << lossPacket[i].sourceAddr << "\t\t\t\t";
			cout << lossPacket[i].destinationAddr << "\t\t " << lossPacket[i].TTL << "\t" << lossPacket[i].data << endl;
		}

		cnt = sendPacket.size();
		cout << endl << "The number of send forwarding packets is : " << cnt << endl;
		cout << "No\t" << "\tsourceAddr\t" << "\tdestinationAddr\t" << "\tTTL\t" << "data" << endl;
		for (int i = 0; i < cnt; i++) {
			cout << (i + 1) << "\t\t    " << sendPacket[i].sourceAddr << "\t\t\t\t";
			cout << sendPacket[i].destinationAddr << "\t\t " << sendPacket[i].TTL << "\t" << sendPacket[i].data << endl;
		}

		cout << endl << "********************************Router " << no << " Forwarding Packet Infomation End  Print ********************************" << endl << endl;
		ReleaseMutex(mutex);
	}
};