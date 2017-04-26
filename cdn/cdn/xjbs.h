/*
* 定义节点的index如下：
* 超级源点 ： 0
* 网络节点 ： i + 1
* 消费节点 ： i + 1 + nsize
* 超级汇点 ： nsize + csize + 1
*/
#ifndef _XJBS_
#define _XJBS_

#include "graph.h"
#include <vector>
#include <set>

#define LINUX

#define MAXPATHLEN 4000

using namespace std;


class Xjbs
{
public:
	Graph g;

	vector<bool> best_chrom;
	vector<int> chromWithLevel;
	int bestcost;
	int deltacost;		// 每次shrink()的减少量

	bool isinit;
	set<int> resiNds;	// 剩余未删除的点集
	vector<vector<int> > levelNds;	// 把节点按层分类
	map<int, int> nodeLife;	// 节点的生命周期
	vector<pair<int, int> > nodescore;
	set<int> bestnodes, betternodes, goodnodes, badnodes;
	set<int> bestnodes_bk, betternodes_bk, goodnodes_bk, badnodes_bk;
	map<int, set<int> > adjGoodNds;	// 每个节点的goodnodes邻接点

public:
	Xjbs(char * topo[], int line_num);

	void init();
	bool shrink(vector<bool>& chrom);

	void work(int seconds);	// 总入口
	
	void print();
	void printFile(char* resbuff);

	void test(int nums);
};
#endif