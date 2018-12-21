#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
@brief taskNum：任务的数量
*/
const int taskNum = 1000;
/**
@brief tasks：任务数组.比如：tasks[0]=10表示第一个任务的任务长度是10.
*/
int tasks[taskNum];
/**
@brief nodeNum：处理节点的数量
*/
const int nodeNum = 20;
/**
@brief nodes：处理节点的数组。比如：nodes[0]=10表示第1个处理节点的处理速度为10.
*/
int nodes[nodeNum];
/**
@brief iteratorNum：蚁群算法一共需要迭代的次数，每次迭代都有antNum只蚂蚁进行任务分配。
*/
const int iteratorNum = 200;
/**
@brief antNum：每次迭代中蚂蚁的数量。每只蚂蚁都是一个任务调度者，
@	每次迭代中的每一只蚂蚁都需要完成所有任务的分配，这也就是一个可行解。
*/
const int antNum = 100;
/**
@brief timeMatrix[i][j]就表示第i个任务分配给第j个节点所需的处理时间。
@	所以，timeMatrix[i][j]=task[i]/nodes[j]。
*/
float timeMatrix[taskNum][nodeNum];
/**
@brief pheromoneMatrix[i][j]=0.5就表示任务i分配给节点j这条路径上的信息素浓度为0.5
*/
float pheromoneMatrix[taskNum][nodeNum];
/**
@brief maxPheromoneMatrix[0]=5表示pheromoneMatrix第0行的所有信息素中，最大信息素的下标是5
*/
int maxPheromoneMatrix[taskNum];
/**
@brief 在一次迭代中，采用随机分配策略的蚂蚁的临界编号,
@	criticalPointMatrix[0]=5，那么也就意味着，如果总共有10只蚂蚁，在分配第0个任务的时候，
@	编号是0～5的蚂蚁根据信息素浓度进行任务分配(即：将任务分配给本行中信息素浓度最高的节点处理)，
@	6～9号蚂蚁则采用随机分配的方式(即：将任务随机分配给任意一个节点处理)。
*/
int criticalPointMatrix[taskNum];
/**
@brief 每完成一次迭代后，信息素衰减的比例。
*/
const float p = 0.5;
/**
@brief 蚂蚁每次经过一条路径，信息素增加的比例。
*/
const float q = 2.0;

/**
@brief 产生down~up间的随机数，写入int数组array中，array长度为length
*/
void random_array(int _array[], int length, int up, int down)
{
	for ( int _i = 0; _i < length; ++_i)
	{
		_array[_i] = rand()%(up - down) + down;
	}
}

/**
* 初始化任务处理时间矩阵
* @param tasks 任务(长度)列表
* @param nodes 节点(处理速度)列表
*/
void initTimeMatrix(int tasks[], int tasks_length, int nodes[], int nodes_length)
{
	for ( int _i = 0; _i < tasks_length; ++_i)
	{
		for ( int _j = 0; _j < nodes_length; ++_j)
		{
			timeMatrix[_i][_j] = 1.0 * tasks[_i] / nodes[_j];
		}
	}
}

/**
* 初始化信息素矩阵(全为1)
* @param taskNum 任务数量
* @param nodeNum 节点数量
*/
void initPheromoneMatrix(int taskNum, int nodeNum)
{
	for ( int _i = 0; _i < taskNum; ++_i)
	{
		for ( int _j = 0; _j < nodeNum; ++_j)
		{
			pheromoneMatrix[_i][_j] = 1;
		}
	}
}

/**
* 将第taskCount个任务分配给某一个节点处理
* @param antCount 蚂蚁编号
* @param taskCount 任务编号
* @param nodes 节点集合
* @param pheromoneMatrix 信息素集合
*/
int assignOneTask(int antCount, int taskCount, int nodes[], float pheromoneMatrix[taskNum][nodeNum])
{
	// 若当前蚂蚁编号在临界点之前，则采用最大信息素的分配方式
	if (antCount <= criticalPointMatrix[taskCount]) 
	{
		return maxPheromoneMatrix[taskCount];
	}
	// 若当前蚂蚁编号在临界点之后，则采用随机分配方式
	return rand()%(nodeNum - 1);
}

