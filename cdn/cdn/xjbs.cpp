#include "xjbs.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;

#ifdef LINUX
#include <csignal>
#include <unistd.h>
std::sig_atomic_t volatile done = 0;
void xjbs_game_over(int) { done = 1; }
#endif


int calmaxtimes = 20;

bool isSmaller(const pair<int, int>& a, const pair<int, int>& b)
{
	return a.second < b.second;
}



Xjbs::Xjbs(char * topo[], int line_num) : g(topo, line_num)
{
	best_chrom.resize(g.nsize, false);
	bestcost = inf;
	deltacost = 0;
	for (auto i = 0; i < g.nsize; i++)
		resiNds.insert(i);
	for (auto i = 0; i < g.nsize; i++)
	{
		auto flow = g.n2cap[i];
		auto roads = g.n2roads[i];
		auto costlevel = g.netCostLevel[g.netNdCosts[i]];
		nodescore.push_back(make_pair(i, flow + roads * 20 + costlevel * 0));
	}
	sort(begin(nodescore), end(nodescore), isBigger);

	for (auto item : nodescore)
	{
		if (goodnodes.size() < 0.30 * g.nsize)
			goodnodes.insert(item.first);
		else
			badnodes.insert(item.first);
	}
	/*for (auto item : nodescore)
	if (goodnodes.count(item.first) == 0)
	break;
	else
	cout << item.first << "\t" << item.second << "\t" << g.n2cap[item.first] << "\t" << g.n2roads[item.first] << "\t" << g.netCostLevel[g.netNdCosts[item.first]] << endl;*/
	goodnodes_bk = goodnodes;
	badnodes_bk = badnodes;
	/*fstream f("D:\\huawei\\第三批练习用例最优解\\2 高级\\lcase1.txt");
	int node, level, size = 0, matchsize = 0;
	while (f >> node >> level)
	if (goodnodes.find(node) != end(goodnodes))
	matchsize++, size++;
	else
	size++, cout << node << "\t" << g.n2cap[node] << "\t" << g.n2roads[node] << "\t" << g.netCostLevel[g.netNdCosts[node]] << endl;
	cout << endl;
	cout << "match : " << static_cast<double>(matchsize) / size << endl;*/
	for (auto i = 0; i < g.nsize; i++)
	{
		for (auto j = g.head[i + 1]; j != -1; j = g.edge[j].next)
		{
			auto node = g.edge[j].v - 1;
			if (node == -1 || node == g.T - 1)
				continue;
			adjGoodNds[i].insert(node);
		}
	}
}


void Xjbs::init()
{
	for (auto ibegin = begin(g.nwithCap); ibegin != end(g.nwithCap); ibegin++)
	{
		best_chrom[ibegin->first] = true;
		resiNds.erase(ibegin->first);
	}
	vector<vector<bool> > chroms(5, best_chrom);
	int nums = 0;
	sort(begin(g.nwithCap), end(g.nwithCap), isSmaller);
	for (auto ibegin = begin(g.nwithCap); ibegin != end(g.nwithCap); ibegin++)
	{
		chroms[0][ibegin->first] = false;
		if (g.dinic_MF(chroms[0]) < g.requiredFlow)
		{
			chroms[0][ibegin->first] = true;
			continue;
		}
		nums++;
		if (nums < 0.70 * g.csize)
		{
			for (auto i = 1; i < chroms.size(); i++)
				chroms[i][ibegin->first] = false;
		}
		else if (nums < 0.75 * g.csize)
		{
			for (auto i = 2; i < chroms.size(); i++)
				chroms[i][ibegin->first] = false;
		}
		else if (nums < 0.80 * g.csize)
		{
			for (auto i = 3; i < chroms.size(); i++)
				chroms[i][ibegin->first] = false;
		}
		else if (nums < 0.85 * g.csize)
		{
			for (auto i = 4; i < chroms.size(); i++)
				chroms[i][ibegin->first] = false;
		}
	}
	for (auto i = 0; i < chroms.size(); i++)
	{
		int tmpcost = g.CostFlow(chroms[i]);
		cout << "chrom " << i << " cost : " << tmpcost << " servernodes : " << g.servernum << endl;
		if (tmpcost < bestcost)
		{
			bestcost = tmpcost;
			best_chrom = chroms[i];
		}
	}
	cout << "best chrom cost : " << bestcost << endl;
}

