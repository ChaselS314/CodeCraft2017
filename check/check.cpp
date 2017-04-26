/* coded by karel */
// 使用说明，我们要写的cdn程序一样。输入两个文件的参数。检验产生的结果是否有效

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
//#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>


using namespace std;

const int MAX_EDGE_NUM = 40000;
#define MAX_LINE_LEN 55000
#define PRINT printf
const int INNER_MAX = 1002;
const int CONSUME_MAX = 500;
const int int_max = 0x7fffff;
const int int_min = -int_max;


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

struct ConsumeNode
{
  int consume_node;
  int inner_node;
  int requirement;
  bool seen;
  ConsumeNode(int consume = -1, int inner = -1, int req = -1, bool s = false) :consume_node(consume), inner_node(inner), requirement(req), seen(s){}
  bool operator< (const ConsumeNode& b) const {
    return requirement > b.requirement;
  }
};
ConsumeNode cnode[CONSUME_MAX];

vector<pair<int, int> > serverInfos(10, make_pair(0, 0));
vector<int> netNdCosts(INNER_MAX, 0);

int cost_table[INNER_MAX][INNER_MAX];
int capacity_table[INNER_MAX][INNER_MAX];
int inn, cnn, server_cost;

void init(char* topo[MAX_EDGE_NUM], int line_num){
  // init cost_table.
  for (int i = 0; i < INNER_MAX; ++i){
    fill(cost_table[i], cost_table[i] + INNER_MAX, int_max);
    cost_table[i][i] = 0;
    fill(capacity_table[i], capacity_table[i] + INNER_MAX, 0);
  }

  int i = 0, link = 0;
  stringstream stream;
  stream << topo[i];
  stream >> inn >> link >> cnn;
  stream.clear();

  i += 2;
  int slevel, scap, scost;
  while (true)
  {
    stream << topo[i++];
    if (stream >> slevel >> scap >> scost)
    {
      serverInfos[slevel] = make_pair(scap, scost);
    }
    else
    {
      stream.clear();
      break;
    }
  }

  int netNd, netCost;
  while (true)
  {
    stream << topo[i++];
    if (stream >> netNd >> netCost)
    {
      netNdCosts[netNd] = netCost;
    }
    else
    {
      stream.clear();
      break;
    }
  }

  
  int temp = i + link;
  int from, to, cap, cost;
  for (; i < temp; ++i){
    stream << topo[i];
    stream >> from >> to >> cap >> cost;
    stream.clear();

    cost_table[from][to] = cost;
    cost_table[to][from] = cost;

    capacity_table[from][to] = cap;
    capacity_table[to][from] = cap;

  }

  stream.str("");
  ++i;
  int consume, inner, req;
  for (int k = 0; k < cnn; ++k){
    stream << topo[i++];
    stream >> consume >> inner >> req;
    stream.clear();
    cnode[k] = ConsumeNode(consume, inner, req);
  }
  stream.str("");
}

int check(char* res[MAX_EDGE_NUM]){
  int server[INNER_MAX];
  for (auto i = 0; i < INNER_MAX; i++)
    server[i] = -1;
  stringstream stream;
  int i = 0;
  int line = 0;
  int cost = 0;
  int num;
  stream << res[i];
  stream >> line;
  stream.clear();
  i = 2;
  int totalflow = 0; ////////////////
  for (; i < 2 + line; ++i){
    vector<int> path;
    stream << res[i];
    while (stream >> num){
      path.push_back(num);
    }
    stream.clear();
    stream.str("");
    int serverlevel = path.back();
    int flow = path[path.size() - 2];
    totalflow += flow;
    for (int i = 0; i < path.size() - 4; ++i){
      capacity_table[path[i]][path[i + 1]] -= flow;
      if (capacity_table[path[i]][path[i + 1]] < 0){
        cout << "The flow error:" << endl;
        for (int i = 0; i < path.size(); ++i){
          cout << path[i] << " ";
        }
        cout << endl;
        return -1;
      }
      cost += flow * cost_table[path[i]][path[i + 1]];
    }

    if (server[path[0]] == -1){
      server[path[0]] = serverInfos[serverlevel].first;
      cost += serverInfos[serverlevel].second;
      cost += netNdCosts[path[0]];
    }
    else
    {
      server[path[0]] -= flow;
      if (server[path[0]] < 0)
      {
        cout << "Server over head on node : " << path[0] << endl;
        return -1;
      }
    }

    cnode[path[path.size() - 3]].requirement -= flow;
  }

  for (int i = 0; i < cnn; ++i){
    if (cnode[i].requirement != 0){
      cout << "consume node " << i << " error: " << cnode[i].requirement << endl;
      //return -1;
    }
  }
  cout << "flow is " << totalflow << endl;
  return cost;
}

int main(int argc, char * argv[]){

  char *topo[MAX_EDGE_NUM];

  int line_num;

  char* topo_file = argv[1];

  line_num = read_file(topo, MAX_EDGE_NUM, topo_file);

  char* result_file = argv[2];

  char* res[MAX_EDGE_NUM];
  read_file(res, MAX_EDGE_NUM, result_file);

  init(topo, line_num);

  int cost = check(res);
  if (cost >= 0){
    cout << "Result correct:\n The cost is: " << cost << endl;
  }
  else {
    cout << "Result error!" << endl;
  }
  return 0;
}
