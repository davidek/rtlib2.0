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
#ifndef __ABSTASKALLOCATION_HPP__
#define __ABSTASKALLOCATION_HPP__

#include <partionedmrtkernel.hpp>
#include <vector>
#include <RTtask.hpp>
#include "server.hpp"
#include "cbserver.hpp"
#include "sporadicserver.hpp"

namespace RTSim {

class PartionedMRTKernel;

typedef std::map<CPU *, Scheduler*>::iterator CPUSCHED_ITER;

/**
    Base class for task allocation exception
    @see PartionedMRTKernel, AbsTaskAllocation

    @author Casini Daniel
*/
class TaskAllocationException : public BaseExc
{

public:
    TaskAllocationException(const string message,
                            const string cl = "TaskAllocation",
                            const string md = "PartionedMRTKernel")
                            : BaseExc(message, cl, md) {}
};

/**
    Exception raised whenever the TaskSet in not allocable in the Partioned
    MultiProcessor Kernelon the given number of processors
    @see PartionedMRTKernel, AbsTaskAllocation

    @author Casini Daniel
*/
class NotAllocableTaskSetException : public TaskAllocationException
{

public:
    NotAllocableTaskSetException(
            const string message = "Task set not allocable with the given number of CPUs",
            const string cl = "TaskAllocation",
            const string md = "PartionedMRTKernel")
            : TaskAllocationException(message, cl, md) {}
};

/**
    Exception raised whenever the task allocation strategy in not available for the
    given type of task
    @see PartionedMRTKernel, AbsTaskAllocation

    @author Casini Daniel
*/
class TaskTypeAllocationException : public TaskAllocationException
{

public:
    TaskTypeAllocationException(
            const string message = "Task allocation algorithm not defined for this type of task",
            const string cl = "TaskAllocation",
            const string md = "PartionedMRTKernel")
            :TaskAllocationException(message, cl, md) {}
};

/**
    Abstract Task Allocation class used to implement the task allocation
    (bin packing) euristich algorithms.
    @see PartionedMRTKernel

    @author Casini Daniel
*/
class AbsTaskAllocation
{

public:

    /**
       Allocates the tasks among the processors of the given kernel
    */
    virtual void    allocate(PartionedMRTKernel *kern);

    /**
      Returns the Worst Case Execution Time of the given task
    */
    static double   getWCET(AbsRTTask* task);

    /**
      Returns the period of the given task
    */
    static double   getPeriod(AbsRTTask* task);

    /**
      Returns the name of the given task
    */
    static string   getName(AbsRTTask *task);

    /**
      Returns the cpu-task allocations status
    */
    multimap<unsigned int, string> getCpuTaskAllocation() const;

    /**
      Returns the list of allocated tasks (tasks not scheduled by a VM, and VMs)
    */
    list<string> getAllocatedTasks() const;

protected:

    /// cpu index - cpu utilization table
    map<unsigned int, double>           cpuUtilization;

    ///  list of allocated tasks (tasks not scheduled by a VM, and VMs)
    list<string>                        allocatedTasks;

    /// cpu index - task name table
    multimap<unsigned int, string>      cpuTaskAllocation;

    /**
       Finds the correct CPU to allocate a task, using the specified policy
    */
    virtual CPU* findCPU(std::map<CPU *, Scheduler*> &cpuSchedulerMap,
                            double taskUtilization, unsigned int nCPU) = 0;
private:
    /**
        Allocates a task in the kernel
    */
    void allocateTask(AbsRTTask* task, PartionedMRTKernel *kern);

};


/**
    Task Allocation class used to implement the task allocation
    (bin packing) Next Fit algorithm
    @see PartionedMRTKernel

    @author Casini Daniel
*/
class NextFitTaskAllocation : public AbsTaskAllocation
{
protected:

    /// Maintains the current cpu index
    static unsigned int startCPUIndex;

    /**
         Finds the correct CPU to allocate a task, using the Next Fit policy
    */
    CPU* findCPU(map<CPU *, Scheduler*> &cpuSchedulerMap,
                double taskUtilization, unsigned int nCPU);

public:
    NextFitTaskAllocation() : AbsTaskAllocation() { startCPUIndex = 0; }

};

/**
    Task Allocation class used to implement the task allocation
    (bin packing) First Fit algorithm
    @see PartionedMRTKernel

    @author Casini Daniel
*/
class FirstFitTaskAllocation : public AbsTaskAllocation
{
    /**
         Finds the correct CPU to allocate a task, using First Fit policy
    */
    CPU* findCPU(map<CPU *, Scheduler*> &cpuSchedulerMap,
                    double taskUtilization, unsigned int nCPU);

    public:
        FirstFitTaskAllocation() : AbsTaskAllocation() {}

};

/**
    Task Allocation class used to implement the task allocation
    (bin packing) First Fit Decreasing algorithm
    @see PartionedMRTKernel

    @author Casini Daniel
*/
class FirstFitDecreasingTaskAllocation : public FirstFitTaskAllocation
{

private:

    struct UtilizationComparator{
        bool operator()(AbsRTTask *t1, AbsRTTask *t2)
        {
            double  utilization1 = AbsTaskAllocation::getWCET(t1) /
                                   AbsTaskAllocation::getPeriod(t1);
            double  utilization2 = AbsTaskAllocation::getWCET(t2) /
                                   AbsTaskAllocation::getPeriod(t2);

            return	utilization1 > utilization2;
        }

    } taskComparator;

public:

    FirstFitDecreasingTaskAllocation() : FirstFitTaskAllocation() {}

    /**
         Performs a sort based on the tasks'utilization and then uses the
         First Fit strategy
    */
    void allocate(PartionedMRTKernel *kern);

};

/**
    Task Allocation class used to implement the task allocation
    (bin packing) Best Fit Decreasing algorithm
    @see PartionedMRTKernel

    @author Casini Daniel
*/
class BestFitTaskAllocation : public AbsTaskAllocation
{
    /**
        Finds the correct CPU to allocate a task, using the Best Fit policy
    */
    CPU* findCPU(   map<CPU *, Scheduler*> &cpuSchedulerMap,
                    double taskUtilization, unsigned int nCPU);
public:
    BestFitTaskAllocation() : AbsTaskAllocation() {}

};

/**
    Task Allocation class used to implement the task allocation
    (bin packing) Worst Fit Decreasing algorithm
    @see PartionedMRTKernel

    @author Casini Daniel
*/
class WorstFitTaskAllocation : public AbsTaskAllocation
{
    /**
        Finds the correct CPU to allocate a task, using the Worst Fit policy
    */
    CPU* findCPU(   map<CPU *, Scheduler*> &cpuSchedulerMap,
                    double taskUtilization, unsigned int nCPU);
public:
    WorstFitTaskAllocation() : AbsTaskAllocation() {}

};

}
#endif
