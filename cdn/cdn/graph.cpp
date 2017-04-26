#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <ctime>
#include <algorithm>
#include <deque>
#include <stack>
#include <set>
#include "graph.h"

using namespace std;


unsigned int fastrand()
{
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}
bool isBigger(const pair<int, int>& a, const pair<int, int>& b)
{
	return a.second > b.second;
}

Graph::Graph(char * topo[], int line_num)
{
	edge = new Edge[maxEdges << 2];
	head = new int[maxTotalNds];
	cur = new int[maxTotalNds];
	dist = new int[maxTotalNds];
	pp = new int[maxTotalNds];
	vis = new bool[maxTotalNds];
	work = new int[maxTotalNds];
	lenE = 0;
	memset(head, -1, sizeof(int)*maxTotalNds);
	S = 0;
	T = 0;
	servernum = 0;
	serverMaxCap = 0;
	mincost = 0;
	maxFlow = 0;
	requiredFlow = 0;
	mfnums = 0;
	mcmfnums = 0;
	h.resize(maxTotalNds, 0);
	netNdCosts.resize(maxNetNds, 0);

	buildGraph(topo, line_num);
	caltimes = 0;
	costlabel = 50;
	// g_seed = 123;
	//std::srand(time(0));
}

Graph::~Graph()
{
	delete[] edge;
	delete[] head;
	delete[] cur;
	delete[] dist;
	delete[] pp;
	delete[] vis;
	delete[] work;
}

void Graph::buildGraph(char * topo[], int line_num)
{
	int lineno = 0;
	for (; -1 != sscanf(topo[lineno], "%d%d%d", &nsize, &esize, &csize); lineno++)
		;

	int level, servercap, servercost;
	for (lineno++; -1 != sscanf(topo[lineno], "%d%d%d", &level, &servercap, &servercost); lineno++)
	{
		serverInfos.push_back(make_pair(servercap, servercost));
		serverMaxCap = serverMaxCap < servercap ? servercap : serverMaxCap;		// 记录最大容量
	}

	int netnode, netcost;
	set<int> netCosts;
	for (lineno++; -1 != sscanf(topo[lineno], "%d%d", &netnode, &netcost); lineno++)
	{
		netNdCosts[netnode] = netcost;
		netCosts.insert(netcost);
	}
	vector<int> vnetCosts;
	for (auto item : netCosts)
		vnetCosts.push_back(item);
	sort(begin(vnetCosts), end(vnetCosts), less<int>());
	for (auto i = 0; i < vnetCosts.size(); i++)
		netCostLevel[vnetCosts[i]] = vnetCosts.size() - i;
	lineno++;

	S = 0;					// 超级源点
							//T = nsize + csize + 1;	// 超级汇点
	T = nsize + 1;	// 超级汇点

	int u, v, cap, cost;
	for (auto i = 0; i < esize; i++)
	{
		// 网络节点边
		sscanf(topo[i + lineno], "%d%d%d%d", &u, &v, &cap, &cost);
		addedge(u + 1, v + 1, cap, cost);
		addedge(v + 1, u + 1, cap, cost);
		n2cap[u] += cap;
		n2cap[v] += cap;
		n2roads[u]++;
		n2roads[v]++;
	}
	lineno += (esize + 1);
	for (auto i = 0; i < csize; i++)
	{
		// 消费节点边
		sscanf(topo[i + lineno], "%d%d%d", &v, &u, &cap);
		requiredFlow += cap;
		n2c[u] = v;
		nwithCap.push_back(make_pair(u, cap));
		n2cap[u] += cap;
		n2roads[u]++;
		/*addedge(u + 1, v + nsize + 1, cap, 0);
		addedge(v + nsize + 1, u + 1, cap, 0);*/
		// 消费节点到超级汇点边
		//addedge(v + nsize + 1, T, inf, 0);
		addedge(u + 1, T, cap, 0);
	}
}

