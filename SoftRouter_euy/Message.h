#pragma once
#include "common.h"
#include "RouterList.h"
#include <sstream>
#define SendRouterList		0
#define ChangeDistance		1
#define RouterListMessage	0
#define ForwardingMessage	1
#define CommandMessage		2

//分组转发的内容
struct ForwardingMsg {
	int sourceAddr;
	int destinationAddr;
	int TTL;
	string data;
};
//命令行内容结构体
struct Command {
	int type;		//0:show router list				1:change the distance between router1 and router2
					//2:show forwarding information		3:send forwarding packet
	int router1;
	int router2;
	int distance;
	ForwardingMsg packet;
};
//根据delimiter将一个s分割成多个字符串
vector<string> split(const string& s, char delimiter)
{
	vector<string> tokens;
	string token;
	istringstream tokenStream(s);
	while (getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}
//根据delimiter截取s，返回s中遇到第一个delimiter前的内容，同时改变s的内容
string splitFirst(string& s, char delimiter)
{
	string token;
	string str;
	istringstream tokenStream(s);
	getline(tokenStream, token, delimiter);
	getline(tokenStream, s);
	return token;
}

class Message {
public:
	int type;					//0:RouterListMessage		1:ForwardingMessage		2:Command
	int routerNo;
	RouterList	routerlist;
	ForwardingMsg	packet;
	Command			cmd;
	string			msg;
public:
	Message() {

	}
	Message(RouterList routerlist,int no) {
		type = 0;
		routerNo = no;
		this->routerlist.distance = routerlist.distance;
		this->routerlist.nextHop = routerlist.nextHop;
	}
	Message(ForwardingMsg packet,int no) {
		type = 1;
		routerNo = no;
		this->packet = packet;
	}
	Message(Command cmd) {
		type = 2;
		this->cmd = cmd;
	}
	Message(char buf[]) {
		string msg(buf);
		string str = splitFirst(msg, ' ');
		type = atoi(str.c_str());
		if (type == 0) {
			str = splitFirst(msg, ' ');
			routerNo = atoi(str.c_str());
			vector<string> set = split(msg, ' ');

			int cnt = set.size();
			for (int i = 0; i < cnt;) {
				int num = atoi(set[i++].c_str());
				routerlist.distance.push_back(num);
				num = atoi(set[i++].c_str());
				routerlist.nextHop.push_back(num);
			}
		}
		else if (type == 1) {
			string num_str = splitFirst(msg, ' ');
			packet.sourceAddr = atoi(num_str.c_str());
			num_str = splitFirst(msg, ' ');
			routerNo = atoi(num_str.c_str());
			num_str = splitFirst(msg, ' ');
			packet.destinationAddr = atoi(num_str.c_str());
			num_str = splitFirst(msg, ' ');
			packet.TTL = atoi(num_str.c_str());
			packet.data = msg;
		}
		else if (type == 2) {
			string str = splitFirst(msg, ' ');
			cmd.type = atoi(str.c_str());
			if (cmd.type == 0 || cmd.type == 2) {

			}
			else if (cmd.type == 1) {
				str = splitFirst(msg, ' ');
				cmd.router1 = atoi(str.c_str());
				str = splitFirst(msg, ' ');
				cmd.router2 = atoi(str.c_str());
				str = splitFirst(msg, ' ');
				cmd.distance = atoi(str.c_str());				
			}
			else if (cmd.type == 3) {
				str = splitFirst(msg, ' ');
				cmd.packet.sourceAddr = atoi(str.c_str());
				str = splitFirst(msg, ' ');
				cmd.packet.destinationAddr = atoi(str.c_str());
				str = splitFirst(msg, ' ');
				cmd.packet.TTL = atoi(str.c_str());
				cmd.packet.data = msg;
			}
			else {

			}
		}
		else {

		}
	}
	//将类Message打包成string
	string PacketMessage() {
		string msg;
		if (type == 0) {
			msg = "0 "+to_string(routerNo)+" ";
			int cnt = routerlist.distance.size();
			for (int i = 0; i < cnt; i++) {
				msg = msg + to_string(routerlist.distance[i]) + " " + to_string(routerlist.nextHop[i]) + " ";
			}
		}
		else if (type == 1) {
			msg = "1 " + to_string(routerNo) + " ";
			msg = msg + to_string(packet.sourceAddr) + " " + to_string(packet.destinationAddr) + " " + to_string(packet.TTL) + " " + packet.data;
			return msg;
		}
		else if (type == 2) {
			if (cmd.type == 0) {
				msg = "2 0";
			}
			else if (cmd.type == 1) {
				msg = "2 1 " + to_string(cmd.router1) + " " + to_string(cmd.router2) + " " + to_string(cmd.distance);
			}
			else if (cmd.type == 2) {
				msg = "2 2";
			}
			else if (cmd.type == 3) {
				msg = "2 3 " + to_string(cmd.packet.sourceAddr) + " " + to_string(cmd.packet.destinationAddr) + " " + to_string(cmd.packet.TTL) + " " + cmd.packet.data;
			}
		}
		return msg;
	}
};