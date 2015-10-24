/***************************************************************************
 *   begin                : Sat Aug 15 17:28:58 CEST 2015
 *   copyright            : (C) 2015 by Daniel Casini
 *   email                : daniel.casini@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <type_traits>
#include <vector>
#include "TaskAllocation.hpp"
#include <scheduler.hpp>
#include <kernel.hpp>
#include <task.hpp>
#include <rttask.hpp>
#include <cbserver.hpp>
#include <kernevt.hpp>
#include <iostream>



namespace RTSim {

using namespace std;

unsigned int NextFitTaskAllocation::startCPUIndex = 0;

CPU* NextFitTaskAllocation::findCPU(map<CPU *, Scheduler*> &cpuSchedulerMap,
                                    double taskUtilization, unsigned int nCPU){

    CPUSCHED_ITER cpuIter;
    cpuIter = cpuSchedulerMap.begin();

    CPU *cpu;

    for(unsigned int i=0; i<startCPUIndex; i++)
        cpuIter++;

    for(unsigned int i=startCPUIndex; i<nCPU; i++){

        if((cpuUtilization[i] + taskUtilization) <= 1){
            cpuUtilization[i] += (taskUtilization);
            cpu = cpuIter->first;
            cpu->setIndex(i);
            return cpu;
        }

        cpuIter++;
        startCPUIndex++;
    }
    throw NotAllocableTaskSetException("TaskSet not allocable with the given number of CPUs");
}

CPU* FirstFitTaskAllocation::findCPU(map<CPU *, Scheduler*> &cpuSchedulerMap,
                                     double taskUtilization, unsigned int nCPU){

    CPUSCHED_ITER cpuIter;
    cpuIter = cpuSchedulerMap.begin();

    CPU *cpu;

    for(unsigned int i=0; i<nCPU; i++){
        if((cpuUtilization[i] + taskUtilization) <= 1){
            cpuUtilization[i] += (taskUtilization);
            cpu = cpuIter->first;
            cpu->setIndex(i);
            return cpu;
        }

        cpuIter++;
    }

    throw NotAllocableTaskSetException("TaskSet not allocable with the given number of CPUs");
}

CPU* BestFitTaskAllocation::findCPU(map<CPU *, Scheduler*> &cpuSchedulerMap,
                                    double taskUtilization, unsigned int nCPU){

    CPUSCHED_ITER cpuIter;
    cpuIter = cpuSchedulerMap.begin();

    CPU* smallestEmptySpaceCPU;

    double smallestEmptySpace = 1;
    int smallestEmptySpaceIndex;
    bool allocable = false;

    for(unsigned int i=0; i<nCPU; i++){
        if((!allocable && ((cpuUtilization[i] + taskUtilization) <= 1)) ||
            ((1 - cpuUtilization[i]) < smallestEmptySpace && ((cpuUtilization[i] + taskUtilization) <= 1))){
            allocable = true;
            smallestEmptySpaceCPU = cpuIter->first;
            smallestEmptySpaceIndex = i;
            smallestEmptySpace = 1 - cpuUtilization[i];
            smallestEmptySpaceCPU->setIndex(i);
        }

        cpuIter++;
    }

    if(!allocable)
        throw NotAllocableTaskSetException("TaskSet not allocable with the given number of CPUs");

    cpuUtilization[smallestEmptySpaceIndex] += (taskUtilization);
    return smallestEmptySpaceCPU;
}

CPU* WorstFitTaskAllocation::findCPU(map<CPU *, Scheduler*> &cpuSchedulerMap,
                                     double taskUtilization, unsigned int nCPU){

    CPUSCHED_ITER   cpuIter;
    cpuIter = cpuSchedulerMap.begin();

    CPU*    largestEmptySpaceCPU;
    double  largestEmptySpace = 0;
    int     largestEmptySpaceIndex;
    bool    allocable = false;

    for(unsigned int i=0; i<nCPU; i++){

        if( (!allocable && ((cpuUtilization[i] + taskUtilization) <= 1)) ||
            ((1 - cpuUtilization[i]) > largestEmptySpace && ((cpuUtilization[i] + taskUtilization) <= 1))){

            allocable = true;
            largestEmptySpaceCPU = cpuIter->first;
            largestEmptySpaceCPU->setIndex(i);
            largestEmptySpaceIndex = i;
            largestEmptySpace = 1 - cpuUtilization[i];
        }

        cpuIter++;
    }
    if(!allocable)
        throw NotAllocableTaskSetException("TaskSet not allocable with the given number of CPUs");

    cpuUtilization[largestEmptySpaceIndex] += (taskUtilization);
    return largestEmptySpaceCPU;
}

void AbsTaskAllocation::allocate(PartionedMRTKernel *kern){

    allocatedTasks.clear();
    cpuTaskAllocation.clear();

    std::deque<AbsRTTask *>::iterator taskIter = kern->_handled.begin();
    for(; taskIter != kern->_handled.end(); taskIter++)
            allocateTask(*taskIter, kern);
}

void AbsTaskAllocation::allocateTask(AbsRTTask *task, PartionedMRTKernel *kern){
    double      wcet, period;
    CPU         *selectedCPU;
    Scheduler   *sched;

    wcet = AbsTaskAllocation::getWCET(task);
    period = AbsTaskAllocation::getPeriod(task);

    try{
         selectedCPU = findCPU(kern->_cpuSchedulerMap, (wcet/period), kern->_nCPU);
    }
    catch(NotAllocableTaskSetException e){
        throw;
    }


    // Tracks the allocation status
    cpuTaskAllocation.insert ( pair<unsigned int, string>(selectedCPU->getIndex(), AbsTaskAllocation::getName(task)) );
    allocatedTasks.push_back(AbsTaskAllocation::getName(task));

    // Sets the CPU to the assigned task in the kernel, updating the related data structures
    kern->_taskCPUMap[task] = selectedCPU;
    kern->_taskSchedulerMap[task] = kern->_cpuSchedulerMap[selectedCPU];
    sched = kern->_cpuSchedulerMap[selectedCPU];

    // Adds the task to the scheduler of the selected cpu
    sched->addTask(task, kern->_taskParam[task]);

}

double AbsTaskAllocation::getWCET(AbsRTTask *task){

    if(PeriodicTask *periodicTask = dynamic_cast<PeriodicTask *> (task))
        return periodicTask->getMaxExecutionTime();
    else if(Server *server = dynamic_cast<Server *> (task))
        return server->getBudget();
    else throw TaskTypeAllocationException();
}

double AbsTaskAllocation::getPeriod(AbsRTTask *task){

    if(PeriodicTask *periodicTask = dynamic_cast<PeriodicTask *>(task))
        return periodicTask->getPeriod();
    else if(Server *server = dynamic_cast<Server *>(task))
        return server->getPeriod();
    else throw TaskTypeAllocationException();
}

string AbsTaskAllocation::getName(AbsRTTask *task){

    if(PeriodicTask *periodicTask = dynamic_cast<PeriodicTask *>(task))
        return periodicTask->getName();
    else if(Server *server = dynamic_cast<Server *>(task))
        return server->getName();
    else throw TaskTypeAllocationException();
}

multimap<unsigned int, string> AbsTaskAllocation::getCpuTaskAllocation() const
{
    return cpuTaskAllocation;
}

list<string> AbsTaskAllocation::getAllocatedTasks() const
{
    return allocatedTasks;
}

void FirstFitDecreasingTaskAllocation::allocate(PartionedMRTKernel *kern){

    //sorts the tasks in decreasing order and calls the allocate method
    std::sort(kern->_handled.begin(), kern->_handled.end(), taskComparator);
    AbsTaskAllocation::allocate(kern);
}

}//namespace


