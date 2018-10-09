#include "stdafx.h"
#include "JobSystem.h"



ThreadPool2<int> ITaskParallel::threadPool;
std::vector<ITaskParallel*> ITaskParallel::scheduleList;
std::vector<std::thread> ITaskParallel::workers;// = std::vector<std::thread>(JobSystem::threadNums);


