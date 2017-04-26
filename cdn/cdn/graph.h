/*
* 定义节点的index如下：
* 超级源点 ： 0						[0]
* 网络节点 ： i + 1					[1, nsize]
* 消费节点 ： i + 1 + nsize			[nsize+1, nsize+csize]
* 超级汇点 ： nsize + csize + 1		[nsize+csize+1]
*/
#ifndef _GRAPH_
#define _GRAPH_
#include <map>
#include <vector>
#include <bitset>
#include <cstring>

#define OUTPUT false

using namespace std;

const int maxNetNds = 1500;
const int maxCumNds = 500;
const int maxTotalNds = 2100;	// 1000 + 500 + 2;
const int maxEdges = 25000;		// 边的上限为：消费节点数*2 + 网络节点数*每节点可连链路最大值20 + 服务器数(上限为消费节点数) = 500*2 + 1000*20 + 500 = 21500
const int serverranges = 10;	// 服务器档次数

const int inf = 1 << 29;		// 表示无穷大

static unsigned int g_seed;

unsigned int fastrand();
bool isBigger(const pair<int, int>& a, const pair<int, int>& b);

class Graph
{
	friend class Xjbs;
private:
	struct Edge
	{
		int u, v, next;		// 弧为(u, v), next指向当前节点连接的下一条出弧
		int cost, cap;		// 费用cost， 容量cap
	}*edge;				// 弧表指针

	int lenE;				// 弧表的长度
	int *head;				// head[i]指向edges中节点i的第一条出弧
	int *cur;
	int *dist;				// 节点到源点的距离
	int *pp;				// 当前最短路上，节点i的前继节点
	bool *vis;				// 标记节点是否已访问过，用于BFS
	int *work;				// 用于dinic的dfs

	vector<int> h;			// 势函数 for Dijkstra

	map<int, int> changecaps;	// 记录每条边的流量变化，方便重置网络
	map<int, int> n2c;			// 网络节点到消费节点
	vector<pair<int, int> > serverFlows;	// 当前解各服务器点的供应流量
	vector<pair<int, int> > serverRed;	// 当前解各服务器点的供应流量冗余
	vector<pair<int, int> > nwithCap;	// 直连消费节点with需求量
	map<int, int> n2cap;		// 网络节点对应的总带宽
	map<int, int> n2roads;		// 网络节点的出度

	vector<int> netNdCosts;	// 网络节点的部署成本
	map<int, int> netCostLevel;	// 网络节点部署成本的等级
	vector<pair<int, int> > serverInfos;	// 服务器的档次和输出能力、硬件成本
	map<int, int>	n2s;		// 网络节点部署的服务器节点档次
	int costlabel;				// ----------
	int caltimes;				// 对于shrink()，若计算caltimes次费用流还没有找到更优解，则认为暂时收敛

public:
	int nsize, csize, esize;// 网络节点数，消费节点数，网络节点边数
	int S, T;				// 超级源点及超级汇点

	int serverCost;			// 服务器固定成本
	int servernum;			// 服务器数量
	int serverMaxCap;		// 服务器最大供应量

	int requiredFlow;		// 目标流量
	int maxFlow;			// 最大流量
	int mincost;			// 最小费用
	int costflow;

	int mfnums;
	int mcmfnums;

	vector<vector<int> > paths;

public:
	Graph(char * topo[], int line_num);
	~Graph();

	void addedge(int u, int v, int cap, int cost);
	void deledge(int u, int v);		// 删除最新的边
	void buildGraph(char * topo[], int line_num);	// 重载buildGraph，对接比赛SDK

	void reset();					// 重置网络流量
									// Dinic for maxFlow
	bool dinic_bfs();
	int dinic_dfs(int src, int flow);
	int dinic_MF(const vector<bool>& chrom);
	int dinic_MF(vector<int>& chromWithLevel);

	// zkw for mincostFlow
	int aug(int u, int f);
	bool modlabel();
	int CostFlow(const vector<bool>& chrom);
	int finalFlow(const vector<bool>& chrom);
	int levelFlow(const vector<int>& chrom);

	void getPath();
};
#endif