void Graph::addedge(int u, int v, int cap, int cost)
{
	edge[lenE].u = u; edge[lenE].v = v; edge[lenE].cap = cap; edge[lenE].cost = cost;
	edge[lenE].next = head[u]; head[u] = lenE++;
	edge[lenE].u = v; edge[lenE].v = u; edge[lenE].cap = 0; edge[lenE].cost = -cost;
	edge[lenE].next = head[v]; head[v] = lenE++;
}

void Graph::deledge(int u, int v)
{
	--lenE;
	head[v] = edge[lenE].next;
	--lenE;
	head[u] = edge[lenE].next;
}

void Graph::reset()
{
	for (auto key : changecaps)
	{
		edge[key.first].cap += key.second;
	}
	changecaps.clear();

	mincost = 0;
	maxFlow = 0;
	servernum = 0;
	n2s.clear();
	serverFlows.clear();
	serverRed.clear();
	paths.clear();
}

int Graph::aug(int u, int f)
{
	if (u == T) return f;
	vis[u] = true;
	for (int now = cur[u]; now != -1; now = edge[now].next)
	{
		if (edge[now].cap > 0 && !vis[edge[now].v] && dist[u] == dist[edge[now].v] + edge[now].cost)
		{
			if (int tmp = aug(edge[now].v, std::min(f, edge[now].cap)))
			{
				edge[now].cap -= tmp, edge[now ^ 1].cap += tmp;
				changecaps[now] += tmp, changecaps[now ^ 1] -= tmp;
				cur[u] = now;
				return tmp;
			}
		}
	}

	return 0;

}

bool Graph::modlabel()
{
	int tmp = inf;
	for (int i = 0; i <= T; i++)
		if (vis[i])
			for (int now = head[i]; now != -1; now = edge[now].next)
				if (edge[now].cap > 0 && !vis[edge[now].v])
					tmp = std::min(tmp, dist[edge[now].v] + edge[now].cost - dist[i]);
	if (tmp == inf)
		return true;
	for (int i = 0; i <= T; i++)
		if (vis[i])
			vis[i] = false, dist[i] += tmp;
	return false;

}

int Graph::CostFlow(const vector<bool>& chrom)
{
	reset();
	caltimes++;
	for (auto i = 0; i < nsize; i++)
	{
		if (chrom[i])
			servernum++;
	}
	auto averflow = requiredFlow / servernum;
	auto averlevel = 0;
	for (; serverInfos[averlevel].first < averflow; averlevel++);

	for (auto i = 0; i < nsize; i++)
	{
		if (chrom[i])
		{
			servernum++;
			bool first = true;
			for (auto j = 0; j < serverInfos.size(); j++)
			{
				if (first)
				{
					addedge(S, i + 1, serverInfos[j].first, 0);
					first = false;
				}
				// 应该改为，在最有性价比档次及一下的cost都为0
				else if (j < averlevel)
				{
					addedge(S, i + 1, serverInfos[j].first - serverInfos[j - 1].first, 0);
				}
				else
				{
					addedge(S, i + 1, serverInfos[j].first - serverInfos[j - 1].first, j * costlabel);
				}
			}
		}
	}

	for (auto i = 0; i <= T; i++)
		dist[i] = 0;
	memset(vis, false, sizeof(bool)*maxTotalNds);

	mincost = 0;
	maxFlow = 0;

	costflow = 0;
	int tmp;
	while (true) {
		for (int i = 0; i <= T; i++)
			cur[i] = head[i];
		while (tmp = aug(S, ~0U >> 1)) {
			costflow += tmp * dist[S];
			maxFlow += tmp;
			memset(vis, false, sizeof(bool)*maxTotalNds);
		}
		if (modlabel())
		{
			break;
		}
	}
	mincost = costflow;
	map<int, int> n2f;
	for (auto i = head[S]; i != -1; i = edge[i].next)
	{

		mincost -= edge[i].cost*edge[i ^ 1].cap;
		n2f[edge[i].v - 1] += edge[i ^ 1].cap;
	}
	for (auto servernode : n2f)
	{
		for (auto j = 0; j < serverranges; j++)
		{
			// j 级别的服务器可以满足需求
			if (servernode.second <= serverInfos[j].first)
			{
				mincost += serverInfos[j].second;		// server cost
				mincost += netNdCosts[servernode.first];	// netnode cost
				n2s[servernode.first] = j;
				serverFlows.push_back(make_pair(servernode.first, servernode.second));
				serverRed.push_back(make_pair(servernode.first, serverInfos[j].first - servernode.second));
				break;
			}
		}
	}


	for (auto i = nsize - 1; i >= 0; i--)
	{
		if (chrom[i])
		{
			//deledge(S, i + 1);
			for (auto j = 0; j < serverInfos.size(); j++)
			{
				deledge(S, i + 1);
			}
		}
	}
	mcmfnums++;
	return mincost;
}