/**
* 计算一次迭代中，所有蚂蚁的任务处理时间
* @param pathMatrix_allAnt 所有蚂蚁的路径
*/
void calTime_oneIt(int*** pathMatrix_allAnt/*[antNum][taskNum][nodeNum]*/, float timeArray_oneIt[])
{
	for (int antIndex = 0; antIndex < antNum; ++antIndex)
	{
		//获取处理时间最长的节点对应的处理时间
		float maxTime = -1;
		for (int nodeIndex = 0; nodeIndex < nodeNum; ++nodeIndex)
		{
			// 计算节点taskIndex的任务处理时间
			float time = 0;
			for (int taskIndex = 0; taskIndex < taskNum; ++taskIndex) 
			{
				if (pathMatrix_allAnt[antIndex][taskIndex][nodeIndex] == 1)
					 time += timeMatrix[taskIndex][nodeIndex];
			}
			// 更新maxTime
			if (time > maxTime) 
			{
				maxTime = time;
			}
		}
		timeArray_oneIt[antIndex] = maxTime;
	}
}

/**
* 更新信息素
* @param pathMatrix_allAnt 本次迭代中所有蚂蚁的行走路径
* @param pheromoneMatrix 信息素矩阵
* @param timeArray_oneIt 本次迭代的任务处理时间的结果集
*/
void updatePheromoneMatrix(int*** pathMatrix_allAnt/*[antNum][taskNum][nodeNum]*/, 
						   float pheromoneMatrix[taskNum][nodeNum], 
						   float timeArray_oneIt[])
{
	// 所有信息素均衰减p%
	for (int _i = 0; _i < taskNum; ++_i) 
	{
		for (int _j = 0; _j < nodeNum; ++_j) 
		{
			pheromoneMatrix[_i][_j] *= p;
		}
	}

	// 找出任务处理时间最短的蚂蚁编号
	float minTime = 9999;
	int minIndex = -1;
	for (int antIndex = 0; antIndex < antNum; ++antIndex)
	{
		if (timeArray_oneIt[antIndex] < minTime) 
		{
			minTime = timeArray_oneIt[antIndex];
			minIndex = antIndex;
		}
	}

	//将本次迭代中最优路径的信息素增加q%
	//for (int taskIndex = 0; taskIndex < taskNum; ++taskIndex)
	//	for (int nodeIndex = 0; nodeIndex < nodeNum; ++nodeIndex)
	//		if (pathMatrix_allAnt[minIndex][taskIndex][nodeIndex] == 1) 
	//			pheromoneMatrix[taskIndex][nodeIndex] *= q;

	for (int antIndex = 0; antIndex < antNum; ++antIndex)
		for (int taskIndex = 0; taskIndex < taskNum; ++taskIndex)
			for (int nodeIndex = 0; nodeIndex < nodeNum; ++nodeIndex)
				if (pathMatrix_allAnt[minIndex][taskIndex][nodeIndex] == 1)
				{
					pheromoneMatrix[taskIndex][nodeIndex] += 1.0/timeMatrix[taskIndex][nodeIndex];
				}

	memset(maxPheromoneMatrix, 0, taskNum);
	memset(criticalPointMatrix, 0, taskNum);

	for (int taskIndex = 0; taskIndex < taskNum; ++taskIndex) 
	{
		float maxPheromone = pheromoneMatrix[taskIndex][0];
		int maxIndex = 0;
		float sumPheromone = pheromoneMatrix[taskIndex][0];
		bool isAllSame = true;

		for (int nodeIndex = 1; nodeIndex < nodeNum; ++nodeIndex)
		{
			if (pheromoneMatrix[taskIndex][nodeIndex] > maxPheromone) 
			{
				maxPheromone = pheromoneMatrix[taskIndex][nodeIndex];
				maxIndex = nodeIndex;
			}

			if (pheromoneMatrix[taskIndex][nodeIndex] != pheromoneMatrix[taskIndex][nodeIndex-1])
			{
				isAllSame = false;
			}

			sumPheromone += pheromoneMatrix[taskIndex][nodeIndex];
		}

		// 若本行信息素全都相等，则随机选择一个作为最大信息素
		if (isAllSame == true) {
			maxIndex = rand()%(nodeNum - 1);
			maxPheromone = pheromoneMatrix[taskIndex][maxIndex];
		}

		// 将本行最大信息素的下标加入maxPheromoneMatrix
		maxPheromoneMatrix[taskIndex] = maxIndex;

		// 将本次迭代的蚂蚁临界编号加入criticalPointMatrix
		//(该临界点之前的蚂蚁的任务分配根据最大信息素原则，而该临界点之后的蚂蚁采用随机分配策略)
		criticalPointMatrix[taskIndex] = static_cast<int>(antNum * (maxPheromone/sumPheromone));
	}
}

