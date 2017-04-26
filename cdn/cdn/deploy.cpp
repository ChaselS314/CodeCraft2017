#include "deploy.h"
#include <stdio.h>
#include "xjbs.h"

#include <ctime>
#include <iostream>

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{

	
	char* resbuff = new char[50000 * 4000];

	Xjbs xjbs(topo, line_num); 
	
	xjbs.work(89);
	xjbs.printFile(resbuff);
	// Pso pso(topo, line_num); 
	
	// pso.work(88);
	// pso.printFile(resbuff);

    // 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
    write_result(resbuff, filename);
}