int Graph::finalFlow(const vector<bool>& chrom)
{
	// 1. 根据chrom得到部署方案
	CostFlow(chrom);
	vector<int> chromWithLevel; //  0表示没有服务器，1到10表示10档服务器，加偏移量1
	for (auto i = 0; i < chrom.size(); i++)
	{
		// 初始化为最高级
		if (chrom[i])
			chromWithLevel.push_back(n2s[i] + 1);
		else
			chromWithLevel.push_back(0);
	}
	dinic_MF(chromWithLevel);
	while (true)
	{
		vector<pair<int, int> > tmpNds;
		for (auto item : serverFlows)
		{
			// 服务器供应量的冗余量
			tmpNds.push_back(make_pair(item.first, serverInfos[chromWithLevel[item.first] - 1].first - item.second));
		}
		// 按服务器供应量的冗余量降序排序
		sort(begin(tmpNds), end(tmpNds), isBigger);

		bool finish = true;
		for (auto ibegin = begin(tmpNds); ibegin != end(tmpNds); ibegin++)
		{
			if (ibegin->second == 0)
			{
				break;
			}
			chromWithLevel[ibegin->first]--;
			if (dinic_MF(chromWithLevel) == requiredFlow)
			{
				finish = false;
				break;
			}

			chromWithLevel[ibegin->first]++;
		}
		if (finish) break;
	}
	// 2. 根据方案，计算最小费用流
	reset();
	stack<int> servernodes;
	for (auto i = 0; i < nsize; i++)
	{
		if (chromWithLevel[i])
		{
			servernodes.push(i);
			servernum++;
			addedge(S, i + 1, serverInfos[chromWithLevel[i] - 1].first, 0);
		}
	}
	for (auto i = 0; i <= T; i++)
		dist[i] = 0;
	memset(vis, false, sizeof(bool)*maxTotalNds);

	mincost = 0;
	maxFlow = 0;

	costflow = 0;
	int tmp;
	while (true) {
		for (int i = 0; i <= T; i++)
			cur[i] = head[i];
		while (tmp = aug(S, ~0U >> 1)) {
			costflow += tmp * dist[S];
			maxFlow += tmp;
			memset(vis, false, sizeof(bool)*maxTotalNds);
		}
		if (modlabel())
			break;
	}
	mincost = costflow;

	for (auto i = head[S]; i != -1; i = edge[i].next)
	{
		for (auto j = 0; j < serverranges; j++)
		{
			// j 级别的服务器可以满足需求
			if (edge[i ^ 1].cap <= serverInfos[j].first)
			{
				mincost += serverInfos[j].second;		// server cost
				mincost += netNdCosts[edge[i].v - 1];	// netnode cost
				n2s[edge[i].v - 1] = j;
				serverFlows.push_back(make_pair(edge[i].v - 1, edge[i ^ 1].cap));
				serverRed.push_back(make_pair(edge[i].v - 1, serverInfos[j].first - edge[i ^ 1].cap));
				break;
			}
		}
	}

	while (!servernodes.empty())
	{
		auto id = servernodes.top();
		servernodes.pop();
		deledge(S, id + 1);
	}
	mcmfnums++;
	return mincost;
}