/**
* 迭代搜索
* @param iteratorNum 迭代次数
* @param antNum 蚂蚁数量
*/
void acaSearch(int iteratorNum, int _antNum)
{
	int ***pathMatrix_allAnt = new int **[antNum];
	for( int _i = 0; _i < antNum; ++_i )
	{
		pathMatrix_allAnt[_i] = new int *[taskNum];
		for (int _j = 0; _j < taskNum; ++_j)
		{
			pathMatrix_allAnt[_i][_j] = new int [nodeNum];
		}
	}

	for (int itCount = 0; itCount < iteratorNum; ++itCount)
	{
		//int pathMatrix_allAnt[antNum][taskNum][nodeNum] = {0};
		//memset(pathMatrix_allAnt, 0, antNum * taskNum * nodeNum);
		for (int i = 0; i < antNum; i ++) {
			for (int j = 0; j < taskNum; j ++) {
				for (int k = 0; k < nodeNum; k ++) {
					pathMatrix_allAnt[i][j][k] = 0;
				}
			}
		}

		for ( int antCount = 0; antCount < _antNum; ++antCount)
		{
			for (int taskCount = 0; taskCount < taskNum; ++taskCount)
			{
				// 将第taskCount个任务分配给第nodeCount个节点处理
				int nodeCount = assignOneTask(antCount, taskCount, nodes, pheromoneMatrix);
				pathMatrix_allAnt[antCount][taskCount][nodeCount] = 1;
			}
		}
		// 计算本次迭代中所有蚂蚁的任务处理时间
		float timeArray_oneIt[antNum];
		calTime_oneIt(pathMatrix_allAnt, timeArray_oneIt);

		printf("第 %d 次迭代每只ant用时：\n", itCount);
		for (int _i = 0; _i < antNum; ++_i)
		{
			printf("%0.3f ", timeArray_oneIt[_i]);
		}
		printf("\n\n");
		// 更新信息素
		updatePheromoneMatrix(pathMatrix_allAnt, pheromoneMatrix, timeArray_oneIt);
	}

	for( int _i = 0; _i < antNum; ++_i )
	{
		for (int _j = 0; _j < taskNum; ++_j)
		{
			delete[] pathMatrix_allAnt[_i][_j];
		}
		delete[] pathMatrix_allAnt[_i];
	}
	delete[] pathMatrix_allAnt;
}

/**
@brief 蚁群算法
*/
void aca(void)
{
	// 初始化任务执行时间矩阵
	initTimeMatrix(tasks, taskNum, nodes, nodeNum);

	// 初始化信息素矩阵
	initPheromoneMatrix(taskNum, nodeNum);

	// 迭代搜索
	acaSearch(iteratorNum, antNum);
}


int main(int argc, char *argv[])
{
	random_array(tasks, taskNum, 100, 50);
	random_array(nodes, nodeNum, 20, 1);
	aca();

	system("pause");
}
