#include "xjbs.h"
#include <ctime>
#include <Windows.h>
#include <iostream>

#define PRINT   printf
#define MAX_LINE_LEN 55000
#define MAX_EDGE_NUM    (2000 * 20)

using namespace std;

int read_file(char ** const buff, const unsigned int spec, const char * const filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		PRINT("Fail to open file %s, %s.\n", filename, strerror(errno));
		return 0;
	}
	PRINT("Open file %s OK.\n", filename);

	char line[MAX_LINE_LEN + 2];
	unsigned int cnt = 0;
	while ((cnt < spec) && !feof(fp))
	{
		line[0] = 0;
		if (fgets(line, MAX_LINE_LEN + 2, fp) == NULL)  continue;
		if (line[0] == 0)   continue;
		buff[cnt] = (char *)malloc(MAX_LINE_LEN + 2);
		strncpy(buff[cnt], line, MAX_LINE_LEN + 2 - 1);
		buff[cnt][MAX_LINE_LEN + 1] = 0;
		cnt++;
	}
	fclose(fp);
	PRINT("There are %d lines in file %s.\n", cnt, filename);

	return cnt;
}
void write_file(const bool cover, const char * const buff, const char * const filename)
{
	if (buff == NULL)
		return;

	const char *write_type = cover ? "w" : "a";//1:覆盖写文件，0:追加写文件
	FILE *fp = fopen(filename, write_type);
	if (fp == NULL)
	{
		PRINT("Fail to open file %s, %s.\n", filename, strerror(errno));
		return;
	}
	PRINT("Open file %s OK.\n", filename);
	fputs(buff, fp);
	fputs("\n", fp);
	fclose(fp);
}
int main(int argc, char* argv[])
{
	LARGE_INTEGER begain, end, frequency;
	QueryPerformanceFrequency(&frequency);

	char *topo[MAX_EDGE_NUM];
	int line_num;

	char *topo_file = argv[1];
	char *filename = argv[2];
	char *resbuff = new char[50000 * 4000];

	line_num = read_file(topo, MAX_EDGE_NUM, topo_file);
	
	
	Xjbs xjbs(topo, line_num); 
	QueryPerformanceCounter(&begain);
	//xjbs.test(100);
	xjbs.work(87);
	QueryPerformanceCounter(&end);

	xjbs.printFile(resbuff);

	//Graph g(topo, line_num);
	//int nodes[] = { 0, 3, 22 };
	//int nums = sizeof(nodes) / sizeof(int);
	//vector<bool> chrom(g.nsize, false);
	//for (auto i = 0; i < nums; i++)
	//{
	//	chrom[nodes[i]] = true;
	//}
	////auto mincost = g.MCMF(chrom);
	//auto mincost = g.CostFlow(chrom);
	//mincost += g.serverCost * g.servernum;
	//auto maxFlow = g.maxFlow;
	//cout << "mincost is " << mincost << endl;
	//cout << "flow is " << maxFlow << endl;
	//cout << "required flow is " << g.requiredFlow << endl;

	write_file(1, resbuff, filename);
	delete[] resbuff;

	cout << "Cost time : " << (double)(end.QuadPart - begain.QuadPart) * 1000 / frequency.QuadPart << "ms" << endl;
	return 0;
}