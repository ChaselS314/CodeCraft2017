/*
* ����ڵ��index���£�
* ����Դ�� �� 0
* ����ڵ� �� i + 1
* ���ѽڵ� �� i + 1 + nsize
* ������� �� nsize + csize + 1
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
	int deltacost;		// ÿ��shrink()�ļ�����

	bool isinit;
	set<int> resiNds;	// ʣ��δɾ���ĵ㼯
	vector<vector<int> > levelNds;	// �ѽڵ㰴�����
	map<int, int> nodeLife;	// �ڵ����������
	vector<pair<int, int> > nodescore;
	set<int> bestnodes, betternodes, goodnodes, badnodes;
	set<int> bestnodes_bk, betternodes_bk, goodnodes_bk, badnodes_bk;
	map<int, set<int> > adjGoodNds;	// ÿ���ڵ��goodnodes�ڽӵ�

public:
	Xjbs(char * topo[], int line_num);

	void init();
	bool shrink(vector<bool>& chrom);

	void work(int seconds);	// �����
	
	void print();
	void printFile(char* resbuff);

	void test(int nums);
};
#endif