int Graph::levelFlow(const vector<int>& chromWithLevel)
{
	reset();
	stack<int> servernodes;
	for (auto i = 0; i < nsize; i++)
	{
		if (chromWithLevel[i])
		{
			servernodes.push(i);
			servernum++;
			addedge(S, i + 1, serverInfos[chromWithLevel[i] - 1].first, 0);
		}
	}
	for (auto i = 0; i <= T; i++)
		dist[i] = 0;
	memset(vis, false, sizeof(bool)*maxTotalNds);

	mincost = 0;
	maxFlow = 0;

	costflow = 0;
	int tmp;
	while (true) {
		for (int i = 0; i <= T; i++)
			cur[i] = head[i];
		while (tmp = aug(S, ~0U >> 1)) {
			costflow += tmp * dist[S];
			maxFlow += tmp;
			memset(vis, false, sizeof(bool)*maxTotalNds);
		}
		if (modlabel())
			break;
	}
	mincost = costflow;

	for (auto i = head[S]; i != -1; i = edge[i].next)
	{
		for (auto j = 0; j < serverranges; j++)
		{
			// j 级别的服务器可以满足需求
			if (edge[i ^ 1].cap <= serverInfos[j].first)
			{
				mincost += serverInfos[j].second;		// server cost
				mincost += netNdCosts[edge[i].v - 1];	// netnode cost
				n2s[edge[i].v - 1] = j;
				serverFlows.push_back(make_pair(edge[i].v - 1, edge[i ^ 1].cap));
				serverRed.push_back(make_pair(edge[i].v - 1, serverInfos[j].first - edge[i ^ 1].cap));
				break;
			}
		}
	}

	while (!servernodes.empty())
	{
		auto id = servernodes.top();
		servernodes.pop();
		deledge(S, id + 1);
	}
	mcmfnums++;
	return mincost;
}

bool Graph::dinic_bfs()
{
	for (auto i = 0; i < maxTotalNds; i++)
		dist[i] = inf;
	memset(vis, false, sizeof(bool)*maxTotalNds);
	queue<int> qu;
	int i, u, v;
	dist[S] = 0;
	vis[S] = true;
	qu.push(S);
	while (!qu.empty())
	{
		u = qu.front();
		qu.pop();
		for (i = head[u]; i != -1; i = edge[i].next)
		{
			v = edge[i].v;
			if (edge[i].cap > 0 && !vis[v])
			{
				dist[v] = dist[u] + 1;
				qu.push(v);
				vis[v] = true;
			}
		}
	}

	return dist[T] != inf;
}

int Graph::dinic_dfs(int src, int flow)
{
	if (src == T)
		return flow;
	for (int& i = work[src]; i != -1; i = edge[i].next)
	{
		if (edge[i].cap <= 0) continue;
		if (dist[edge[i].v] == dist[src] + 1)
		{
			int df = dinic_dfs(edge[i].v, min(flow, edge[i].cap));
			if (df > 0)
			{
				edge[i].cap -= df;
				changecaps[i] += df;
				edge[i ^ 1].cap += df;
				changecaps[i ^ 1] -= df;
				return df;
			}
		}
	}
	return 0;
}

int Graph::dinic_MF(const vector<bool>& chrom)
{
	reset();
	for (auto i = 0; i < nsize; i++)
	{
		if (chrom[i])
		{
			addedge(S, i + 1, serverMaxCap, 0);
			servernum++;
		}
	}

	int flow = 0;
	while (dinic_bfs())
	{
		for (auto i = 0; i < maxTotalNds; i++)
			work[i] = head[i];
		while (int delta = dinic_dfs(S, inf))
		{
			flow += delta;
		}
	}
	for (auto i = nsize - 1; i >= 0; i--)
	{
		if (chrom[i])
		{
			deledge(S, i + 1);
		}
	}
	mfnums++;
	return flow;
}

