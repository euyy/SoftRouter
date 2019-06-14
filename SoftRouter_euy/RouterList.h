#pragma once
#include "common.h"

class RouterList {
public:
	vector<int> distance;		
	vector<int> nextHop;
public:
	RouterList() {

	}
	//·�ɱ��distanceȫ��ΪINFIDIST��nextHopȫ��Ϊhop
	RouterList(int hop,int routerNum) {
		setNextHop(hop, routerNum);
		setDistance(INFIDIST, routerNum);
	}
	//nextHopȫ��Ϊhop
	void setNextHop(int hop,int routerNum) {
		vector<int> arr(routerNum, hop);
		nextHop = arr;
	}
	//distanceȫ��Ϊdist
	void setDistance(int dist, int routerNum) {
		vector<int> arr(routerNum, dist);
		distance = arr;
	}
	//distance����ֵȫ��value
	void addToDist(int value) {
		int cnt = distance.size();
		for (int i = 0; i < cnt; i++) {
			distance[i] += value;
		}
	}
};