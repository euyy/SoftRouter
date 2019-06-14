#pragma once
#include "common.h"

class RouterList {
public:
	vector<int> distance;		
	vector<int> nextHop;
public:
	RouterList() {

	}
	//路由表的distance全置为INFIDIST，nextHop全置为hop
	RouterList(int hop,int routerNum) {
		setNextHop(hop, routerNum);
		setDistance(INFIDIST, routerNum);
	}
	//nextHop全置为hop
	void setNextHop(int hop,int routerNum) {
		vector<int> arr(routerNum, hop);
		nextHop = arr;
	}
	//distance全置为dist
	void setDistance(int dist, int routerNum) {
		vector<int> arr(routerNum, dist);
		distance = arr;
	}
	//distance所有值全加value
	void addToDist(int value) {
		int cnt = distance.size();
		for (int i = 0; i < cnt; i++) {
			distance[i] += value;
		}
	}
};