int Graph::dinic_MF(vector<int>& chromWithLevel)
{
	reset();
	stack<int> servernodes;
	for (auto i = 0; i < nsize; i++)
	{
		if (chromWithLevel[i])
		{
			servernodes.push(i);
			addedge(S, i + 1, serverInfos[chromWithLevel[i] - 1].first, 0);
			servernum++;
		}
	}

	int flow = 0;
	while (dinic_bfs())
	{
		for (auto i = 0; i < maxTotalNds; i++)
			work[i] = head[i];
		while (int delta = dinic_dfs(S, inf))
		{
			flow += delta;
		}
	}

	for (auto i = head[S]; i != -1; i = edge[i].next)
	{
		// 不做多余的事
		auto curNd = edge[i].v - 1;
		n2s[curNd] = chromWithLevel[curNd] - 1;
		serverFlows.push_back(make_pair(curNd, edge[i ^ 1].cap));
		serverRed.push_back(make_pair(curNd, edge[i].cap));

	}
	while (!servernodes.empty())
	{
		auto id = servernodes.top();
		servernodes.pop();
		deledge(S, id + 1);
	}
	mfnums++;
	return flow;
}

void Graph::getPath()
{
	// 初始化邻接表
	lenE = 0;
	memset(head, -1, sizeof(int)*maxTotalNds);
	memset(vis, false, sizeof(bool)*maxTotalNds);

	bool print = true;

	// 把有效边保存在邻接表里
	for (auto ibegin = begin(changecaps); ibegin != end(changecaps); ibegin++)
	{
		if (print)
		{
			auto from = edge[ibegin->first].u;
			auto to = edge[ibegin->first].v;
			/*addedge(from, to, ibegin->second, 0);
			addedge(to, from, ibegin->second, 0);*/
			edge[lenE].u = from; edge[lenE].v = to; edge[lenE].cap = ibegin->second; edge[lenE].cost = 0;
			edge[lenE].next = head[from]; head[from] = lenE++;
			/*edge[lenE].u = to; edge[lenE].v = from; edge[lenE].cap = ibegin->second; edge[lenE].cost = 0;
			edge[lenE].next = head[to]; head[to] = lenE++;*/
		}
		print = !print;
	}

	queue<int> nodes;
	// 寻找路径，保存在paths
	while (true)
	{
		bool endflag = true;
		// 清空nodes
		while (!nodes.empty())
		{
			nodes.pop();
		}
		// 初始化vis[]
		memset(vis, false, sizeof(bool)*maxTotalNds);
		memset(pp, -1, sizeof(bool)*maxTotalNds);

		nodes.push(S);
		vis[S] = true;
		int curnode = S;
		while (!nodes.empty())
		{
			// 队列顶点出队
			curnode = nodes.front();
			nodes.pop();
			if (curnode == T)
			{
				endflag = false;
				break;
			}
			// 遍历curnode的所有出弧
			for (auto i = head[curnode]; i != -1; i = edge[i].next)
			{
				// 将有流量且未访问过的邻点入队
				if (edge[i].cap > 0 && vis[edge[i].v] == false)
				{
					pp[edge[i].v] = i;
					nodes.push(edge[i].v);
					vis[edge[i].v] = true;
				}
			}
		}
		// 没有新的路径时，结束
		if (endflag) break;

		// 将刚找到的路保存在paths里
		int mincap = inf;
		paths.push_back(vector<int>());
		// 逆序寻路
		for (auto i = pp[T]; i != -1;)
		{
			auto u = edge[i].u;
			mincap = min(mincap, edge[i].cap);
			paths.back().push_back(u - 1);
			i = pp[u];
		}
		paths.back().pop_back();
		reverse(begin(paths.back()), end(paths.back()));
		int fianlnetNd = paths.back()[0];
		paths.back().push_back(n2c[paths.back().back()]);
		paths.back().push_back(mincap);
		paths.back().push_back(n2s[fianlnetNd]);
		for (auto i = pp[T]; i != -1; i = pp[edge[i].u])
		{
			// 更新流量
			edge[i].cap -= mincap;
		}
	}
}