bool Xjbs::shrink(vector<bool>& chrom)
{
	bool constriction = true;
	auto cost = g.CostFlow(chrom);
	auto cost_bk = cost;
	std::cout << cost << endl;

	auto serverFlows(g.serverFlows);
	sort(begin(serverFlows), end(serverFlows), isSmaller);

	int curNd, tmpcost;
	for (auto i = 0; i < serverFlows.size(); i++)
	{
#ifdef LINUX
		if (done) return constriction;
#endif

		// 1. 选择当前供应流量最少的服务器点
		curNd = serverFlows[i].first;
		chrom[curNd] = false;

		// 2. 判断是否能删除
		if (g.dinic_MF(chrom) == g.requiredFlow && (tmpcost = g.CostFlow(chrom)) < cost)
		{
			// 当满足最大流且费用更小时，删除该点
			std::cout << "really delete " << curNd << endl;
			cost = tmpcost;
			if (cost < bestcost)
			{
				bestcost = cost;
				best_chrom = chrom;
			}
			constriction = false;
			continue;
		}
		// 3. 若不能删除，则往深处转移
		else
		{
			chrom[curNd] = true;
			for (auto item : adjGoodNds[curNd])
			{
#ifdef LINUX
				if (done) return constriction;
#endif
				int tmpNd = item;
				if (chrom[tmpNd] == true)
					continue;

				chrom[curNd] = false;
				chrom[tmpNd] = true;

				if (g.dinic_MF(chrom) == g.requiredFlow && (tmpcost = g.CostFlow(chrom)) < cost)
				{
					// 当满足最大流且费用更小时，转移该点
					std::cout << "swap " << curNd << " with " << tmpNd << endl;
					cost = tmpcost;
					if (cost < bestcost)
					{
						bestcost = cost;
						best_chrom = chrom;
					}
					constriction = false;
					break;
				}
				else
				{
					chrom[tmpNd] = false;
					chrom[curNd] = true;
				}
			}
		}
	}
	deltacost = cost_bk - cost;
	return constriction;
}


void Xjbs::work(int seconds)
{
#ifdef LINUX
	std::signal(SIGALRM, xjbs_game_over);
	alarm(seconds);
#endif
	init();
	auto cur_chrom(best_chrom);


#ifdef LINUX
	while (true)
#else
	for (auto i = 0; i < 500; i++)
#endif
	{
#ifdef LINUX
		if (done) break;
#endif
		if (shrink(cur_chrom) || deltacost < 1000)
		{
			g.CostFlow(cur_chrom) > bestcost;
			cur_chrom = best_chrom;
			for (int j = 0; j < 5; j++)
			{
				auto it = begin(goodnodes);
				auto randnum = fastrand() % goodnodes.size();
				for (int k = 0; k < randnum; k++)
					it++;
				cur_chrom[*it] = true;
			}
		}
	}

	print();
}



void Xjbs::print()
{
	bestcost = g.CostFlow(best_chrom);
	for (auto i = 0; i < g.nsize; i++)
	{
		if (best_chrom[i])
		{
			std::cout << i << ",";
		}
	}
	std::cout << endl;
	std::cout << "costflow : " << g.costflow << endl;
	std::cout << "mincost : " << g.mincost << endl;
	std::cout << "maxFlow : " << g.maxFlow << endl;
	std::cout << "requiredFlow : " << g.requiredFlow << endl;
	std::cout << "server nodes : " << g.servernum << endl;
	std::cout << "mf nums : " << g.mfnums << endl;
	std::cout << "mcmf nums : " << g.mcmfnums << endl;
}

void Xjbs::printFile(char* resbuff)
{
	//g.CostFlow(best_chrom);
	auto cost = g.finalFlow(best_chrom);
	auto tmp(g.serverRed);
	sort(begin(tmp), end(tmp), isBigger);
	for (auto item : tmp)
		if (item.second > 0)
			cout << item.first << "\t" << item.second << endl;
	cout << "final cost is " << cost << endl;
	if (g.maxFlow < g.requiredFlow)
	{
		sprintf(resbuff, "NA");
		return;
	}
	g.getPath();
	char *strtmp = new char[MAXPATHLEN];
	sprintf(strtmp, "%d\n\n", g.paths.size());
	strcat(resbuff, strtmp);

	for (auto path : g.paths)
	{
		bool first = true;
		for (auto node : path)
		{
			if (first)
			{
				sprintf(strtmp, "%d", node);
				strcat(resbuff, strtmp);
				first = false;
			}
			else {
				sprintf(strtmp, " %d", node);
				strcat(resbuff, strtmp);
			}
		}
		strcat(resbuff, "\n");
	}
	resbuff[strlen(resbuff) - 1] = '\0';
	delete strtmp;
}

void Xjbs::test(int nums)
{
	init();
	while (nums--)
	{
		int pa = fastrand() % g.nsize;
		int pb = fastrand() % g.nsize;
		best_chrom[pa] = !best_chrom[pa];
		best_chrom[pb] = !best_chrom[pb];
		g.dinic_MF(best_chrom);
		//g.CostFlow(best_chrom);
	}

}