#include "Router.h"
#include "common.h"
#include "UDPSetting.h"
using namespace std;
string TopologyFileName = "Topology.txt";
string PortFileName = "Port.txt";

vector<Router*> routerSet;
vector< vector<int> > Topology;
vector< vector<int> > PortSet;

vector< vector<int> > ReadConfigFile(string FileName) {
	FILE *fp;
	fp = fopen(FileName.c_str(), "r");
	if (fp == NULL) {
		cout << "Open File " << FileName << " Failed!" << endl;
	}

	vector< vector<int> > FileContent;
	vector<int> arr;
	int num;
	char ch;
	while (fscanf(fp, "%d", &num) != EOF) {
		arr.push_back(num);
		ch = fgetc(fp);
		if (ch == '\n') {
			FileContent.push_back(arr);
			arr.clear();
		}
	}
	return FileContent;
}

void InitialRouters() {

	Topology = ReadConfigFile(TopologyFileName);
	PortSet = ReadConfigFile(PortFileName);
	int routerNumber = PortSet[0].size();
	for (int i = 0; i < routerNumber; i++) {
		routerSet.push_back(new Router(i, Topology[i], PortSet[0]));
	}
}

void PrintTopology() {
	int cnt = Topology.size();
	for (int i = 0; i < cnt; i++) {
		for (int j = 0; j < cnt; j++) {
			if (Topology[i][j] == INFIDIST) {
				cout << "\t";
			}
			else {
				cout << Topology[i][j] << "\t";
			}
		}
		cout << endl;
	}
}

void runRouter(int no) {
	char szFilename[MAX_PATH];
	char szCmdLine[MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetModuleFileName(NULL, szFilename, MAX_PATH);
	sprintf(szCmdLine, "\"%s\" %d", szFilename, no);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	//�����ӽ���
	BOOL bCreateOK = CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
}

void ReceiveThread(int no) {
	routerSet[no]->ReceiveThread();
}

void SendThread(int no) {
	routerSet[no]->SendThread();
}

int main(int argc, char* argv[]) {
	InitialRouters();

	//main process
	if (argc < 2) {		
		vector<HANDLE> mutexSet;	//�˿ںŵĻ��������ϣ���ֹͬʱ������������ͬһ���˿ڷ�����Ϣ
		
		//����·����
		int cnt = routerSet.size();
		for (int i = 0; i < cnt; i++) {
			runRouter(i);
			string mutexName = to_string(i);
			mutexSet.push_back(CreateMutex(NULL, 0, mutexName.c_str()));
		}

		//�����̵�UDP����
		UDPSetting udp;
		udp.Server(AdministratorPort);
		
		//�����������д���
		string cmd;
		while (1) {
			cin >> cmd;
			//RT����
			if (cmd == "RT") {
				PrintTopology();
			}
			//SP����
			else if (cmd == "SP") {
				ForwardingMsg packet;
				cout << "Please input the Source Adderss : " << endl;
				cin >> packet.sourceAddr;
				cout << "Please input the Destination Adderss : " << endl;
				cin >> packet.destinationAddr;
				cout << "Please input the TTL : " << endl;
				cin >> packet.TTL;
				cout << "Please input the Data : " << endl;
				cin >> packet.data;
				Command cmd;
				cmd.type = 3;
				cmd.packet = packet;
				Message msg(cmd);
				int nextRouterNo = packet.sourceAddr;
				int port = PortSet[0][nextRouterNo];
				string message = msg.PacketMessage();
				udp.Client(port);
				WaitForSingleObject(mutexSet[nextRouterNo], TimeOut);
				if (sendto(udp.SendSocket, message.c_str(), message.size(), 0, (SOCKADDR *)&(udp.RecvAddr), sizeof(udp.RecvAddr)) == SOCKET_ERROR) {
					cout << "send " << WSAGetLastError() << endl;
				}
				ReleaseMutex(mutexSet[nextRouterNo]);
			}
			//ST����
			else if (cmd == "ST") {
				string message = "2 2 ";
				int cnt = PortSet[0].size();
				for (int i = 0; i < cnt; i++) {
					int port = PortSet[0][i];
					udp.Client(port);
					WaitForSingleObject(mutexSet[i], TimeOut);
					int wordNum = 0;
					if ((wordNum = sendto(udp.SendSocket, message.c_str(), message.size(), 0, (SOCKADDR *)&(udp.RecvAddr), sizeof(udp.RecvAddr))) == SOCKET_ERROR) {
						cout << "send " << WSAGetLastError() << endl;
					}
					ReleaseMutex(mutexSet[i]);
				}
			}
			//exit����˳�������
			else if (cmd == "exit") {
				return 0;
			}
		}
	}
	//�ӽ��̣���·�ɽ���
	else if (argc == 2) {
		int no = atoi(argv[1]);
		cout << "Router " << no << " has Run" << endl;
		routerSet[no]->NetworkSetting();		//·��UDP��ʼ��

		//���������̺߳ͽ����߳�
		HANDLE rThread, sThread;
		DWORD  threadId, threadId1;
		rThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveThread, (LPVOID)no, 0, &threadId);
		sThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendThread, (LPVOID)no, 0, &threadId1);
		WaitForSingleObject(rThread, INFINITE);
		WaitForSingleObject(sThread, INFINITE);
	}
}